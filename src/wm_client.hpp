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

#ifndef WINDOWMANAGER_CLIENT_HPP
#define WINDOWMANAGER_CLIENT_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include "wm_error.hpp"

extern "C"
{
#include <afb/afb-binding.h>
}

namespace wm
{

enum WM_CLIENT_ERROR_EVENT
{
    UNKNOWN_ERROR
};

class WMClient
{
  public:
    WMClient();
    WMClient(const std::string &appid, unsigned layer,
            unsigned surface, const std::string &role);
    WMClient(const std::string &appid, const std::string &role);
    WMClient(const std::string &appid, unsigned layer, const std::string &role);
    WMClient(const std::string &appid, unsigned layer,
        const std::string& layer_name, unsigned surface, const std::string &role);
    ~WMClient() = default;

    std::string appID() const;
    std::string role() const;
    unsigned layerID() const;
    unsigned surfaceID() const;
    WMError addSurface(unsigned surface);
    bool removeSurfaceIfExist(unsigned surface);

#if GTEST_ENABLED
    bool subscribe(afb_req_t req, const std::string &event_name);
    void emitError(WM_CLIENT_ERROR_EVENT ev);
#endif

    void dumpInfo();

  private:
    std::string id;
    unsigned layer;
    std::string main_role;
    std::string area;
    unsigned surface; // currently, main application has only one surface.
    //std::vector<std::string> role_list;
    std::unordered_map<std::string, unsigned> role2surface;
#if GTEST_ENABLED
    // This is for unit test. afb_make_event occurs sig11 if call not in afb-binding
    std::unordered_map<std::string, std::string> event2list;
#else
    std::unordered_map<std::string, afb_event_t> evname2list;
#endif
};
} // namespace wm

#endif
