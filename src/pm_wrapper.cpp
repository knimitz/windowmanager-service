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

#include "pm_wrapper.hpp"
#include "json_helper.hpp"
#include "util.hpp"

namespace wm
{

static PMWrapper *g_context;

namespace
{

static void onStateTransitioned(json_object *json_out)
{
    g_context->updateStates(json_out);
}

static void onError(json_object *json_out)
{
    HMI_DEBUG("error message from PolicyManager:%s",
              json_object_get_string(json_out));

    g_context->processError();
}

} // namespace

PMWrapper::PMWrapper() {}

int PMWrapper::initialize()
{
    int ret = 0;

    ret = this->pm.initialize();
    if (0 > ret)
    {
        HMI_ERROR("Faild to initialize PolicyManager");
    }

    g_context = this;

    return ret;
}

void PMWrapper::registerCallback(StateTransitionHandler on_state_transitioned,
                                 ErrorHandler on_error)
{
    this->on_state_transitioned = on_state_transitioned;
    this->on_error = on_error;

    PolicyManager::CallbackTable my_callback;
    my_callback.onStateTransitioned = onStateTransitioned;
    my_callback.onError = onError;
    this->pm.registerCallback(my_callback);
}

int PMWrapper::setInputEventData(Task task, std::string role, std::string area)
{
    const char* event;
    if (Task::TASK_ALLOCATE == task)
    {
        event = "activate";
    }
    else if (Task::TASK_RELEASE == task)
    {
        event = "deactivate";
    }
    else
    {
        event = "";
    }

    json_object *json_in = json_object_new_object();
    json_object_object_add(json_in, "event", json_object_new_string(event));
    json_object_object_add(json_in, "role", json_object_new_string(role.c_str()));
    json_object_object_add(json_in, "area", json_object_new_string(area.c_str()));

    int ret;
    ret = this->pm.setInputEventData(json_in);
    if (0 > ret)
    {
        HMI_ERROR("Faild to set input event data to PolicyManager");
    }
    json_object_put(json_in);

    return ret;
}

int PMWrapper::executeStateTransition()
{
    int ret;
    ret = this->pm.executeStateTransition();
    if (0 > ret)
    {
        HMI_ERROR("Failed to execute state transition for PolicyManager");
    }

    return ret;
}

void PMWrapper::undoState()
{
    this->pm.undoState();

    this->crrlayer2rolestate = this->prvlayer2rolestate;
}

void PMWrapper::updateStates(json_object *json_out)
{
    std::vector<WMAction> actions;

    HMI_DEBUG("json_out dump:%s", json_object_get_string(json_out));

    this->createLayoutChangeAction(json_out, actions);

    this->on_state_transitioned(actions);
}

void PMWrapper::createLayoutChangeAction(json_object *json_out, std::vector<WMAction> &actions)
{
    // Get displayed roles from previous layout
    json_object *json_layers;
    if (!json_object_object_get_ex(json_out, "layers", &json_layers))
    {
        HMI_DEBUG("Not found key \"layers\"");
        return;
    }

    int len = json_object_array_length(json_layers);
    HMI_DEBUG("json_layers len:%d", len);

    for (int i = 0; i < len; i++)
    {
        json_object *json_tmp = json_object_array_get_idx(json_layers, i);

        std::string layer_name = jh::getStringFromJson(json_tmp, "name");
        json_bool changed = jh::getBoolFromJson(json_tmp, "changed");
        HMI_DEBUG("layer:%s changed:%d", layer_name.c_str(), changed);

        if (changed)
        {
            json_object *json_areas;
            if (!json_object_object_get_ex(json_tmp, "areas", &json_areas))
            {
                HMI_DEBUG("Not found key \"areas\"");
                return;
            }

            int len = json_object_array_length(json_areas);
            HMI_DEBUG("json_layers len:%d", len);

            // Store previous role state in this layer
            this->prvlayer2rolestate[layer_name] = this->crrlayer2rolestate[layer_name];

            RoleState crr_roles;
            RoleState prv_roles = this->prvlayer2rolestate[layer_name];
            for (int j = 0; j < len; j++)
            {
                json_object *json_tmp2 = json_object_array_get_idx(json_areas, j);

                std::string area_name = jh::getStringFromJson(json_tmp2, "name");
                std::string role_name = jh::getStringFromJson(json_tmp2, "role");

                crr_roles[role_name] = area_name;

                auto i_prv = prv_roles.find(role_name);
                HMI_DEBUG("current role:%s area:%s",
                          role_name.c_str(), area_name.c_str());

                // If current role does not exist in previous
                if (prv_roles.end() == i_prv)
                {
                    HMI_DEBUG("current role does not exist in previous");

                    // Set activate action
                    bool end_draw_finished = false;
                    WMAction act
                    {
                        "",
                        role_name,
                        area_name,
                        TaskVisible::VISIBLE,
                        end_draw_finished
                    };
                    actions.push_back(act);
                }
                else
                {
                    HMI_DEBUG("previous role:%s area:%s",
                              i_prv->first.c_str(), i_prv->second.c_str());

                    // If current role exists in previous and area is different with previous
                    if (area_name != i_prv->second)
                    {
                        HMI_DEBUG("current role exists in previous and area is different with previous");

                        // Set activate action
                        bool end_draw_finished = false;
                        WMAction act
                        {
                            "",
                            role_name,
                            area_name,
                            TaskVisible::VISIBLE,
                            end_draw_finished
                        };
                        actions.push_back(act);
                    }

                    // Remove role which exist in current list from previous list
                    prv_roles.erase(i_prv);
                }
            }

            // Deactivate roles which remains in previous list
            // because these are not displayed in current layout
            for (auto i_prv : prv_roles)
            {
                HMI_DEBUG("Deactivate role:%s", i_prv.first.c_str());

                // Set deactivate action
                bool end_draw_finished = true;
                WMAction act
                {
                    "",
                    i_prv.first,
                    "",
                    TaskVisible::INVISIBLE,
                    end_draw_finished
                };
                actions.push_back(act);
            }

            // Update previous role list
            this->crrlayer2rolestate[layer_name] = crr_roles;
        }
    }
}

void PMWrapper::processError()
{
    this->on_error();
}

} // namespace wm
