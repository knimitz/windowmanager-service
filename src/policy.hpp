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

#ifndef TMCAGLWM_POLICY_HPP
#define TMCAGLWM_POLICY_HPP

#include "layout.hpp"
#include "hmi-debug.h"

namespace wm
{

class Policy
{
  public:
    bool layout_is_valid(LayoutState const & /* layout */)
    {
        // We do not check for policy currently
        HMI_DEBUG("wm", "Policy check returns positive");
        return true;
    }
};

} // namespace wm

#endif //TMCAGLWM_POLICY_HPP
