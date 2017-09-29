/*
 * Copyright (C) 2017 Mentor Graphics Development (Deutschland) GmbH
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
      logerror("The compositor hung up, dying now.");
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

//  _     _           _ _                 _       _ _    ____
// | |__ (_)_ __   __| (_)_ __   __ _    (_)_ __ (_) |_ / /\ \
// | '_ \| | '_ \ / _` | | '_ \ / _` |   | | '_ \| | __| |  | |
// | |_) | | | | | (_| | | | | | (_| |   | | | | | | |_| |  | |
// |_.__/|_|_| |_|\__,_|_|_| |_|\__, |___|_|_| |_|_|\__| |  | |
//                              |___/_____|             \_\/_/
int binding_init_() {
   lognotice("WinMan ver. %s", WINMAN_VERSION_STRING);

   if (g_afb_instance != nullptr) {
      logerror("Wayland context already initialized?");
      return 0;
   }

   if (getenv("XDG_RUNTIME_DIR") == nullptr) {
      logerror("Environment variable XDG_RUNTIME_DIR not set");
      goto error;
   }

   {
      // wait until wayland compositor starts up.
      int cnt = 0;
      g_afb_instance = new afb_instance;
      while (!g_afb_instance->display->ok()) {
         cnt++;
         if (20 <= cnt) {
            logerror("Could not connect to compositor");
            goto error;
         }
         logerror("Wait to start weston ...");
         sleep(1);
         delete g_afb_instance;
         g_afb_instance = new afb_instance;
      }
   }

   if (g_afb_instance->init() == -1) {
      logerror("Could not connect to compositor");
      goto error;
   }

   {
      int ret = sd_event_add_io(afb_daemon_get_event_loop(), nullptr,
                                g_afb_instance->display->get_fd(), EPOLLIN,
                                display_event_callback, g_afb_instance);
      if (ret < 0) {
         logerror("Could not initialize afb_instance event handler: %d", -ret);
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
      logerror("Uncaught exception in binding_init(): %s", e.what());
   }
   return -1;
}

}  // namespace

#include "afb_binding_glue.inl"

// XXX implement send_event right here...
namespace wm {
void binding_api::send_event(char const *evname, char const *label) {
   logdebug("%s: %s(%s)", __func__, evname, label);
   int ret = afb_daemon_broadcast_event(evname, json_object_new_string(label));
   if (ret != 0) {
      logdebug("afb_event_broadcast failed: %m");
   }
}
} // namespace wm

extern "C" const struct afb_binding_v2 afbBindingV2 = {
   "windowmanager", nullptr, nullptr, windowmanager_verbs, nullptr, binding_init, nullptr, 0};
