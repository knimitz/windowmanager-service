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

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <ilm/ilm_control.h>
#include "wm_error.hpp"
#include "util.hpp"

namespace wm {

class Screen {
  public:
    Screen(unsigned w, unsigned h) : _width(w), _height(h){}
    unsigned width() { return _width; }
    unsigned height() { return _height; }
  private:
    unsigned _width;
    unsigned _height;
    unsigned _pysical_width = 0;
    unsigned _pysical_height = 0;
};

class LayerControlCallbacks {
  public:
    LayerControlCallbacks() {};
    ~LayerControlCallbacks() = default;
    LayerControlCallbacks(const LayerControlCallbacks &obj) = default;

    // callback functions
    std::function<void(unsigned, unsigned)> surfaceCreated;
    std::function<void(unsigned)> surfaceDestroyed;
    /*
    std::function<void(unsigned)> layerCreated;
    std::function<void(unsigned)> layerDestroyed;
    */
};

class WMLayer;
class LayerState;
class WMAction;
class WMClient;

class LayerControl
{
  public:
    explicit LayerControl(const std::string& root);
    ~LayerControl() = default;
    WMError init(const LayerControlCallbacks& cb);
    void createLayers();
    unsigned getLayerID(const std::string& role);
    std::shared_ptr<WMLayer> getWMLayer(unsigned layer);
    std::shared_ptr<WMLayer> getWMLayer(std::string layer_name);
    struct rect getAreaSize(const std::string& area);
    void setupArea(const rectangle& base_rct, double scaling);
    Screen getScreenInfo();
    double scale();
    WMError renderLayers();
    WMError setXDGSurfaceOriginSize(unsigned surface);
    WMError layoutChange(const WMAction& action);
    WMError visibilityChange(const WMAction &action);
    void addSurface(unsigned surface, unsigned layer);

    // Don't use this function.
    void dispatchCreateEvent(ilmObjectType object, unsigned id, bool created);
    void dispatchSurfacePropChangeEvent(unsigned id, struct ilmSurfaceProperties*, t_ilm_notification_mask);
    void dispatchLayerPropChangeEvent(unsigned id, struct ilmLayerProperties*, t_ilm_notification_mask);

  private:
    WMError makeVisible(const std::shared_ptr<WMClient> client, const std::string& role);
    WMError makeInvisible(const std::shared_ptr<WMClient> client, const std::string& role);
    bool moveForeGround(const std::shared_ptr<WMClient> client, const std::string& role);
    bool moveBackGround(const std::shared_ptr<WMClient> client, const std::string& role);
    WMError loadLayerSetting(const std::string& path);
    WMError loadAreaDb(const std::string& path);

    std::vector<unsigned> surface_bg; // For CES demo
    std::vector<std::shared_ptr<WMLayer>> wm_layers;
    std::unordered_map<unsigned, unsigned> lid2wmlid;
    std::unordered_map<std::string, struct rect> area2size;
    unsigned screenID;
    struct ilmScreenProperties screen_prop;
    double scaling;
    int offset_x;
    int offset_y;
    LayerControlCallbacks cb;
};

} // namespace wm