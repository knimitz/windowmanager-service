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

#include "config.hpp"
#include "hmi-debug.h"

namespace wm {

config::config() : cfg() {
   // Supply default values for these...
   char const *path_layers_json = getenv("AFM_APP_INSTALL_DIR");

   if (!path_layers_json) {
      HMI_ERROR("wm", "AFM_APP_INSTALL_DIR is not defined");
   }
   else {
      this->cfg["layers.json"] = std::string(path_layers_json) + std::string("/etc/layers.json");
   }
}

}  // namespace wm
