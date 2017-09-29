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

#include "libwindowmanager.h"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include <queue>

#include <unistd.h>

#include <systemd/sd-event.h>

#include <json-c/json.h>
#include <pthread.h>

extern "C" {
#include <afb/afb-ws-client.h>
#include <afb/afb-wsj1.h>
}

#define UNUSED(x) (void)(x)

//       _                 ___                 _
//   ___| | __ _ ___ ___  |_ _|_ __ ___  _ __ | |
//  / __| |/ _` / __/ __|  | || '_ ` _ \| '_ \| |
// | (__| | (_| \__ \__ \  | || | | | | | |_) | |
//  \___|_|\__,_|___/___/ |___|_| |_| |_| .__/|_|
//                                      |_|
class LibWindowmanager::Impl {
    friend class LibWindowmanager;

    // This is the LibWindowmanager interface impl
    int init(int port, char const *token);

    // WM API
    int requestSurface(const char *label);
    int activateSurface(const char *label);
    int deactivateSurface(const char *label);
    int endDraw(const char *label);

    void set_event_handler(enum EventType et, handler_fun func);

    Impl();
    ~Impl();

    struct afb_wsj1 *wsj1;
    struct sd_event *loop;

    std::set<std::string> labels;
    std::map<EventType, handler_fun> handlers;
    std::queue<std::pair<handler_fun, std::string>> handler_queue;

    int api_call(const char *verb, json_object *object,
                 const std::function<void(bool, json_object *)> &onReply);

public:
    void event(char const *et, char const *label);
private:
  int runEventLoop();
};

