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

#include <utility>

#include "util.hpp"
#include "wayland.hpp"
#include "hmi-debug.h"

//                                                                  _
//  _ __   __ _ _ __ ___   ___  ___ _ __   __ _  ___ ___  __      _| |
// | '_ \ / _` | '_ ` _ \ / _ \/ __| '_ \ / _` |/ __/ _ \ \ \ /\ / / |
// | | | | (_| | | | | | |  __/\__ \ |_) | (_| | (_|  __/  \ V  V /| |
// |_| |_|\__,_|_| |_| |_|\___||___/ .__/ \__,_|\___\___|   \_/\_/ |_|
//                                 |_|
namespace wl {

//      _ _           _
//   __| (_)___ _ __ | | __ _ _   _
//  / _` | / __| '_ \| |/ _` | | | |
// | (_| | \__ \ |_) | | (_| | |_| |
//  \__,_|_|___/ .__/|_|\__,_|\__, |
//             |_|            |___/
display::display()
   : d(std::unique_ptr<struct wl_display, void (*)(struct wl_display *)>(
        wl_display_connect(nullptr), &wl_display_disconnect)),
     r(d.get()) {}

bool display::ok() const { return d && wl_display_get_error(d.get()) == 0; }

void display::roundtrip() { wl_display_roundtrip(this->d.get()); }

int display::dispatch() { return wl_display_dispatch(this->d.get()); }

int display::dispatch_pending() { return wl_display_dispatch_pending(this->d.get()); }

int display::read_events() {
   ST();
   // XXX: uhm, how?!
   while (wl_display_prepare_read(this->d.get()) == -1) {
      STN(pending_events_dispatch);
      if (wl_display_dispatch_pending(this->d.get()) == -1) {
         return -1;
      }
   }

   if (wl_display_flush(this->d.get()) == -1) {
      return -1;
   }

   if (wl_display_read_events(this->d.get()) == -1) {
      wl_display_cancel_read(this->d.get());
   }

   return 0;
}

void display::flush() { wl_display_flush(this->d.get()); }

int display::get_fd() const { return wl_display_get_fd(this->d.get()); }

int display::get_error() { return wl_display_get_error(this->d.get()); }

//                 _     _
//  _ __ ___  __ _(_)___| |_ _ __ _   _
// | '__/ _ \/ _` | / __| __| '__| | | |
// | | |  __/ (_| | \__ \ |_| |  | |_| |
// |_|  \___|\__, |_|___/\__|_|   \__, |
//           |___/                |___/
namespace {
void registry_global(void *data, struct wl_registry * /*r*/, uint32_t name,
                     char const *iface, uint32_t v) {
   static_cast<struct registry *>(data)->global(name, iface, v);
}

void registry_global_remove(void *data, struct wl_registry * /*r*/,
                            uint32_t name) {
   static_cast<struct registry *>(data)->global_remove(name);
}

constexpr struct wl_registry_listener registry_listener = {
   registry_global, registry_global_remove};
}  // namespace

registry::registry(struct wl_display *d)
   : wayland_proxy(d == nullptr ? nullptr : wl_display_get_registry(d)) {
   if (this->proxy != nullptr) {
      wl_registry_add_listener(this->proxy.get(), &registry_listener, this);
   }
}

void registry::add_global_handler(char const *iface, binder bind) {
   this->bindings[iface] = std::move(bind);
}

void registry::global(uint32_t name, char const *iface, uint32_t v) {
   auto b = this->bindings.find(iface);
   if (b != this->bindings.end()) {
      b->second(this->proxy.get(), name, v);
   }
   HMI_DEBUG("wm", "wl::registry @ %p global n %u i %s v %u", this->proxy.get(), name,
            iface, v);
}

void registry::global_remove(uint32_t /*name*/) {}

//              _               _
//   ___  _   _| |_ _ __  _   _| |_
//  / _ \| | | | __| '_ \| | | | __|
// | (_) | |_| | |_| |_) | |_| | |_
//  \___/ \__,_|\__| .__/ \__,_|\__|
//                 |_|
namespace {
void output_geometry(void *data, struct wl_output * /*wl_output*/, int32_t x,
                     int32_t y, int32_t physical_width, int32_t physical_height,
                     int32_t subpixel, const char *make, const char *model,
                     int32_t transform) {
   static_cast<struct output *>(data)->geometry(
      x, y, physical_width, physical_height, subpixel, make, model, transform);
}

void output_mode(void *data, struct wl_output * /*wl_output*/, uint32_t flags,
                 int32_t width, int32_t height, int32_t refresh) {
   static_cast<struct output *>(data)->mode(flags, width, height, refresh);
}

void output_done(void *data, struct wl_output * /*wl_output*/) {
   static_cast<struct output *>(data)->done();
}

void output_scale(void *data, struct wl_output * /*wl_output*/,
                  int32_t factor) {
   static_cast<struct output *>(data)->scale(factor);
}

constexpr struct wl_output_listener output_listener = {
   output_geometry, output_mode, output_done, output_scale};
}  // namespace

output::output(struct wl_registry *r, uint32_t name, uint32_t v)
   : wayland_proxy(wl_registry_bind(r, name, &wl_output_interface, v)) {
   wl_output_add_listener(this->proxy.get(), &output_listener, this);
}

void output::geometry(int32_t x, int32_t y, int32_t pw, int32_t ph,
                      int32_t subpel, char const *make, char const *model,
                      int32_t tx) {
   HMI_DEBUG("wm",
      "wl::output %s @ %p x %i y %i w %i h %i spel %x make %s model %s tx %i",
      __func__, this->proxy.get(), x, y, pw, ph, subpel, make, model, tx);
   this->transform = tx;
}

void output::mode(uint32_t flags, int32_t w, int32_t h, int32_t r) {
   HMI_DEBUG("wm", "wl::output %s @ %p f %x w %i h %i r %i", __func__,
            this->proxy.get(), flags, w, h, r);
   if ((flags & WL_OUTPUT_MODE_CURRENT) != 0u) {
      this->width = w;
      this->height = h;
      this->refresh = r;
   }
}

void output::done() {
   HMI_DEBUG("wm", "wl::output %s @ %p done", __func__, this->proxy.get());
   // Let's just disregard the flipped ones...
   if (this->transform == WL_OUTPUT_TRANSFORM_90 ||
       this->transform == WL_OUTPUT_TRANSFORM_270) {
      std::swap(this->width, this->height);
   }
}

void output::scale(int32_t factor) {
   HMI_DEBUG("wm", "wl::output %s @ %p f %i", __func__, this->proxy.get(), factor);
}
}  // namespace wl

