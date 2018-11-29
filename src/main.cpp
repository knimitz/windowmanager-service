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

#include <unistd.h>
#include <algorithm>
#include <mutex>
#include <json.h>
#include "window_manager.hpp"
#include "json_helper.hpp"

extern "C"
{
#include <afb/afb-binding.h>
#include <systemd/sd-event.h>
}

typedef struct WMClientCtxt
{
    std::string name;
    std::string role;
    WMClientCtxt(const char *appName, const char* appRole)
    {
        name = appName;
        role = appRole;
    }
} WMClientCtxt;

struct afb_instance
{
    wm::WindowManager wmgr;

    afb_instance() : wmgr() {}
    ~afb_instance() = default;

    int init();
};

struct afb_instance *g_afb_instance;
std::mutex binding_m;

int afb_instance::init()
{
    return this->wmgr.init();
}

static int _binding_init()
{
    HMI_NOTICE("WinMan ver. %s", WINMAN_VERSION_STRING);

    g_afb_instance = new afb_instance;

    if (g_afb_instance->init() == -1)
    {
        HMI_ERROR("Could not connect to compositor");
        goto error;
    }

    atexit([] { delete g_afb_instance; });

    return 0;

error:
    delete g_afb_instance;
    g_afb_instance = nullptr;
    return -1;
}

static int binding_init (afb_api_t api) noexcept
{
    try
    {
        return _binding_init();
    }
    catch (std::exception &e)
    {
        HMI_ERROR("Uncaught exception in binding_init(): %s", e.what());
    }
    return -1;
}

static void cbRemoveClientCtxt(void *data)
{
    WMClientCtxt *ctxt = (WMClientCtxt *)data;
    if (ctxt == nullptr)
    {
        return;
    }
    HMI_DEBUG("remove app %s", ctxt->name.c_str());

    // Policy Manager does not know this app was killed,
    // so notify it by deactivate request.
    g_afb_instance->wmgr.api_deactivate_window(
        ctxt->name.c_str(), ctxt->role.c_str(),
        [](const char *) {});

    g_afb_instance->wmgr.removeClient(ctxt->name);
    delete ctxt;
}

static void createSecurityContext(afb_req_t req, const char* appid, const char* role)
{
    WMClientCtxt *ctxt = (WMClientCtxt *)afb_req_context_get(req);
    if (!ctxt)
    {
        // Create Security Context at first time
        WMClientCtxt *ctxt = new WMClientCtxt(appid, role);
        HMI_DEBUG("create session for %s", ctxt->name.c_str());
        afb_req_session_set_LOA(req, 1);
        afb_req_context_set(req, ctxt, cbRemoveClientCtxt);
    }
}

void windowmanager_requestsurface(afb_req_t req) noexcept
{
    std::lock_guard<std::mutex> guard(binding_m);
    if (g_afb_instance == nullptr)
    {
        afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
        return;
    }

    try
    {
        const char *a_drawing_name = afb_req_value(req, "drawing_name");
        if (!a_drawing_name)
        {
            afb_req_fail(req, "failed", "Need char const* argument drawing_name");
            return;
        }

        char *appid = afb_req_get_application_id(req);
        if(appid)
        {
            auto ret = g_afb_instance->wmgr.api_request_surface(
                appid, a_drawing_name);
            if (ret.is_err())
            {
                afb_req_fail(req, "failed", ret.unwrap_err());
            }
            else
            {
                createSecurityContext(req, appid, a_drawing_name);
                afb_req_success(req, json_object_new_int(ret.unwrap()), "success");
            }
            free(appid);
        }
        else
        {
            afb_req_fail(req, "failed", nullptr);
        }
    }
    catch (std::exception &e)
    {
        afb_req_fail_f(req, "failed", "Uncaught exception while calling requestsurface: %s", e.what());
        return;
    }
}

