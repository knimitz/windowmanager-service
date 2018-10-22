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
#include "util.hpp"
#include <ilm/ilm_control.h>

#define INVALID_SURFACE_ID 0

using std::string;
using std::vector;

namespace wm
{

static const vector<string> kWMEvents = {
    // Private event for applications
    "syncDraw", "flushDraw", "visible", "invisible", "active", "inactive", "error"};
static const vector<string> kErrorDescription = {
    "unknown-error"};

static const char kKeyDrawingName[] = "drawing_name";
static const char kKeyrole[] = "role";
static const char kKeyError[] = "error";
static const char kKeyErrorDesc[] = "kErrorDescription";

WMClient::WMClient(const string &appid, unsigned layer, unsigned surface, const string &role)
    : id(appid), layer(layer),
      role2surface(0)
{
    role2surface[role] = surface;
    for (auto x : kWMEvents)
    {
#if GTEST_ENABLED
        string ev = x;
#else
        afb_event ev = afb_daemon_make_event(x.c_str());
#endif
        evname2list[x] = ev;
    }
}

WMClient::WMClient(const string &appid, const string &role)
    : id(appid),
      layer(0),
      role2surface(0),
      evname2list(0)
{
    role2surface[role] = INVALID_SURFACE_ID;
    for (auto x : kWMEvents)
    {
#if GTEST_ENABLED
        string ev = x;
#else
        afb_event ev = afb_daemon_make_event(x.c_str());
#endif
        evname2list[x] = ev;
    }
}

WMClient::WMClient(const string &appid, unsigned layer, const string &role)
    : id(appid),
      layer(layer),
      main_role(role),
      role2surface(0),
      evname2list(0)
{
    role2surface[role] = INVALID_SURFACE_ID;
    for (auto x : kWMEvents)
    {
#if GTEST_ENABLED
        string ev = x;
#else
        afb_event ev = afb_daemon_make_event(x.c_str());
#endif
        evname2list[x] = ev;
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


#if GTEST_ENABLED
bool WMClient::subscribe(afb_req req, const string &evname)
{
    if(evname != kKeyError){
        HMI_DEBUG("error is only enabeled for now");
        return false;
    }
    int ret = afb_req_subscribe(req, this->evname2list[evname]);
    if (ret)
    {
        HMI_DEBUG("Failed to subscribe %s", evname.c_str());
        return false;
    }
    return true;
}

void WMClient::emitError(WM_CLIENT_ERROR_EVENT ev)
{
    if (!afb_event_is_valid(this->evname2list[kKeyError])){
        HMI_ERROR("event err is not valid");
        return;
    }
    json_object *j = json_object_new_object();
    json_object_object_add(j, kKeyError, json_object_new_int(ev));
    json_object_object_add(j, kKeyErrorDesc, json_object_new_string(kErrorDescription[ev].c_str()));
    HMI_DEBUG("error: %d, description:%s", ev, kErrorDescription[ev].c_str());

    int ret = afb_event_push(this->evname2list[kKeyError], j);
    if (ret != 0)
    {
        HMI_DEBUG("afb_event_push failed: %m");
    }
}
#endif

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