//  _ __   __ _ _ __ ___   ___  ___ _ __   __ _  ___ ___
// | '_ \ / _` | '_ ` _ \ / _ \/ __| '_ \ / _` |/ __/ _ \
// | | | | (_| | | | | | |  __/\__ \ |_) | (_| | (_|  __/
// |_| |_|\__,_|_| |_| |_|\___||___/ .__/ \__,_|\___\___|
//                                 |_|

// namespace compositor
namespace compositor {

//                  _             _ _
//   ___ ___  _ __ | |_ _ __ ___ | | | ___ _ __
//  / __/ _ \| '_ \| __| '__/ _ \| | |/ _ \ '__|
// | (_| (_) | | | | |_| | | (_) | | |  __/ |
//  \___\___/|_| |_|\__|_|  \___/|_|_|\___|_|
//
namespace {
void controller_screen(void *data, struct ivi_controller * /*ivi_controller*/,
                       uint32_t id_screen,
                       struct ivi_controller_screen *screen) {
   static_cast<struct controller *>(data)->controller_screen(id_screen, screen);
}

void controller_layer(void *data, struct ivi_controller * /*ivi_controller*/,
                      uint32_t id_layer) {
   static_cast<struct controller *>(data)->controller_layer(id_layer);
}

void controller_surface(void *data, struct ivi_controller * /*ivi_controller*/,
                        uint32_t id_surface) {
   static_cast<struct controller *>(data)->controller_surface(id_surface);
}

void controller_error(void *data, struct ivi_controller * /*ivi_controller*/,
                      int32_t object_id, int32_t object_type,
                      int32_t error_code, const char *error_text) {
   static_cast<struct controller *>(data)->controller_error(
      object_id, object_type, error_code, error_text);
}

constexpr struct ivi_controller_listener listener = {
   controller_screen, controller_layer, controller_surface, controller_error};
}  // namespace compositor

controller::controller(struct wl_registry *r, uint32_t name, uint32_t version)
   : wayland_proxy(
        wl_registry_bind(r, name, &ivi_controller_interface, version)),
     output_size{} {
   ivi_controller_add_listener(this->proxy.get(), &listener, this);
}

void controller::layer_create(uint32_t id, int32_t w, int32_t h) {
   this->layers[id] = std::make_unique<struct layer>(id, w, h, this);
}

void controller::surface_create(uint32_t id) {
   this->surfaces[id] = std::make_unique<struct surface>(id, this);
}

void controller::controller_screen(uint32_t id,
                                   struct ivi_controller_screen *screen) {
   HMI_DEBUG("wm", "compositor::controller @ %p screen %u (%x) @ %p", this->proxy.get(),
            id, id, screen);
   this->screens[id] = std::make_unique<struct screen>(id, this, screen);
}

void controller::controller_layer(uint32_t id) {
   HMI_DEBUG("wm", "compositor::controller @ %p layer %u (%x)", this->proxy.get(), id, id);
   if (this->layers.find(id) != this->layers.end()) {
      HMI_ERROR("wm", "Someone created a layer without asking US! (%d)", id);
   } else {
      auto &l = this->layers[id] = std::make_unique<struct layer>(id, this);
      l->clear_surfaces();
   }
}

void controller::controller_surface(uint32_t id) {
   HMI_DEBUG("wm", "compositor::controller @ %p surface %u (%x)", this->proxy.get(), id,
            id);
   if (this->surfaces.find(id) == this->surfaces.end()) {
      this->surfaces[id] = std::make_unique<struct surface>(id, this);
      this->chooks->surface_created(id);
   }
}

void controller::controller_error(int32_t object_id, int32_t object_type,
                                  int32_t error_code, const char *error_text) {
   HMI_DEBUG("wm", "compositor::controller @ %p error o %i t %i c %i text %s",
            this->proxy.get(), object_id, object_type, error_code, error_text);
}

//  _
// | | __ _ _   _  ___ _ __
// | |/ _` | | | |/ _ \ '__|
// | | (_| | |_| |  __/ |
// |_|\__,_|\__, |\___|_|
//          |___/
namespace {
void layer_visibility(void *data,
                      struct ivi_controller_layer * /*ivi_controller_layer*/,
                      int32_t visibility) {
   auto l = static_cast<struct layer *>(data);
   l->parent->layer_visibility(l, visibility);
}

void layer_opacity(void *data,
                   struct ivi_controller_layer * /*ivi_controller_layer*/,
                   wl_fixed_t opacity) {
   auto l = static_cast<struct layer *>(data);
   l->parent->layer_opacity(l, float(wl_fixed_to_double(opacity)));
}

void layer_source_rectangle(
   void *data, struct ivi_controller_layer * /*ivi_controller_layer*/,
   int32_t x, int32_t y, int32_t width, int32_t height) {
   auto l = static_cast<struct layer *>(data);
   l->parent->layer_source_rectangle(l, x, y, width, height);
}

void layer_destination_rectangle(
   void *data, struct ivi_controller_layer * /*ivi_controller_layer*/,
   int32_t x, int32_t y, int32_t width, int32_t height) {
   auto l = static_cast<struct layer *>(data);
   l->parent->layer_destination_rectangle(l, x, y, width, height);
}

void layer_configuration(void *data,
                         struct ivi_controller_layer * /*ivi_controller_layer*/,
                         int32_t width, int32_t height) {
   auto l = static_cast<struct layer *>(data);
   l->parent->layer_configuration(l, width, height);
}

void layer_orientation(void *data,
                       struct ivi_controller_layer * /*ivi_controller_layer*/,
                       int32_t orientation) {
   auto l = static_cast<struct layer *>(data);
   l->parent->layer_orientation(l, orientation);
}

void layer_screen(void *data,
                  struct ivi_controller_layer * /*ivi_controller_layer*/,
                  struct wl_output *screen) {
   auto l = static_cast<struct layer *>(data);
   l->parent->layer_screen(l, screen);
}

void layer_destroyed(void *data,
                     struct ivi_controller_layer * /*ivi_controller_layer*/) {
   auto l = static_cast<struct layer *>(data);
   l->parent->layer_destroyed(l);
}

constexpr struct ivi_controller_layer_listener layer_listener = {
   layer_visibility,       layer_opacity,
   layer_source_rectangle, layer_destination_rectangle,
   layer_configuration,    layer_orientation,
   layer_screen,           layer_destroyed,
};
}  // namespace

layer::layer(uint32_t i, struct controller *c) : layer(i, 0, 0, c) {}

layer::layer(uint32_t i, int32_t w, int32_t h, struct controller *c)
   : wayland_proxy(ivi_controller_layer_create(c->proxy.get(), i, w, h),
                   [c, i](ivi_controller_layer *l) {
                      HMI_DEBUG("wm", "~layer layer %i @ %p", i, l);
                      c->remove_proxy_to_id_mapping(l);
                      ivi_controller_layer_destroy(l, 1);
                   }),
     controller_child(c, i) {
   this->parent->add_proxy_to_id_mapping(this->proxy.get(), i);
   ivi_controller_layer_add_listener(this->proxy.get(), &layer_listener, this);
}

void layer::set_visibility(uint32_t visibility) {
   ivi_controller_layer_set_visibility(this->proxy.get(), visibility);
}

void layer::set_opacity(wl_fixed_t opacity) {
   ivi_controller_layer_set_opacity(this->proxy.get(), opacity);
}

void layer::set_source_rectangle(int32_t x, int32_t y, int32_t width,
                                 int32_t height) {
   ivi_controller_layer_set_source_rectangle(this->proxy.get(), x, y, width,
                                             height);
}

void layer::set_destination_rectangle(int32_t x, int32_t y, int32_t width,
                                      int32_t height) {
   ivi_controller_layer_set_destination_rectangle(this->proxy.get(), x, y,
                                                  width, height);
}

void layer::set_configuration(int32_t width, int32_t height) {
   ivi_controller_layer_set_configuration(this->proxy.get(), width, height);
}

void layer::set_orientation(int32_t orientation) {
   ivi_controller_layer_set_orientation(this->proxy.get(), orientation);
}

void layer::screenshot(const char *filename) {
   ivi_controller_layer_screenshot(this->proxy.get(), filename);
}

void layer::clear_surfaces() {
   ivi_controller_layer_clear_surfaces(this->proxy.get());
}

void layer::add_surface(struct surface *surface) {
   ivi_controller_layer_add_surface(this->proxy.get(), surface->proxy.get());
}

void layer::remove_surface(struct surface *surface) {
   ivi_controller_layer_remove_surface(this->proxy.get(), surface->proxy.get());
}

void layer::set_render_order(std::vector<uint32_t> const &ro) {
   struct wl_array wlro {
      .size = ro.size() * sizeof(ro[0]), .alloc = ro.capacity() * sizeof(ro[0]),
      .data = const_cast<void *>(static_cast<void const *>(ro.data()))
   };
   ivi_controller_layer_set_render_order(this->proxy.get(), &wlro);
}

void controller::layer_visibility(struct layer *l, int32_t visibility) {
   HMI_DEBUG("wm", "compositor::layer %s @ %d v %i", __func__, l->id, visibility);
   this->lprops[l->id].visibility = visibility;
}

void controller::layer_opacity(struct layer *l, float opacity) {
   HMI_DEBUG("wm", "compositor::layer %s @ %d o %f", __func__, l->id, opacity);
   this->lprops[l->id].opacity = opacity;
}

void controller::layer_source_rectangle(struct layer *l, int32_t x, int32_t y,
                                        int32_t width, int32_t height) {
   HMI_DEBUG("wm", "compositor::layer %s @ %d x %i y %i w %i h %i", __func__,
            l->id, x, y, width, height);
   this->lprops[l->id].src_rect = rect{width, height, x, y};
}

void controller::layer_destination_rectangle(struct layer *l, int32_t x,
                                             int32_t y, int32_t width,
                                             int32_t height) {
   HMI_DEBUG("wm", "compositor::layer %s @ %d x %i y %i w %i h %i", __func__,
            l->id, x, y, width, height);
   this->lprops[l->id].dst_rect = rect{width, height, x, y};
}

void controller::layer_configuration(struct layer *l, int32_t width,
                                     int32_t height) {
   HMI_DEBUG("wm", "compositor::layer %s @ %d w %i h %i", __func__, l->id,
            width, height);
   this->lprops[l->id].size = size{uint32_t(width), uint32_t(height)};
}

void controller::layer_orientation(struct layer *l, int32_t orientation) {
   HMI_DEBUG("wm", "compositor::layer %s @ %d o %i", __func__, l->id,
            orientation);
   this->lprops[l->id].orientation = orientation;
}

void controller::layer_screen(struct layer *l, struct wl_output *screen) {
   HMI_DEBUG("wm", "compositor::layer %s @ %d s %p", __func__, l->id, screen);
}

void controller::layer_destroyed(struct layer *l) {
   HMI_DEBUG("wm", "compositor::layer %s @ %d", __func__, l->id);
   this->lprops.erase(l->id);
   this->layers.erase(l->id);
}

//                  __
//  ___ _   _ _ __ / _| __ _  ___ ___
// / __| | | | '__| |_ / _` |/ __/ _ \
// \__ \ |_| | |  |  _| (_| | (_|  __/
// |___/\__,_|_|  |_|  \__,_|\___\___|
//
namespace {

void surface_visibility(
   void *data, struct ivi_controller_surface * /*ivi_controller_surface*/,
   int32_t visibility) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_visibility(s, visibility);
}

void surface_opacity(void *data,
                     struct ivi_controller_surface * /*ivi_controller_surface*/,
                     wl_fixed_t opacity) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_opacity(s, float(wl_fixed_to_double(opacity)));
}