void windowmanager_requestsurfacexdg(afb_req_t req) noexcept
{
    std::lock_guard<std::mutex> guard(binding_m);
    if (g_afb_instance == nullptr)
    {
        afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
        return;
    }

    try
    {
        json_object *jreq = afb_req_json(req);

        json_object *j_drawing_name = nullptr;
        if (!json_object_object_get_ex(jreq, "drawing_name", &j_drawing_name))
        {
            afb_req_fail(req, "failed", "Need char const* argument drawing_name");
            return;
        }
        char const *a_drawing_name = json_object_get_string(j_drawing_name);

        json_object *j_ivi_id = nullptr;
        if (!json_object_object_get_ex(jreq, "ivi_id", &j_ivi_id))
        {
            afb_req_fail(req, "failed", "Need char const* argument ivi_id");
            return;
        }
        char const *a_ivi_id = json_object_get_string(j_ivi_id);
        char *appid = afb_req_get_application_id(req);
        if(appid)
        {
            auto ret = g_afb_instance->wmgr.api_request_surface(
                appid, a_drawing_name, a_ivi_id);

            if (ret != nullptr)
            {
                afb_req_fail(req, "failed", ret);
            }
            else
            {
                createSecurityContext(req, appid, a_drawing_name);
                afb_req_success(req, NULL, "success");
            }
            free(appid);
        }
    }
    catch (std::exception &e)
    {
        afb_req_fail_f(req, "failed", "Uncaught exception while calling requestsurfacexdg: %s", e.what());
        return;
    }
}

void windowmanager_activatewindow(afb_req_t req) noexcept
{
    std::lock_guard<std::mutex> guard(binding_m);
    if (g_afb_instance == nullptr)
    {
        afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
        return;
    }

    try
    {
        const char *a_drawing_name = afb_req_value(req, "drawing_name");
        if (!a_drawing_name)
        {
            afb_req_fail(req, "failed", "Need char const* argument drawing_name");
            return;
        }

        const char *a_drawing_area = afb_req_value(req, "drawing_area");
        if (!a_drawing_area)
        {
            afb_req_fail(req, "failed", "Need char const* argument drawing_area");
            return;
        }

        char* appid = afb_req_get_application_id(req);
        if(appid)
        {
            g_afb_instance->wmgr.api_activate_window(
                appid, a_drawing_name, a_drawing_area,
                [&req](const char *errmsg) {
                    if (errmsg != nullptr)
                    {
                        HMI_ERROR(errmsg);
                        afb_req_fail(req, "failed", errmsg);
                        return;
                    }
                    afb_req_success(req, NULL, "success");
                });
            free(appid);
        }
    }
    catch (std::exception &e)
    {
        HMI_WARNING("failed: Uncaught exception while calling activatesurface: %s", e.what());
        g_afb_instance->wmgr.exceptionProcessForTransition();
        return;
    }
}

void windowmanager_deactivatewindow(afb_req_t req) noexcept
{
    std::lock_guard<std::mutex> guard(binding_m);
    if (g_afb_instance == nullptr)
    {
        afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
        return;
    }

    try
    {
        const char *a_drawing_name = afb_req_value(req, "drawing_name");
        if (!a_drawing_name)
        {
            afb_req_fail(req, "failed", "Need char const* argument drawing_name");
            return;
        }

        char* appid = afb_req_get_application_id(req);
        if(appid)
        {
            g_afb_instance->wmgr.api_deactivate_window(
                appid, a_drawing_name,
                [&req](const char *errmsg) {
                    if (errmsg != nullptr)
                    {
                        HMI_ERROR(errmsg);
                        afb_req_fail(req, "failed", errmsg);
                        return;
                    }
                    afb_req_success(req, NULL, "success");
                });
            free(appid);
        }
    }
    catch (std::exception &e)
    {
        HMI_WARNING("failed: Uncaught exception while calling deactivatesurface: %s", e.what());
        g_afb_instance->wmgr.exceptionProcessForTransition();
        return;
    }
}

void windowmanager_enddraw(afb_req_t req) noexcept
{
    std::lock_guard<std::mutex> guard(binding_m);
    if (g_afb_instance == nullptr)
    {
        afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
        return;
    }

    try
    {
        const char *a_drawing_name = afb_req_value(req, "drawing_name");
        if (!a_drawing_name)
        {
            afb_req_fail(req, "failed", "Need char const* argument drawing_name");
            return;
        }
        afb_req_success(req, NULL, "success");

        char* appid = afb_req_get_application_id(req);
        if(appid)
        {
            g_afb_instance->wmgr.api_enddraw(appid, a_drawing_name);
            free(appid);
        }
    }
    catch (std::exception &e)
    {
        HMI_WARNING("failed: Uncaught exception while calling enddraw: %s", e.what());
        g_afb_instance->wmgr.exceptionProcessForTransition();
        return;
    }
}

void windowmanager_getdisplayinfo_thunk(afb_req_t req) noexcept
{
    std::lock_guard<std::mutex> guard(binding_m);
    if (g_afb_instance == nullptr)
    {
        afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
        return;
    }

    try
    {
        auto ret = g_afb_instance->wmgr.api_get_display_info();
        if (ret.is_err())
        {
            afb_req_fail(req, "failed", ret.unwrap_err());
            return;
        }

        afb_req_success(req, ret.unwrap(), "success");
    }
    catch (std::exception &e)
    {
        afb_req_fail_f(req, "failed", "Uncaught exception while calling getdisplayinfo: %s", e.what());
        return;
    }
}

