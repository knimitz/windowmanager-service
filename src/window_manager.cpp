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

#include <fstream>
#include <regex>

#include "window_manager.hpp"
#include "json_helper.hpp"
#include "applist.hpp"

extern "C"
{
#include <systemd/sd-event.h>
}

using std::string;
using std::vector;

namespace wm
{

static const uint64_t kTimeOut = 3ULL; /* 3s */

/* DrawingArea name used by "{layout}.{area}" */
const char kNameLayoutNormal[] = "normal";
const char kNameLayoutSplit[]  = "split";
const char kNameAreaFull[]     = "full";
const char kNameAreaMain[]     = "main";
const char kNameAreaSub[]      = "sub";

/* Key for json obejct */
const char kKeyDrawingName[] = "drawing_name";
const char kKeyDrawingArea[] = "drawing_area";
const char kKeyDrawingRect[] = "drawing_rect";
const char kKeyX[]           = "x";
const char kKeyY[]           = "y";
const char kKeyWidth[]       = "width";
const char kKeyHeight[]      = "height";
const char kKeyWidthPixel[]  = "width_pixel";
const char kKeyHeightPixel[] = "height_pixel";
const char kKeyWidthMm[]     = "width_mm";
const char kKeyHeightMm[]    = "height_mm";
const char kKeyScale[]       = "scale";
const char kKeyIds[]         = "ids";

static const vector<string> kListEventName{
        "active",
        "inactive",
        "visible",
        "invisible",
        "syncDraw",
        "flushDraw",
        "screenUpdated",
        "error"};

static sd_event_source *g_timer_ev_src = nullptr;
static AppList g_app_list;
static WindowManager *g_context;

namespace
{

static int processTimerHandler(sd_event_source *s, uint64_t usec, void *userdata)
{
    HMI_NOTICE("Time out occurs because the client replys endDraw slow, so revert the request");
    reinterpret_cast<wm::WindowManager *>(userdata)->timerHandler();
    return 0;
}

static void onStateTransitioned(vector<WMAction> actions)
{
    g_context->startTransitionWrapper(actions);
}

static void onError()
{
    g_context->processError(WMError::LAYOUT_CHANGE_FAIL);
}
} // namespace

/**
 * WindowManager Impl
 */
WindowManager::WindowManager()
    : id_alloc{}
{
    const char *path = getenv("AFM_APP_INSTALL_DIR");
    if (!path)
    {
        HMI_ERROR("AFM_APP_INSTALL_DIR is not defined");
    }
    string root = path;

    this->lc = std::make_shared<LayerControl>(root);

    HMI_DEBUG("Layer Controller initialized");
}

int WindowManager::init()
{
    LayerControlCallbacks lmcb;
    lmcb.surfaceCreated = [&](unsigned pid, unsigned surface){
        this->surface_created(surface);
        };
    lmcb.surfaceDestroyed = [&](unsigned surface){
        this->surface_removed(surface);
    };

    if(this->lc->init(lmcb) != WMError::SUCCESS)
    {
        return -1;
    }

    // TODO: application requests by old role,
    //       so create role map (old, new)
    // Load old_role.db
    this->loadOldRoleDb();

    // Store my context for calling callback from PolicyManager
    g_context = this;

    // Initialize PMWrapper
    this->pmw.initialize();

    // Register callback to PolicyManager
    this->pmw.registerCallback(onStateTransitioned, onError);

    // Make afb event
    for (int i = Event_Val_Min; i <= Event_Val_Max; i++)
    {
        map_afb_event[kListEventName[i]] = afb_daemon_make_event(kListEventName[i].c_str());
    }

    const struct rect css_bg = this->lc->getAreaSize("fullscreen");
    Screen screen = this->lc->getScreenInfo();
    rectangle dp_bg(screen.width(), screen.height());

    dp_bg.set_aspect(static_cast<double>(css_bg.w) / css_bg.h);
    dp_bg.fit(screen.width(), screen.height());
    dp_bg.center(screen.width(), screen.height());
    HMI_DEBUG("SCALING: CSS BG(%dx%d) -> DDP %dx%d,(%dx%d)",
              css_bg.w, css_bg.h, dp_bg.left(), dp_bg.top(), dp_bg.width(), dp_bg.height());

    double scale = static_cast<double>(dp_bg.height()) / css_bg.h;
    this->lc->setupArea(dp_bg, scale);

    this->lc->createLayers();

    return 0;
}

result<int> WindowManager::api_request_surface(char const *appid, char const *drawing_name)
{
    // TODO: application requests by old role,
    //       so convert role old to new
    const char *role = this->convertRoleOldToNew(drawing_name);

    // auto lid = this->layers.get_layer_id(string(role));
    unsigned lid = this->lc->getLayerID(string(role));
    if (lid == 0)
    {
        /**
       * register drawing_name as fallback and make it displayed.
       */
        // lid = this->layers.get_layer_id(string("fallback"));
        lid = this->lc->getLayerID(string("fallback"));
        HMI_DEBUG("%s is not registered in layers.json, then fallback as normal app", role);
        if (lid == 0)
        {
            return Err<int>("Drawing name does not match any role, fallback is disabled");
        }
    }

    auto rname = this->id_alloc.lookup(role);
    if (!rname)
    {
        // name does not exist yet, allocate surface id...
        auto id = int(this->id_alloc.generate_id(role));
        // this->layers.add_surface(id, *lid);
        this->tmp_surface2app[id] = {string(appid), lid};

        // add client into the db
        string appid_str(appid);
        g_app_list.addClient(appid_str, lid, id, string(role));

        // Set role map of (new, old)
        this->rolenew2old[role] = string(drawing_name);

        return Ok<int>(id);
    }

    // Check currently registered drawing names if it is already there.
    return Err<int>("Surface already present");
}

char const *WindowManager::api_request_surface(char const *appid, char const *drawing_name,
                                     char const *ivi_id)
{
    // TODO: application requests by old role,
    //       so convert role old to new
    const char *role = this->convertRoleOldToNew(drawing_name);

    unsigned sid = std::stol(ivi_id);
    WMError ret = this->lc->setXDGSurfaceOriginSize(sid);
    if(ret != SUCCESS)
    {
        HMI_ERROR("%s", errorDescription(ret));
        HMI_WARNING("The main user of this API is runXDG");
        return "fail";
    }

    // auto lid = this->layers.get_layer_id(string(role));
    auto lid = this->lc->getLayerID(string(role));
    if (lid == 0)
    {
        /**
       * register drawing_name as fallback and make it displayed.
       */
        lid = this->lc->getLayerID(string("fallback"));
        HMI_DEBUG("%s is not registered in layers.json, then fallback as normal app", role);
        if (lid == 0)
        {
            return "Drawing name does not match any role, fallback is disabled";
        }
    }

    auto rname = this->id_alloc.lookup(role);

    if (rname)
    {
        return "Surface already present";
    }

    // register pair drawing_name and ivi_id
    this->id_alloc.register_name_id(role, sid);
    this->lc->addSurface(sid, lid);

    HMI_DEBUG("surface_id is %u, layer_id is %u", sid, lid);

    // add client into the list
    string appid_str(appid);
    g_app_list.addClient(appid_str, lid, sid, string(role));

    // Set role map of (new, old)
    this->rolenew2old[role] = string(drawing_name);

    return nullptr;
}

void WindowManager::api_activate_surface(char const *appid, char const *drawing_name,
                               char const *drawing_area, const reply_func &reply)
{
    // TODO: application requests by old role,
    //       so convert role old to new
    const char *c_role = this->convertRoleOldToNew(drawing_name);

    string id = appid;
    string role = c_role;
    string area = drawing_area;

    if(!g_app_list.contains(id))
    {
        reply("app doesn't request 'requestSurface' or 'setRole' yet");
        return;
    }

    auto client = g_app_list.lookUpClient(id);
    Task task = Task::TASK_ALLOCATE;
    unsigned req_num = 0;
    WMError ret = WMError::UNKNOWN;

    ret = this->setRequest(id, role, area, task, &req_num);

    if(ret != WMError::SUCCESS)
    {
        HMI_ERROR(errorDescription(ret));
        reply("Failed to set request");
        return;
    }

    reply(nullptr);
    if (req_num != g_app_list.currentRequestNumber())
    {
        // Add request, then invoked after the previous task is finished
        HMI_SEQ_DEBUG(req_num, "request is accepted");
        return;
    }

    /*
     * Do allocate tasks
     */
    ret = this->checkPolicy(req_num);

    if (ret != WMError::SUCCESS)
    {
        //this->emit_error()
        HMI_SEQ_ERROR(req_num, errorDescription(ret));
        g_app_list.removeRequest(req_num);
        this->processNextRequest();
    }
}

void WindowManager::api_deactivate_surface(char const *appid, char const *drawing_name,
                                 const reply_func &reply)
{
    // TODO: application requests by old role,
    //       so convert role old to new
    const char *c_role = this->convertRoleOldToNew(drawing_name);

    /*
    * Check Phase
    */
    string id = appid;
    string role = c_role;
    string area = ""; //drawing_area;
    Task task = Task::TASK_RELEASE;
    unsigned req_num = 0;
    WMError ret = WMError::UNKNOWN;

    ret = this->setRequest(id, role, area, task, &req_num);

    if (ret != WMError::SUCCESS)
    {
        HMI_ERROR(errorDescription(ret));
        reply("Failed to set request");
        return;
    }

    reply(nullptr);
    if (req_num != g_app_list.currentRequestNumber())
    {
        // Add request, then invoked after the previous task is finished
        HMI_SEQ_DEBUG(req_num, "request is accepted");
        return;
    }

    /*
    * Do allocate tasks
    */
    ret = this->checkPolicy(req_num);

    if (ret != WMError::SUCCESS)
    {
        //this->emit_error()
        HMI_SEQ_ERROR(req_num, errorDescription(ret));
        g_app_list.removeRequest(req_num);
        this->processNextRequest();
    }
}

void WindowManager::api_enddraw(char const *appid, char const *drawing_name)
{
    // TODO: application requests by old role,
    //       so convert role old to new
    const char *c_role = this->convertRoleOldToNew(drawing_name);

    string id = appid;
    string role = c_role;
    unsigned current_req = g_app_list.currentRequestNumber();
    bool result = g_app_list.setEndDrawFinished(current_req, id, role);

    if (!result)
    {
        HMI_ERROR("%s is not in transition state", id.c_str());
        return;
    }

    if (g_app_list.endDrawFullfilled(current_req))
    {
        // do task for endDraw
        this->stopTimer();
        WMError ret = this->doEndDraw(current_req);

        if(ret != WMError::SUCCESS)
        {
            //this->emit_error();

            // Undo state of PolicyManager
            this->pmw.undoState();
        }
        this->emitScreenUpdated(current_req);
        HMI_SEQ_INFO(current_req, "Finish request status: %s", errorDescription(ret));

        g_app_list.removeRequest(current_req);

        this->processNextRequest();
    }
    else
    {
        HMI_SEQ_INFO(current_req, "Wait other App call endDraw");
        return;
    }
}

int WindowManager::api_subscribe(afb_req req, int event_id)
{
    struct afb_event event = this->map_afb_event[kListEventName[event_id]];
    return afb_req_subscribe(req, event);
}

result<json_object *> WindowManager::api_get_display_info()
{
    Screen screen = this->lc->getScreenInfo();

    json_object *object = json_object_new_object();
    json_object_object_add(object, kKeyWidthPixel, json_object_new_int(screen.width()));
    json_object_object_add(object, kKeyHeightPixel, json_object_new_int(screen.height()));
    // TODO: set size
    json_object_object_add(object, kKeyWidthMm, json_object_new_int(0));
    json_object_object_add(object, kKeyHeightMm, json_object_new_int(0));
    json_object_object_add(object, kKeyScale, json_object_new_double(this->lc->scale()));

    return Ok<json_object *>(object);
}

result<json_object *> WindowManager::api_get_area_info(char const *drawing_name)
{
    HMI_DEBUG("called");

    // TODO: application requests by old role,
    //       so convert role old to new
    const char *role = this->convertRoleOldToNew(drawing_name);

    // Check drawing name, surface/layer id
    auto const &surface_id = this->id_alloc.lookup(role);
    if (!surface_id)
    {
        return Err<json_object *>("Surface does not exist");
    }

    // Set area rectangle
    struct rect area_info = this->area_info[*surface_id];
    json_object *object = json_object_new_object();
    json_object_object_add(object, kKeyX, json_object_new_int(area_info.x));
    json_object_object_add(object, kKeyY, json_object_new_int(area_info.y));
    json_object_object_add(object, kKeyWidth, json_object_new_int(area_info.w));
    json_object_object_add(object, kKeyHeight, json_object_new_int(area_info.h));

    return Ok<json_object *>(object);
}

void WindowManager::send_event(const string& evname, const string& role)
{
    json_object *j = json_object_new_object();
    json_object_object_add(j, kKeyDrawingName, json_object_new_string(role.c_str()));

    int ret = afb_event_push(this->map_afb_event[evname], j);
    if (ret != 0)
    {
        HMI_DEBUG("afb_event_push failed: %m");
    }
}

void WindowManager::send_event(const string& evname, const string& role, const string& area,
                     int x, int y, int w, int h)
{
    json_object *j_rect = json_object_new_object();
    json_object_object_add(j_rect, kKeyX, json_object_new_int(x));
    json_object_object_add(j_rect, kKeyY, json_object_new_int(y));
    json_object_object_add(j_rect, kKeyWidth, json_object_new_int(w));
    json_object_object_add(j_rect, kKeyHeight, json_object_new_int(h));

    json_object *j = json_object_new_object();
    json_object_object_add(j, kKeyDrawingName, json_object_new_string(role.c_str()));
    json_object_object_add(j, kKeyDrawingArea, json_object_new_string(area.c_str()));
    json_object_object_add(j, kKeyDrawingRect, j_rect);

    int ret = afb_event_push(this->map_afb_event[evname], j);
    if (ret != 0)
    {
        HMI_DEBUG("afb_event_push failed: %m");
    }
}

/**
 * proxied events
 */
void WindowManager::surface_created(unsigned surface_id)
{
    // requestSurface
    if(this->tmp_surface2app.count(surface_id) != 0)
    {
        unsigned layer_id = this->tmp_surface2app[surface_id].layer;
        this->lc->addSurface(surface_id, layer_id);
        this->tmp_surface2app.erase(surface_id);
        HMI_DEBUG("surface_id is %u, layer_id is %u", surface_id, layer_id);
    }
}

void WindowManager::surface_removed(unsigned surface_id)
{
    HMI_DEBUG("Delete surface_id %u", surface_id);
    this->id_alloc.remove_id(surface_id);
    g_app_list.removeSurface(surface_id);
}

void WindowManager::removeClient(const string &appid)
{
    HMI_DEBUG("Remove clinet %s from list", appid.c_str());
    g_app_list.removeClient(appid);
}

void WindowManager::exceptionProcessForTransition()
{
    unsigned req_num = g_app_list.currentRequestNumber();
    HMI_SEQ_NOTICE(req_num, "Process exception handling for request. Remove current request %d", req_num);
    g_app_list.removeRequest(req_num);
    HMI_SEQ_NOTICE(g_app_list.currentRequestNumber(), "Process next request if exists");
    this->processNextRequest();
}

void WindowManager::timerHandler()
{
    unsigned req_num = g_app_list.currentRequestNumber();
    HMI_SEQ_DEBUG(req_num, "Timer expired remove Request");
    g_app_list.reqDump();
    g_app_list.removeRequest(req_num);
    this->processNextRequest();
}

void WindowManager::startTransitionWrapper(vector<WMAction> &actions)
{
    WMError ret;
    unsigned req_num = g_app_list.currentRequestNumber();

    if (actions.empty())
    {
        if (g_app_list.haveRequest())
        {
            HMI_SEQ_DEBUG(req_num, "There is no WMAction for this request");
            goto proc_remove_request;
        }
        else
        {
            HMI_SEQ_DEBUG(req_num, "There is no request");
            return;
        }
    }

    for (auto &act : actions)
    {
        if ("" != act.role)
        {
            bool found;
            auto const &surface_id = this->id_alloc.lookup(act.role.c_str());
            if(surface_id == nullopt)
            {
                goto proc_remove_request;
            }
            string appid = g_app_list.getAppID(*surface_id, act.role, &found);
            if (!found)
            {
                if (TaskVisible::INVISIBLE == act.visible)
                {
                    // App is killed, so do not set this action
                    continue;
                }
                else
                {
                    HMI_SEQ_ERROR(req_num, "appid which is visible is not found");
                    ret = WMError::FAIL;
                    goto error;
                }
            }
            auto client = g_app_list.lookUpClient(appid);
            act.req_num = req_num;
            act.client = client;
        }

        ret = g_app_list.setAction(req_num, act);
        if (ret != WMError::SUCCESS)
        {
            HMI_SEQ_ERROR(req_num, "Setting action is failed");
            goto error;
        }
    }

    HMI_SEQ_DEBUG(req_num, "Start transition.");
    ret = this->startTransition(req_num);
    if (ret != WMError::SUCCESS)
    {
        if (ret == WMError::NO_LAYOUT_CHANGE)
        {
            goto proc_remove_request;
        }
        else
        {
            HMI_SEQ_ERROR(req_num, "Transition state is failed");
            goto error;
        }
    }

    return;

error:
    //this->emit_error()
    HMI_SEQ_ERROR(req_num, errorDescription(ret));
    this->pmw.undoState();

proc_remove_request:
    g_app_list.removeRequest(req_num);
    this->processNextRequest();
}

void WindowManager::processError(WMError error)
{
    unsigned req_num = g_app_list.currentRequestNumber();

    //this->emit_error()
    HMI_SEQ_ERROR(req_num, errorDescription(error));
    g_app_list.removeRequest(req_num);
    this->processNextRequest();
}

/*
 ******* Private Functions *******
 */

void WindowManager::emit_activated(const string& role)
{
    this->send_event(kListEventName[Event_Active], role);
}

void WindowManager::emit_deactivated(const string& role)
{
    this->send_event(kListEventName[Event_Inactive], role);
}

void WindowManager::emit_syncdraw(const string& role, char const *area, int x, int y, int w, int h)
{
    this->send_event(kListEventName[Event_SyncDraw], role, area, x, y, w, h);
}

void WindowManager::emit_syncdraw(const string &role, const string &area)
{
    struct rect rect = this->lc->getAreaSize(area);
    this->send_event(kListEventName[Event_SyncDraw],
        role.c_str(), area.c_str(), rect.x, rect.y, rect.w, rect.h);
}

void WindowManager::emit_flushdraw(const string& role)
{
    this->send_event(kListEventName[Event_FlushDraw], role);
}

void WindowManager::emit_visible(const string& role, bool is_visible)
{
    this->send_event(is_visible ? kListEventName[Event_Visible] : kListEventName[Event_Invisible], role);
}

void WindowManager::emit_invisible(const string& role)
{
    return emit_visible(role, false);
}

void WindowManager::emit_visible(const string& role) { return emit_visible(role, true); }

WMError WindowManager::setRequest(const string& appid, const string &role, const string &area,
                            Task task, unsigned* req_num)
{
    if (!g_app_list.contains(appid))
    {
        return WMError::NOT_REGISTERED;
    }

    auto client = g_app_list.lookUpClient(appid);

    /*
     * Queueing Phase
     */
    unsigned current = g_app_list.currentRequestNumber();
    unsigned requested_num = g_app_list.getRequestNumber(appid);
    if (requested_num != 0)
    {
        HMI_SEQ_INFO(requested_num,
            "%s %s %s request is already queued", appid.c_str(), role.c_str(), area.c_str());
        return REQ_REJECTED;
    }

    WMRequest req = WMRequest(appid, role, area, task);
    unsigned new_req = g_app_list.addRequest(req);
    *req_num = new_req;
    g_app_list.reqDump();

    HMI_SEQ_DEBUG(current, "%s start sequence with %s, %s", appid.c_str(), role.c_str(), area.c_str());

    return WMError::SUCCESS;
}

WMError WindowManager::checkPolicy(unsigned req_num)
{
    /*
    * Check Policy
    */
    // get current trigger
    bool found = false;
    WMError ret = WMError::LAYOUT_CHANGE_FAIL;
    auto trigger = g_app_list.getRequest(req_num, &found);
    if (!found)
    {
        ret = WMError::NO_ENTRY;
        return ret;
    }
    string req_area = trigger.area;

    if (trigger.task == Task::TASK_ALLOCATE)
    {
        const char *msg = this->check_surface_exist(trigger.role.c_str());

        if (msg)
        {
            HMI_SEQ_ERROR(req_num, msg);
            return ret;
        }
    }

    // Input event data to PolicyManager
    if (0 > this->pmw.setInputEventData(trigger.task, trigger.role, trigger.area))
    {
        HMI_SEQ_ERROR(req_num, "Failed to set input event data to PolicyManager");
        return ret;
    }

    // Execute state transition of PolicyManager
    if (0 > this->pmw.executeStateTransition())
    {
        HMI_SEQ_ERROR(req_num, "Failed to execute state transition of PolicyManager");
        return ret;
    }

    ret = WMError::SUCCESS;

    g_app_list.reqDump();

    return ret;
}

WMError WindowManager::startTransition(unsigned req_num)
{
    bool sync_draw_happen = false;
    bool found = false;
    WMError ret = WMError::SUCCESS;
    auto actions = g_app_list.getActions(req_num, &found);
    if (!found)
    {
        ret = WMError::NO_ENTRY;
        HMI_SEQ_ERROR(req_num,
            "Window Manager bug :%s : Action is not set", errorDescription(ret));
        return ret;
    }

    for (const auto &action : actions)
    {
        if (action.visible == TaskVisible::VISIBLE)
        {
            sync_draw_happen = true;

            // TODO: application requests by old role,
            //       so convert role new to old for emitting event
            string old_role = this->rolenew2old[action.role];

            this->emit_syncdraw(old_role, action.area);
            /* TODO: emit event for app not subscriber
            if(g_app_list.contains(y.appid))
                g_app_list.lookUpClient(y.appid)->emit_syncdraw(y.role, y.area); */
        }
    }

    if (sync_draw_happen)
    {
        this->setTimer();
    }
    else
    {
        // deactivate only, no syncDraw
        // Make it deactivate here
        for (const auto &x : actions)
        {
            this->lc->visibilityChange(x);
            string old_role = this->rolenew2old[x.role];
            emit_deactivated(old_role.c_str());
            /* if (g_app_list.contains(x.client->appID()))
            {
                auto client = g_app_list.lookUpClient(x.client->appID());
                this->deactivate(client->surfaceID(x.role));
            } */
        }
        ret = WMError::NO_LAYOUT_CHANGE;
    }
    return ret;
}

WMError WindowManager::doEndDraw(unsigned req_num)
{
    // get actions
    bool found;
    auto actions = g_app_list.getActions(req_num, &found);
    WMError ret = WMError::SUCCESS;
    if (!found)
    {
        ret = WMError::NO_ENTRY;
        return ret;
    }

    HMI_SEQ_INFO(req_num, "do endDraw");

    // layout change and make it visible
    for (const auto &act : actions)
    {
        if(act.visible != TaskVisible::NO_CHANGE)
        {
            // layout change
            ret = this->lc->layoutChange(act);
            if(ret != WMError::SUCCESS)
            {
                HMI_SEQ_WARNING(req_num,
                    "Failed to manipulate surfaces while state change : %s", errorDescription(ret));
                return ret;
            }
            ret = this->lc->visibilityChange(act);

            // Emit active/deactive event
            string old_role = this->rolenew2old[act.role];
            if(act.visible == VISIBLE)
            {
                emit_activated(old_role.c_str());
            }
            else
            {
                emit_deactivated(old_role.c_str());
            }

            if (ret != WMError::SUCCESS)
            {
                HMI_SEQ_WARNING(req_num,
                    "Failed to manipulate surfaces while state change : %s", errorDescription(ret));
                return ret;
            }
            HMI_SEQ_DEBUG(req_num, "visible %s", act.role.c_str());
        }
    }

    HMI_SEQ_INFO(req_num, "emit flushDraw");

    for(const auto &act_flush : actions)
    {
        if(act_flush.visible == TaskVisible::VISIBLE)
        {
            // TODO: application requests by old role,
            //       so convert role new to old for emitting event
            string old_role = this->rolenew2old[act_flush.role];

            this->emit_flushdraw(old_role.c_str());
        }
    }

    return ret;
}

void WindowManager::emitScreenUpdated(unsigned req_num)
{
    // Get visible apps
    HMI_SEQ_DEBUG(req_num, "emit screen updated");
    bool found = false;
    auto actions = g_app_list.getActions(req_num, &found);

    // create json object
    json_object *j = json_object_new_object();
    json_object *jarray = json_object_new_array();

    for(const auto& action: actions)
    {
        if(action.visible != TaskVisible::INVISIBLE)
        {
            json_object_array_add(jarray, json_object_new_string(action.client->appID().c_str()));
        }
    }
    json_object_object_add(j, kKeyIds, jarray);
    HMI_SEQ_INFO(req_num, "Visible app: %s", json_object_get_string(j));

    int ret = afb_event_push(
        this->map_afb_event[kListEventName[Event_ScreenUpdated]], j);
    if (ret != 0)
    {
        HMI_DEBUG("afb_event_push failed: %m");
    }
}

void WindowManager::setTimer()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_BOOTTIME, &ts) != 0) {
        HMI_ERROR("Could't set time (clock_gettime() returns with error");
        return;
    }

    HMI_SEQ_DEBUG(g_app_list.currentRequestNumber(), "Timer set activate");
    if (g_timer_ev_src == nullptr)
    {
        // firsttime set into sd_event
        int ret = sd_event_add_time(afb_daemon_get_event_loop(), &g_timer_ev_src,
            CLOCK_BOOTTIME, (uint64_t)(ts.tv_sec + kTimeOut) * 1000000ULL, 1, processTimerHandler, this);
        if (ret < 0)
        {
            HMI_ERROR("Could't set timer");
        }
    }
    else
    {
        // update timer limitation after second time
        sd_event_source_set_time(g_timer_ev_src, (uint64_t)(ts.tv_sec + kTimeOut) * 1000000ULL);
        sd_event_source_set_enabled(g_timer_ev_src, SD_EVENT_ONESHOT);
    }
}