void surface_source_rectangle(
   void *data, struct ivi_controller_surface * /*ivi_controller_surface*/,
   int32_t x, int32_t y, int32_t width, int32_t height) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_source_rectangle(s, x, y, width, height);
}

void surface_destination_rectangle(
   void *data, struct ivi_controller_surface * /*ivi_controller_surface*/,
   int32_t x, int32_t y, int32_t width, int32_t height) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_destination_rectangle(s, x, y, width, height);
}

void surface_configuration(
   void *data, struct ivi_controller_surface * /*ivi_controller_surface*/,
   int32_t width, int32_t height) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_configuration(s, width, height);
}

void surface_orientation(
   void *data, struct ivi_controller_surface * /*ivi_controller_surface*/,
   int32_t orientation) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_orientation(s, orientation);
}

void surface_pixelformat(
   void *data, struct ivi_controller_surface * /*ivi_controller_surface*/,
   int32_t pixelformat) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_pixelformat(s, pixelformat);
}

void surface_layer(void *data,
                   struct ivi_controller_surface * /*ivi_controller_surface*/,
                   struct ivi_controller_layer *layer) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_layer(s, layer);
}

void surface_stats(void *data,
                   struct ivi_controller_surface * /*ivi_controller_surface*/,
                   uint32_t redraw_count, uint32_t frame_count,
                   uint32_t update_count, uint32_t pid,
                   const char *process_name) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_stats(s, redraw_count, frame_count, update_count, pid,
                            process_name);
}

