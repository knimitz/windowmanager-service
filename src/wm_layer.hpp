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

class WMClient;
class LayerState
{
  public:
    LayerState();
    ~LayerState() = default;
    const std::unordered_map<std::string, std::string> getCurrentState();
    const std::vector<unsigned> getIviIdList();
    void addLayer(unsigned layer);
    void removeLayer(unsigned layer);
    void attachAppToArea(const std::string& app, const std::string& area);

    // Debug
    void dump();

  private:
    std::vector<unsigned> render_order;
    std::unordered_map<std::string, std::string> area2appid;
};

class WMLayer
{
  public:
    explicit WMLayer(json_object* j, unsigned wm_layer_id);
    ~WMLayer() = default;

    // Status & Setting API
    unsigned getNewLayerID(const std::string& role);
    unsigned idBegin() { return this->id_begin; }
    unsigned idEnd()   { return this->id_end; }
    unsigned getWMLayerID() { return this->wm_layer_id; }
    const std::string& layerName();
    void appendArea(const std::string& area);
    LayerState& getLayerState() { return tmp_state; }
    WMError setLayerState(const LayerState& l);
    bool hasLayerID(unsigned id);
    bool hasRole(const std::string& role);

    // Manipulation
    void addLayerToState(unsigned layer);
    void removeLayerFromState(unsigned layer);
    void attachAppToArea(const std::string& app, const std::string& area);
    std::string attachedApp(const std::string& area);
    void update();
    void undo();

    // Event
    void appTerminated(unsigned layer);

    // Debug
    void dump();

  private:
    LayerState tmp_state;
    LayerState state;
    unsigned wm_layer_id;
    std::string name = ""; // Layer name
    std::string role_list;
    std::vector<std::string> area_list;
    std::vector<unsigned>    id_list;
    unsigned id_begin;
    unsigned id_end;
};

} // namespace wm

#endif // WM_LAYER_HPP
