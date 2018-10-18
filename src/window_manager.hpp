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

#ifndef WINDOW_MANAGER_HPP
#define WINDOW_MANAGER_HPP

#include <atomic>
#include <memory>
#include <unordered_map>
#include <experimental/optional>
#include "result.hpp"
#include "pm_wrapper.hpp"
#include "util.hpp"
#include "request.hpp"
#include "wm_error.hpp"
#include "wm_layer_control.hpp"
extern "C"
{
#include <afb/afb-binding.h>
}

struct json_object;

namespace wm
{

using std::experimental::optional;

/* DrawingArea name used by "{layout}.{area}" */
extern const char kNameLayoutNormal[];
extern const char kNameLayoutSplit[];
extern const char kNameAreaFull[];
extern const char kNameAreaMain[];
extern const char kNameAreaSub[];

/* Key for json obejct */
extern const char kKeyDrawingName[];
extern const char kKeyDrawingArea[];
extern const char kKeyDrawingRect[];
extern const char kKeyX[];
extern const char kKeyY[];
extern const char kKeyWidth[];
extern const char kKeyHeigh[];
extern const char kKeyWidthPixel[];
extern const char kKeyHeightPixel[];
extern const char kKeyWidthMm[];
extern const char kKeyHeightMm[];
extern const char kKeyScale[];
extern const char kKeyIds[];

struct id_allocator
{
    unsigned next = 1;
    // Surfaces that where requested but not yet created
    std::unordered_map<unsigned, std::string> id2name;
    std::unordered_map<std::string, unsigned> name2id;

    id_allocator(id_allocator const &) = delete;
    id_allocator(id_allocator &&) = delete;
    id_allocator &operator=(id_allocator const &);
    id_allocator &operator=(id_allocator &&) = delete;

    // Insert and return a new ID
    unsigned generate_id(std::string const &name)
    {
        unsigned sid = this->next++;
        this->id2name[sid] = name;
        this->name2id[name] = sid;
        HMI_DEBUG("allocated new id %u with name %s", sid, name.c_str());
        return sid;
    }

    // Insert a new ID which defined outside
    void register_name_id(std::string const &name, unsigned sid)
    {
        this->id2name[sid] = name;
        this->name2id[name] = sid;
        HMI_DEBUG("register id %u with name %s", sid, name.c_str());
        return;
    }

    // Lookup by ID or by name
    optional<unsigned> lookup(std::string const &name) const
    {
        auto i = this->name2id.find(name);
        return i == this->name2id.end() ? nullopt : optional<unsigned>(i->second);
    }

    optional<std::string> lookup(unsigned id) const
    {
        auto i = this->id2name.find(id);
        return i == this->id2name.end() ? nullopt
                                        : optional<std::string>(i->second);
    }

    // Remove a surface id and name
    void remove_id(std::string const &name)
    {
        auto i = this->name2id.find(name);
        if (i != this->name2id.end())
        {
            this->id2name.erase(i->second);
            this->name2id.erase(i);
        }
    }

    void remove_id(unsigned id)
    {
        auto i = this->id2name.find(id);
        if (i != this->id2name.end())
        {
            this->name2id.erase(i->second);
            this->id2name.erase(i);
        }
    }
};

struct TmpClient
{
    std::string   appid;
    unsigned layer;
};


class WindowManager
{
  public:
    typedef std::unordered_map<uint32_t, struct rect> rect_map;
    typedef std::function<void(const char *err_msg)> reply_func;

    enum EventType
    {
        Event_Val_Min = 0,

        Event_Active = Event_Val_Min,
        Event_Inactive,

        Event_Visible,
        Event_Invisible,

        Event_SyncDraw,
        Event_FlushDraw,

        Event_ScreenUpdated,

        Event_Error,

        Event_Val_Max = Event_Error,
    };

    explicit WindowManager();
    ~WindowManager() = default;

    WindowManager(WindowManager const &) = delete;
    WindowManager &operator=(WindowManager const &) = delete;
    WindowManager(WindowManager &&) = delete;
    WindowManager &operator=(WindowManager &&) = delete;

    int init();

    result<int> api_request_surface(char const *appid, char const *role);
    char const *api_request_surface(char const *appid, char const *role, char const *ivi_id);
    void api_activate_window(char const *appid, char const *role, char const *drawing_area, const reply_func &reply);
    void api_deactivate_window(char const *appid, char const *role, const reply_func &reply);
    void api_enddraw(char const *appid, char const *role);
    int  api_subscribe(afb_req req, int event_id);
    result<json_object *> api_get_display_info();
    result<json_object *> api_get_area_info(char const *role);
    void api_ping();
    void send_event(const std::string& evname, const std::string& role);
    void send_event(const std::string& evname, const std::string& role, const std::string& area, int x, int y, int w, int h);

    // Events from the compositor we are interested in
    void surface_created(unsigned surface_id);
    void surface_removed(unsigned surface_id);

    void removeClient(const std::string &appid);
    void exceptionProcessForTransition();
    const char* convertRoleOldToNew(char const *role);

    // Do not use this function
    void timerHandler();
    void startTransitionWrapper(std::vector<WMAction> &actions);
    void processError(WMError error);

  private:
    // WM Events to clients
    void emit_activated(const std::string& role);
    void emit_deactivated(const std::string& role);
    void emit_syncdraw(const std::string& role, char const *area, int x, int y, int w, int h);
    void emit_syncdraw(const std::string &role, const std::string &area);
    void emit_flushdraw(const std::string& role);
    void emit_visible(const std::string& role, bool is_visible);
    void emit_invisible(const std::string& role);
    void emit_visible(const std::string& role);

    WMError setRequest(const std::string &appid, const std::string &role, const std::string &area,
                             Task task, unsigned *req_num);
    WMError checkPolicy(unsigned req_num);
    WMError startTransition(unsigned req_num);

    WMError doEndDraw(unsigned req_num);
    void    emitScreenUpdated(unsigned req_num);

    void setTimer();
    void stopTimer();
    void processNextRequest();

    int loadOldRoleDb();

    const char *check_surface_exist(const char *role);

  private:
    std::map<std::string, struct afb_event> map_afb_event;
    std::unordered_map<std::string, struct rect> area2size;
    std::unordered_map<std::string, std::string> roleold2new;
    std::unordered_map<std::string, std::string> rolenew2old;
    std::shared_ptr<LayerControl> lc;
    PMWrapper pmw;
    rect_map area_info;
    struct id_allocator id_alloc;

    // ID allocation and proxy methods for lookup
    std::unordered_map<unsigned, struct TmpClient> tmp_surface2app;
    static const char* kDefaultOldRoleDb;
};

} // namespace wm

#endif // WINDOW_MANAGER_HPP
