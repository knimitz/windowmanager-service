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

struct afb_instance {
   std::unique_ptr<wl::display> display;
   wm::App app;

   afb_instance() : display{new wl::display}, app{this->display.get()} {}

   int init();
};

struct afb_instance *g_afb_instance;
std::mutex binding_m;

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

void windowmanager_requestsurface(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   const char* a_drawing_name = afb_req_value(req, "drawing_name");
   if(!a_drawing_name){
       afb_req_fail(req, "failed", "Need char const* argument drawing_name");
       return;
   }

   auto ret = g_afb_instance->app.api.requestsurface(a_drawing_name);
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling requestsurface: %s", e.what());
      return;
   }

}

void windowmanager_requestsurfacexdg(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   json_object *jreq = afb_req_json(req);

   json_object *j_drawing_name = nullptr;
   if (! json_object_object_get_ex(jreq, "drawing_name", &j_drawing_name)) {
      afb_req_fail(req, "failed", "Need char const* argument drawing_name");
      return;
   }
   char const* a_drawing_name = json_object_get_string(j_drawing_name);

   json_object *j_ivi_id = nullptr;
   if (! json_object_object_get_ex(jreq, "ivi_id", &j_ivi_id)) {
      afb_req_fail(req, "failed", "Need char const* argument ivi_id");
      return;
   }
   char const* a_ivi_id = json_object_get_string(j_ivi_id);

   auto ret = g_afb_instance->app.api.requestsurfacexdg(a_drawing_name, a_ivi_id);
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling requestsurfacexdg: %s", e.what());
      return;
   }

}

void windowmanager_activatesurface(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   const char* a_drawing_name = afb_req_value(req, "drawing_name");
   if(!a_drawing_name){
       afb_req_fail(req, "failed", "Need char const* argument drawing_name");
       return;
   }

   const char* a_drawing_area = afb_req_value(req, "drawing_area");
   if(!a_drawing_area){
       afb_req_fail(req, "failed", "Need char const* argument drawing_area");
       return;
   }

   auto ret = g_afb_instance->app.api.activatesurface(a_drawing_name, a_drawing_area);
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling activatesurface: %s", e.what());
      return;
   }

}

void windowmanager_deactivatesurface(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   const char* a_drawing_name = afb_req_value(req, "drawing_name");
   if(!a_drawing_name){
       afb_req_fail(req, "failed", "Need char const* argument drawing_name");
       return;
   }

   auto ret = g_afb_instance->app.api.deactivatesurface(a_drawing_name);
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling deactivatesurface: %s", e.what());
      return;
   }

}

void windowmanager_enddraw(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   const char* a_drawing_name = afb_req_value(req, "drawing_name");
   if(!a_drawing_name){
       afb_req_fail(req, "failed", "Need char const* argument drawing_name");
       return;
   }

   auto ret = g_afb_instance->app.api.enddraw(a_drawing_name);
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling enddraw: %s", e.what());
      return;
   }

}

void windowmanager_wm_subscribe(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   json_object *jreq = afb_req_json(req);
   json_object *j = nullptr;
   if (! json_object_object_get_ex(jreq, "event", &j)) {
      afb_req_fail(req, "failed", "Need char const* argument event");
      return;
   }
   int event_type = json_object_get_int(j);
   const char *event_name = g_afb_instance->app.kListEventName[event_type];
   struct afb_event event = g_afb_instance->app.map_afb_event[event_name];
   int ret = afb_req_subscribe(req, event);
   if (ret) {
      afb_req_fail(req, "failed", "Error: afb_req_subscribe()");
      return;
   }
   afb_req_success(req, NULL, "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling wm_subscribe: %s", e.what());
      return;
   }

}

void windowmanager_list_drawing_names(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   auto ret = g_afb_instance->app.api.list_drawing_names();
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling list_drawing_names: %s", e.what());
      return;
   }

}

void windowmanager_ping(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   auto ret = g_afb_instance->app.api.ping();
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling ping: %s", e.what());
      return;
   }

}

void windowmanager_debug_status(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   auto ret = g_afb_instance->app.api.debug_status();
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling debug_status: %s", e.what());
      return;
   }

}

void windowmanager_debug_layers(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   auto ret = g_afb_instance->app.api.debug_layers();
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling debug_layers: %s", e.what());
      return;
   }

}

void windowmanager_debug_surfaces(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   auto ret = g_afb_instance->app.api.debug_surfaces();
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling debug_surfaces: %s", e.what());
      return;
   }

}

void windowmanager_debug_terminate(afb_req req) noexcept {
   std::lock_guard<std::mutex> guard(binding_m);
   #ifdef ST
   ST();
   #endif
   if (g_afb_instance == nullptr) {
      afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
      return;
   }

   try {
   // BEGIN impl
   auto ret = g_afb_instance->app.api.debug_terminate();
   if (ret.is_err()) {
      afb_req_fail(req, "failed", ret.unwrap_err());
      return;
   }

   afb_req_success(req, ret.unwrap(), "success");
   // END impl
   } catch (std::exception &e) {
      afb_req_fail_f(req, "failed", "Uncaught exception while calling debug_terminate: %s", e.what());
      return;
   }

}

const struct afb_verb_v2 windowmanager_verbs[] = {
   { "requestsurface", windowmanager_requestsurface, nullptr, nullptr, AFB_SESSION_NONE },
   { "requestsurfacexdg", windowmanager_requestsurfacexdg, nullptr, nullptr, AFB_SESSION_NONE },
   { "activatesurface", windowmanager_activatesurface, nullptr, nullptr, AFB_SESSION_NONE },
   { "deactivatesurface", windowmanager_deactivatesurface, nullptr, nullptr, AFB_SESSION_NONE },
   { "enddraw", windowmanager_enddraw, nullptr, nullptr, AFB_SESSION_NONE },
   { "wm_subscribe", windowmanager_wm_subscribe, nullptr, nullptr, AFB_SESSION_NONE },
   { "list_drawing_names", windowmanager_list_drawing_names, nullptr, nullptr, AFB_SESSION_NONE },
   { "ping", windowmanager_ping, nullptr, nullptr, AFB_SESSION_NONE },
   { "debug_status", windowmanager_debug_status, nullptr, nullptr, AFB_SESSION_NONE },
   { "debug_layers", windowmanager_debug_layers, nullptr, nullptr, AFB_SESSION_NONE },
   { "debug_surfaces", windowmanager_debug_surfaces, nullptr, nullptr, AFB_SESSION_NONE },
   { "debug_terminate", windowmanager_debug_terminate, nullptr, nullptr, AFB_SESSION_NONE },
   {}
};

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

void binding_api::send_event(char const *evname, char const *label, char const *area) {
   HMI_DEBUG("wm", "%s: %s(%s, %s)", __func__, evname, label, area);

   json_object *j = json_object_new_object();
   json_object_object_add(j, kKeyDrawingName, json_object_new_string(label));
   json_object_object_add(j, kKeyDrawingArea, json_object_new_string(area));

   int ret = afb_event_push(g_afb_instance->app.map_afb_event[evname], j);
   if (ret != 0) {
      HMI_DEBUG("wm", "afb_event_push failed: %m");
   }
}
} // namespace wm

extern "C" const struct afb_binding_v2 afbBindingV2 = {
   "windowmanager", nullptr, nullptr, windowmanager_verbs, nullptr, binding_init, nullptr, 0};