void surface_destroyed(
   void *data, struct ivi_controller_surface * /*ivi_controller_surface*/) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_destroyed(s);
}

void surface_content(void *data,
                     struct ivi_controller_surface * /*ivi_controller_surface*/,
                     int32_t content_state) {
   auto s = static_cast<struct surface *>(data);
   s->parent->surface_content(s, content_state);
}

constexpr struct ivi_controller_surface_listener surface_listener = {
   surface_visibility,
   surface_opacity,
   surface_source_rectangle,
   surface_destination_rectangle,
   surface_configuration,
   surface_orientation,
   surface_pixelformat,
   surface_layer,
   surface_stats,
   surface_destroyed,
   surface_content,
};
}  // namespace

surface::surface(uint32_t i, struct controller *c)
   : wayland_proxy(ivi_controller_surface_create(c->proxy.get(), i),
                   [c, i](ivi_controller_surface *s) {
                      HMI_DEBUG("wm", "~surface surface %i @ %p", i, s);
                      c->remove_proxy_to_id_mapping(s);
                      ivi_controller_surface_destroy(s, 1);
                   }),
     controller_child(c, i) {
   this->parent->add_proxy_to_id_mapping(this->proxy.get(), i);
   ivi_controller_surface_add_listener(this->proxy.get(), &surface_listener,
                                       this);
}

