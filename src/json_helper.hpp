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

#ifndef TMCAGLWM_JSON_HELPER_HPP
#define TMCAGLWM_JSON_HELPER_HPP

#include "result.hpp"
#include "wayland.hpp"
#include <json.hpp>

struct json_object;

json_object *to_json(compositor::screen const *s);
json_object *to_json(compositor::controller::props_map const &s);
json_object *to_json(std::vector<uint32_t> const &v);

#endif  // TMCAGLWM_JSON_HELPER_HPP
