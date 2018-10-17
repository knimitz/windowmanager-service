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

#include <regex>
#include <ilm/ilm_control.h>
#include <stdlib.h>
#include "wm_client.hpp"
#include "wm_layer.hpp"
#include "json_helper.hpp"
#include "util.hpp"

using std::string;

namespace wm
{

WMLayer::WMLayer(json_object* j)
{
    this->name = jh::getStringFromJson(j, "name");
    this->role_list = jh::getStringFromJson(j, "role");
    this->layer_id = static_cast<unsigned>(jh::getIntFromJson(j, "layer_id"));
}

bool WMLayer::hasRole(const string& role)
{
    auto re = std::regex(this->role_list);
    if (std::regex_match(role, re))
    {
        HMI_DEBUG("role %s matches layer %s", role.c_str(), this->name.c_str());
        return true;
    }
    return false;
}

} // namespace wm
