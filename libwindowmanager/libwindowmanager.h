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

#ifndef LIBWINDOWMANAGER_H
#define LIBWINDOWMANAGER_H

#include <functional>

class LibWindowmanager {
public:
    LibWindowmanager();
    ~LibWindowmanager();

    LibWindowmanager(const LibWindowmanager &) = delete;
    LibWindowmanager &operator=(const LibWindowmanager &) = delete;

    using handler_fun = std::function<void(const char *)>;

    enum EventType {
       Event_Active = 1,
       Event_Inactive,

       Event_Visible,
       Event_Invisible,

       Event_SyncDraw,
       Event_FlushDraw,
    };

    int init(int port, char const *token);

    // WM API
    int requestSurface(const char *label);
    int activateSurface(const char *label);
    int deactivateSurface(const char *label);
    int endDraw(const char *label);

    void set_event_handler(enum EventType et, handler_fun f);

    struct Impl;

private:
    Impl *const d;
};
#endif // LIBWINDOWMANAGER_H
