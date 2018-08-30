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

static sd_event_source *g_timer_ev_src = nullptr;
static AppList g_app_list;
static WindowManager *g_context;

namespace
{

using nlohmann::json;

result<json> file_to_json(char const *filename)
{
    json j;
    std::ifstream i(filename);
    if (i.fail())
    {
        HMI_DEBUG("wm", "Could not open config file, so use default layer information");
        j = default_layers_json;
    }
    else
    {
        i >> j;
    }

    return Ok(j);
}

struct result<layer_map> load_layer_map(char const *filename)
{
    HMI_DEBUG("wm", "loading IDs from %s", filename);

    auto j = file_to_json(filename);
    if (j.is_err())
    {
        return Err<layer_map>(j.unwrap_err());
    }
    json jids = j.unwrap();

    return to_layer_map(jids);
}

static int processTimerHandler(sd_event_source *s, uint64_t usec, void *userdata)
{
    HMI_NOTICE("wm", "Time out occurs because the client replys endDraw slow, so revert the request");
    reinterpret_cast<wm::WindowManager *>(userdata)->timerHandler();
    return 0;
}

static void onStateTransitioned(std::vector<WMAction> actions)
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
WindowManager::WindowManager(wl::display *d)
    : chooks{this},
      display{d},
      controller{},
      outputs(),
      layers(),
      id_alloc{},
      pending_events(false)
{
    char const *path_layers_json = getenv("AFM_APP_INSTALL_DIR");
    std::string path;
    if (!path_layers_json)
    {
        HMI_ERROR("wm", "AFM_APP_INSTALL_DIR is not defined");
        path = std::string(path_layers_json);
    }
    else
    {
        path = std::string(path_layers_json) + std::string("/etc/layers.json");
    }

    try
    {
        {
            auto l = load_layer_map(path.c_str());
            if (l.is_ok())
            {
                this->layers = l.unwrap();
            }
            else
            {
                HMI_ERROR("wm", "%s", l.err().value());
            }
        }
    }
    catch (std::exception &e)
    {
        HMI_ERROR("wm", "Loading of configuration failed: %s", e.what());
    }
}

int WindowManager::init()
{
    if (!this->display->ok())
    {
        return -1;
    }

    if (this->layers.mapping.empty())
    {
        HMI_ERROR("wm", "No surface -> layer mapping loaded");
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
        map_afb_event[kListEventName[i]] = afb_daemon_make_event(kListEventName[i]);
    }

    this->display->add_global_handler(
        "wl_output", [this](wl_registry *r, uint32_t name, uint32_t v) {
            this->outputs.emplace_back(std::make_unique<wl::output>(r, name, v));
        });

    this->display->add_global_handler(
        "ivi_wm", [this](wl_registry *r, uint32_t name, uint32_t v) {
            this->controller =
                std::make_unique<struct compositor::controller>(r, name, v);

            // Init controller hooks
            this->controller->chooks = &this->chooks;

            // This protocol needs the output, so lets just add our mapping here...
            this->controller->add_proxy_to_id_mapping(
                this->outputs.front()->proxy.get(),
                wl_proxy_get_id(reinterpret_cast<struct wl_proxy *>(
                    this->outputs.front()->proxy.get())));

            // Create screen
            this->controller->create_screen(this->outputs.front()->proxy.get());

            // Set display to controller
            this->controller->display = this->display;
        });

    // First level objects
    this->display->roundtrip();
    // Second level objects
    this->display->roundtrip();
    // Third level objects
    this->display->roundtrip();

    return init_layers();
}

int WindowManager::dispatch_pending_events()
{
    if (this->pop_pending_events())
    {
        this->display->dispatch_pending();
        return 0;
    }
    return -1;
}

void WindowManager::set_pending_events()
{
    this->pending_events.store(true, std::memory_order_release);
}

result<int> WindowManager::api_request_surface(char const *appid, char const *drawing_name)
{
    // TODO: application requests by old role,
    //       so convert role old to new
    const char *role = this->convertRoleOldToNew(drawing_name);

    auto lid = this->layers.get_layer_id(std::string(role));
    if (!lid)
    {
        /**
       * register drawing_name as fallback and make it displayed.
       */
        lid = this->layers.get_layer_id(std::string("fallback"));
        HMI_DEBUG("wm", "%s is not registered in layers.json, then fallback as normal app", role);
        if (!lid)
        {
            return Err<int>("Drawing name does not match any role, fallback is disabled");
        }
    }

    auto rname = this->lookup_id(role);
    if (!rname)
    {
        // name does not exist yet, allocate surface id...
        auto id = int(this->id_alloc.generate_id(role));
        this->layers.add_surface(id, *lid);

        // set the main_surface[_name] here and now
        if (!this->layers.main_surface_name.empty() &&
            this->layers.main_surface_name == drawing_name)
        {
            this->layers.main_surface = id;
            HMI_DEBUG("wm", "Set main_surface id to %u", id);
        }

        // add client into the db
        std::string appid_str(appid);
        g_app_list.addClient(appid_str, *lid, id, std::string(role));

        // Set role map of (new, old)
        this->rolenew2old[role] = std::string(drawing_name);

        return Ok<int>(id);
    }

    // Check currently registered drawing names if it is already there.
    return Err<int>("Surface already present");
}

char const *WindowManager::api_request_surface(char const *appid, char const *drawing_name,
                                     char const *ivi_id)
{
    ST();

    // TODO: application requests by old role,
    //       so convert role old to new
    const char *role = this->convertRoleOldToNew(drawing_name);

    auto lid = this->layers.get_layer_id(std::string(role));
    unsigned sid = std::stol(ivi_id);

    if (!lid)
    {
        /**
       * register drawing_name as fallback and make it displayed.
       */
        lid = this->layers.get_layer_id(std::string("fallback"));
        HMI_DEBUG("wm", "%s is not registered in layers.json, then fallback as normal app", role);
        if (!lid)
        {
            return "Drawing name does not match any role, fallback is disabled";
        }
    }

    auto rname = this->lookup_id(role);

    if (rname)
    {
        return "Surface already present";
    }

    // register pair drawing_name and ivi_id
    this->id_alloc.register_name_id(role, sid);
    this->layers.add_surface(sid, *lid);

    // this surface is already created
    HMI_DEBUG("wm", "surface_id is %u, layer_id is %u", sid, *lid);

    this->controller->layers[*lid]->add_surface(sid);
    this->layout_commit();

    // add client into the db
    std::string appid_str(appid);
    g_app_list.addClient(appid_str, *lid, sid, std::string(role));

    // Set role map of (new, old)
    this->rolenew2old[role] = std::string(drawing_name);

    return nullptr;
}

void WindowManager::api_activate_surface(char const *appid, char const *drawing_name,
                               char const *drawing_area, const reply_func &reply)
{
    ST();

    // TODO: application requests by old role,
    //       so convert role old to new
    const char *c_role = this->convertRoleOldToNew(drawing_name);

    std::string id = appid;
    std::string role = c_role;
    std::string area = drawing_area;
    Task task = Task::TASK_ALLOCATE;
    unsigned req_num = 0;
    WMError ret = WMError::UNKNOWN;

    ret = this->setRequest(id, role, area, task, &req_num);

    if(ret != WMError::SUCCESS)
    {
        HMI_ERROR("wm", errorDescription(ret));
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
    ret = this->doTransition(req_num);

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
    ST();

    // TODO: application requests by old role,
    //       so convert role old to new
    const char *c_role = this->convertRoleOldToNew(drawing_name);

    /*
    * Check Phase
    */
    std::string id = appid;
    std::string role = c_role;
    std::string area = ""; //drawing_area;
    Task task = Task::TASK_RELEASE;
    unsigned req_num = 0;
    WMError ret = WMError::UNKNOWN;

    ret = this->setRequest(id, role, area, task, &req_num);

    if (ret != WMError::SUCCESS)
    {
        HMI_ERROR("wm", errorDescription(ret));
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
    ret = this->doTransition(req_num);

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

    std::string id = appid;
    std::string role = c_role;
    unsigned current_req = g_app_list.currentRequestNumber();
    bool result = g_app_list.setEndDrawFinished(current_req, id, role);

    if (!result)
    {
        HMI_ERROR("wm", "%s is not in transition state", id.c_str());
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

result<json_object *> WindowManager::api_get_display_info()
{
    if (!this->display->ok())
    {
        return Err<json_object *>("Wayland compositor is not available");
    }

    // Set display info
    compositor::size o_size = this->controller->output_size;
    compositor::size p_size = this->controller->physical_size;

    json_object *object = json_object_new_object();
    json_object_object_add(object, kKeyWidthPixel, json_object_new_int(o_size.w));
    json_object_object_add(object, kKeyHeightPixel, json_object_new_int(o_size.h));
    json_object_object_add(object, kKeyWidthMm, json_object_new_int(p_size.w));
    json_object_object_add(object, kKeyHeightMm, json_object_new_int(p_size.h));
    json_object_object_add(object, kKeyScale, json_object_new_double(this->controller->scale));

    return Ok<json_object *>(object);
}

result<json_object *> WindowManager::api_get_area_info(char const *drawing_name)
{
    HMI_DEBUG("wm", "called");

    // TODO: application requests by old role,
    //       so convert role old to new
    const char *role = this->convertRoleOldToNew(drawing_name);

    // Check drawing name, surface/layer id
    auto const &surface_id = this->lookup_id(role);
    if (!surface_id)
    {
        return Err<json_object *>("Surface does not exist");
    }

    if (!this->controller->surface_exists(*surface_id))
    {
        return Err<json_object *>("Surface does not exist in controller!");
    }

    auto layer_id = this->layers.get_layer_id(*surface_id);
    if (!layer_id)
    {
        return Err<json_object *>("Surface is not on any layer!");
    }

    // Set area rectangle
    compositor::rect area_info = this->area_info[*surface_id];
    json_object *object = json_object_new_object();
    json_object_object_add(object, kKeyX, json_object_new_int(area_info.x));
    json_object_object_add(object, kKeyY, json_object_new_int(area_info.y));
    json_object_object_add(object, kKeyWidth, json_object_new_int(area_info.w));
    json_object_object_add(object, kKeyHeight, json_object_new_int(area_info.h));

    return Ok<json_object *>(object);
}

void WindowManager::api_ping() { this->dispatch_pending_events(); }

void WindowManager::send_event(char const *evname, char const *label)
{
    HMI_DEBUG("wm", "%s: %s(%s)", __func__, evname, label);

    json_object *j = json_object_new_object();
    json_object_object_add(j, kKeyDrawingName, json_object_new_string(label));

    int ret = afb_event_push(this->map_afb_event[evname], j);
    if (ret != 0)
    {
        HMI_DEBUG("wm", "afb_event_push failed: %m");
    }
}

void WindowManager::send_event(char const *evname, char const *label, char const *area,
                     int x, int y, int w, int h)
{
    HMI_DEBUG("wm", "%s: %s(%s, %s) x:%d y:%d w:%d h:%d",
              __func__, evname, label, area, x, y, w, h);

    json_object *j_rect = json_object_new_object();
    json_object_object_add(j_rect, kKeyX, json_object_new_int(x));
    json_object_object_add(j_rect, kKeyY, json_object_new_int(y));
    json_object_object_add(j_rect, kKeyWidth, json_object_new_int(w));
    json_object_object_add(j_rect, kKeyHeight, json_object_new_int(h));

    json_object *j = json_object_new_object();
    json_object_object_add(j, kKeyDrawingName, json_object_new_string(label));
    json_object_object_add(j, kKeyDrawingArea, json_object_new_string(area));
    json_object_object_add(j, kKeyDrawingRect, j_rect);

    int ret = afb_event_push(this->map_afb_event[evname], j);
    if (ret != 0)
    {
        HMI_DEBUG("wm", "afb_event_push failed: %m");
    }
}

/**
 * proxied events
 */
void WindowManager::surface_created(uint32_t surface_id)
{
    this->controller->get_surface_properties(surface_id, IVI_WM_PARAM_SIZE);

    auto layer_id = this->layers.get_layer_id(surface_id);
    if (!layer_id)
    {
        HMI_DEBUG("wm", "Newly created surfce %d is not associated with any layer!",
                  surface_id);
        return;
    }

    HMI_DEBUG("wm", "surface_id is %u, layer_id is %u", surface_id, *layer_id);

    this->controller->layers[*layer_id]->add_surface(surface_id);
    this->layout_commit();
}

void WindowManager::surface_removed(uint32_t surface_id)
{
    HMI_DEBUG("wm", "Delete surface_id %u", surface_id);
    this->id_alloc.remove_id(surface_id);
    this->layers.remove_surface(surface_id);
    g_app_list.removeSurface(surface_id);
}

void WindowManager::removeClient(const std::string &appid)
{
    HMI_DEBUG("wm", "Remove clinet %s from list", appid.c_str());
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

void WindowManager::startTransitionWrapper(std::vector<WMAction> &actions)
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
            auto const &surface_id = this->lookup_id(act.role.c_str());
            if(surface_id == nullopt)
            {
                goto proc_remove_request;
            }
            std::string appid = g_app_list.getAppID(*surface_id, act.role, &found);
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
            act.appid = appid;
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

bool WindowManager::pop_pending_events()
{
    bool x{true};
    return this->pending_events.compare_exchange_strong(
        x, false, std::memory_order_consume);
}

optional<int> WindowManager::lookup_id(char const *name)
{
    return this->id_alloc.lookup(std::string(name));
}
optional<std::string> WindowManager::lookup_name(int id)
{
    return this->id_alloc.lookup(id);
}

/**
 * init_layers()
 */
int WindowManager::init_layers()
{
    if (!this->controller)
    {
        HMI_ERROR("wm", "ivi_controller global not available");
        return -1;
    }

    if (this->outputs.empty())
    {
        HMI_ERROR("wm", "no output was set up!");
        return -1;
    }

    auto &c = this->controller;

    auto &o = this->outputs.front();
    auto &s = c->screens.begin()->second;
    auto &layers = c->layers;

    // Write output dimensions to ivi controller...
    c->output_size = compositor::size{uint32_t(o->width), uint32_t(o->height)};
    c->physical_size = compositor::size{uint32_t(o->physical_width),
                                        uint32_t(o->physical_height)};


    HMI_DEBUG("wm", "SCALING: screen (%dx%d), physical (%dx%d)",
              o->width, o->height, o->physical_width, o->physical_height);

    this->layers.loadAreaDb();

    const compositor::rect css_bg = this->layers.getAreaSize("fullscreen");
    rectangle dp_bg(o->width, o->height);

    dp_bg.set_aspect(static_cast<double>(css_bg.w) / css_bg.h);
    dp_bg.fit(o->width, o->height);
    dp_bg.center(o->width, o->height);
    HMI_DEBUG("wm", "SCALING: CSS BG(%dx%d) -> DDP %dx%d,(%dx%d)",
              css_bg.w, css_bg.h, dp_bg.left(), dp_bg.top(), dp_bg.width(), dp_bg.height());

    // Clear scene
    layers.clear();

    // Clear screen
    s->clear();

    // Quick and dirty setup of layers
    for (auto const &i : this->layers.mapping)
    {
        c->layer_create(i.second.layer_id, dp_bg.width(), dp_bg.height());
        auto &l = layers[i.second.layer_id];
        l->set_destination_rectangle(dp_bg.left(), dp_bg.top(), dp_bg.width(), dp_bg.height());
        l->set_visibility(1);
        HMI_DEBUG("wm", "Setting up layer %s (%d) for surface role match \"%s\"",
                  i.second.name.c_str(), i.second.layer_id, i.second.role.c_str());
    }

    // Add layers to screen
    s->set_render_order(this->layers.layers);

    this->layout_commit();

    c->scale = static_cast<double>(dp_bg.height()) / css_bg.h;
    this->layers.setupArea(c->scale);

    return 0;
}

void WindowManager::surface_set_layout(int surface_id, const std::string& area)
{
    if (!this->controller->surface_exists(surface_id))
    {
        HMI_ERROR("wm", "Surface %d does not exist", surface_id);
        return;
    }

    auto o_layer_id = this->layers.get_layer_id(surface_id);

    if (!o_layer_id)
    {
        HMI_ERROR("wm", "Surface %d is not associated with any layer!", surface_id);
        return;
    }

    uint32_t layer_id = *o_layer_id;

    auto const &layer = this->layers.get_layer(layer_id);
    auto rect = this->layers.getAreaSize(area);
    HMI_SEQ_DEBUG(g_app_list.currentRequestNumber(), "%s : x:%d y:%d w:%d h:%d", area.c_str(),
                    rect.x, rect.y, rect.w, rect.h);
    auto &s = this->controller->surfaces[surface_id];

    int x = rect.x;
    int y = rect.y;
    int w = rect.w;
    int h = rect.h;

    HMI_DEBUG("wm", "surface_set_layout for surface %u on layer %u", surface_id,
              layer_id);

    // set destination to the display rectangle
    s->set_destination_rectangle(x, y, w, h);

    // update area information
    this->area_info[surface_id].x = x;
    this->area_info[surface_id].y = y;
    this->area_info[surface_id].w = w;
    this->area_info[surface_id].h = h;

    HMI_DEBUG("wm", "Surface %u now on layer %u with rect { %d, %d, %d, %d }",
              surface_id, layer_id, x, y, w, h);
}

void WindowManager::layout_commit()
{
    this->controller->commit_changes();
    this->display->flush();
}

void WindowManager::emit_activated(char const *label)
{
    this->send_event(kListEventName[Event_Active], label);
}

void WindowManager::emit_deactivated(char const *label)
{
    this->send_event(kListEventName[Event_Inactive], label);
}

void WindowManager::emit_syncdraw(char const *label, char const *area, int x, int y, int w, int h)
{
    this->send_event(kListEventName[Event_SyncDraw], label, area, x, y, w, h);
}

void WindowManager::emit_syncdraw(const std::string &role, const std::string &area)
{
    compositor::rect rect = this->layers.getAreaSize(area);
    this->send_event(kListEventName[Event_SyncDraw],
        role.c_str(), area.c_str(), rect.x, rect.y, rect.w, rect.h);
}

void WindowManager::emit_flushdraw(char const *label)
{
    this->send_event(kListEventName[Event_FlushDraw], label);
}

void WindowManager::emit_visible(char const *label, bool is_visible)
{
    this->send_event(is_visible ? kListEventName[Event_Visible] : kListEventName[Event_Invisible], label);
}

void WindowManager::emit_invisible(char const *label)
{
    return emit_visible(label, false);
}

void WindowManager::emit_visible(char const *label) { return emit_visible(label, true); }

void WindowManager::activate(int id)
{
    auto ip = this->controller->sprops.find(id);
    if (ip != this->controller->sprops.end())
    {
        this->controller->surfaces[id]->set_visibility(1);
        char const *label =
            this->lookup_name(id).value_or("unknown-name").c_str();

         // FOR CES DEMO >>>
        if ((0 == strcmp(label, "radio")) ||
            (0 == strcmp(label, "music")) ||
            (0 == strcmp(label, "video")) ||
            (0 == strcmp(label, "map")))
        {
            for (auto i = surface_bg.begin(); i != surface_bg.end(); ++i)
            {
                if (id == *i)
                {
                    // Remove id
                    this->surface_bg.erase(i);

                    // Remove from BG layer (999)
                    HMI_DEBUG("wm", "Remove %s(%d) from BG layer", label, id);
                    this->controller->layers[999]->remove_surface(id);

                    // Add to FG layer (1001)
                    HMI_DEBUG("wm", "Add %s(%d) to FG layer", label, id);
                    this->controller->layers[1001]->add_surface(id);

                    for (int j : this->surface_bg)
                    {
                        HMI_DEBUG("wm", "Stored id:%d", j);
                    }
                    break;
                }
            }
        }
        // <<< FOR CES DEMO

        this->layout_commit();

        // TODO: application requests by old role,
        //       so convert role new to old for emitting event
        const char* old_role = this->rolenew2old[label].c_str();

        this->emit_visible(old_role);
        this->emit_activated(old_role);
    }
}

void WindowManager::deactivate(int id)
{
    auto ip = this->controller->sprops.find(id);
    if (ip != this->controller->sprops.end())
    {
        char const *label =
            this->lookup_name(id).value_or("unknown-name").c_str();

        // FOR CES DEMO >>>
        if ((0 == strcmp(label, "radio")) ||
            (0 == strcmp(label, "music")) ||
            (0 == strcmp(label, "video")) ||
            (0 == strcmp(label, "map")))
        {

            // Store id
            this->surface_bg.push_back(id);

            // Remove from FG layer (1001)
            HMI_DEBUG("wm", "Remove %s(%d) from FG layer", label, id);
            this->controller->layers[1001]->remove_surface(id);

            // Add to BG layer (999)
            HMI_DEBUG("wm", "Add %s(%d) to BG layer", label, id);
            this->controller->layers[999]->add_surface(id);

            for (int j : surface_bg)
            {
                HMI_DEBUG("wm", "Stored id:%d", j);
            }
        }
        else
        {
            this->controller->surfaces[id]->set_visibility(0);
        }
        // <<< FOR CES DEMO

        this->layout_commit();

        // TODO: application requests by old role,
        //       so convert role new to old for emitting event
        const char* old_role = this->rolenew2old[label].c_str();

        this->emit_deactivated(old_role);
        this->emit_invisible(old_role);
    }
}

WMError WindowManager::setRequest(const std::string& appid, const std::string &role, const std::string &area,
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

WMError WindowManager::doTransition(unsigned req_num)
{
    HMI_SEQ_DEBUG(req_num, "check policy");
    WMError ret = this->checkPolicy(req_num);
    return ret;
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
    std::string req_area = trigger.area;

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
            std::string old_role = this->rolenew2old[action.role];

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
            if (g_app_list.contains(x.appid))
            {
                auto client = g_app_list.lookUpClient(x.appid);
                this->deactivate(client->surfaceID(x.role));
            }
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
            if(!g_app_list.contains(act.appid)){
                ret = WMError::NOT_REGISTERED;
            }
            ret = this->layoutChange(act);
            if(ret != WMError::SUCCESS)
            {
                HMI_SEQ_WARNING(req_num,
                    "Failed to manipulate surfaces while state change : %s", errorDescription(ret));
                return ret;
            }
            ret = this->visibilityChange(act);
            if (ret != WMError::SUCCESS)
            {
                HMI_SEQ_WARNING(req_num,
                    "Failed to manipulate surfaces while state change : %s", errorDescription(ret));
                return ret;
            }
            HMI_SEQ_DEBUG(req_num, "visible %s", act.role.c_str());
            //this->lm_enddraw(act.role.c_str());
        }
    }
    this->layout_commit();

    HMI_SEQ_INFO(req_num, "emit flushDraw");

    for(const auto &act_flush : actions)
    {
        if(act_flush.visible == TaskVisible::VISIBLE)
        {
            // TODO: application requests by old role,
            //       so convert role new to old for emitting event
            std::string old_role = this->rolenew2old[act_flush.role];

            this->emit_flushdraw(old_role.c_str());
        }
    }

    return ret;
}

WMError WindowManager::layoutChange(const WMAction &action)
{
    if (action.visible == TaskVisible::INVISIBLE)
    {
        // Visibility is not change -> no redraw is required
        return WMError::SUCCESS;
    }
    auto client = g_app_list.lookUpClient(action.appid);
    unsigned surface = client->surfaceID(action.role);
    if (surface == 0)
    {
        HMI_SEQ_ERROR(g_app_list.currentRequestNumber(),
                      "client doesn't have surface with role(%s)", action.role.c_str());
        return WMError::NOT_REGISTERED;
    }
    // Layout Manager
    WMError ret = this->setSurfaceSize(surface, action.area);
    return ret;
}

WMError WindowManager::visibilityChange(const WMAction &action)
{
    HMI_SEQ_DEBUG(g_app_list.currentRequestNumber(), "Change visibility");
    if(!g_app_list.contains(action.appid)){
        return WMError::NOT_REGISTERED;
    }
    auto client = g_app_list.lookUpClient(action.appid);
    unsigned surface = client->surfaceID(action.role);
    if(surface == 0)
    {
        HMI_SEQ_ERROR(g_app_list.currentRequestNumber(),
                      "client doesn't have surface with role(%s)", action.role.c_str());
        return WMError::NOT_REGISTERED;
    }

    if (action.visible != TaskVisible::INVISIBLE)
    {
        this->activate(surface); // Layout Manager task
    }
    else
    {
        this->deactivate(surface); // Layout Manager task
    }
    return WMError::SUCCESS;
}

WMError WindowManager::setSurfaceSize(unsigned surface, const std::string &area)
{
    this->surface_set_layout(surface, area);

    return WMError::SUCCESS;
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
            json_object_array_add(jarray, json_object_new_string(action.appid.c_str()));
        }
    }
    json_object_object_add(j, kKeyIds, jarray);
    HMI_SEQ_INFO(req_num, "Visible app: %s", json_object_get_string(j));

    int ret = afb_event_push(
        this->map_afb_event[kListEventName[Event_ScreenUpdated]], j);
    if (ret != 0)
    {
        HMI_DEBUG("wm", "afb_event_push failed: %m");
    }
}

void WindowManager::setTimer()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_BOOTTIME, &ts) != 0) {
        HMI_ERROR("wm", "Could't set time (clock_gettime() returns with error");
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
            HMI_ERROR("wm", "Could't set timer");
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
        WMError rc = doTransition(req_num);
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

    HMI_DEBUG("wm", "old:%s -> new:%s", old_role, new_role);

    return new_role;
}

int WindowManager::loadOldRoleDb()
{
    // Get afm application installed dir
    char const *afm_app_install_dir = getenv("AFM_APP_INSTALL_DIR");
    HMI_DEBUG("wm", "afm_app_install_dir:%s", afm_app_install_dir);

    std::string file_name;
    if (!afm_app_install_dir)
    {
        HMI_ERROR("wm", "AFM_APP_INSTALL_DIR is not defined");
    }
    else
    {
        file_name = std::string(afm_app_install_dir) + std::string("/etc/old_roles.db");
    }

    // Load old_role.db
    json_object* json_obj;
    int ret = jh::inputJsonFilie(file_name.c_str(), &json_obj);
    if (0 > ret)
    {
        HMI_ERROR("wm", "Could not open old_role.db, so use default old_role information");
        json_obj = json_tokener_parse(kDefaultOldRoleDb);
    }
    HMI_DEBUG("wm", "json_obj dump:%s", json_object_get_string(json_obj));

    // Perse apps
    json_object* json_cfg;
    if (!json_object_object_get_ex(json_obj, "old_roles", &json_cfg))
    {
        HMI_ERROR("wm", "Parse Error!!");
        return -1;
    }

    int len = json_object_array_length(json_cfg);
    HMI_DEBUG("wm", "json_cfg len:%d", len);
    HMI_DEBUG("wm", "json_cfg dump:%s", json_object_get_string(json_cfg));

    for (int i=0; i<len; i++)
    {
        json_object* json_tmp = json_object_array_get_idx(json_cfg, i);

        const char* old_role = jh::getStringFromJson(json_tmp, "name");
        if (nullptr == old_role)
        {
            HMI_ERROR("wm", "Parse Error!!");
            return -1;
        }

        const char* new_role = jh::getStringFromJson(json_tmp, "new");
        if (nullptr == new_role)
        {
            HMI_ERROR("wm", "Parse Error!!");
            return -1;
        }

        this->roleold2new[old_role] = std::string(new_role);
    }

    // Check
    for(auto itr = this->roleold2new.begin();
      itr != this->roleold2new.end(); ++itr)
    {
        HMI_DEBUG("wm", ">>> role old:%s new:%s",
                  itr->first.c_str(), itr->second.c_str());
    }

    // Release json_object
    json_object_put(json_obj);

    return 0;
}

const char *WindowManager::check_surface_exist(const char *drawing_name)
{
    auto const &surface_id = this->lookup_id(drawing_name);
    if (!surface_id)
    {
        return "Surface does not exist";
    }

    if (!this->controller->surface_exists(*surface_id))
    {
        return "Surface does not exist in controller!";
    }

    auto layer_id = this->layers.get_layer_id(*surface_id);

    if (!layer_id)
    {
        return "Surface is not on any layer!";
    }

    HMI_DEBUG("wm", "surface %d is detected", *surface_id);
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

/**
 * controller_hooks
 */
void controller_hooks::surface_created(uint32_t surface_id)
{
    this->wmgr->surface_created(surface_id);
}

void controller_hooks::surface_removed(uint32_t surface_id)
{
    this->wmgr->surface_removed(surface_id);
}

void controller_hooks::surface_visibility(uint32_t /*surface_id*/,
                                          uint32_t /*v*/) {}

void controller_hooks::surface_destination_rectangle(uint32_t /*surface_id*/,
                                                     uint32_t /*x*/,
                                                     uint32_t /*y*/,
                                                     uint32_t /*w*/,
                                                     uint32_t /*h*/) {}

} // namespace wm
