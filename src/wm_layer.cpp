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
using std::vector;
using std::unordered_map;

#define BG_LAYER_NAME "BackGroundLayer"

namespace wm
{

LayerState::LayerState()
    :  render_order(),
       area2appid()
{}

const vector<unsigned> LayerState::getIviIdList()
{
    return this->render_order;
}

void LayerState::addLayer(unsigned layer)
{
    auto result = std::find(this->render_order.begin(), this->render_order.end(), layer);
    if(result == this->render_order.end())
        this->render_order.push_back(layer);
}

void LayerState::removeLayer(unsigned layer)
{
    auto fwd_itr = std::remove_if(
        this->render_order.begin(), this->render_order.end(),
        [layer](unsigned elm) {
            if(elm == layer)
                HMI_DEBUG("remove layer %d", elm);
            return elm == layer;
        }
    );
    this->render_order.erase(fwd_itr, this->render_order.end());
}

void LayerState::attachAppToArea(const string& app, const string& area)
{
    this->area2appid[area] = app;
}

void LayerState::dump()
{
    std::string ids, apps;
    for(const auto& ro : this->render_order)
    {
        ids += std::to_string(ro);
        ids += ",";
    }
    for(const auto& area : this->area2appid)
    {
        apps += area.first;
        apps += ":";
        apps += area.second;
        apps += ",";
    }
    DUMP("    render order : %s", ids.c_str());
    DUMP("    area, app    : %s", apps.c_str());
}

WMLayer::WMLayer(json_object* j, unsigned wm_layer_id) : tmp_state(), state(), wm_layer_id(wm_layer_id)
{
    this->name = jh::getStringFromJson(j, "name");
    this->role_list = jh::getStringFromJson(j, "role");
    this->id_begin = static_cast<unsigned>(jh::getIntFromJson(j, "id_range_begin"));
    this->id_end = static_cast<unsigned>(jh::getIntFromJson(j, "id_range_end"));

    if (name.empty())
    {
        HMI_ERROR("Parse Error!!");
        exit(1);
    }
    if(this->id_begin > this->id_end)
    {
        HMI_ERROR("INVALID");
        exit(1);
    }
}

unsigned WMLayer::getNewLayerID(const string& role)
{
    unsigned ret = 0;
    if(this->name == BG_LAYER_NAME)
        return ret;

    // generate new layer id;
    if(this->hasRole(role))
    {
        if(this->id_list.size() == 0)
        {
            ret = this->idBegin();
            this->id_list.push_back(ret);
        }
        else
        {
            ret = this->id_list.back() + 1;
        }
        HMI_INFO("Generate new id: %d", ret);
    }
    else
    {
        return ret;
    }

    size_t count = std::count(id_list.begin(), id_list.end(), ret);
    if( (ret > this->idEnd()) || (count > 1))
    {
        HMI_NOTICE("id %d is not available then generate new id", ret);
        ret = 0; // reset
        for(unsigned i = this->idBegin(); i < this->idEnd(); i++)
        {
            auto ret_found = std::find(id_list.begin(), id_list.end(), i);
            if(ret_found == id_list.cend())
            {
                HMI_INFO("set new id: %d", i);
                ret = i;
                break;
            }
        }
    }

    if(ret != 0)
    {
        id_list.push_back(ret);
    }
    else
    {
        HMI_ERROR("failed to get New ID");
    }
    return ret;
}

const string& WMLayer::layerName()
{
    return this->name;
}

WMError WMLayer::setLayerState(const LayerState& l)
{
    this->tmp_state = l;
    return WMError::SUCCESS;
}

void WMLayer::addLayerToState(unsigned layer)
{
    this->tmp_state.addLayer(layer);
}

void WMLayer::removeLayerFromState(unsigned layer)
{
    this->tmp_state.removeLayer(layer);
}

void WMLayer::attachAppToArea(const string& app, const string& area)
{
    this->tmp_state.attachAppToArea(app, area);
}

void WMLayer::appendArea(const string& area)
{
    this->area_list.push_back(area);
}

void WMLayer::appTerminated(unsigned id)
{
    auto fwd_itr = std::remove_if(this->id_list.begin(), this->id_list.end(),
        [id](unsigned elm) {
            return elm == id;
        });
    this->id_list.erase(fwd_itr, this->id_list.end());
    this->tmp_state.removeLayer(id);
    this->state.removeLayer(id);
    ilm_layerRemove(id);
}

bool WMLayer::hasLayerID(unsigned id)
{
    bool ret = (id >= this->idBegin() && id <= this->idEnd());
    if(!ret)
        return ret;
    auto itr = std::find(this->id_list.begin(), this->id_list.end(), id);
    return (itr != this->id_list.end()) ? true : false;
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

void WMLayer::update()
{
    this->state = this->tmp_state;
}

void WMLayer::undo()
{
    this->tmp_state = this->state;
}

void WMLayer::dump()
{
    DUMP("===== wm layer status =====");
    DUMP("Layer :%s", this->name.c_str());
    this->tmp_state.dump();
    this->state.dump();
    DUMP("===== wm layer status end =====");

}

} // namespace wm