namespace {

constexpr const int token_maxlen = 20;
constexpr const char *const wmAPI = "windowmanager";

#define CONCAT_(X, Y) X##Y
#define CONCAT(X, Y) CONCAT_(X, Y)

#ifndef SCOPE_TRACING
#define TRACE()
#define TRACEN(N)
#else
#define TRACE() \
    ScopeTrace __attribute__((unused)) CONCAT(trace_scope_, __LINE__)(__func__)
#define TRACEN(N) \
    ScopeTrace __attribute__((unused)) CONCAT(named_trace_scope_, __LINE__)(#N)

struct ScopeTrace {
    thread_local static int indent;
    char const *f{};
    explicit ScopeTrace(char const *func) : f(func) {
        fprintf(stderr, "%*s%s -->\n", 2 * indent++, "", this->f);
    }
    ~ScopeTrace() { fprintf(stderr, "%*s%s <--\n", 2 * --indent, "", this->f); }
};
thread_local int ScopeTrace::indent = 0;
#endif

/* called when wsj1 receives a method invocation */
void onCall(void *closure, const char *api, const char *verb,
            struct afb_wsj1_msg *msg) {
    TRACE();
    UNUSED(closure);
    UNUSED(verb);
    UNUSED(api);
    UNUSED(msg);
}

/* called when wsj1 receives an event */
void onEvent(void *closure, const char *event, afb_wsj1_msg *msg) {
    TRACE();

    // check API name in event
    if (0 != strncmp(wmAPI, event, strlen(wmAPI))) {
        return;
    }

    reinterpret_cast<LibWindowmanager::Impl *>(closure)->event(
        event, json_object_get_string(
                   json_object_object_get(afb_wsj1_msg_object_j(msg), "data")));
}

/* called when wsj1 hangsup */
void onHangup(void *closure, afb_wsj1 *wsj1) {
    TRACE();
    UNUSED(closure);
    UNUSED(wsj1);
    fputs("Hangup, the WindowManager vanished\n", stderr);
    exit(1);  // XXX: there should be something ... *better* here.
}

constexpr struct afb_wsj1_itf itf = {
    onHangup, onCall, onEvent,
};

// XXX: I am not sure this is the right thing to do though...
std::recursive_mutex dispatch_mutex;

json_object *drawing_name_json_argument(char const *label) {
    json_object *j = json_object_new_object();
    json_object_object_add(j, "drawing_name", json_object_new_string(label));
    return j;
}

}  // namespace

//       _                 ___                 _   _                 _
//   ___| | __ _ ___ ___  |_ _|_ __ ___  _ __ | | (_)_ __ ___  _ __ | |
//  / __| |/ _` / __/ __|  | || '_ ` _ \| '_ \| | | | '_ ` _ \| '_ \| |
// | (__| | (_| \__ \__ \  | || | | | | | |_) | | | | | | | | | |_) | |
//  \___|_|\__,_|___/___/ |___|_| |_| |_| .__/|_| |_|_| |_| |_| .__/|_|
//                                      |_|                   |_|
LibWindowmanager::Impl::Impl() : wsj1{}, loop{}, labels(), handlers() { TRACE(); }

LibWindowmanager::Impl::~Impl() {
    TRACE();
    afb_wsj1_unref(wsj1);
    sd_event_unref(loop);
}

int LibWindowmanager::Impl::init(int port, char const *token) {
    TRACE();
    char *uribuf = nullptr;
    int rc = -1;

    if (this->loop != nullptr && this->wsj1 != nullptr) {
        fputs("LibWindowmanager instance is already initialized!\n", stderr);
        rc = -EALREADY;
        goto fail;
    }

    if (token == nullptr) {
        fputs("Token is invalid\n", stderr);
        rc = -EINVAL;
        goto fail;
    }

    if (port < 1 || port > 0xffff) {
        fputs("Port is invalid\n", stderr);
        rc = -EINVAL;
        goto fail;
    }

    /* get the default event loop */
    rc = sd_event_default(&this->loop);
    if (rc < 0) {
        fprintf(stderr, "Connection to default event loop failed: %s\n",
                strerror(-rc));
        goto fail;
    }

    asprintf(&uribuf, "ws://localhost:%d/api?token=%s", port, token);

    /* connect the websocket wsj1 to the uri given by the first argument */
    this->wsj1 = afb_ws_client_connect_wsj1(
        this->loop, uribuf, const_cast<struct afb_wsj1_itf *>(&itf), this);
    if (this->wsj1 == nullptr) {
        sd_event_unref(this->loop);
        this->loop = nullptr;
        fprintf(stderr, "Connection to %s failed: %m\n", uribuf);
        rc = -errno;
        goto fail;
    }

    this->runEventLoop();

    return 0;

fail:
    return rc;
}

int LibWindowmanager::Impl::requestSurface(const char *label) {
    TRACE();

    if (this->labels.find(label) != this->labels.end()) {
        fputs("Surface label already known!\n", stderr);
        return -EINVAL;
    }

    json_object *j = drawing_name_json_argument(label);

    int rc = -1;
    /* send the request */
    int rc2 =
        this->api_call("RequestSurface", j, [&rc](bool ok, json_object *j) {
            if (ok) {
                int id =
                    json_object_get_int(json_object_object_get(j, "response"));
                char *buf;
                asprintf(&buf, "%d", id);
                printf("setenv(\"QT_IVI_SURFACE_ID\", %s, 1)\n", buf);
                if (setenv("QT_IVI_SURFACE_ID", buf, 1) != 0) {
                    fprintf(stderr, "putenv failed: %m\n");
                    rc = -errno;
                } else {
                    rc = 0;  // Single point of success
                }
            } else {
                fprintf(stderr, "Could not get surface ID from WM: %s\n",
                        j != nullptr ? json_object_to_json_string_ext(
                                j, JSON_C_TO_STRING_PRETTY)
                          : "no-info");
                rc = -EINVAL;
            }
        });

    if (rc2 < 0) {
        rc = rc2;
    }

    if (rc >= 0) {
        this->labels.insert(this->labels.end(), label);
    }

    return rc;
}

int LibWindowmanager::Impl::activateSurface(const char *label) {
    TRACE();
    json_object *j = drawing_name_json_argument(label);
    return this->api_call("ActivateSurface", j, [](bool ok, json_object *j) {
        if (!ok) {
            fprintf(stderr, "API Call activate_surface() failed: %s\n",
                    j != nullptr ? json_object_to_json_string_ext(
                                       j, JSON_C_TO_STRING_PRETTY)
                  : "no-info");
        }
    });
}

int LibWindowmanager::Impl::deactivateSurface(const char *label) {
    TRACE();
    json_object *j = drawing_name_json_argument(label);
    return this->api_call("DeactivateSurface", j, [](bool ok, json_object *j) {
        if (!ok) {
            fprintf(stderr, "API Call deactivate_surface() failed: %s\n",
                    j != nullptr ? json_object_to_json_string_ext(
                                       j, JSON_C_TO_STRING_PRETTY)
                  : "no-info");
        }
    });
}

int LibWindowmanager::Impl::endDraw(const char *label) {
    TRACE();
    json_object *j = drawing_name_json_argument(label);
    return this->api_call("EndDraw", j, [](bool ok, json_object *j) {
        if (!ok) {
            fprintf(stderr, "API Call endDraw() failed: %s\n",
                    j != nullptr ? json_object_to_json_string_ext(
                                       j, JSON_C_TO_STRING_PRETTY)
                  : "no-info");
        }
    });
}

void LibWindowmanager::Impl::set_event_handler(
    enum EventType et, std::function<void(char const *)> func) {
    TRACE();

