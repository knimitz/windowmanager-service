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
#include <assert.h>
#include <unistd.h>
#include "wm_layer_control.hpp"
#include "wm_layer.hpp"
#include "wm_client.hpp"
#include "request.hpp"
#include "json_helper.hpp"

#define LC_AREA_PATH "/etc/areas.db"
#define LC_LAYER_SETTING_PATH "/etc/layers.json"
#define LC_DEFAULT_AREA "fullscreen"
#define BACK_GROUND_LAYER "BackGroundLayer"

using std::string;
using std::vector;
using std::shared_ptr;

namespace wm {

LayerControl* g_lc_ctxt;

static void createCallback_static(ilmObjectType object,
                            t_ilm_uint id,
                            t_ilm_bool created,
                            void* data)
{
    static_cast<LayerControl*>(data)->dispatchCreateEvent(object, id, created);
}

static void surfaceCallback_static(t_ilm_surface surface,
            struct ilmSurfaceProperties* surface_prop,
            t_ilm_notification_mask mask)
{
    g_lc_ctxt->dispatchSurfacePropChangeEvent(surface, surface_prop, mask);
}

static void layerCallback_static(t_ilm_layer layer,
            struct ilmLayerProperties* layer_prop,
            t_ilm_notification_mask mask)
{
    g_lc_ctxt->dispatchLayerPropChangeEvent(layer, layer_prop, mask);
}

LayerControl::LayerControl(const std::string& root)
{
    string area_path = root + LC_AREA_PATH;
    string layer_path= root + LC_LAYER_SETTING_PATH;
    // load layers.setting.json
    WMError ret = this->loadLayerSetting(layer_path);
    assert(ret == WMError::SUCCESS);
    // load areas.json
    ret = this->loadAreaDb(area_path);
    assert(ret == WMError::SUCCESS);
}

WMError LayerControl::init(const LayerControlCallbacks& cb)
{
    HMI_DEBUG("Initialize of ilm library and display");
    t_ilm_uint num = 0;
    t_ilm_uint *ids;
    int cnt = 0;
    ilmErrorTypes rc = ilm_init();

    while (rc != ILM_SUCCESS)
    {
        cnt++;
        if (20 <= cnt)
        {
            HMI_ERROR("Could not connect to compositor");
            goto lc_init_error;
        }
        HMI_ERROR("Wait to start weston ...");
        sleep(1);
        rc = ilm_init();
    }
    if(rc != ILM_SUCCESS) goto lc_init_error;

    // Get current screen setting
    rc = ilm_getScreenIDs(&num, &ids);

    if(rc != ILM_SUCCESS) goto lc_init_error;

    for(unsigned i = 0; i < num; i++)
    {
        HMI_INFO("get screen: %d", ids[i]);
    }
    // Currently, 0 is only available
    this->screenID = ids[0];

    rc = ilm_getPropertiesOfScreen(this->screenID, &this->screen_prop);

    if(rc != ILM_SUCCESS) goto lc_init_error;

    // Register Callback to Window Manager and from ILM
    this->cb = cb;
    ilm_registerNotification(createCallback_static, this);

    return WMError::SUCCESS;

lc_init_error:
    HMI_ERROR("Failed to initialize. Terminate WM");

    return WMError::FAIL;
}

unsigned LayerControl::getLayerID(const string& role)
{
    unsigned ret = 0;
    for(const auto& l: this->wm_layers)
    {
        if(l->hasRole(role))
        {
            ret = l->layerID();
        }
    }
    return ret;
}

void LayerControl::addSurface(unsigned surface, unsigned layer)
{
    ilm_layerAddSurface(layer, surface);
    ilm_commitChanges();
}

void LayerControl::createLayers()
{
    for(const auto &layer : this->wm_layers)
    {
        unsigned id = layer->layerID();
        HMI_INFO("create new ID :%d", id);
        struct rect rct = this->area2size[LC_DEFAULT_AREA];
        ilm_layerCreateWithDimension(&id, rct.w, rct.h);
        //ilm_layerSetSourceRectangle(id, rct.x, rct.y, rct.w, rct.h);
        ilm_layerSetDestinationRectangle(id, this->offset_x, this->offset_y, rct.w, rct.h);
        ilm_layerSetOpacity(id, 1.0);
        ilm_layerSetVisibility(id, ILM_TRUE);
        ilm_commitChanges();
        /* auto wm_layer = getWMLayer(id);
        wm_layer->addLayerToState(id); */
    }
    this->renderLayers();
}

shared_ptr<WMLayer> LayerControl::getWMLayer(unsigned layer)
{
    unsigned uuid = this->lid2wmlid[layer];
    return this->wm_layers[uuid];
}

std::shared_ptr<WMLayer> LayerControl::getWMLayer(std::string layer_name)
{
    for(auto &l : this->wm_layers)
    {
        if(l->layerName() == layer_name)
        {
            return l;
        }
    }
    return nullptr;
}

struct rect LayerControl::getAreaSize(const std::string& area)
{
    return area2size[area];
}

void LayerControl::setupArea(const rectangle& base_rct, double scaling)
{
    this->scaling = scaling;
    this->offset_x = base_rct.left();
    this->offset_y = base_rct.top();

    for (auto &i : this->area2size)
    {
        i.second.x = static_cast<int>(scaling * i.second.x + 0.5);
        i.second.y = static_cast<int>(scaling * i.second.y + 0.5);
        i.second.w = static_cast<int>(scaling * i.second.w + 0.5);
        i.second.h = static_cast<int>(scaling * i.second.h + 0.5);

        HMI_DEBUG("area:%s size(after) : x:%d y:%d w:%d h:%d",
            i.first.c_str(), i.second.x, i.second.y, i.second.w, i.second.h);
    }
}

Screen LayerControl::getScreenInfo()
{
    return Screen(this->screen_prop.screenWidth, this->screen_prop.screenHeight);
}

double LayerControl::scale()
{
    return this->scaling;
}

WMError LayerControl::renderLayers()
{
    HMI_INFO("Commit change");
    WMError rc = WMError::SUCCESS;

    // Create render order
    t_ilm_layer* id_array = new t_ilm_layer[this->wm_layers.size()];
    if(id_array == nullptr)
    {
        HMI_WARNING("short memory");
        return WMError::FAIL;
    }
    int count = 0;
    for(const auto& i : this->wm_layers)
    {
        id_array[count] = i->layerID();
        ++count;
    }

    // Display
    ilmErrorTypes ret = ilm_displaySetRenderOrder(this->screenID, id_array, this->wm_layers.size());
    if(ret != ILM_SUCCESS)
    {
        rc = WMError::FAIL;
    }
    ilm_commitChanges();
    delete id_array;
    return rc;
}

WMError LayerControl::setXDGSurfaceOriginSize(unsigned surface)
{
    WMError ret = WMError::NOT_REGISTERED;
    ilmSurfaceProperties prop;
    ilmErrorTypes rc = ilm_getPropertiesOfSurface(surface, &prop);
    if(rc == ILM_SUCCESS)
    {
        HMI_INFO("xdg surface info %d, %d", prop.origSourceWidth, prop.origSourceHeight);
        ilm_surfaceSetSourceRectangle(surface, 0, 0, prop.origSourceWidth, prop.origSourceHeight);
        ret = WMError::SUCCESS;
    }
    return ret;
}

WMError LayerControl::loadLayerSetting(const string &path)
{
    HMI_DEBUG("loading WMLayer(Application Containers) Setting from %s", path);

    json_object *json_obj, *json_cfg;
    int ret = jh::inputJsonFilie(path.c_str(), &json_obj);
    if (0 > ret)
    {
        HMI_ERROR("Could not open %s", path.c_str());
        return WMError::FAIL;
    }
    HMI_INFO("json_obj dump:%s", json_object_get_string(json_obj));

    if (!json_object_object_get_ex(json_obj, "mappings", &json_cfg))
    {
        HMI_ERROR("Parse Error!!");
        return WMError::FAIL;
    }

    int len = json_object_array_length(json_cfg);
    HMI_DEBUG("json_cfg len:%d", len);

    for (int i = 0; i < len; i++)
    {
        json_object *json_tmp = json_object_array_get_idx(json_cfg, i);
        HMI_DEBUG("> json_tmp dump:%s", json_object_get_string(json_tmp));

        this->wm_layers.emplace_back(std::make_shared<WMLayer>(json_tmp));
    }
    json_object_put(json_obj);

    return WMError::SUCCESS;
}

WMError LayerControl::loadAreaDb(const std::string& path)
{
    // Load area.db
    json_object *json_obj;
    int ret = jh::inputJsonFilie(path.c_str(), &json_obj);
    if (0 > ret)
    {
        HMI_ERROR("Could not open %s", path.c_str());
        return WMError::FAIL;
    }
    HMI_INFO("json_obj dump:%s", json_object_get_string(json_obj));

    // Perse areas
    json_object *json_cfg;
    if (!json_object_object_get_ex(json_obj, "areas", &json_cfg))
    {
        HMI_ERROR("Parse Error!!");
        return WMError::FAIL;
    }

    int len = json_object_array_length(json_cfg);
    HMI_DEBUG("json_cfg len:%d", len);

    const char *area;
    for (int i = 0; i < len; i++)
    {
        json_object *json_tmp = json_object_array_get_idx(json_cfg, i);
        HMI_DEBUG("> json_tmp dump:%s", json_object_get_string(json_tmp));

        area = jh::getStringFromJson(json_tmp, "name");
        if (nullptr == area)
        {
            HMI_ERROR("Parse Error!!");
            return WMError::FAIL;
        }
        HMI_DEBUG("> area:%s", area);

        json_object *json_rect;
        if (!json_object_object_get_ex(json_tmp, "rect", &json_rect))
        {
            HMI_ERROR("Parse Error!!");
            return WMError::FAIL;
        }
        HMI_DEBUG("> json_rect dump:%s", json_object_get_string(json_rect));

        struct rect area_size;
        area_size.x = jh::getIntFromJson(json_rect, "x");
        area_size.y = jh::getIntFromJson(json_rect, "y");
        area_size.w = jh::getIntFromJson(json_rect, "w");
        area_size.h = jh::getIntFromJson(json_rect, "h");

        this->area2size[area] = area_size;
    }

    // Check
    for (const auto& itr : this->area2size)
    {
        HMI_DEBUG("area:%s x:%d y:%d w:%d h:%d",
                  itr.first.c_str(), itr.second.x, itr.second.y,
                  itr.second.w, itr.second.h);
    }

    // Release json_object
    json_object_put(json_obj);

    return WMError::SUCCESS;
}

WMError LayerControl::layoutChange(const WMAction& action)
{
    if (action.visible == TaskVisible::INVISIBLE)
    {
        // Visibility is not change -> no redraw is required
        return WMError::SUCCESS;
    }
    if(action.client == nullptr)
    {
        HMI_SEQ_ERROR(action.req_num, "client may vanish");
        return WMError::NOT_REGISTERED;
    }
    unsigned surface = action.client->surfaceID(action.role);

    auto rect = this->getAreaSize(action.area);
    HMI_DEBUG("Set layout %d, %d, %d, %d",rect.x, rect.y, rect.w, rect.h);
    ilm_commitChanges();
    ilm_surfaceSetDestinationRectangle(surface, rect.x, rect.y, rect.w, rect.h);
    ilm_commitChanges();

    return WMError::SUCCESS;
}

WMError LayerControl::visibilityChange(const WMAction& action)
{
    WMError ret = WMError::FAIL;
    if(action.client == nullptr)
    {
        HMI_SEQ_ERROR(action.req_num, "client may vanish");
        return WMError::NOT_REGISTERED;
    }

    if (action.visible == TaskVisible::VISIBLE)
    {
        ret = this->makeVisible(action.client, action.role);
    }
    else if (action.visible == TaskVisible::INVISIBLE)
    {
        ret = this->makeInvisible(action.client, action.role);
    }
    ilm_commitChanges();
    return ret;
}

void LayerControl::dispatchCreateEvent(ilmObjectType object, unsigned id, bool created)
{
    if (ILM_SURFACE == object)
    {
        if (created)
        {
            ilmSurfaceProperties sp;
            ilmErrorTypes rc;
            rc = ilm_getPropertiesOfSurface(id, &sp);
            if(rc != ILM_SUCCESS)
            {
                HMI_ERROR("Failed to get surface %d property due to %d", id, ilm_getError());
                return;
            }
            this->cb.surfaceCreated(sp.creatorPid, id);
            ilm_surfaceAddNotification(id, surfaceCallback_static);
            ilm_surfaceSetType(id, ILM_SURFACETYPE_DESKTOP);
        }
        else
        {
            this->cb.surfaceDestroyed(id);
        }
    }
    if (ILM_LAYER == object)
    {
        if(created)
        {
            ilm_layerAddNotification(id, layerCallback_static);
        }
        else
        {
            // Ignore here. Nothing to do currently.
            // Process of application dead is handled by Window Manager
            // from binder notification
        }
    }
}

void LayerControl::dispatchSurfacePropChangeEvent(unsigned id,
        struct ilmSurfaceProperties* sprop,
        t_ilm_notification_mask mask)
{
    /*
      ILM_NOTIFICATION_CONTENT_AVAILABLE & ILM_NOTIFICATION_CONTENT_REMOVED
      are not handled here, handled in create/destroy event
     */
    if (ILM_NOTIFICATION_VISIBILITY & mask)
    {
        HMI_DEBUG("surface %d turns visibility %d", id, sprop->visibility);
    }
    if (ILM_NOTIFICATION_OPACITY & mask)
    {
        HMI_DEBUG("surface %d turns opacity %f", id, sprop->opacity);
    }
    if (ILM_NOTIFICATION_SOURCE_RECT & mask)
    {
        HMI_DEBUG("surface %d source rect changes", id);
    }
    if (ILM_NOTIFICATION_DEST_RECT & mask)
    {
        HMI_DEBUG("surface %d dest rect changes", id);
    }
    if (ILM_NOTIFICATION_CONFIGURED & mask)
    {
        HMI_DEBUG("surface %d size %d, %d, %d, %d", id,
            sprop->sourceX, sprop->sourceY, sprop->origSourceWidth, sprop->origSourceHeight);
        ilm_surfaceSetSourceRectangle(id, 0, 0, sprop->origSourceWidth, sprop->origSourceHeight);
    }
}

void LayerControl::dispatchLayerPropChangeEvent(unsigned id,
        struct ilmLayerProperties* lprop,
        t_ilm_notification_mask mask)
{
    if (ILM_NOTIFICATION_VISIBILITY & mask)
    {
        HMI_DEBUG("layer %d turns visibility %d", id, lprop->visibility);
    }
    if (ILM_NOTIFICATION_OPACITY & mask)
    {
        HMI_DEBUG("layer %d turns opacity %f", id, lprop->opacity);
    }
    if (ILM_NOTIFICATION_SOURCE_RECT & mask)
    {
        HMI_DEBUG("layer %d source rect changes", id);
    }
    if (ILM_NOTIFICATION_DEST_RECT & mask)
    {
        HMI_DEBUG("layer %d dest rect changes", id);
    }
}

WMError LayerControl::makeVisible(const shared_ptr<WMClient> client, const string& role)
{
    WMError ret = WMError::SUCCESS;
    // Don't check here the client is not nullptr
    unsigned surface = client->surfaceID(role);

    this->moveForeGround(client, role);

    ilm_surfaceSetVisibility(surface, ILM_TRUE);

    return ret;
}

WMError LayerControl::makeInvisible(const shared_ptr<WMClient> client, const string& role)
{
    WMError ret = WMError::SUCCESS;
    unsigned surface = client->surfaceID(role); // Don't check here the client is not nullptr

    bool mv_ok = this->moveBackGround(client, role);

    if(!mv_ok)
    {
        HMI_INFO("make invisible client %s", client->appID().c_str());
        ilm_surfaceSetVisibility(surface, ILM_FALSE);
    }

    return ret;
}

bool LayerControl::moveBackGround(const shared_ptr<WMClient> client, const string& role)
{
    bool ret = false;
    const char* label = role.c_str();

    if ((0 == strcmp(label, "radio")) ||
        (0 == strcmp(label, "music")) ||
        (0 == strcmp(label, "video")) ||
        (0 == strcmp(label, "map")))
    {
        unsigned surface = client->surfaceID(role);
        this->surface_bg.push_back(surface);
        auto bg = this->getWMLayer(BACK_GROUND_LAYER);
        auto layer = 0;
        for(const auto& l : this->wm_layers)
        {
            if(l->hasRole(role))
            {
                layer = l->layerID();
                ilm_layerRemoveSurface(layer, surface);
                ilm_layerAddSurface(bg->layerID(), surface);
                ret = true;
                break;
            }
        }
    }
    ilm_commitChanges();
    return ret;
}

bool LayerControl::moveForeGround(const shared_ptr<WMClient> client, const string& role)
{
    bool ret = false;
    const char* label = role.c_str();

    // Move foreground from foreground layer
    if ((0 == strcmp(label, "radio")) ||
        (0 == strcmp(label, "music")) ||
        (0 == strcmp(label, "video")) ||
        (0 == strcmp(label, "map")))
    {
        for (auto i = surface_bg.begin(); i != surface_bg.end(); ++i)
        {
            if (client->surfaceID(role) == *i)
            {
                // Remove id
                unsigned surface = *i;

                this->surface_bg.erase(i);

                // Remove from BG layer (999)
                HMI_DEBUG("wm", "Remove %s(%d) from BG layer", label, surface);
                auto bg = this->getWMLayer(BACK_GROUND_LAYER);
                unsigned layer = 0;
                for(const auto& l : this->wm_layers)
                {
                    if(l->hasRole(role))
                    {
                        layer = l->layerID();
                        break;
                    }
                }
                ilm_layerRemoveSurface(bg->layerID(), surface);
                ilm_layerAddSurface(layer, surface);
                ret = true;
                break;
            }
        }
    }
    ilm_commitChanges();
    return ret;
}

} // namespace wm