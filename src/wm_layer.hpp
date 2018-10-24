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

#ifndef WM_LAYER_HPP
#define WM_LAYER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "wm_error.hpp"

struct json_object;

namespace wm
{

class WMLayer
{
  public:
    explicit WMLayer(json_object* j);
    ~WMLayer() = default;
    // A more or less descriptive name?
    const std::string& layerName(){ return this->role_list;}
    unsigned layerID(){ return this->layer_id;}
    const std::string& roleList();
    bool hasRole(const std::string& role);
  private:
    std::string name = "";
    // The actual layer ID
    int layer_id = 0;

    // Specify a role prefix for surfaces that should be
    // put on this layer.
    std::string role_list;
};

} // namespace wm

#endif // WM_LAYER_HPP
