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

#include <json-c/json.h>
#include "wm_client.hpp"
#include <ilm/ilm_control.h>

#define INVALID_SURFACE_ID 0

using std::string;
using std::vector;

namespace wm
{

static const char kActive[] = "active";
static const char kInactive[] = "inactive";
static const char kVisible[] = "visible";
static const char kInvisible[] = "invisible";
static const char kSyncDraw[] = "syncDraw";
static const char kFlushDraw[] = "flushDraw";
static const char kKeyDrawingName[] = "drawing_name";
static const char kKeyDrawingArea[] = "drawing_area";
static const char kKeyRole[] = "role";
static const char kKeyArea[] = "area";
static const char kKeyrole[] = "role";
static const char kKeyError[] = "error";
static const char kKeyErrorDesc[] = "errorDescription";
static const char kKeyX[] = "x";
static const char kKeyY[] = "y";
static const char kKeyWidth[] = "width";
static const char kKeyHeight[] = "height";
static const char kKeyDrawingRect[] = "drawing-rect";

static const vector<string> kWMEvents = {
    // Private event for applications
    kActive, kInactive,
    kVisible, kInvisible,
    kSyncDraw, kFlushDraw,
    kKeyError};

WMClient::WMClient(const string &appid, unsigned layer, unsigned surface, const string &role)
    : id(appid), layer(layer), is_source_set(false),
      role2surface(0)
{
    role2surface[role] = surface;
    for (auto x : kWMEvents)
    {
#if GTEST_ENABLED
        string ev = x;
#else
        afb_event_t ev = afb_api_make_event(afbBindingV3root, x.c_str());
#endif
        evname2afb_event[x] = ev;
    }
}

WMClient::WMClient(const string &appid, const string &role)
    : id(appid),
      layer(0),
      is_source_set(false),
      role2surface(0),
      evname2afb_event(0)
{
    role2surface[role] = INVALID_SURFACE_ID;
    for (auto x : kWMEvents)
    {
#if GTEST_ENABLED
        string ev = x;
#else
        afb_event_t ev = afb_api_make_event(afbBindingV3root, x.c_str());
#endif
        evname2afb_event[x] = ev;
    }
}

WMClient::WMClient(const string &appid, unsigned layer, const string &role)
    : id(appid),
      layer(layer),
      main_role(role),
      role2surface(0),
      evname2afb_event(0)
{
    role2surface[role] = INVALID_SURFACE_ID;
    for (auto x : kWMEvents)
    {
#if GTEST_ENABLED
        string ev = x;
#else
        afb_event_t ev = afb_api_make_event(afbBindingV3root, x.c_str());
#endif
        evname2afb_event[x] = ev;
    }
}

string WMClient::appID() const
{
    return this->id;
}

string WMClient::role() const
{
    return this->main_role;
}

unsigned WMClient::layerID() const
{
    return this->layer;
}

unsigned WMClient::surfaceID() const
{
    return this->surface;
}

void WMClient::registerSurface(unsigned surface)
{
    this->surface = surface;
}

/**
 * Add surface to the client
 *
 * This function add main surface to the client(ivi_layer).
 *
 * @param     string[in] role
 * @return    WMError
 */
WMError WMClient::addSurface(unsigned surface)
{
    this->surface = surface;
    ilmErrorTypes err = ilm_layerAddSurface(this->layer, surface);

    if(err == ILM_SUCCESS)
    {
        err = ilm_commitChanges();
    }
    return (err == ILM_SUCCESS) ? WMError::SUCCESS : WMError::FAIL;
}

void WMClient::setSurfaceSizeCorrectly()
{
    this->is_source_set = true;
}

bool WMClient::isSourceSizeSet()
{
    return this->is_source_set;
}

bool WMClient::removeSurfaceIfExist(unsigned surface)
{
    bool ret = false;
    if(surface == this->surface)
    {
        this->surface = INVALID_SURFACE_ID;
        ret = true;
    }
    return ret;
}

bool WMClient::subscribe(afb_req_t req, const string &evname)
{
    int ret = afb_req_subscribe(req, this->evname2afb_event[evname]);
    if (ret)
    {
        HMI_DEBUG("Failed to subscribe %s", evname.c_str());
        return false;
    }
    return true;
}

void WMClient::emitVisible(bool visible)
{
    // error check
    bool allow_send = false;
    if(visible)
    {
        allow_send = afb_event_is_valid(this->evname2afb_event[kVisible]);
    }
    else
    {
        allow_send = afb_event_is_valid(this->evname2afb_event[kInvisible]);
    }
    if(allow_send)
    {
        json_object* j = json_object_new_object();
        json_object_object_add(j, kKeyRole, json_object_new_string(this->role().c_str()));
        json_object_object_add(j, kKeyDrawingName, json_object_new_string(this->role().c_str()));

        if(visible)
        {
            afb_event_push(this->evname2afb_event[kVisible], j);
        }
        else
        {
            afb_event_push(this->evname2afb_event[kInvisible], j);
        }
    }
    else
    {
        HMI_ERROR("Failed to send %s", __func__);
    }
}

void WMClient::emitActive(bool active)
{
    // error check
    bool allow_send = false;
    if(active)
    {
        allow_send = afb_event_is_valid(this->evname2afb_event[kActive]);
    }
    else
    {
        allow_send = afb_event_is_valid(this->evname2afb_event[kInactive]);
    }
    if(allow_send)
    {
        json_object* j = json_object_new_object();
        json_object_object_add(j, kKeyRole, json_object_new_string(this->role().c_str()));
        json_object_object_add(j, kKeyDrawingName, json_object_new_string(this->role().c_str()));

        if(active)
        {
            afb_event_push(this->evname2afb_event[kActive], j);
        }
        else
        {
            afb_event_push(this->evname2afb_event[kInactive], j);
        }
    }
    else
    {
        HMI_ERROR("Failed to send %s", __func__);
    }
}

void WMClient::emitSyncDraw(const string& area, struct rect& r)
{
    HMI_NOTICE("trace");
    if(afb_event_is_valid(this->evname2afb_event[kSyncDraw]))
    {
        json_object *j_rect = json_object_new_object();
        json_object_object_add(j_rect, kKeyX, json_object_new_int(r.x));
        json_object_object_add(j_rect, kKeyY, json_object_new_int(r.y));
        json_object_object_add(j_rect, kKeyWidth, json_object_new_int(r.w));
        json_object_object_add(j_rect, kKeyHeight, json_object_new_int(r.h));

        json_object* j = json_object_new_object();
        json_object_object_add(j, kKeyRole, json_object_new_string(this->role().c_str()));
        json_object_object_add(j, kKeyDrawingName, json_object_new_string(this->role().c_str()));
        json_object_object_add(j, kKeyDrawingArea, json_object_new_string(area.c_str()));
        json_object_object_add(j, kKeyArea, json_object_new_string(this->role().c_str()));

        json_object_object_add(j, kKeyDrawingRect, j_rect);
        afb_event_push(this->evname2afb_event[kSyncDraw], j);
    }
    else
    {
        HMI_ERROR("Failed to send %s", __func__);
    }
}

void WMClient::emitFlushDraw()
{
    if(afb_event_is_valid(this->evname2afb_event[kFlushDraw]))
    {
        json_object* j = json_object_new_object();
        json_object_object_add(j, kKeyRole, json_object_new_string(this->role().c_str()));
        json_object_object_add(j, kKeyDrawingName, json_object_new_string(this->role().c_str()));
        afb_event_push(this->evname2afb_event[kFlushDraw], nullptr);
    }
    else
    {
        HMI_ERROR("Failed to send %s", __func__);
    }
}

void WMClient::emitError(WMError error)
{
    if (!afb_event_is_valid(this->evname2afb_event[kKeyError])){
        HMI_ERROR("event err is not valid");
        return;
    }
    json_object *j = json_object_new_object();
    json_object_object_add(j, kKeyError, json_object_new_int(error));
    json_object_object_add(j, kKeyErrorDesc, json_object_new_string(errorDescription(error)));
    HMI_DEBUG("error: %d, description:%s", error, errorDescription(error));
    int ret = afb_event_push(this->evname2afb_event[kKeyError], j);
    if (ret != 0)
    {
        HMI_DEBUG("afb_event_push failed: %m");
    }
}

void WMClient::dumpInfo()
{
    DUMP("APPID : %s", id.c_str());
    DUMP("  LAYER : %d", layer);
    for (const auto &x : this->role2surface)
    {
        DUMP("  ROLE  : %s , SURFACE : %d", x.first.c_str(), x.second);
    }
}

} // namespace wm