void WindowManager::stopTimer()
{
    unsigned req_num = g_app_list.currentRequestNumber();
    HMI_SEQ_DEBUG(req_num, "Timer stop");
    int rc = sd_event_source_set_enabled(g_timer_ev_src, SD_EVENT_OFF);
    if (rc < 0)
    {
        HMI_SEQ_ERROR(req_num, "Timer stop failed");
    }
}

void WindowManager::processNextRequest()
{
    g_app_list.next();
    g_app_list.reqDump();
    unsigned req_num = g_app_list.currentRequestNumber();
    if (g_app_list.haveRequest())
    {
        HMI_SEQ_DEBUG(req_num, "Process next request");
        WMError rc = checkPolicy(req_num);
        if (rc != WMError::SUCCESS)
        {
            HMI_SEQ_ERROR(req_num, errorDescription(rc));
        }
    }
    else
    {
        HMI_SEQ_DEBUG(req_num, "Nothing Request. Waiting Request");
    }
}

const char* WindowManager::convertRoleOldToNew(char const *old_role)
{
    const char *new_role = nullptr;

    for (auto const &on : this->roleold2new)
    {
        std::regex regex = std::regex(on.first);
        if (std::regex_match(old_role, regex))
        {
            // role is old. So convert to new.
            new_role = on.second.c_str();
            break;
        }
    }

    if (nullptr == new_role)
    {
        // role is new or fallback.
        new_role = old_role;
    }

    HMI_DEBUG("old:%s -> new:%s", old_role, new_role);

    return new_role;
}