void surface::set_visibility(uint32_t visibility) {
   ivi_controller_surface_set_visibility(this->proxy.get(), visibility);
}

void surface::set_opacity(wl_fixed_t opacity) {
   ivi_controller_surface_set_opacity(this->proxy.get(), opacity);
}

void surface::set_source_rectangle(int32_t x, int32_t y, int32_t width,
                                   int32_t height) {
   ivi_controller_surface_set_source_rectangle(this->proxy.get(), x, y, width,
                                               height);
}

void surface::set_destination_rectangle(int32_t x, int32_t y, int32_t width,
                                        int32_t height) {
   ivi_controller_surface_set_destination_rectangle(this->proxy.get(), x, y,
                                                    width, height);
}

void surface::set_configuration(int32_t width, int32_t height) {
   ivi_controller_surface_set_configuration(this->proxy.get(), width, height);
}

void surface::set_orientation(int32_t orientation) {
   ivi_controller_surface_set_orientation(this->proxy.get(), orientation);
}

void surface::screenshot(const char *filename) {
   ivi_controller_surface_screenshot(this->proxy.get(), filename);
}

void surface::send_stats() {
   ivi_controller_surface_send_stats(this->proxy.get());
}

void surface::destroy(int32_t destroy_scene_object) {
   ivi_controller_surface_destroy(this->proxy.get(), destroy_scene_object);
}

