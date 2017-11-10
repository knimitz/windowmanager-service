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

#include "app.hpp"
#include "json_helper.hpp"

#include <csignal>

#include <json.hpp>

using json = nlohmann::json;

#include <json-c/json.h>

namespace wm {
/**
 * binding_api impl
 */
binding_api::result_type binding_api::requestsurface(
   char const *drawing_name) {
   auto r = this->app->api_request_surface(drawing_name);
   if (r.is_err()) {
      return Err<json_object *>(r.unwrap_err());
   }
   return Ok(json_object_new_int(r.unwrap()));
}

binding_api::result_type binding_api::activatesurface(
   char const *drawing_name, char const *drawing_area) {
  HMI_DEBUG("wm", "%s drawing_name %s, drawing_area %s", __func__, drawing_name, drawing_area);
  auto r = this->app->api_activate_surface(drawing_name, drawing_area);
   if (r != nullptr) {
      HMI_DEBUG("wm", "%s failed with error: %s", __func__, r);
      return Err<json_object *>(r);
   }
   return Ok(json_object_new_object());
}

binding_api::result_type binding_api::deactivatesurface(char const* drawing_name) {
   HMI_DEBUG("wm", "%s drawing_name %s", __func__, drawing_name);
   auto r = this->app->api_deactivate_surface(drawing_name);
   if (r != nullptr) {
      HMI_DEBUG("wm", "%s failed with error: %s", __func__, r);
      return Err<json_object *>(r);
   }
   return Ok(json_object_new_object());
}

binding_api::result_type binding_api::enddraw(char const* drawing_name) {
   HMI_DEBUG("wm", "%s drawing_name %s", __func__, drawing_name);
   auto r = this->app->api_enddraw(drawing_name);
   if (r != nullptr) {
      HMI_DEBUG("wm", "%s failed with error: %s", __func__, r);
      return Err<json_object *>(r);
   }
   return Ok(json_object_new_object());
}

binding_api::result_type binding_api::list_drawing_names() {
   HMI_DEBUG("wm", "%s", __func__);
   json j = this->app->id_alloc.name2id;
   return Ok(json_tokener_parse(j.dump().c_str()));
}

binding_api::result_type binding_api::debug_layers() {
   HMI_DEBUG("wm", "%s", __func__);
   return Ok(json_tokener_parse(this->app->layers.to_json().dump().c_str()));
}

binding_api::result_type binding_api::debug_surfaces() {
   HMI_DEBUG("wm", "%s", __func__);
   return Ok(to_json(this->app->controller->sprops));
}

binding_api::result_type binding_api::debug_status() {
   HMI_DEBUG("wm", "%s", __func__);
   json_object *jr = json_object_new_object();
   json_object_object_add(jr, "surfaces",
                          to_json(this->app->controller->sprops));
   json_object_object_add(jr, "layers", to_json(this->app->controller->lprops));
   return Ok(jr);
}

binding_api::result_type binding_api::debug_terminate() {
   HMI_DEBUG("wm", "%s", __func__);
   if (getenv("WINMAN_DEBUG_TERMINATE") != nullptr) {
      raise(SIGKILL);  // afb-daemon kills it's pgroup using TERM, which
                       // doesn't play well with perf
   }
   return Ok(json_object_new_object());
}

binding_api::result_type binding_api::ping() {
   this->app->api_ping();
   return Ok(json_object_new_object());
}

}  // namespace wm
