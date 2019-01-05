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

    // Store my context for calling callback from PolicyManager
    g_context = this;

    // Initialize PMWrapper
    this->pmw.initialize();

    // Register callback to PolicyManager
    this->pmw.registerCallback(onStateTransitioned, onError);

    // Make afb event for subscriber
    for (int i = Event_ScreenUpdated; i < Event_Error; i++)
    {
        map_afb_event[kListEventName[i]] = afb_api_make_event(afbBindingV3root, kListEventName[i].c_str());
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

    return 0;
}

result<int> WindowManager::api_request_surface(char const *appid, char const *drawing_name)
{
    string str_id = appid;
    string role = drawing_name;
    unsigned lid = 0;

    if(!g_app_list.contains(str_id))
    {
        lid = this->lc->getNewLayerID(role);
        if (lid == 0)
        {
            // register drawing_name as fallback and make it displayed.
            lid = this->lc->getNewLayerID(string("fallback"));
            HMI_DEBUG("%s is not registered in layers.json, then fallback as normal app", role);
            if (lid == 0)
            {
                return Err<int>("Designated role does not match any role, fallback is disabled");
            }
        }
        this->lc->createNewLayer(lid);
        // add client into the db
        g_app_list.addClient(str_id, lid, role);
    }

    // generate surface ID for ivi-shell application
    auto rname = this->id_alloc.lookup(role);
    if (!rname)
    {
        // name does not exist yet, allocate surface id...
        auto id = int(this->id_alloc.generate_id(role));
        this->tmp_surface2app[id] = {str_id, lid};

        auto client = g_app_list.lookUpClient(str_id);
        client->registerSurface(id);

        return Ok<int>(id);
    }

    // Check currently registered drawing names if it is already there.
    return Err<int>("Surface already present");
}

char const *WindowManager::api_request_surface(char const *appid, char const *drawing_name,
                                     char const *ivi_id)
{
    string str_id   = appid;
    string role = drawing_name;

    unsigned sid = std::stol(ivi_id);
    HMI_DEBUG("This API(requestSurfaceXDG) is for XDG Application using runXDG");
    /*
     * IVI-shell doesn't send surface_size event via ivi-wm protocol
     * if the application is using XDG surface.
     * So WM has to set surface size with original size here
     */
    WMError ret = this->lc->setXDGSurfaceOriginSize(sid);
    if(ret != SUCCESS)
    {
        HMI_ERROR("%s", errorDescription(ret));
        HMI_WARNING("The main user of this API is runXDG");
        return "fail";
    }

    if(!g_app_list.contains(str_id))
    {
        unsigned l_id = this->lc->getNewLayerID(role);
        if (l_id == 0)
        {
            // register drawing_name as fallback and make it displayed.
            l_id = this->lc->getNewLayerID("fallback");
            HMI_DEBUG("%s is not registered in layers.json, then fallback as normal app", role);
            if (l_id == 0)
            {
                return "Designated role does not match any role, fallback is disabled";
            }
        }
        this->lc->createNewLayer(l_id);
        // add client into the db
        g_app_list.addClient(str_id, l_id, role);
    }

    auto rname = this->id_alloc.lookup(role);

    if (rname)
    {
        return "Surface already present";
    }

    // register pair drawing_name and ivi_id
    this->id_alloc.register_name_id(role, sid);

    auto client = g_app_list.lookUpClient(str_id);
    client->addSurface(sid);

    return nullptr;
}

void WindowManager::api_activate_window(char const *appid, char const *drawing_name,
                               char const *drawing_area, const reply_func &reply)
{
    string id = appid;
    string role = drawing_name;
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

     // Do allocate tasks
    ret = this->checkPolicy(req_num);

    if (ret != WMError::SUCCESS)
    {
        //this->emit_error()
        HMI_SEQ_ERROR(req_num, errorDescription(ret));
        g_app_list.removeRequest(req_num);
        this->processNextRequest();
    }
}

