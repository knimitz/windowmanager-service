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

#include "wayland.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

// pretend we are a WM ...
namespace wm {

struct App {
   controller_hooks chooks;
   std::unique_ptr<wl::display> display;
   std::unique_ptr<genivi::controller> controller;
   std::vector<std::unique_ptr<wl::output>> outputs;

   App();
   void commit();
   void surface_visibility(uint32_t surface_id, uint32_t v);
   void surface_destination_rectangle(uint32_t surface_id, uint32_t x,
                                      uint32_t y, uint32_t w, uint32_t h);
   void try_fix(uint32_t surface_id);
};

void controller_hooks::surface_created(uint32_t /*surface_id*/) {}

void controller_hooks::surface_removed(uint32_t /*surface_id*/) {}

void controller_hooks::surface_visibility(uint32_t surface_id, uint32_t v) {
   this->app->surface_visibility(surface_id, v);
}

void controller_hooks::surface_destination_rectangle(uint32_t surface_id,
                                                     uint32_t x, uint32_t y,
                                                     uint32_t w, uint32_t h) {
   this->app->surface_destination_rectangle(surface_id, x, y, w, h);
}

App::App() : chooks{this}, display{new wl::display}, controller{}, outputs{} {
   // The same init, the WM does, at least we can reuse the wayland stuff
   if (!this->display->ok()) {
      return;
   }

   this->display->add_global_handler(
      "wl_output", [this](wl_registry *r, uint32_t name, uint32_t v) {
         this->outputs.emplace_back(std::make_unique<wl::output>(r, name, v));
      });

   this->display->add_global_handler(
      "ivi_controller", [this](wl_registry *r, uint32_t name, uint32_t v) {
         this->controller =
            std::make_unique<struct genivi::controller>(r, name, v);

         // Init controller hooks
         this->controller->chooks = &this->chooks;

         this->controller->add_proxy_to_id_mapping(
            this->outputs.back()->proxy.get(),
            wl_proxy_get_id(reinterpret_cast<struct wl_proxy *>(
               this->outputs.back()->proxy.get())));
      });

   for (int i : {1, 2, 3}) {
      this->display->roundtrip();
}
}

void App::commit() {
   this->controller->commit_changes();
   this->display->roundtrip();  // read: flush()++
}

void App::surface_visibility(uint32_t surface_id, uint32_t v) {
   fprintf(stderr, "surface %u visibility %u\n", surface_id, v);

   if (v == 1) {
      this->try_fix(surface_id);
   }
}

void App::surface_destination_rectangle(uint32_t surface_id, uint32_t x,
                                        uint32_t y, uint32_t w, uint32_t h) {
   fprintf(stderr, "surface %u dst %u %u %u %u\n", surface_id, x, y, w, h);

   if (w != 1 && h != 1 && this->controller->sprops[surface_id].visibility != 0) {
      this->try_fix(surface_id);
   }
}

void App::try_fix(uint32_t surface_id) {
   this->controller->surfaces[surface_id]->set_opacity(255);
   this->commit();
   std::this_thread::sleep_for(200ms);
   this->controller->surfaces[surface_id]->set_opacity(256);
   this->commit();
}

}  // namespace wm

int main(int /*argc*/, char ** /*argv*/) {
   wm::App app;
   if (!app.display->ok()) {
      fputs("Could not init wayland display\n", stderr);
      return 1;
   }
   while (app.display->dispatch() != -1) {
      ;
   }
   return 0;
}
