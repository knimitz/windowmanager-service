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

#include "layers.hpp"
#include "hmi-debug.h"

namespace wm
{

using json = nlohmann::json;

layer::layer(nlohmann::json const &j)
{
    this->role = j["role"];
    this->name = j["name"];
    this->layer_id = j["layer_id"];
    this->rect = compositor::full_rect;
    if (j["area"]["type"] == "rect")
    {
        auto jr = j["area"]["rect"];
        this->rect = compositor::rect{
            jr["width"],
            jr["height"],
            jr["x"],
            jr["y"],
        };
    }

    // Init flag of normal layout only
    this->is_normal_layout_only = true;

    auto split_layouts = j.find("split_layouts");
    if (split_layouts != j.end())
    {

        // Clear flag of normal layout only
        this->is_normal_layout_only = false;

        auto &sls = j["split_layouts"];
        // this->layouts.reserve(sls.size());
        std::transform(std::cbegin(sls), std::cend(sls),
                       std::back_inserter(this->layouts), [this](json const &sl) {
                           struct split_layout l
                           {
                               sl["name"], sl["main_match"], sl["sub_match"]
                           };
                           HMI_DEBUG("wm",
                                     "layer %d add split_layout \"%s\" (main: \"%s\") (sub: "
                                     "\"%s\")",
                                     this->layer_id,
                                     l.name.c_str(), l.main_match.c_str(),
                                     l.sub_match.c_str());
                           return l;
                       });
    }
    HMI_DEBUG("wm", "layer_id:%d is_normal_layout_only:%d\n",
              this->layer_id, this->is_normal_layout_only);
}

struct result<struct layer_map> to_layer_map(nlohmann::json const &j)
{
    try
    {
        layer_map stl{};
        auto m = j["mappings"];

        std::transform(std::cbegin(m), std::cend(m),
                       std::inserter(stl.mapping, stl.mapping.end()),
                       [](nlohmann::json const &j) {
                           return std::pair<int, struct layer>(
                               j.value("layer_id", -1), layer(j));
                       });

        // TODO: add sanity checks here?
        // * check for double IDs
        // * check for double names/roles

        stl.layers.reserve(m.size());
        std::transform(std::cbegin(stl.mapping), std::cend(stl.mapping),
                       std::back_inserter(stl.layers),
                       [&stl](std::pair<int, struct layer> const &k) {
                           stl.roles.emplace_back(
                               std::make_pair(k.second.role, k.second.layer_id));
                           return unsigned(k.second.layer_id);
                       });

        std::sort(stl.layers.begin(), stl.layers.end());

        for (auto i : stl.mapping)
        {
            if (i.second.name.empty())
            {
                return Err<struct layer_map>("Found mapping w/o name");
            }
            if (i.second.layer_id == -1)
            {
                return Err<struct layer_map>("Found invalid/unset IDs in mapping");
            }
        }

        auto msi = j.find("main_surface");
        if (msi != j.end())
        {
            stl.main_surface_name = msi->value("surface_role", "");
            stl.main_surface = -1;
        }

        return Ok(stl);
    }
    catch (std::exception &e)
    {
        return Err<struct layer_map>(e.what());
    }
}

optional<int>
layer_map::get_layer_id(int surface_id)
{
    auto i = this->surfaces.find(surface_id);
    if (i != this->surfaces.end())
    {
        return optional<int>(i->second);
    }
    return nullopt;
}

optional<int> layer_map::get_layer_id(std::string const &role)
{
    for (auto const &r : this->roles)
    {
        auto re = std::regex(r.first);
        if (std::regex_match(role, re))
        {
            HMI_DEBUG("wm", "role %s matches layer %d", role.c_str(), r.second);
            return optional<int>(r.second);
        }
    }
    HMI_DEBUG("wm", "role %s does NOT match any layer", role.c_str());
    return nullopt;
}

json layer::to_json() const
{
    auto is_full = this->rect == compositor::full_rect;

    json r{};
    if (is_full)
    {
        r = {{"type", "full"}};
    }
    else
    {
        r = {{"type", "rect"},
             {"rect",
              {{"x", this->rect.x},
               {"y", this->rect.y},
               {"width", this->rect.w},
               {"height", this->rect.h}}}};
    }

    return {
        {"name", this->name},
        {"role", this->role},
        {"layer_id", this->layer_id},
        {"area", r},
    };
}

json layer_map::to_json() const
{
    json j{};
    for (auto const &i : this->mapping)
    {
        j.push_back(i.second.to_json());
    }
    return j;
}

} // namespace wm
