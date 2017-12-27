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
#include "layers.hpp"
#include "layout.hpp"
#include "util.hpp"
#include "wayland.hpp"

#include <cstdio>
#include <memory>

#include <cassert>

#include <json-c/json.h>

#include <algorithm>
#include <csignal>
#include <fstream>
#include <json.hpp>
#include <regex>
#include <thread>


namespace wm {

namespace {

using nlohmann::json;

result<json> file_to_json(char const *filename) {
   json j;
   std::ifstream i(filename);
   if (i.fail()) {
      HMI_DEBUG("wm", "Could not open config file, so use default layer information");
      j = default_layers_json;
   }
   else {
      i >> j;
   }

   return Ok(j);
}

struct result<layer_map> load_layer_map(char const *filename) {
   HMI_DEBUG("wm", "loading IDs from %s", filename);

   auto j = file_to_json(filename);
   if (j.is_err()) {
      return Err<layer_map>(j.unwrap_err());
   }
   json jids = j.unwrap();

   return to_layer_map(jids);
}

}  // namespace


/**
 * App Impl
 */
App::App(wl::display *d)
   : chooks{this},
     display{d},
     controller{},
     outputs(),
     config(),
     layers(),
     id_alloc{},
     pending_events(false),
     policy{} {
   try {
      {
         auto l = load_layer_map(
            this->config.get_string("layers.json").value().c_str());
         if (l.is_ok()) {
            this->layers = l.unwrap();
         } else {
            HMI_ERROR("wm", "%s", l.err().value());
         }
      }
   } catch (std::exception &e) {
      HMI_ERROR("wm", "Loading of configuration failed: %s", e.what());
   }
}

int App::init() {
   if (!this->display->ok()) {
      return -1;
   }

   if (this->layers.mapping.empty()) {
      HMI_ERROR("wm", "No surface -> layer mapping loaded");
      return -1;
   }

   // Make afb event
   for (int i=Event_Val_Min; i<=Event_Val_Max; i++) {
      map_afb_event[kListEventName[i]] = afb_daemon_make_event(kListEventName[i]);
   }

   this->display->add_global_handler(
      "wl_output", [this](wl_registry *r, uint32_t name, uint32_t v) {
         this->outputs.emplace_back(std::make_unique<wl::output>(r, name, v));
      });

   this->display->add_global_handler(
      "ivi_controller", [this](wl_registry *r, uint32_t name, uint32_t v) {
         this->controller =
            std::make_unique<struct compositor::controller>(r, name, v);

         // Init controller hooks
         this->controller->chooks = &this->chooks;

         // This protocol needs the output, so lets just add our mapping here...
         this->controller->add_proxy_to_id_mapping(
            this->outputs.back()->proxy.get(),
            wl_proxy_get_id(reinterpret_cast<struct wl_proxy *>(
               this->outputs.back()->proxy.get())));
      });

   // First level objects
   this->display->roundtrip();
   // Second level objects
   this->display->roundtrip();
   // Third level objects
   this->display->roundtrip();

   return init_layers();
}

int App::dispatch_events() {
   if (this->dispatch_events() == 0) {
      return 0;
   }

   int ret = this->display->dispatch();
   if (ret == -1) {
      HMI_ERROR("wm", "wl_display_dipatch() returned error %d",
               this->display->get_error());
      return -1;
   }
   this->display->flush();

   return 0;
}

int App::dispatch_pending_events() {
   if (this->pop_pending_events()) {
      this->display->dispatch_pending();
      return 0;
   }
   return -1;
}

bool App::pop_pending_events() {
   bool x{true};
   return this->pending_events.compare_exchange_strong(
      x, false, std::memory_order_consume);
}

void App::set_pending_events() {
   this->pending_events.store(true, std::memory_order_release);
}

optional<int> App::lookup_id(char const *name) {
   return this->id_alloc.lookup(std::string(name));
}
optional<std::string> App::lookup_name(int id) {
   return this->id_alloc.lookup(id);
}

/**
 * init_layers()
 */
int App::init_layers() {
   if (!this->controller) {
      HMI_ERROR("wm", "ivi_controller global not available");
      return -1;
   }

   if (this->outputs.empty()) {
      HMI_ERROR("wm", "no output was set up!");
      return -1;
   }

   auto &c = this->controller;

   auto &o = this->outputs.front();
   auto &s = c->screens.begin()->second;
   auto &layers = c->layers;

   // Write output dimensions to ivi controller...
   c->output_size = compositor::size{uint32_t(o->width), uint32_t(o->height)};

   // Clear scene
   layers.clear();

   // Clear screen
   s->clear();

   // Quick and dirty setup of layers
   for (auto const &i : this->layers.mapping) {
      c->layer_create(i.second.layer_id, o->width, o->height);
      auto &l = layers[i.second.layer_id];
      l->set_destination_rectangle(0, 0, o->width, o->height);
      l->set_visibility(1);
      HMI_DEBUG("wm", "Setting up layer %s (%d) for surface role match \"%s\"",
               i.second.name.c_str(), i.second.layer_id, i.second.role.c_str());
   }

   // Add layers to screen
   s->set_render_order(this->layers.layers);

   this->layout_commit();

   return 0;
}

void App::surface_set_layout(int surface_id, optional<int> sub_surface_id) {
   if (!this->controller->surface_exists(surface_id)) {
      HMI_ERROR("wm", "Surface %d does not exist", surface_id);
      return;
   }

   auto o_layer_id = this->layers.get_layer_id(surface_id);

   if (!o_layer_id) {
      HMI_ERROR("wm", "Surface %d is not associated with any layer!", surface_id);
      return;
   }

   uint32_t layer_id = *o_layer_id;

   auto const &layer = this->layers.get_layer(layer_id);
   auto rect = layer.value().rect;
   auto &s = this->controller->surfaces[surface_id];

   int x = rect.x;
   int y = rect.y;
   int w = rect.w;
   int h = rect.h;

   // less-than-0 values refer to MAX + 1 - $VALUE
   // e.g. MAX is either screen width or height
   if (w < 0) {
      w = this->controller->output_size.w + 1 + w;
   }
   if (h < 0) {
      h = this->controller->output_size.h + 1 + h;
   }

   if (sub_surface_id) {
      if (o_layer_id != this->layers.get_layer_id(*sub_surface_id)) {
         HMI_ERROR("wm",
            "surface_set_layout: layers of surfaces (%d and %d) don't match!",
            surface_id, *sub_surface_id);
         return;
      }

      int x_off = 0;
      int y_off = 0;

      // split along major axis
      if (w > h) {
         w /= 2;
         x_off = w;
      } else {
         h /= 2;
         y_off = h;
      }

      auto &ss = this->controller->surfaces[*sub_surface_id];

      HMI_DEBUG("wm", "surface_set_layout for sub surface %u on layer %u",
               *sub_surface_id, layer_id);

      // configure surface to wxh dimensions
      ss->set_configuration(w, h);
      // set source reactangle, even if we should not need to set it.
      ss->set_source_rectangle(0, 0, w, h);
      // set destination to the display rectangle
      ss->set_destination_rectangle(x + x_off, y + y_off, w, h);

   }

   HMI_DEBUG("wm", "surface_set_layout for surface %u on layer %u", surface_id,
            layer_id);

   // configure surface to wxh dimensions
   s->set_configuration(w, h);
   // set source reactangle, even if we should not need to set it.
   s->set_source_rectangle(0, 0, w, h);

   // set destination to the display rectangle
   s->set_destination_rectangle(x, y, w, h);

   HMI_DEBUG("wm", "Surface %u now on layer %u with rect { %d, %d, %d, %d }",
            surface_id, layer_id, x, y, w, h);
}

void App::layout_commit() {
   this->controller->commit_changes();
   this->display->flush();
}

char const *App::api_activate_surface(char const *drawing_name, char const *drawing_area) {
   ST();

   auto const &surface_id = this->lookup_id(drawing_name);

   if (!surface_id) {
      return "Surface does not exist";
   }

   if (!this->controller->surface_exists(*surface_id)) {
      return "Surface does not exist in controller!";
   }

   auto layer_id = this->layers.get_layer_id(*surface_id);

   if (!layer_id) {
      return "Surface is not on any layer!";
   }

   auto o_state = *this->layers.get_layout_state(*surface_id);

   if (o_state == nullptr) {
      return "Could not find layer for surface";
   }

   struct LayoutState &state = *o_state;

   // disable layers that are above our current layer
   for (auto const &l : this->layers.mapping) {
      if (l.second.layer_id <= *layer_id) {
         continue;
      }

      bool flush = false;
      if (l.second.state.main != -1) {
         this->deactivate(l.second.state.main);
         l.second.state.main = -1;
         flush = true;
      }

      if (l.second.state.sub != -1) {
         this->deactivate(l.second.state.sub);
         l.second.state.sub = -1;
         flush = true;
      }

      if (flush) {
         this->layout_commit();
      }
   }

   auto layer = this->layers.get_layer(*layer_id);

   if (state.main == -1) {
      this->try_layout(
         state, LayoutState{*surface_id}, [&] (LayoutState const &nl) {
            HMI_DEBUG("wm", "Layout: %s", kNameLayoutNormal);
            this->surface_set_layout(*surface_id);
            state = nl;

            // Commit for configuraton
            this->layout_commit();

            if (!(layer->is_normal_layout_only)) {
               // Wait for configuration listener
               controller->is_configured = false;
               while (!(controller->is_configured)) {
                  dispatch_pending_events();
               }
            }

            std::string str_area = std::string(kNameLayoutNormal) + "." + std::string(kNameAreaFull);
            this->emit_syncdraw(drawing_name, str_area.c_str());
            this->enqueue_flushdraw(state.main);
         });
   } else {
      if (0 == strcmp(drawing_name, "HomeScreen")) {
         this->try_layout(
            state, LayoutState{*surface_id}, [&] (LayoutState const &nl) {
               HMI_DEBUG("wm", "Layout: %s", kNameLayoutNormal);
               std::string str_area = std::string(kNameLayoutNormal) + "." + std::string(kNameAreaFull);
               this->emit_syncdraw(drawing_name, str_area.c_str());
               this->enqueue_flushdraw(state.main);
            });
      } else {
         bool can_split = this->can_split(state, *surface_id);

         if (can_split) {
            this->try_layout(
               state,
               LayoutState{state.main, *surface_id},
               [&] (LayoutState const &nl) {
                  HMI_DEBUG("wm", "Layout: %s", kNameLayoutSplit);
                  std::string main =
                     std::move(*this->lookup_name(state.main));

                  this->surface_set_layout(state.main, surface_id);
                  if (state.sub != *surface_id) {
                      if (state.sub != -1) {
                         this->deactivate(state.sub);
                      }
                  }
                  state = nl;

                  // Commit for configuraton and visibility(0)
                  this->layout_commit();

                  // Wait for configuration listener
                  controller->is_configured = false;
                  while (!(controller->is_configured)) {
                     dispatch_pending_events();
                  }

                  std::string str_area_main = std::string(kNameLayoutSplit) + "." + std::string(kNameAreaMain);
                  std::string str_area_sub = std::string(kNameLayoutSplit) + "." + std::string(kNameAreaSub);
                  this->emit_syncdraw(main.c_str(), str_area_main.c_str());
                  this->emit_syncdraw(drawing_name, str_area_sub.c_str());
                  this->enqueue_flushdraw(state.main);
                  this->enqueue_flushdraw(state.sub);
               });
         } else {
            this->try_layout(
               state, LayoutState{*surface_id}, [&] (LayoutState const &nl) {
                  HMI_DEBUG("wm", "Layout: %s", kNameLayoutNormal);

                  this->surface_set_layout(*surface_id);
                  if (state.main != *surface_id) {
                      this->deactivate(state.main);
                  }
                  if (state.sub != -1) {
                      if (state.sub != *surface_id) {
                         this->deactivate(state.sub);
                      }
                  }
                  state = nl;

                  // Commit for configuraton and visibility(0)
                  this->layout_commit();

                  if (!(layer->is_normal_layout_only)) {
                     // Wait for configuration listener
                     controller->is_configured = false;
                     while (!(controller->is_configured)) {
                        dispatch_pending_events();
                     }
                  }

                  std::string str_area = std::string(kNameLayoutNormal) + "." + std::string(kNameAreaFull);
                  this->emit_syncdraw(drawing_name, str_area.c_str());
                  this->enqueue_flushdraw(state.main);
               });
         }
      }
   }

   // no error
   return nullptr;
}

char const *App::api_deactivate_surface(char const *drawing_name) {
   ST();
   auto const &surface_id = this->lookup_id(drawing_name);

   if (!surface_id) {
         return "Surface does not exist";
      }

   if (*surface_id == this->layers.main_surface) {
      return "Cannot deactivate main_surface";
   }

   auto o_state = *this->layers.get_layout_state(*surface_id);

   if (o_state == nullptr) {
      return "Could not find layer for surface";
   }

   struct LayoutState &state = *o_state;

   if (state.main == -1) {
      return "No surface active";
   }

   // Check against main_surface, main_surface_name is the configuration item.
   if (*surface_id == this->layers.main_surface) {
      HMI_DEBUG("wm", "Refusing to deactivate main_surface %d", *surface_id);
      return nullptr;
   }

   if (state.main == *surface_id) {
      if (state.sub != -1) {
         this->try_layout(
            state, LayoutState{state.sub, -1}, [&] (LayoutState const &nl) {
               std::string sub = std::move(*this->lookup_name(state.sub));

               this->deactivate(*surface_id);
               this->surface_set_layout(state.sub);
               state = nl;

               this->layout_commit();
               std::string str_area = std::string(kNameLayoutNormal) + "." + std::string(kNameAreaFull);
               this->emit_syncdraw(sub.c_str(), str_area.c_str());
               this->enqueue_flushdraw(state.sub);
            });
      } else {
         this->try_layout(state, LayoutState{-1, -1}, [&] (LayoutState const &nl) {
            this->deactivate(*surface_id);
            state = nl;
            this->layout_commit();
         });
      }
   } else if (state.sub == *surface_id) {
      this->try_layout(
         state, LayoutState{state.main, -1}, [&] (LayoutState const &nl) {
            std::string main = std::move(*this->lookup_name(state.main));

            this->deactivate(*surface_id);
            this->surface_set_layout(state.main);
            state = nl;

            this->layout_commit();
            std::string str_area = std::string(kNameLayoutNormal) + "." + std::string(kNameAreaFull);
            this->emit_syncdraw(main.c_str(), str_area.c_str());
            this->enqueue_flushdraw(state.main);
         });
   } else {
      return "Surface is not active";
   }

   return nullptr;
}

void App::enqueue_flushdraw(int surface_id) {
   this->check_flushdraw(surface_id);
   HMI_DEBUG("wm", "Enqueuing EndDraw for surface_id %d", surface_id);
   this->pending_end_draw.push_back(surface_id);
}

void App::check_flushdraw(int surface_id) {
   auto i = std::find(std::begin(this->pending_end_draw),
                      std::end(this->pending_end_draw), surface_id);
   if (i != std::end(this->pending_end_draw)) {
      auto n = this->lookup_name(surface_id);
      HMI_ERROR("wm", "Application %s (%d) has pending EndDraw call(s)!",
               n ? n->c_str() : "unknown-name", surface_id);
      std::swap(this->pending_end_draw[std::distance(
                   std::begin(this->pending_end_draw), i)],
                this->pending_end_draw.back());
      this->pending_end_draw.resize(this->pending_end_draw.size() - 1);
   }
}

char const *App::api_enddraw(char const *drawing_name) {
   for (unsigned i = 0, iend = this->pending_end_draw.size(); i < iend; i++) {
      auto n = this->lookup_name(this->pending_end_draw[i]);
      if (n && *n == drawing_name) {
         std::swap(this->pending_end_draw[i], this->pending_end_draw[iend - 1]);
         this->pending_end_draw.resize(iend - 1);
         this->activate(this->pending_end_draw[i]);
         this->layout_commit();
         this->emit_flushdraw(drawing_name);
         return nullptr;
      }
   }
   return "No EndDraw pending for surface";
}

void App::api_ping() { this->dispatch_pending_events(); }

void App::send_event(char const *evname, char const *label){
   HMI_DEBUG("wm", "%s: %s(%s)", __func__, evname, label);

   json_object *j = json_object_new_object();
   json_object_object_add(j, kKeyDrawingName, json_object_new_string(label));

   int ret = afb_event_push(this->map_afb_event[evname], j);
   if (ret != 0) {
      HMI_DEBUG("wm", "afb_event_push failed: %m");
   }
}

void App::send_event(char const *evname, char const *label, char const *area){
   HMI_DEBUG("wm", "%s: %s(%s, %s)", __func__, evname, label, area);

   json_object *j = json_object_new_object();
   json_object_object_add(j, kKeyDrawingName, json_object_new_string(label));
   json_object_object_add(j, kKeyDrawingArea, json_object_new_string(area));

   int ret = afb_event_push(this->map_afb_event[evname], j);
   if (ret != 0) {
      HMI_DEBUG("wm", "afb_event_push failed: %m");
   }
}

/**
 * proxied events
 */
void App::surface_created(uint32_t surface_id) {
   auto layer_id = this->layers.get_layer_id(surface_id);
   if (!layer_id) {
      HMI_DEBUG("wm", "Newly created surfce %d is not associated with any layer!",
               surface_id);
      return;
   }

   HMI_DEBUG("wm", "surface_id is %u, layer_id is %u", surface_id, *layer_id);

   this->controller->layers[*layer_id]->add_surface(
      this->controller->surfaces[surface_id].get());
   this->layout_commit();
   // activate the main_surface right away
   /*if (surface_id == static_cast<unsigned>(this->layers.main_surface)) {
      HMI_DEBUG("wm", "Activating main_surface (%d)", surface_id);

      this->api_activate_surface(
         this->lookup_name(surface_id).value_or("unknown-name").c_str());
   }*/
}

void App::surface_removed(uint32_t surface_id) {
   HMI_DEBUG("wm", "surface_id is %u", surface_id);

   // We cannot normally deactivate the main_surface, so be explicit
   // about it:
   if (int(surface_id) == this->layers.main_surface) {
      this->deactivate_main_surface();
   } else {
      auto drawing_name = this->lookup_name(surface_id);
      if (drawing_name) {
         this->api_deactivate_surface(drawing_name->c_str());
      }
   }

   this->id_alloc.remove_id(surface_id);
   this->layers.remove_surface(surface_id);
}

void App::emit_activated(char const *label) {
   this->send_event(kListEventName[Event_Active], label);
}

void App::emit_deactivated(char const *label) {
   this->send_event(kListEventName[Event_Inactive], label);
}

void App::emit_syncdraw(char const *label, char const *area) {
    this->send_event(kListEventName[Event_SyncDraw], label, area);
}

void App::emit_flushdraw(char const *label) {
   this->send_event(kListEventName[Event_FlushDraw], label);
}

void App::emit_visible(char const *label, bool is_visible) {
   this->send_event(is_visible ? kListEventName[Event_Visible] : kListEventName[Event_Invisible], label);
}

void App::emit_invisible(char const *label) {
   return emit_visible(label, false);
}

void App::emit_visible(char const *label) { return emit_visible(label, true); }

result<int> App::api_request_surface(char const *drawing_name) {
   auto lid = this->layers.get_layer_id(std::string(drawing_name));
   if (!lid) {
      // TODO: Do we need to put these applications on the App layer?
      return Err<int>("Drawing name does not match any role");
   }

   auto rname = this->lookup_id(drawing_name);
   if (!rname) {
      // name does not exist yet, allocate surface id...
      auto id = int(this->id_alloc.generate_id(drawing_name));
      this->layers.add_surface(id, *lid);

      // set the main_surface[_name] here and now
      if (!this->layers.main_surface_name.empty() &&
          this->layers.main_surface_name == drawing_name) {
         this->layers.main_surface = id;
         HMI_DEBUG("wm", "Set main_surface id to %u", id);
      }

      return Ok<int>(id);
   }

   // Check currently registered drawing names if it is already there.
   return Err<int>("Surface already present");
}

char const *App::api_request_surface(char const *drawing_name,
                                     char const *ivi_id) {
   ST();

   auto lid = this->layers.get_layer_id(std::string(drawing_name));
   unsigned sid = std::stol(ivi_id);

   if (!lid) {
       return "Drawing name does not match any role";
   }

   auto rname = this->lookup_id(drawing_name);

   if (rname) {
       return "Surface already present";
   }

   // register pair drawing_name and ivi_id
   this->id_alloc.register_name_id(drawing_name, sid);
   this->layers.add_surface(sid, *lid);

   // this surface is already created
   HMI_DEBUG("wm", "surface_id is %u, layer_id is %u", sid, *lid);

   this->controller->layers[*lid]->add_surface(
       this->controller->surfaces[sid].get());
   this->layout_commit();

   return nullptr;
}

void App::activate(int id) {
   auto ip = this->controller->sprops.find(id);
   if (ip != this->controller->sprops.end()) {
      this->controller->surfaces[id]->set_visibility(0);
      this->layout_commit();
      this->controller->surfaces[id]->set_visibility(1);
      char const *label =
         this->lookup_name(id).value_or("unknown-name").c_str();

      // FOR CES DEMO >>>
      if ((0 == strcmp(label, "Radio"))
          || (0 == strcmp(label, "MediaPlayer"))
          || (0 == strcmp(label, "Music"))
          || (0 == strcmp(label, "Navigation"))) {
        for (auto i = surface_bg.begin(); i != surface_bg.end(); ++i) {
            if (id == *i) {
               // Remove id
               this->surface_bg.erase(i);

               // Remove from BG layer (999)
               HMI_DEBUG("wm", "Remove %s(%d) from BG layer", label, id);
               this->controller->layers[999]->remove_surface(
                  this->controller->surfaces[id].get());

               // Add to FG layer (1001)
               HMI_DEBUG("wm", "Add %s(%d) to FG layer", label, id);
               this->controller->layers[1001]->add_surface(
                  this->controller->surfaces[id].get());

               for (int j : this->surface_bg) {
                 HMI_DEBUG("wm", "Stored id:%d", j);
               }
               break;
            }
         }
      }
      // <<< FOR CES DEMO

      this->emit_visible(label);
      this->emit_activated(label);
   }
}

void App::deactivate(int id) {
   auto ip = this->controller->sprops.find(id);
   if (ip != this->controller->sprops.end() && ip->second.visibility != 0) {
      char const *label =
         this->lookup_name(id).value_or("unknown-name").c_str();

      // FOR CES DEMO >>>
      if ((0 == strcmp(label, "Radio"))
          || (0 == strcmp(label, "MediaPlayer"))
          || (0 == strcmp(label, "Music"))
          || (0 == strcmp(label, "Navigation"))) {

         // Store id
         this->surface_bg.push_back(id);

         // Remove from FG layer (1001)
         HMI_DEBUG("wm", "Remove %s(%d) from FG layer", label, id);
         this->controller->layers[1001]->remove_surface(
            this->controller->surfaces[id].get());

         // Add to BG layer (999)
         HMI_DEBUG("wm", "Add %s(%d) to BG layer", label, id);
         this->controller->layers[999]->add_surface(
            this->controller->surfaces[id].get());

         for (int j : surface_bg) {
            HMI_DEBUG("wm", "Stored id:%d", j);
         }
      }
      else {
         this->controller->surfaces[id]->set_visibility(0);
      }
      // <<< FOR CES DEMO

      this->emit_deactivated(label);
      this->emit_invisible(label);
   }
}

void App::deactivate_main_surface() {
   this->layers.main_surface = -1;
   this->api_deactivate_surface(this->layers.main_surface_name.c_str());
}

bool App::can_split(struct LayoutState const &state, int new_id) {
   if (state.main != -1 && state.main != new_id) {
      auto new_id_layer = this->layers.get_layer_id(new_id).value();
      auto current_id_layer = this->layers.get_layer_id(state.main).value();

      // surfaces are on separate layers, don't bother.
      if (new_id_layer != current_id_layer) {
         return false;
      }

      std::string const &new_id_str = this->lookup_name(new_id).value();
      std::string const &cur_id_str = this->lookup_name(state.main).value();

      auto const &layer = this->layers.get_layer(new_id_layer);

      HMI_DEBUG("wm", "layer info name: %s", layer->name.c_str());

      if (layer->layouts.empty()) {
         return false;
      }

      for (auto i = layer->layouts.cbegin(); i != layer->layouts.cend(); i++) {
         HMI_DEBUG("wm", "%d main_match '%s'", new_id_layer, i->main_match.c_str());
         auto rem = std::regex(i->main_match);
         if (std::regex_match(cur_id_str, rem)) {
            // build the second one only if the first already matched
            HMI_DEBUG("wm", "%d sub_match '%s'", new_id_layer, i->sub_match.c_str());
            auto res = std::regex(i->sub_match);
            if (std::regex_match(new_id_str, res)) {
               HMI_DEBUG("wm", "layout matched!");
               return true;
            }
         }
      }
   }

   return false;
}

void App::try_layout(struct LayoutState & /*state*/,
                     struct LayoutState const &new_layout,
                     std::function<void(LayoutState const &nl)> apply) {
   if (this->policy.layout_is_valid(new_layout)) {
      apply(new_layout);
   }
}

/**
 * controller_hooks
 */
void controller_hooks::surface_created(uint32_t surface_id) {
   this->app->surface_created(surface_id);
}

void controller_hooks::surface_removed(uint32_t surface_id) {
   this->app->surface_removed(surface_id);
}

void controller_hooks::surface_visibility(uint32_t /*surface_id*/,
                                          uint32_t /*v*/) {}

void controller_hooks::surface_destination_rectangle(uint32_t /*surface_id*/,
                                                     uint32_t /*x*/,
                                                     uint32_t /*y*/,
                                                     uint32_t /*w*/,
                                                     uint32_t /*h*/) {}

}  // namespace wm