void controller::surface_visibility(struct surface *s, int32_t visibility) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d v %i", __func__, s->id,
            visibility);
   this->sprops[s->id].visibility = visibility;
   this->chooks->surface_visibility(s->id, visibility);
}

void controller::surface_opacity(struct surface *s, float opacity) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d o %f", __func__, s->id,
            opacity);
   this->sprops[s->id].opacity = opacity;
}

void controller::surface_source_rectangle(struct surface *s, int32_t x,
                                          int32_t y, int32_t width,
                                          int32_t height) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d x %i y %i w %i h %i", __func__,
            s->id, x, y, width, height);
   this->sprops[s->id].src_rect = rect{width, height, x, y};
}

void controller::surface_destination_rectangle(struct surface *s, int32_t x,
                                               int32_t y, int32_t width,
                                               int32_t height) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d x %i y %i w %i h %i", __func__,
            s->id, x, y, width, height);
   this->sprops[s->id].dst_rect = rect{width, height, x, y};
   this->chooks->surface_destination_rectangle(s->id, x, y, width, height);
}

void controller::surface_configuration(struct surface *s, int32_t width,
                                       int32_t height) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d w %i h %i", __func__, s->id,
            width, height);
   this->sprops[s->id].size = size{uint32_t(width), uint32_t(height)};
   is_configured = true;
}

void controller::surface_orientation(struct surface *s, int32_t orientation) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d o %i", __func__, s->id,
            orientation);
   this->sprops[s->id].orientation = orientation;
}

void controller::surface_pixelformat(struct surface * s,
                                     int32_t pixelformat) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d f %i", __func__, s->id,
            pixelformat);
}

