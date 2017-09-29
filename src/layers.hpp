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

#ifndef TMCAGLWM_LAYERS_H
#define TMCAGLWM_LAYERS_H

#include <json.hpp>

#include <regex>
#include <set>
#include <string>

#include "layout.hpp"
#include "result.hpp"
#include "wayland.hpp"

namespace wm {

struct split_layout {
   std::string name;
   std::string main_match;
   std::string sub_match;
};

struct layer {
   using json = nlohmann::json;

   // A more or less descriptive name?
   std::string name = "";
   // The actual layer ID
   int layer_id = -1;
   // The rectangular region surfaces are allowed to draw on
   // this layer, note however, width and hieght of the rect
   // can be negative, in which case they specify that
   // the actual value is computed using MAX + 1 - w
   // That is; allow us to specify dimensions dependent on
   // e.g. screen dimension, w/o knowing the actual screen size.
   genivi::rect rect;
   // Specify a role prefix for surfaces that should be
   // put on this layer.
   std::string role;
   // XXX perhaps a zorder is needed here?
   std::vector<struct split_layout> layouts;
   // XXX need to change the way we store these things...
   mutable struct LayoutState state;

   explicit layer(nlohmann::json const &j);

   json to_json() const;
};

struct layer_map {
   using json = nlohmann::json;

   using storage_type = std::map<int, struct layer>;
   using layers_type = std::vector<uint32_t>;
   using role_to_layer_map = std::vector<std::pair<std::string, int>>;
   using addsurf_layer_map = std::map<int, int>;

   // XXX: we also will need a layer_id to layer map, perhaps
   // make this the primary map, and the surface_id->layer a
   // secondary map.

   storage_type mapping;  // map surface_id to layer
   layers_type layers;    // the actual layer IDs we have
   int main_surface;
   std::string main_surface_name;
   role_to_layer_map roles;
   addsurf_layer_map surfaces;  // additional surfaces on layers

   optional<int> get_layer_id(int surface_id);
   optional<int> get_layer_id(std::string const &role);
   optional<struct LayoutState*> get_layout_state(int surface_id) {
      int layer_id = *this->get_layer_id(surface_id);
      auto i = this->mapping.find(layer_id);
      return i == this->mapping.end()
                ? nullopt
                : optional<struct LayoutState *>(&i->second.state);
   }
   optional<struct layer> get_layer(int layer_id) {
      auto i = this->mapping.find(layer_id);
      return i == this->mapping.end() ? nullopt
                                      : optional<struct layer>(i->second);
   }

   layers_type::size_type get_layers_count() const {
      return this->layers.size();
   }

   void add_surface(int surface_id, int layer_id) {
      this->surfaces[surface_id] = layer_id;
   }

   void remove_surface(int surface_id) {
      this->surfaces.erase(surface_id);
   }

   json to_json() const;
};

struct result<struct layer_map> to_layer_map(nlohmann::json const &j);

}  // namespace wm

#endif  // TMCAGLWM_LAYERS_H