int WindowManager::loadOldRoleDb()
{
    // Get afm application installed dir
    char const *afm_app_install_dir = getenv("AFM_APP_INSTALL_DIR");
    HMI_DEBUG("afm_app_install_dir:%s", afm_app_install_dir);

    string file_name;
    if (!afm_app_install_dir)
    {
        HMI_ERROR("AFM_APP_INSTALL_DIR is not defined");
    }
    else
    {
        file_name = string(afm_app_install_dir) + string("/etc/old_roles.db");
    }

    // Load old_role.db
    json_object* json_obj;
    int ret = jh::inputJsonFilie(file_name.c_str(), &json_obj);
    if (0 > ret)
    {
        HMI_ERROR("Could not open old_role.db, so use default old_role information");
        json_obj = json_tokener_parse(kDefaultOldRoleDb);
    }
    HMI_DEBUG("json_obj dump:%s", json_object_get_string(json_obj));

    // Perse apps
    json_object* json_cfg;
    if (!json_object_object_get_ex(json_obj, "old_roles", &json_cfg))
    {
        HMI_ERROR("Parse Error!!");
        return -1;
    }

    int len = json_object_array_length(json_cfg);
    HMI_DEBUG("json_cfg len:%d", len);
    HMI_DEBUG("json_cfg dump:%s", json_object_get_string(json_cfg));

    for (int i=0; i<len; i++)
    {
        json_object* json_tmp = json_object_array_get_idx(json_cfg, i);

        const char* old_role = jh::getStringFromJson(json_tmp, "name");
        if (nullptr == old_role)
        {
            HMI_ERROR("Parse Error!!");
            return -1;
        }

        const char* new_role = jh::getStringFromJson(json_tmp, "new");
        if (nullptr == new_role)
        {
            HMI_ERROR("Parse Error!!");
            return -1;
        }

        this->roleold2new[old_role] = string(new_role);
    }

    // Check
    for(auto itr = this->roleold2new.begin();
      itr != this->roleold2new.end(); ++itr)
    {
        HMI_DEBUG(">>> role old:%s new:%s",
                  itr->first.c_str(), itr->second.c_str());
    }

    // Release json_object
    json_object_put(json_obj);

    return 0;
}