void windowmanager_getareainfo_thunk(afb_req_t req) noexcept
{
    std::lock_guard<std::mutex> guard(binding_m);
    if (g_afb_instance == nullptr)
    {
        afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
        return;
    }

    try
    {
        json_object *jreq = afb_req_json(req);

        json_object *j_drawing_name = nullptr;
        if (!json_object_object_get_ex(jreq, "drawing_name", &j_drawing_name))
        {
            afb_req_fail(req, "failed", "Need char const* argument drawing_name");
            return;
        }
        char const *a_drawing_name = json_object_get_string(j_drawing_name);

        auto ret = g_afb_instance->wmgr.api_get_area_info(a_drawing_name);
        if (ret.is_err())
        {
            afb_req_fail(req, "failed", ret.unwrap_err());
            return;
        }

        afb_req_success(req, ret.unwrap(), "success");
    }
    catch (std::exception &e)
    {
        afb_req_fail_f(req, "failed", "Uncaught exception while calling getareainfo: %s", e.what());
        return;
    }
}

void windowmanager_wm_subscribe(afb_req_t req) noexcept
{
    std::lock_guard<std::mutex> guard(binding_m);
    if (g_afb_instance == nullptr)
    {
        afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
        return;
    }

    try
    {
        json_object *jreq = afb_req_json(req);
        json_object *j = nullptr;
        if (!json_object_object_get_ex(jreq, "event", &j))
        {
            afb_req_fail(req, "failed", "Need char const* argument event");
            return;
        }
        int event_id = json_object_get_int(j);
        int ret = g_afb_instance->wmgr.api_subscribe(req, event_id);

        if (ret)
        {
            afb_req_fail(req, "failed", "Error: afb_req_subscribe()");
            return;
        }
        afb_req_success(req, NULL, "success");
    }
    catch (std::exception &e)
    {
        afb_req_fail_f(req, "failed", "Uncaught exception while calling wm_subscribe: %s", e.what());
        return;
    }
}

void windowmanager_ping(afb_req_t req) noexcept
{
    std::lock_guard<std::mutex> guard(binding_m);

    if (g_afb_instance == nullptr)
    {
        afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
        return;
    }
    else
    {
        afb_req_success(req, NULL, "success");
    }
}

void windowmanager_debug_terminate(afb_req_t req) noexcept
{
    std::lock_guard<std::mutex> guard(binding_m);
    if (g_afb_instance == nullptr)
    {
        afb_req_fail(req, "failed", "Binding not initialized, did the compositor die?");
        return;
    }

    try
    {

        if (getenv("WINMAN_DEBUG_TERMINATE") != nullptr)
        {
            raise(SIGKILL); // afb-daemon kills it's pgroup using TERM, which
                            // doesn't play well with perf
        }

        afb_req_success(req, NULL, "success");
    }
    catch (std::exception &e)
    {
        afb_req_fail_f(req, "failed", "Uncaught exception while calling debug_terminate: %s", e.what());
        return;
    }
}

const afb_verb_t windowmanager_verbs[] = {
    { .verb = "requestSurface", .callback = windowmanager_requestsurface },
    { .verb = "requestSurfaceXDG", .callback = windowmanager_requestsurfacexdg },
    { .verb = "activateWindow", .callback = windowmanager_activatewindow },
    { .verb = "deactivateWindow", .callback = windowmanager_deactivatewindow },
    { .verb = "endDraw", .callback = windowmanager_enddraw },
    { .verb = "getDisplayInfo", .callback = windowmanager_getdisplayinfo_thunk },
    { .verb = "getAreaInfo", .callback = windowmanager_getareainfo_thunk },
    { .verb = "wm_subscribe", .callback = windowmanager_wm_subscribe },
    { .verb = "ping", .callback = windowmanager_ping },
    { .verb = "debug_terminate", .callback = windowmanager_debug_terminate },
    {} };

extern "C" const afb_binding_t afbBindingExport = {
  .api = "windowmanager",
  .specification = "windowmanager",
  .info = "windowmanager",
  .verbs = windowmanager_verbs,
  .preinit = nullptr,
  .init = binding_init,
  .onevent = nullptr,
  .userdata = nullptr,
  .provide_class = nullptr,
  .require_class = nullptr,
  .require_api = nullptr,
  .noconcurrency = 0
};