void WindowManager::api_deactivate_window(char const *appid, char const *drawing_name,
                                 const reply_func &reply)
{
    // Check Phase
    string id = appid;
    string role = drawing_name;
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

    // Do allocate tasks
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
    string id = appid;
    string role = drawing_name;
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
            this->lc->undoUpdate();
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

bool WindowManager::api_subscribe(afb_req_t req, EventType event_id)
{
    bool ret = false;
    char* appid = afb_req_get_application_id(req);
    if(event_id < Event_Val_Min || event_id > Event_Val_Max)
    {
        HMI_ERROR("not defined in Window Manager", event_id);
        return ret;
    }
    HMI_INFO("%s subscribe %s : %d", appid, kListEventName[event_id], event_id);
    if(event_id == Event_ScreenUpdated)
    {
        // Event_ScreenUpdated should be emitted to subscriber
        afb_event_t event = this->map_afb_event[kListEventName[event_id]];
        int rc = afb_req_subscribe(req, event);
        if(rc == 0)
        {
            ret = true;
        }
    }
    else if(appid)
    {
        string id = appid;
        free(appid);
        auto client = g_app_list.lookUpClient(id);
        if(client != nullptr)
        {
            ret = client->subscribe(req, kListEventName[event_id]);
        }
    }
    return ret;
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

    string role = drawing_name;

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

/**
 * proxied events
 */
void WindowManager::surface_created(unsigned surface_id)
{
    // requestSurface
    if(this->tmp_surface2app.count(surface_id) != 0)
    {
        string appid = this->tmp_surface2app[surface_id].appid;
        auto client = g_app_list.lookUpClient(appid);
        if(client != nullptr)
        {
            WMError ret = client->addSurface(surface_id);
            HMI_INFO("Add surface %d to \"%s\"", surface_id, appid.c_str());
            if(ret != WMError::SUCCESS)
            {
                HMI_ERROR("Failed to add surface to client %s", client->appID().c_str());
            }
        }
        this->tmp_surface2app.erase(surface_id);
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
    auto client = g_app_list.lookUpClient(appid);
    this->lc->appTerminated(client);
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
            auto const &surface_id = this->id_alloc.lookup(act.role);
            if(surface_id == nullopt)
            {
                goto proc_remove_request;
            }
            string appid = g_app_list.getAppID(*surface_id, &found);
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

    g_app_list.reqDump();
    for (const auto &action : actions)
    {
        if (action.visible == TaskVisible::VISIBLE)
        {
            sync_draw_happen = true;
            struct rect r = this->lc->getAreaSize(action.area);
            action.client->emitSyncDraw(action.area, r);
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
            x.client->emitActive(false);
            x.client->emitVisible(false);
        }
        this->lc->renderLayers();
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

            act.client->emitActive((act.visible == VISIBLE));
            act.client->emitVisible((act.visible == VISIBLE));

            if (ret != WMError::SUCCESS)
            {
                HMI_SEQ_WARNING(req_num,
                    "Failed to manipulate surfaces while state change : %s", errorDescription(ret));
                return ret;
            }
            HMI_SEQ_DEBUG(req_num, "visible %s", act.role.c_str());
        }
    }
    this->lc->renderLayers();

    HMI_SEQ_INFO(req_num, "emit flushDraw");

    for(const auto &act_flush : actions)
    {
        if(act_flush.visible == TaskVisible::VISIBLE)
        {
            act_flush.client->emitFlushDraw();
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
    if (!found)
    {
        HMI_SEQ_ERROR(req_num,
                      "Window Manager bug :%s : Action is not set",
                      errorDescription(WMError::NO_ENTRY));
        return;
    }

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
    if (ret < 0)
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
        int ret = sd_event_add_time(afb_api_get_event_loop(afbBindingV3root), &g_timer_ev_src,
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

} // namespace wm
