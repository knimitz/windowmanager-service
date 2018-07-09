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

#ifndef TMCAGLWM_PM_WRAPPER_HPP
#define TMCAGLWM_PM_WRAPPER_HPP

#include <functional>
#include <vector>
#include <string>
#include <map>
#include "policy_manager.hpp"
#include "request.hpp"

struct json_object;
struct sd_event;
struct sd_event_source;
struct StmState;

namespace wm
{

class PMWrapper
{
  public:
    explicit PMWrapper();
    ~PMWrapper() = default;

    using StateTransitionHandler = std::function<void(std::vector<WMAction>)>;
    using ErrorHandler = std::function<void(void)>;

    int initialize();
    void registerCallback(StateTransitionHandler on_state_transitioned,
                          ErrorHandler on_error);
    int setInputEventData(Task task, std::string role, std::string area);
    int executeStateTransition();
    void undoState();

    // Do not use these functions
    void updateStates(json_object *json_out);
    void processError();

  private:
    // Disable copy and move
    PMWrapper(PMWrapper const &) = delete;
    PMWrapper &operator=(PMWrapper const &) = delete;
    PMWrapper(PMWrapper &&) = delete;
    PMWrapper &operator=(PMWrapper &&) = delete;

    typedef std::map<std::string, std::string> RoleState;

    PolicyManager pm;
    StateTransitionHandler on_state_transitioned;
    ErrorHandler on_error;
    std::map<std::string, RoleState> prvlayer2rolestate;
    std::map<std::string, RoleState> crrlayer2rolestate;

    void createLayoutChangeAction(json_object *json_out, std::vector<WMAction> &actions);
};

} // namespace wm

#endif // TMCAGLWM_PM_WRAPPER_HPP
