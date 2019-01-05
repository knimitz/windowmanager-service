/*
 * Copyright (c) 2018 TOYOTA MOTOR CORPORATION
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

#include <fstream>
#include <sstream>
#include <istream>
#include <thread>
#include <map>
#include <algorithm>
#include <json-c/json.h>
#include "policy_manager.hpp"
#include "util.hpp"

extern "C"
{
#define AFB_BINDING_VERSION 3
#include <afb/afb-binding.h>
#include <systemd/sd-event.h>
#include "stm.h"
}

namespace pm
{
static const int kInvisibleRoleHistoryNum = 5;

static PolicyManager *g_context;

static int transitionStateWrapper(sd_event_source *source, void *data)
{
    int ret = g_context->transitionState(source, data);
    return ret;
}

static int timerEventWrapper(sd_event_source *source, uint64_t usec, void *data)
{
    int ret = g_context->timerEvent(source, usec, data);
    return ret;
}

} // namespace pm

PolicyManager::PolicyManager()
    : eventname2no(),
      categoryname2no(),
      areaname2no(),
      role2category(),
      category2role(),
      category2areas()
{}

int PolicyManager::initialize()
{
    int ret = 0;

    // Create convert map
    for (int i = StmEvtNoMin; i <= StmEvtNoMax; i++)
    {
        HMI_DEBUG("event name:%s no:%d", kStmEventName[i], i);
        this->eventname2no[kStmEventName[i]] = i;
    }

    for (int i = StmCtgNoMin; i <= StmCtgNoMax; i++)
    {
        HMI_DEBUG("category name:%s no:%d", kStmCategoryName[i], i);
        this->categoryname2no[kStmCategoryName[i]] = i;
    }

    for (int i = StmAreaNoMin; i <= StmAreaNoMax; i++)
    {
        HMI_DEBUG("area name:%s no:%d", kStmAreaName[i], i);
        this->areaname2no[kStmAreaName[i]] = i;
    }

    // Load roles.db
    ret = this->loadRoleDb();
    if (0 > ret)
    {
        HMI_ERROR("Load roles.db Error!!");
        return ret;
    }

    // Load states.db
    ret = this->loadStateDb();
    if (0 > ret)
    {
        HMI_ERROR("Load states.db Error!!");
        return ret;
    }

    // Initialize state which is managed by PolicyManager
    this->initializeState();

    // Initialize StateTransitioner
    stmInitialize();

    // Store instance
    pm::g_context = this;

    return ret;
}

void PolicyManager::registerCallback(CallbackTable callback)
{
    this->callback.onStateTransitioned = callback.onStateTransitioned;
    this->callback.onError = callback.onError;
}

int PolicyManager::setInputEventData(json_object *json_in)
{
    // Check arguments
    if (nullptr == json_in)
    {
        HMI_ERROR("Argument is NULL!!");
        return -1;
    }

    // Get event from json_object
    const char *event = this->getStringFromJson(json_in, "event");
    int event_no = StmEvtNoNone;
    if (nullptr != event)
    {
        // Convert name to number
        auto itr = this->eventname2no.find(event);
        if (this->eventname2no.end() != itr)
        {
            event_no = this->eventname2no[event];
            HMI_DEBUG("event(%s:%d)", event, event_no);
        }
        else
        {
            HMI_ERROR("Invalid event name!!");
            return -1;
        }
    }
    else
    {
        HMI_ERROR("Event is not set!!");
        return -1;
    }

    // Get role from json_object
    const char *role = this->getStringFromJson(json_in, "role");
    std::string category = "";
    int category_no = StmCtgNoNone;
    if (nullptr != role)
    {
        HMI_DEBUG("role(%s)", role);

        // Convert role to category
        auto itr = this->role2category.find(role);
        if (this->role2category.end() != itr)
        {
            category = this->role2category[role];
        }
        else
        {
            itr = this->role2category.find("fallback");
            if (this->role2category.end() != itr)
            {
                HMI_DEBUG("Role:%s is not registered in roles.db, fallback as normal app", role);
                category = this->role2category["fallback"];
            }
        }

        if ("" != category)
        {
            // Convert name to number
            category_no = categoryname2no[category];
            HMI_DEBUG("category(%s:%d)", category.c_str(), category_no);
        }
    }
    if (StmCtgNoNone == category_no)
    {
        role = "";
    }

    // Get area from json_object
    const char *area = this->getStringFromJson(json_in, "area");
    int area_no = StmAreaNoNone;
    if ((nullptr != area) && (StmCtgNoNone != category_no))
    {
        for (const auto &x : this->category2areas[category])
        {
            if (x == std::string(area))
            {
                area_no = this->areaname2no[area];
                break;
            }
        }
        if (StmAreaNoNone == area_no)
        {
            area = this->category2areas[category].front().c_str();
            area_no = this->areaname2no[area];
        }
        HMI_DEBUG("area(%s:%d)", area, area_no);
    }

    // Set event info to the queue
    EventInfo event_info;
    int event_id = STM_CREATE_EVENT_ID(event_no, category_no, area_no);
    event_info.event = event_id;
    event_info.role = std::string(role);
    event_info.delay = 0;
    this->event_info_queue.push(event_info);

    return 0;
}

int PolicyManager::executeStateTransition()
{
    int ret;
    EventInfo event_info;

    // Get event info from queue and delete
    event_info = this->event_info_queue.front();
    this->event_info_queue.pop();

    // Set event info to check policy
    ret = this->setStateTransitionProcessToSystemd(event_info.event,
                                                   event_info.delay,
                                                   event_info.role);
    return ret;
}

void PolicyManager::undoState()
{
    HMI_DEBUG("Undo State !!!");

    // Undo state of STM
    stmUndoState();

    HMI_DEBUG(">>>>>>>>>> BEFORE UNDO");
    this->dumpLayerState(this->crr_layers);

    this->crr_layers = this->prv_layers;

    HMI_DEBUG(">>>>>>>>>> AFTER UNDO");
    this->dumpLayerState(this->crr_layers);
}

void PolicyManager::initializeState()
{
    this->initializeLayerState();
}

void PolicyManager::initializeLayerState()
{
    AreaState init_area;
    LayoutState init_layout;
    init_area.name = kStmAreaName[StmAreaNoNone];
    init_area.category = kStmCategoryName[StmCtgNoNone];
    init_area.role = "";
    init_layout.name = kStmLayoutName[StmLayoutNoNone];
    init_layout.area_list.push_back(init_area);

    for (int i = StmLayerNoMin; i <= StmLayerNoMax; i++)
    {
        const char *layer_name = kStmLayerName[i];
        this->crr_layers[layer_name].name = layer_name;
        this->crr_layers[layer_name].layout_state = init_layout;
        this->crr_layers[layer_name].changed = false;
    }

    this->prv_layers = this->crr_layers;
}

void PolicyManager::addStateToJson(const char *name, bool changed,
                                   std::string state, json_object **json_out)
{
    if ((nullptr == name) || (nullptr == json_out))
    {
        HMI_ERROR("Invalid argument!!!");
        return;
    }

    json_object_object_add(*json_out, "name", json_object_new_string(name));
    json_object_object_add(*json_out, "state", json_object_new_string(state.c_str()));
    json_object_object_add(*json_out, "changed", json_object_new_boolean(changed));
}

void PolicyManager::addStateToJson(const char *layer_name, bool changed,
                                   AreaList area_list, json_object **json_out)
{
    if ((nullptr == layer_name) || (nullptr == json_out))
    {
        HMI_ERROR("Invalid argument!!!");
        return;
    }

    json_object *json_areas = json_object_new_array();
    json_object *json_tmp;
    for (const auto &as : area_list)
    {
        json_tmp = json_object_new_object();
        json_object_object_add(json_tmp, "name", json_object_new_string(as.name.c_str()));
        json_object_object_add(json_tmp, "role", json_object_new_string(as.role.c_str()));
        json_object_array_add(json_areas, json_tmp);
    }

    json_object_object_add(*json_out, "name", json_object_new_string(layer_name));
    json_object_object_add(*json_out, "changed", json_object_new_boolean(changed));
    json_object_object_add(*json_out, "areas", json_areas);
}

void PolicyManager::updateState(int event_id, StmState crr_state)
{
    this->updateLayer(event_id, crr_state);
}

void PolicyManager::updateLayer(int event_id, StmState crr_state)
{
    for (int layer_no = StmLayerNoMin;
         layer_no <= StmLayerNoMax; layer_no++)
    {
        HMI_DEBUG(">>> LAYER:%s CHANGED:%d LAYOUT:%s",
                  kStmLayerName[layer_no], crr_state.layer[layer_no].changed,
                  kStmLayoutName[crr_state.layer[layer_no].state]);
    }

    // Store previous layers
    this->prv_layers = this->crr_layers;

    // Update layers
    for (int layer_no = StmLayerNoMin;
         layer_no <= StmLayerNoMax; layer_no++)
    {
        const char *layer_name = kStmLayerName[layer_no];

        // This layer is changed?
        int changed = crr_state.layer[layer_no].changed;
        if (changed)
        {
            HMI_DEBUG(">>>>>>>>>> Update layout of layer:%s", layer_name);

            // Get current layout name of this layer
            int crr_layout_state_no = crr_state.layer[layer_no].state;
            std::string crr_layout_name = std::string(kStmLayoutName[crr_layout_state_no]);

            LayoutState crr_layout_state;
            this->updateLayout(event_id, layer_no,
                               crr_layout_name, crr_layout_state);

            // Update current layout of this layer
            this->crr_layers[layer_name].layout_state = crr_layout_state;
        }
        // Update changed flag
        this->crr_layers[layer_name].changed = (changed) ? true : false;
    }

    // Erase role for the event_id from list
    this->req_role_list.erase(event_id);

    HMI_DEBUG(">>>>>>>>>> DUMP LAYERS (BEFORE)");
    this->dumpLayerState(this->prv_layers);

    HMI_DEBUG(">>>>>>>>>> DUMP LAYERS (AFTER)");
    this->dumpLayerState(this->crr_layers);

    this->dumpInvisibleRoleHistory();
}

int PolicyManager::updateLayout(int event_id, int layer_no,
                                std::string crr_layout_name, LayoutState &crr_layout_state)
{
    int changed;

    int event_no = STM_GET_EVENT_FROM_ID(event_id);
    int category_no = STM_GET_CATEGORY_FROM_ID(event_id);
    int area_no = STM_GET_AREA_FROM_ID(event_id);

    std::string req_evt = kStmEventName[event_no];
    std::string req_ctg = kStmCategoryName[category_no];
    std::string req_area = kStmAreaName[area_no];
    std::string req_role = this->req_role_list[event_id];

    const char *layer_name = kStmLayerName[layer_no];

    // Get previous layout name of this layer
    LayoutState prv_layout_state = this->prv_layers[layer_name].layout_state;
    std::string prv_layout_name = prv_layout_state.name;

    if ((prv_layout_name == crr_layout_name) &&
        (kStmLayoutName[StmLayoutNoNone] == crr_layout_name))
    {
        // If previous and current layout are none
        // Copy previous layout state for current
        crr_layout_state = prv_layout_state;
        changed = 0;
    }
    else
    {
        crr_layout_state = prv_layout_state;
        changed = 1;

        HMI_DEBUG("-- layout name previous:%s current:%s",
                  prv_layout_name.c_str(), crr_layout_name.c_str());
        if (prv_layout_name == crr_layout_name)
        {
            HMI_DEBUG("---- Previous layout is same with current");
        }
        else
        {
            // If previous layout is NOT same with current,
            // current areas is set with default value
            HMI_DEBUG("---- Previous layout is NOT same with current");
            crr_layout_state.name = this->default_layouts[crr_layout_name].name;
            crr_layout_state.category_num = this->default_layouts[crr_layout_name].category_num;
            crr_layout_state.area_list = this->default_layouts[crr_layout_name].area_list;
        }

        // Create candidate list
        std::map<std::string, AreaList> cand_list;
        // for (int ctg_no = StmCtgNoMin;
        //      ctg_no <= StmCtgNoMax; ctg_no++)
        // {
        for (const auto &ctg : this->layer2categories[layer_name])
        {
            // if (ctg_no == StmCtgNoNone)
            // {
            //     continue;
            // }

            // const char *ctg = kStmCategoryName[ctg_no];
            HMI_DEBUG("-- Create candidate list for ctg:%s", ctg.c_str());

            AreaList tmp_cand_list;
            int candidate_num = 0;
            int blank_num = crr_layout_state.category_num[ctg];

            // If requested event is "activate"
            // and there are requested category and area,
            // update area with requested role in current layout.
            bool request_for_this_layer = false;
            std::string used_role = "";
            if ((ctg == req_ctg) && ("activate" == req_evt))
            {
                HMI_DEBUG("---- Requested event is activate");
                for (AreaState &as : crr_layout_state.area_list)
                {
                    if (as.category == req_ctg)
                    {
                        request_for_this_layer = true;

                        if (as.name == req_area)
                        {
                            as.role = req_role;
                            used_role = req_role;
                            blank_num--;
                            HMI_DEBUG("------ Update current layout: area:%s category:%s role:%s",
                                      as.name.c_str(), as.category.c_str(), as.role.c_str());
                            break;
                        }
                    }
                }
            }

            // Create candidate list for category from the previous displayed categories
            for (AreaState area_state : prv_layout_state.area_list)
            {
                if ((ctg == area_state.category) &&
                    (used_role != area_state.role))
                {
                    // If there is the category
                    // which is same with new category and not used for updating yet,
                    // push it to list
                    HMI_DEBUG("---- Push previous(category:%s role:%s) to candidate list",
                              area_state.category.c_str(), area_state.role.c_str());
                    tmp_cand_list.push_back(area_state);
                    candidate_num++;
                }
            }

            // If NOT updated by requested area:
            // there is not requested area in new layout,
            // so push requested role to candidate list
            if (request_for_this_layer && ("" == used_role))
            {
                HMI_DEBUG("---- Push request(area:%s category:%s role:%s) to candidate list",
                          req_area.c_str(), req_ctg.c_str(), req_role.c_str());
                AreaState area_state;
                area_state.name = req_area;
                area_state.category = req_ctg;
                area_state.role = req_role;
                tmp_cand_list.push_back(area_state);
                candidate_num++;
            }

            HMI_DEBUG("---- blank_num:%d candidate_num:%d", blank_num, candidate_num);

            // Compare number of candidate/blank,
            // And remove role in order of the oldest as necessary
            if (candidate_num < blank_num)
            {
                // Refer history stack
                // and add to the top of tmp_cand_list in order to the newest
                while (candidate_num != blank_num)
                {
                    AreaState area_state;
                    area_state.name = kStmAreaName[StmAreaNoNone];
                    area_state.category = ctg;
                    area_state.role = this->popInvisibleRoleHistory(ctg);
                    if ("" == area_state.role)
                    {
                        HMI_ERROR("There is no role in history stack!!");
                    }
                    tmp_cand_list.push_back(area_state);
                    HMI_DEBUG("------ Add role:%s to candidate list",
                              area_state.role.c_str());
                    candidate_num++;
                }
            }
            else if (candidate_num > blank_num)
            {
                // Remove the oldest role from candidate list
                while (candidate_num != blank_num)
                {
                    std::string removed_role = tmp_cand_list.begin()->role;
                    HMI_DEBUG("------ Remove the oldest role:%s from candidate list",
                              removed_role.c_str());
                    tmp_cand_list.erase(tmp_cand_list.begin());
                    candidate_num--;

                    // Push removed data to history stack
                    this->pushInvisibleRoleHistory(ctg, removed_role);

                    // Remove from current layout
                    for (AreaState &as : crr_layout_state.area_list)
                    {
                        if (as.role == removed_role)
                        {
                            as.role = "";
                        }
                    }
                }
            }
            else
            { // (candidate_num == blank_num)
                // nop
            }

            cand_list[ctg] = tmp_cand_list;
        }

        // Update areas
        HMI_DEBUG("-- Update areas by using candidate list");
        for (AreaState &as : crr_layout_state.area_list)
        {
            HMI_DEBUG("---- Check area:%s category:%s role:%s",
                      as.name.c_str(), as.category.c_str(), as.role.c_str());
            if ("" == as.role)
            {
                HMI_DEBUG("------ Update this area with role:%s",
                          cand_list[as.category].begin()->role.c_str());
                as.role = cand_list[as.category].begin()->role;
                cand_list[as.category].erase(cand_list[as.category].begin());
            }
        }
    }
    return changed;
}

void PolicyManager::createOutputInformation(StmState crr_state, json_object **json_out)
{
    json_object *json_tmp;

    // Create layout information
    //
    //     "layers": [
    //     {
    //         "homescreen": {
    //             "changed": <bool>,
    //             "areas": [
    //             {
    //                 "name":<const char*>,
    //                 "role":<const char*>
    //             }.
    //             ...
    //             ]
    //         }
    //     },
    //     ...
    json_object *json_layer = json_object_new_array();
    const char *layer_name;
    for (int layer_no = StmLayerNoMin;
         layer_no <= StmLayerNoMax; layer_no++)
    {
        layer_name = kStmLayerName[layer_no];
        json_tmp = json_object_new_object();
        this->addStateToJson(layer_name,
                             this->crr_layers[layer_name].changed,
                             this->crr_layers[layer_name].layout_state.area_list,
                             &json_tmp);
        json_object_array_add(json_layer, json_tmp);
    }
    json_object_object_add(*json_out, "layers", json_layer);
}

int PolicyManager::transitionState(sd_event_source *source, void *data)
{
    HMI_DEBUG(">>>>>>>>>> START STATE TRANSITION");

    int event_id = *((int *)data);

    int event_no, category_no, area_no;
    event_no = STM_GET_EVENT_FROM_ID(event_id);
    category_no = STM_GET_CATEGORY_FROM_ID(event_id);
    area_no = STM_GET_AREA_FROM_ID(event_id);
    HMI_DEBUG(">>>>>>>>>> EVENT:%s CATEGORY:%s AREA:%s",
              kStmEventName[event_no],
              kStmCategoryName[category_no],
              kStmAreaName[area_no]);

    // Transition state
    StmState crr_state;
    int ret = stmTransitionState(event_id, &crr_state);
    if (0 > ret)
    {
        HMI_ERROR("Failed transition state");
        if (nullptr != this->callback.onError)
        {
            json_object *json_out = json_object_new_object();
            json_object_object_add(json_out, "message",
                                   json_object_new_string("Failed to transition state"));
            json_object_object_add(json_out, "event",
                                   json_object_new_string(kStmEventName[event_no]));
            json_object_object_add(json_out, "role",
                                   json_object_new_string(this->req_role_list[event_id].c_str()));
            json_object_object_add(json_out, "area",
                                   json_object_new_string(kStmAreaName[area_no]));
            this->callback.onError(json_out);
            json_object_put(json_out);
        }
        return -1;
    }

    // Update state which is managed by PolicyManager
    this->updateState(event_id, crr_state);

    // Create output information for ResourceManager
    json_object *json_out = json_object_new_object();
    this->createOutputInformation(crr_state, &json_out);

    // Notify changed state
    if (nullptr != this->callback.onStateTransitioned)
    {
        this->callback.onStateTransitioned(json_out);
    }

    // Release json_object
    json_object_put(json_out);

    // Release data
    delete (int *)data;

    // Destroy sd_event_source object
    sd_event_source_unref(source);

    // Remove event source from list
    if (this->event_source_list.find(event_id) != this->event_source_list.end())
    {
        this->event_source_list.erase(event_id);
    }

    HMI_DEBUG(">>>>>>>>>> FINISH STATE TRANSITION");
    return 0;
}

int PolicyManager::timerEvent(sd_event_source *source, uint64_t usec, void *data)
{
    HMI_DEBUG("Call");

    int ret = this->transitionState(source, data);
    return ret;
}

int PolicyManager::setStateTransitionProcessToSystemd(int event_id, uint64_t delay_ms, std::string role)
{
    struct sd_event_source *event_source;
    HMI_DEBUG("wm:pm event_id:0x%x delay:%d role:%s", event_id, delay_ms, role.c_str());

    if (0 == delay_ms)
    {
        int ret = sd_event_add_defer(afb_api_get_event_loop(afbBindingV3root), &event_source,
                                     &pm::transitionStateWrapper, new int(event_id));
        if (0 > ret)
        {
            HMI_ERROR("wm:pm Failed to sd_event_add_defer: errno:%d", ret);
            return -1;
        }
    }
    else
    {
        // Get current time
        struct timespec time_spec;
        clock_gettime(CLOCK_BOOTTIME, &time_spec);

        // Calculate timer fired time
        uint64_t usec = (time_spec.tv_sec * 1000000) + (time_spec.tv_nsec / 1000) + (delay_ms * 1000);

        // Set timer
        int ret = sd_event_add_time(afb_api_get_event_loop(afbBindingV3root), &event_source,
                                    CLOCK_BOOTTIME, usec, 1,
                                    &pm::timerEventWrapper, new int(event_id));
        if (0 > ret)
        {
            HMI_ERROR("wm:pm Failed to sd_event_add_time: errno:%d", ret);
            return -1;
        }
    }
    // Store event source
    this->event_source_list[event_id] = event_source;
    // Store requested role
    this->req_role_list[event_id] = role;
    return 0;
}

int PolicyManager::loadRoleDb()
{
    std::string file_name;

    // Get afm application installed dir
    char const *afm_app_install_dir = getenv("AFM_APP_INSTALL_DIR");
    HMI_DEBUG("afm_app_install_dir:%s", afm_app_install_dir);

    if (!afm_app_install_dir)
    {
        HMI_ERROR("AFM_APP_INSTALL_DIR is not defined");
    }
    else
    {
        file_name = std::string(afm_app_install_dir) + std::string("/etc/roles.db");
    }

    // Load roles.db
    json_object *json_obj;
    int ret = this->inputJsonFilie(file_name.c_str(), &json_obj);
    if (0 > ret)
    {
        HMI_ERROR("Could not open roles.db, so use default role information");
        json_obj = json_tokener_parse(kDefaultRoleDb);
    }
    HMI_DEBUG("json_obj dump:%s", json_object_get_string(json_obj));

    json_object *json_roles;
    if (!json_object_object_get_ex(json_obj, "roles", &json_roles))
    {
        HMI_ERROR("Parse Error!!");
        return -1;
    }

    int len = json_object_array_length(json_roles);
    HMI_DEBUG("json_cfg len:%d", len);
    HMI_DEBUG("json_cfg dump:%s", json_object_get_string(json_roles));

    json_object *json_tmp;
    const char *category;
    const char *roles;
    const char *areas;
    const char *layer;
    for (int i = 0; i < len; i++)
    {
        json_tmp = json_object_array_get_idx(json_roles, i);

        category = this->getStringFromJson(json_tmp, "category");
        roles = this->getStringFromJson(json_tmp, "role");
        areas = this->getStringFromJson(json_tmp, "area");
        layer = this->getStringFromJson(json_tmp, "layer");

        if ((nullptr == category) || (nullptr == roles) ||
            (nullptr == areas) || (nullptr == layer))
        {
            HMI_ERROR("Parse Error!!");
            return -1;
        }

        // Parse roles by '|'
        std::vector<std::string> vct_roles;
        vct_roles = this->parseString(std::string(roles), '|');

        // Parse areas by '|'
        Areas vct_areas;
        vct_areas = this->parseString(std::string(areas), '|');

        // Set role, category, areas
        for (auto itr = vct_roles.begin(); itr != vct_roles.end(); ++itr)
        {
            this->role2category[*itr] = std::string(category);
        }
        this->category2role[category] = std::string(roles);
        this->category2areas[category] = vct_areas;
        this->layer2categories[layer].push_back(category);
    }

    // Check
    HMI_DEBUG("Check role2category");
    for (const auto &x : this->role2category)
    {
        HMI_DEBUG("key:%s, val:%s", x.first.c_str(), x.second.c_str());
    }

    HMI_DEBUG("Check category2role");
    for (const auto &x : this->category2role)
    {
        HMI_DEBUG("key:%s, val:%s", x.first.c_str(), x.second.c_str());
    }

    HMI_DEBUG("Check category2areas");
    for (const auto &x : this->category2areas)
    {
        for (const auto &y : x.second)
        {
            HMI_DEBUG("key:%s, val:%s", x.first.c_str(), y.c_str());
        }
    }
    return 0;
}

int PolicyManager::loadStateDb()
{
    HMI_DEBUG("Call");

    // Get afm application installed dir
    char const *afm_app_install_dir = getenv("AFM_APP_INSTALL_DIR");
    HMI_DEBUG("afm_app_install_dir:%s", afm_app_install_dir);

    std::string file_name;
    if (!afm_app_install_dir)
    {
        HMI_ERROR("AFM_APP_INSTALL_DIR is not defined");
    }
    else
    {
        file_name = std::string(afm_app_install_dir) + std::string("/etc/states.db");
    }

    // Load states.db
    json_object *json_obj;
    int ret = this->inputJsonFilie(file_name.c_str(), &json_obj);
    if (0 > ret)
    {
        HMI_DEBUG("Could not open states.db, so use default layout information");
        json_obj = json_tokener_parse(kDefaultStateDb);
    }
    HMI_DEBUG("json_obj dump:%s", json_object_get_string(json_obj));

    // Perse states
    HMI_DEBUG("Perse states");
    json_object *json_cfg;
    if (!json_object_object_get_ex(json_obj, "states", &json_cfg))
    {
        HMI_ERROR("Parse Error!!");
        return -1;
    }

    int len = json_object_array_length(json_cfg);
    HMI_DEBUG("json_cfg len:%d", len);
    HMI_DEBUG("json_cfg dump:%s", json_object_get_string(json_cfg));

    const char *layout;
    const char *role;
    const char *category;
    for (int i = 0; i < len; i++)
    {
        json_object *json_tmp = json_object_array_get_idx(json_cfg, i);

        layout = this->getStringFromJson(json_tmp, "name");
        if (nullptr == layout)
        {
            HMI_ERROR("Parse Error!!");
            return -1;
        }
        HMI_DEBUG("> layout:%s", layout);

        json_object *json_area_array;
        if (!json_object_object_get_ex(json_tmp, "areas", &json_area_array))
        {
            HMI_ERROR("Parse Error!!");
            return -1;
        }

        int len_area = json_object_array_length(json_area_array);
        HMI_DEBUG("json_area_array len:%d", len_area);
        HMI_DEBUG("json_area_array dump:%s", json_object_get_string(json_area_array));

        LayoutState layout_state;
        AreaState area_state;
        std::map<std::string, int> category_num;
        for (int ctg_no = StmCtgNoMin;
             ctg_no <= StmCtgNoMax; ctg_no++)
        {
            const char *ctg_name = kStmCategoryName[ctg_no];
            category_num[ctg_name] = 0;
        }

        for (int j = 0; j < len_area; j++)
        {
            json_object *json_area = json_object_array_get_idx(json_area_array, j);

            // Get area name
            const char *area = this->getStringFromJson(json_area, "name");
            if (nullptr == area)
            {
                HMI_ERROR("Parse Error!!");
                return -1;
            }
            area_state.name = std::string(area);
            HMI_DEBUG(">> area:%s", area);

            // Get app attribute of the area
            category = this->getStringFromJson(json_area, "category");
            if (nullptr == category)
            {
                HMI_ERROR("Parse Error!!");
                return -1;
            }
            area_state.category = std::string(category);
            category_num[category]++;
            HMI_DEBUG(">>> category:%s", category);

            role = this->getStringFromJson(json_area, "role");
            if (nullptr != role)
            {
                // Role is NOT essential here
                area_state.role = std::string(role);
            }
            else
            {
                area_state.role = std::string("");
            }
            HMI_DEBUG(">>> role:%s", role);

            layout_state.area_list.push_back(area_state);
        }

        layout_state.name = layout;
        layout_state.category_num = category_num;
        this->default_layouts[layout] = layout_state;
    }

    // initialize for none layout
    LayoutState none_layout_state;
    memset(&none_layout_state, 0, sizeof(none_layout_state));
    none_layout_state.name = "none";
    this->default_layouts["none"] = none_layout_state;

    // Check
    for (auto itr_layout = this->default_layouts.begin();
         itr_layout != this->default_layouts.end(); ++itr_layout)
    {
        HMI_DEBUG(">>> layout:%s", itr_layout->first.c_str());

        for (auto itr_area = itr_layout->second.area_list.begin();
             itr_area != itr_layout->second.area_list.end(); ++itr_area)
        {
            HMI_DEBUG(">>> >>> area    :%s", itr_area->name.c_str());
            HMI_DEBUG(">>> >>> category:%s", itr_area->category.c_str());
            HMI_DEBUG(">>> >>> role    :%s", itr_area->role.c_str());
        }
    }

    // Release json_object
    json_object_put(json_obj);

    return 0;
}

void PolicyManager::pushInvisibleRoleHistory(std::string category, std::string role)
{
    auto i = std::remove_if(this->invisible_role_history[category].begin(),
                            this->invisible_role_history[category].end(),
                            [role](std::string x) { return (role == x); });

    if (this->invisible_role_history[category].end() != i)
    {
        this->invisible_role_history[category].erase(i);
    }

    this->invisible_role_history[category].push_back(role);

    if (pm::kInvisibleRoleHistoryNum < invisible_role_history[category].size())
    {
        this->invisible_role_history[category].erase(
            this->invisible_role_history[category].begin());
    }
}

std::string PolicyManager::popInvisibleRoleHistory(std::string category)
{
    std::string role;
    if (invisible_role_history[category].empty())
    {
        role = "";
    }
    else
    {
        role = this->invisible_role_history[category].back();
        this->invisible_role_history[category].pop_back();
    }
    return role;
}

const char *PolicyManager::getStringFromJson(json_object *obj, const char *key)
{
    json_object *tmp;
    if (!json_object_object_get_ex(obj, key, &tmp))
    {
        HMI_DEBUG("Not found key \"%s\"", key);
        return nullptr;
    }

    return json_object_get_string(tmp);
}

int PolicyManager::inputJsonFilie(const char *file, json_object **obj)
{
    const int input_size = 128;
    int ret = -1;

    HMI_DEBUG("Input file: %s", file);

    // Open json file
    FILE *fp = fopen(file, "rb");
    if (nullptr == fp)
    {
        HMI_ERROR("Could not open file");
        return ret;
    }

    // Parse file data
    struct json_tokener *tokener = json_tokener_new();
    enum json_tokener_error json_error;
    char buffer[input_size];
    int block_cnt = 1;
    while (1)
    {
        size_t len = fread(buffer, sizeof(char), input_size, fp);
        *obj = json_tokener_parse_ex(tokener, buffer, len);
        if (nullptr != *obj)
        {
            HMI_DEBUG("File input is success");
            ret = 0;
            break;
        }

        json_error = json_tokener_get_error(tokener);
        if ((json_tokener_continue != json_error) || (input_size > len))
        {
            HMI_ERROR("Failed to parse file (byte:%d err:%s)",
                      (input_size * block_cnt), json_tokener_error_desc(json_error));
            HMI_ERROR("\n%s", buffer);
            *obj = nullptr;
            break;
        }
        block_cnt++;
    }

    // Close json file
    fclose(fp);

    // Free json_tokener
    json_tokener_free(tokener);

    return ret;
}

void PolicyManager::dumpLayerState(std::unordered_map<std::string, LayerState> &layers)
{
    HMI_DEBUG("-------------------------------------------------------------------------------------------------------");
    HMI_DEBUG("|%-15s|%s|%-20s|%-20s|%-20s|%-20s|",
              "LAYER", "C", "LAYOUT", "AREA", "CATEGORY", "ROLE");
    for (const auto &itr : layers)
    {
        LayerState ls = itr.second;
        const char* layer   = ls.name.c_str();
        const char* changed = (ls.changed) ? "T" : "f";
        const char* layout  = ls.layout_state.name.c_str();
        bool first = true;
        for (const auto &as : ls.layout_state.area_list)
        {
            if (first)
            {
                first = false;
                HMI_DEBUG("|%-15s|%1s|%-20s|%-20s|%-20s|%-20s|",
                          layer, changed, layout,
                          as.name.c_str(), as.category.c_str(), as.role.c_str());
            }
            else
                HMI_DEBUG("|%-15s|%1s|%-20s|%-20s|%-20s|%-20s|",
                          "", "", "", as.name.c_str(), as.category.c_str(), as.role.c_str());
        }
    }
    HMI_DEBUG("-------------------------------------------------------------------------------------------------------");
}

void PolicyManager::dumpInvisibleRoleHistory()
{
    HMI_DEBUG(">>>>>>>>>> DUMP INVISIBLE ROLE HISTORY ( category [older > newer] )");
    for (int ctg_no = StmCtgNoMin; ctg_no <= StmCtgNoMax; ctg_no++)
    {
        if (ctg_no == StmCtgNoNone)
            continue;

        std::string category = std::string(kStmCategoryName[ctg_no]);

        std::string str = category + " [ ";
        for (const auto &i : this->invisible_role_history[category])
            str += (i + " > ");

        str += "]";
        HMI_DEBUG("%s", str.c_str());
    }
}

std::vector<std::string> PolicyManager::parseString(std::string str, char delimiter)
{
    // Parse string by delimiter
    std::vector<std::string> vct;
    std::stringstream ss{str};
    std::string buf;
    while (std::getline(ss, buf, delimiter))
    {
        if (!buf.empty())
        {
            // Delete space and push back to vector
            vct.push_back(this->deleteSpace(buf));
        }
    }
    return vct;
}

std::string PolicyManager::deleteSpace(std::string str)
{
    std::string ret = str;
    size_t pos;
    while ((pos = ret.find_first_of(" ")) != std::string::npos)
    {
        ret.erase(pos, 1);
    }
    return ret;
}

const char *PolicyManager::kDefaultRoleDb = "{ \
    \"roles\":[ \
    { \
        \"category\": \"homescreen\", \
        \"role\": \"homescreen\", \
        \"area\": \"fullscreen\", \
    }, \
    { \
        \"category\": \"map\", \
        \"role\": \"map\", \
        \"area\": \"normal.full | split.main\", \
    }, \
    { \
        \"category\": \"general\", \
        \"role\": \"launcher | poi | browser | sdl | mixer | radio | hvac | debug | phone | video | music\", \
        \"area\": \"normal.full\", \
    }, \
    { \
        \"category\": \"system\", \
        \"role\": \"settings | dashboard\", \
        \"area\": \"normal.full\", \
    }, \
    { \
        \"category\": \"software_keyboard\", \
        \"role\": \"software_keyboard\", \
        \"area\": \"software_keyboard\", \
    }, \
    { \
        \"category\": \"restriction\", \
        \"role\": \"restriction\", \
        \"area\": \"restriction.normal | restriction.split.main | restriction.split.sub\", \
    }, \
    { \
        \"category\": \"pop_up\", \
        \"role\": \"pop_up\", \
        \"area\": \"on_screen\", \
    }, \
    { \
        \"category\": \"system_alert\", \
        \"role\": \"system_alert\", \
        \"area\": \"on_screen\", \
    } \
    ] \
}";

const char *PolicyManager::kDefaultStateDb = "{ \
    \"states\": [ \
        { \
            \"name\": \"homescreen\", \
            \"layer\": \"far_homescreen\", \
            \"areas\": [ \
                { \
                    \"name\": \"fullscreen\", \
                    \"category\": \"homescreen\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"map.normal\", \
            \"layer\": \"apps\", \
            \"areas\": [ \
                { \
                    \"name\": \"normal.full\", \
                    \"category\": \"map\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"map.split\", \
            \"layer\": \"apps\", \
            \"areas\": [ \
                { \
                    \"name\": \"split.main\", \
                    \"category\": \"map\" \
                }, \
                { \
                    \"name\": \"split.sub\", \
                    \"category\": \"splitable\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"map.fullscreen\", \
            \"layer\": \"apps\", \
            \"areas\": [ \
                { \
                    \"name\": \"fullscreen\", \
                    \"category\": \"map\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"splitable.normal\", \
            \"layer\": \"apps\", \
            \"areas\": [ \
                { \
                    \"name\": \"normal.full\", \
                    \"category\": \"splitable\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"splitable.split\", \
            \"layer\": \"apps\", \
            \"areas\": [ \
                { \
                    \"name\": \"split.main\", \
                    \"category\": \"splitable\" \
                }, \
                { \
                    \"name\": \"split.sub\", \
                    \"category\": \"splitable\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"general.normal\", \
            \"layer\": \"apps\", \
            \"areas\": [ \
                { \
                    \"name\": \"normal.full\", \
                    \"category\": \"general\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"system.normal\", \
            \"layer\": \"apps\", \
            \"areas\": [ \
                { \
                    \"name\": \"normal.full\", \
                    \"category\": \"system\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"software_keyboard\", \
            \"layer\": \"near_homescreen\", \
            \"areas\": [ \
                { \
                    \"name\": \"software_keyboard\", \
                    \"category\": \"software_keyboard\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"restriction.normal\", \
            \"layer\": \"restriction\", \
            \"areas\": [ \
                { \
                    \"name\": \"restriction.normal\", \
                    \"category\": \"restriction\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"restriction.split.main\", \
            \"layer\": \"restriction\", \
            \"areas\": [ \
                { \
                    \"name\": \"restriction.split.main\", \
                    \"category\": \"restriction\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"restriction.split.sub\", \
            \"layer\": \"restriction\", \
            \"areas\": [ \
                { \
                    \"name\": \"restriction.split.sub\", \
                    \"category\": \"restriction\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"pop_up\", \
            \"layer\": \"on_screen\", \
            \"areas\": [ \
                { \
                    \"name\": \"on_screen\", \
                    \"category\": \"pop_up\" \
                } \
            ] \
        }, \
        { \
            \"name\": \"system_alert\", \
            \"layer\": \"on_screen\", \
            \"areas\": [ \
                { \
                    \"name\": \"on_screen\", \
                    \"category\": \"system_alert\" \
                } \
            ] \
        } \
    ] \
}";
