/*
 * Copyright (c) 2017 TOYOTA MOTOR CORPORATION
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unistd.h>
#include "app.hpp"
#include "json_helper.hpp"
#include "util.hpp"
#include "wayland.hpp"

#include <algorithm>
#include <mutex>

#include <json.h>

extern "C" {
#include <afb/afb-binding.h>
#include <systemd/sd-event.h>
}

namespace {

struct afb_instance {
   std::unique_ptr<wl::display> display;
   wm::App app;

   afb_instance() : display{new wl::display}, app{this->display.get()} {}

   int init();
};

struct afb_instance *g_afb_instance;

int afb_instance::init() {
   return this->app.init();
}

int display_event_callback(sd_event_source *evs, int /*fd*/, uint32_t events,
                           void * /*data*/) {
   ST();

   if ((events & EPOLLHUP) != 0) {
      HMI_ERROR("wm", "The compositor hung up, dying now.");
      delete g_afb_instance;
      g_afb_instance = nullptr;
      goto error;
   }

   if ((events & EPOLLIN) != 0u) {
      {
         STN(display_read_events);
         g_afb_instance->app.display->read_events();
         g_afb_instance->app.set_pending_events();
      }
      {
         // We want do dispatch pending wayland events from within
         // the API context
         STN(winman_ping_api_call);
         afb_service_call("windowmanager", "ping", json_object_new_object(),
                          [](void *c, int st, json_object *j) {
                             STN(winman_ping_api_call_return);
                          },
                          nullptr);
      }
   }

   return 0;

error:
   sd_event_source_unref(evs);
   if (getenv("WINMAN_EXIT_ON_HANGUP") != nullptr) {
   exit(1);
}
   return -1;
}

/**
 * binding_init_()
 */
int binding_init_() {
   HMI_NOTICE("wm", "WinMan ver. %s", WINMAN_VERSION_STRING);

   if (g_afb_instance != nullptr) {
      HMI_ERROR("wm", "Wayland context already initialized?");
      return 0;
   }

   if (getenv("XDG_RUNTIME_DIR") == nullptr) {
      HMI_ERROR("wm", "Environment variable XDG_RUNTIME_DIR not set");
      goto error;
   }

   {
      // wait until wayland compositor starts up.
      int cnt = 0;
      g_afb_instance = new afb_instance;
      while (!g_afb_instance->display->ok()) {
         cnt++;
         if (20 <= cnt) {
            HMI_ERROR("wm", "Could not connect to compositor");
            goto error;
         }
         HMI_ERROR("wm", "Wait to start weston ...");
         sleep(1);
         delete g_afb_instance;
         g_afb_instance = new afb_instance;
      }
   }

   if (g_afb_instance->init() == -1) {
      HMI_ERROR("wm", "Could not connect to compositor");
      goto error;
   }

   {
      int ret = sd_event_add_io(afb_daemon_get_event_loop(), nullptr,
                                g_afb_instance->display->get_fd(), EPOLLIN,
                                display_event_callback, g_afb_instance);
      if (ret < 0) {
         HMI_ERROR("wm", "Could not initialize afb_instance event handler: %d", -ret);
         goto error;
      }
   }

   atexit([] { delete g_afb_instance; });

   return 0;

error:
   delete g_afb_instance;
   g_afb_instance = nullptr;
   return -1;
}

int binding_init() noexcept {
   try {
      return binding_init_();
   } catch (std::exception &e) {
      HMI_ERROR("wm", "Uncaught exception in binding_init(): %s", e.what());
   }
   return -1;
}

}  // namespace

#include "afb_binding_glue.inl"

namespace wm {
void binding_api::send_event(char const *evname, char const *label) {
   HMI_DEBUG("wm", "%s: %s(%s)", __func__, evname, label);

   json_object *j = json_object_new_object();
   json_object_object_add(j, kKeyDrawingName, json_object_new_string(label));

   int ret = afb_event_push(g_afb_instance->app.map_afb_event[evname], j);
   if (ret != 0) {
      HMI_DEBUG("wm", "afb_event_push failed: %m");
   }
}

void binding_api::send_event(char const *evname, char const *label, char const *area,
                             int x, int y, int w, int h) {
   HMI_DEBUG("wm", "%s: %s(%s, %s) x:%d y:%d w:%d h:%d",
             __func__, evname, label, area, x, y, w, h);

   json_object *j_rect = json_object_new_object();
   json_object_object_add(j_rect, kKeyX,      json_object_new_int(x));
   json_object_object_add(j_rect, kKeyY,      json_object_new_int(y));
   json_object_object_add(j_rect, kKeyWidth,  json_object_new_int(w));
   json_object_object_add(j_rect, kKeyHeight, json_object_new_int(h));

   json_object *j = json_object_new_object();
   json_object_object_add(j, kKeyDrawingName, json_object_new_string(label));
   json_object_object_add(j, kKeyDrawingArea, json_object_new_string(area));
   json_object_object_add(j, kKeyDrawingRect, j_rect);

   int ret = afb_event_push(g_afb_instance->app.map_afb_event[evname], j);
   if (ret != 0) {
      HMI_DEBUG("wm", "afb_event_push failed: %m");
   }
}
} // namespace wm

extern "C" const struct afb_binding_v2 afbBindingV2 = {
   "windowmanager", nullptr, nullptr, windowmanager_verbs, nullptr, binding_init, nullptr, 0};