const char *WindowManager::check_surface_exist(const char *drawing_name)
{
    auto const &surface_id = this->id_alloc.lookup(drawing_name);
    if (!surface_id)
    {
        return "Surface does not exist";
    }
    return nullptr;
}

const char* WindowManager::kDefaultOldRoleDb = "{ \
    \"old_roles\": [ \
        { \
            \"name\": \"HomeScreen\", \
            \"new\": \"homescreen\" \
        }, \
        { \
            \"name\": \"Music\", \
            \"new\": \"music\" \
        }, \
        { \
            \"name\": \"MediaPlayer\", \
            \"new\": \"music\" \
        }, \
        { \
            \"name\": \"Video\", \
            \"new\": \"video\" \
        }, \
        { \
            \"name\": \"VideoPlayer\", \
            \"new\": \"video\" \
        }, \
        { \
            \"name\": \"WebBrowser\", \
            \"new\": \"browser\" \
        }, \
        { \
            \"name\": \"Radio\", \
            \"new\": \"radio\" \
        }, \
        { \
            \"name\": \"Phone\", \
            \"new\": \"phone\" \
        }, \
        { \
            \"name\": \"Navigation\", \
            \"new\": \"map\" \
        }, \
        { \
            \"name\": \"HVAC\", \
            \"new\": \"hvac\" \
        }, \
        { \
            \"name\": \"Settings\", \
            \"new\": \"settings\" \
        }, \
        { \
            \"name\": \"Dashboard\", \
            \"new\": \"dashboard\" \
        }, \
        { \
            \"name\": \"POI\", \
            \"new\": \"poi\" \
        }, \
        { \
            \"name\": \"Mixer\", \
            \"new\": \"mixer\" \
        }, \
        { \
            \"name\": \"Restriction\", \
            \"new\": \"restriction\" \
        }, \
        { \
            \"name\": \"^OnScreen.*\", \
            \"new\": \"on_screen\" \
        } \
    ] \
}";

} // namespace wm