    if (et >= 1 && et <= 6) {  // Yeah ... just go with it!
        this->handlers[et] = std::move(func);
    }
}

namespace {
std::pair<bool, LibWindowmanager::EventType> make_event_type(char const *et) {
    // Event have the form "$API/$EVENT", just try to find the first / and
    // get on with it.
    char const *et2 = strchr(et, '/');
    if (et2 != nullptr) {
        et = et2 + 1;
    }

#define ET(N, A)                                               \
    do {                                                       \
        if (strcasecmp(et, N) == 0)                            \
            return std::pair<bool, LibWindowmanager::EventType>( \
                true, CONCAT(LibWindowmanager::Event_, A));           \
    } while (false)

    ET("active", Active);
    ET("inactive", Inactive);
    ET("visible", Visible);
    ET("invisible", Invisible);
    ET("syncdraw", SyncDraw);
    ET("flushdraw", FlushDraw);
#undef ET

    return std::pair<bool, LibWindowmanager::EventType>(false,
                                                      LibWindowmanager::Event_Active);
}
}  // namespace

static void _on_reply_static(void *closure, struct afb_wsj1_msg *msg)
{
    // nop
}


/// object will be json_object_put
int LibWindowmanager::Impl::api_call(
    const char *verb, json_object *object,
    const std::function<void(bool, json_object *)> &onReply) {
    TRACE();

    int rc = 0;
    if (0 == strcmp("RequestSurface", verb)) {
        // We need to wrap the actual onReply call once in order to
        // *look* like a normal functions pointer (std::functions<>
        // with captures cannot convert to function pointers).
        // Alternatively we could setup a local struct and use it as
        // closure, but I think it is cleaner this way.
        int call_rc = 0;
        std::atomic<bool> returned{};
        returned.store(false, std::memory_order_relaxed);
        std::function<void(bool, json_object *)> wrappedOnReply =
            [&returned, &call_rc, &onReply](bool ok, json_object *j) {
                TRACEN(wrappedOnReply);
                call_rc = ok ? 0 : -EINVAL;
                // We know it failed, but there may be an explanation in the
                // json object.
                {
                    TRACEN(onReply);
                    onReply(ok, j);
                }
                returned.store(true, std::memory_order_release);
            };

        // make the actual call, use wrappedOnReply as closure
        rc = afb_wsj1_call_j(
            this->wsj1, wmAPI, verb, object,
            [](void *closure, afb_wsj1_msg *msg) {
                TRACEN(callClosure);
                auto *onReply =
                    reinterpret_cast<std::function<void(bool, json_object *)> *>(
                        closure);
                (*onReply)(!(afb_wsj1_msg_is_reply_ok(msg) == 0),
                           afb_wsj1_msg_object_j(msg));
            },
            &wrappedOnReply);

        if (0 == rc) {
            // We need to wait until "returned" got set, this is necessary
            // if events get triggered by the call (and would be dispatched before
            // the actual call-reply).
            while (!returned.load(std::memory_order_consume)) {
                sd_event_run(loop, 16);
            }

            // return the actual API call result
            rc = call_rc;
        }
    }
    else {
        rc = afb_wsj1_call_j(this->wsj1, wmAPI, verb, object, _on_reply_static, this);
    }

    if (rc < 0) {
        fprintf(
            stderr, "calling %s/%s(%s) failed: %m\n", wmAPI, verb,
            json_object_to_json_string_ext(object, JSON_C_TO_STRING_PRETTY));
        // Call the reply handler regardless with a NULL json_object*
        onReply(false, nullptr);
    }

    return rc;
}

void LibWindowmanager::Impl::event(char const *et, char const *label) {
    TRACE();
    auto oet = make_event_type(et);
    if (!oet.first) {
        fprintf(stderr, "Unknown event type string '%s'\n", et);
        return;
    }

    auto i = this->handlers.find(oet.second);
    if (i != this->handlers.end()) {
        if (this->labels.find(label) != this->labels.end()) {
            i->second(label);
        }
    }
}

static void *event_loop_run(void *args){
    struct sd_event* loop = (struct sd_event*)(args);
    for(;;)
        sd_event_run(loop, 30000000);
}

int LibWindowmanager::Impl::runEventLoop() {
    if(this->wsj1 && this->loop)
    {
        pthread_t thread_id;
        int ret = pthread_create(&thread_id, NULL, event_loop_run, this->loop);
        if(ret != 0)
        {
            printf("Cannot run eventloop due to error:%d", errno);
            return -1;
        }
        else
            return thread_id;
        }
    else
    {
        printf("Connecting is not established yet");
        return -1;
    }
}

//       _                    _    _____ ____   ____ _ _            _
//   ___| | __ _ ___ ___     / \  |  ___| __ ) / ___| (_) ___ _ __ | |_
//  / __| |/ _` / __/ __|   / _ \ | |_  |  _ \| |   | | |/ _ \ '_ \| __|
// | (__| | (_| \__ \__ \  / ___ \|  _| | |_) | |___| | |  __/ | | | |_
//  \___|_|\__,_|___/___/ /_/   \_\_|   |____/ \____|_|_|\___|_| |_|\__|
//
int LibWindowmanager::init(int port, char const *token) {
    return this->d->init(port, token);
}

int LibWindowmanager::requestSurface(const char *label) {
    return this->d->requestSurface(label);
}

int LibWindowmanager::activateSurface(const char *label) {
    return this->d->activateSurface(label);
}

int LibWindowmanager::deactivateSurface(const char *label) {
    return this->d->deactivateSurface(label);
}

int LibWindowmanager::endDraw(const char *label) { return this->d->endDraw(label); }

void LibWindowmanager::set_event_handler(enum EventType et,
                                  std::function<void(char const *label)> f) {
    return this->d->set_event_handler(et, std::move(f));
}

LibWindowmanager::LibWindowmanager() : d(new Impl) {}

LibWindowmanager::~LibWindowmanager() { delete d; }
