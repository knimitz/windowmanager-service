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

#ifndef TMCAGLWM_POLICY_MANAGER_HPP
#define TMCAGLWM_POLICY_MANAGER_HPP

#include <functional>
#include <unordered_map>
#include <vector>
#include <queue>

struct json_object;
struct sd_event;
struct sd_event_source;
struct StmState;

class PolicyManager
{

  public:
    explicit PolicyManager();
    ~PolicyManager() = default;

    using Handler = std::function<void(json_object *)>;

    typedef struct
    {
        Handler onStateTransitioned;
        Handler onError;
    } CallbackTable;

    int initialize();
    void registerCallback(CallbackTable callback_table);
    int setInputEventData(json_object *json_in);
    int executeStateTransition();
    void undoState();

    // Do not use these functions
    int transitionState(sd_event_source *source, void *data);
    int timerEvent(sd_event_source *source, uint64_t usec, void *data);

  private:
    // Disable copy and move
    PolicyManager(PolicyManager const &) = delete;
    PolicyManager &operator=(PolicyManager const &) = delete;
    PolicyManager(PolicyManager &&) = delete;
    PolicyManager &operator=(PolicyManager &&) = delete;

    typedef struct EventInfo
    {
        int event;
        std::string role;
        uint64_t delay;
    } EventInfo;

    typedef struct AreaState
    {
        std::string name;
        std::string category;
        std::string role;
    } AreaState;

    typedef std::vector<AreaState> AreaList;
    typedef struct LayoutState
    {
        std::string name;
        std::map<std::string, int> category_num;
        AreaList area_list;
    } LayoutState;

    typedef struct LayerState
    {
        std::string name;
        LayoutState layout_state;
        bool changed;
    } LayerState;

    // Convert map
    std::unordered_map<std::string, int> eventname2no;
    std::unordered_map<std::string, int> categoryname2no;
    std::unordered_map<std::string, int> areaname2no;

    std::unordered_map<std::string, std::string> role2category;
    std::unordered_map<std::string, std::string> category2role;
    std::unordered_map<std::string, std::string> role2defaultarea;

    struct sd_event *event_loop;
    std::queue<EventInfo> event_info_queue;
    std::map<int, struct sd_event_source *> event_source_list;
    std::map<int, std::string> req_role_list;

    CallbackTable callback;

    std::unordered_map<std::string, LayerState> prv_layers;
    std::unordered_map<std::string, LayerState> crr_layers;

    std::unordered_map<std::string, LayoutState> default_layouts;

    std::map<std::string, std::vector<std::string>> role_history;

    void initializeState();
    void initializeLayerState();
    void updateState(int event_data, StmState crr_state);
    void updateLayerState(int event_data, StmState crr_state);
    void createOutputInformation(StmState crr_state, json_object **json_out);
    int setStateTransitionProcessToSystemd(int event, uint64_t delay_ms, std::string role);

    void pushRoleHistory(std::string category, std::string role);
    std::string popRoleHistory(std::string category);

    int loadRoleDb();
    int loadLayoutDb();

    void dumpLayerState(std::unordered_map<std::string, LayerState> &layers);
    void dumpRoleHistory();

    void addStateToJson(const char *name, bool changed,
                        std::string state, json_object **json_out);
    void addStateToJson(const char *layer_name, bool changed,
                        AreaList area_list, json_object **json_out);
    const char *getStringFromJson(json_object *obj, const char *key);
    int inputJsonFilie(const char *file, json_object **obj);

    std::vector<std::string> parseString(std::string str, char delimiter);
    std::string deleteSpace(std::string str);

    static const char *kDefaultRoleDb;
    static const char *kDefaultLayoutDb;
};

#endif // TMCAGLWM_POLICY_MANAGER_HPP