void controller::surface_layer(struct surface * s,
                               struct ivi_controller_layer *layer) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d l %u @ %p", __func__, s->id,
            this->layer_proxy_to_id[uintptr_t(layer)], layer);
}

void controller::surface_stats(struct surface *s, uint32_t redraw_count,
                               uint32_t frame_count, uint32_t update_count,
                               uint32_t pid, const char *process_name) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d r %u f %u u %u pid %u p %s", __func__,
            s->id, redraw_count, frame_count, update_count, pid,
            process_name);
}

void controller::surface_destroyed(struct surface *s) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d", __func__, s->id);
   this->chooks->surface_removed(s->id);
   // XXX: do I need to actually remove the surface late, i.e. using add_task()?
   this->sprops.erase(s->id);
   this->surfaces.erase(s->id);
}

void controller::surface_content(struct surface *s, int32_t content_state) {
   HMI_DEBUG("wm", "compositor::surface %s @ %d s %i", __func__, s->id,
            content_state);
   if (content_state == IVI_CONTROLLER_SURFACE_CONTENT_STATE_CONTENT_REMOVED) {
      // XXX is this the right thing to do?
      this->chooks->surface_removed(s->id);
      this->sprops.erase(s->id);
      this->surfaces.erase(s->id);
   }
}

void controller::add_proxy_to_id_mapping(struct ivi_controller_surface *p,
                                         uint32_t id) {
   HMI_DEBUG("wm", "Add surface proxy mapping for %p (%u)", p, id);
   this->surface_proxy_to_id[uintptr_t(p)] = id;
   this->sprops[id].id = id;
}

void controller::remove_proxy_to_id_mapping(struct ivi_controller_surface *p) {
   HMI_DEBUG("wm", "Remove surface proxy mapping for %p", p);
   this->surface_proxy_to_id.erase(uintptr_t(p));
}

void controller::add_proxy_to_id_mapping(struct ivi_controller_layer *p,
                                         uint32_t id) {
   HMI_DEBUG("wm", "Add layer proxy mapping for %p (%u)", p, id);
   this->layer_proxy_to_id[uintptr_t(p)] = id;
   this->lprops[id].id = id;
}

void controller::remove_proxy_to_id_mapping(struct ivi_controller_layer *p) {
   HMI_DEBUG("wm", "Remove layer proxy mapping for %p", p);
   this->layer_proxy_to_id.erase(uintptr_t(p));
}

void controller::add_proxy_to_id_mapping(struct wl_output *p, uint32_t id) {
   HMI_DEBUG("wm", "Add screen proxy mapping for %p (%u)", p, id);
   this->screen_proxy_to_id[uintptr_t(p)] = id;
}

void controller::remove_proxy_to_id_mapping(struct wl_output *p) {
   HMI_DEBUG("wm", "Remove screen proxy mapping for %p", p);
   this->screen_proxy_to_id.erase(uintptr_t(p));
}

//
//  ___  ___ _ __ ___  ___ _ __
// / __|/ __| '__/ _ \/ _ \ '_ \
// \__ \ (__| | |  __/  __/ | | |
// |___/\___|_|  \___|\___|_| |_|
//
screen::screen(uint32_t i, struct controller *c,
               struct ivi_controller_screen *p)
   : wayland_proxy(p), controller_child(c, i) {
   HMI_DEBUG("wm", "compositor::screen @ %p id %u", p, i);
}

void screen::clear() { ivi_controller_screen_clear(this->proxy.get()); }

void screen::add_layer(layer *l) {
   ivi_controller_screen_add_layer(this->proxy.get(), l->proxy.get());
}

void screen::set_render_order(std::vector<uint32_t> const &ro) {
   struct wl_array wlro {
      .size = ro.size() * sizeof(ro[0]), .alloc = ro.capacity() * sizeof(ro[0]),
      .data = const_cast<void *>(static_cast<void const *>(ro.data()))
   };
   ivi_controller_screen_set_render_order(this->proxy.get(), &wlro);
}

}  // namespace compositor
