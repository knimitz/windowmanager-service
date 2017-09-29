/*
 * Copyright (C) 2017 Mentor Graphics Development (Deutschland) GmbH
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

#ifndef TMCAGLWM_CONFIG_HPP
#define TMCAGLWM_CONFIG_HPP

#include <experimental/optional>
#include <map>

namespace wm {

using std::experimental::optional;
using std::experimental::nullopt;

struct config {
   typedef std::map<std::string, std::string> map;

   map cfg;

   config();

   optional<std::string> get_string(char const *s) {
      auto i = this->cfg.find(s);
      return i != this->cfg.end() ? optional<std::string>(i->second) : nullopt;
   }

   optional<int> get_int(char const *s) {
      auto i = this->cfg.find(s);
      return i != this->cfg.end() ? optional<int>(std::stoi(i->second))
                                  : nullopt;
   }
};

}  // namespace wm

#endif  // TMCAGLWM_CONFIG_HPP
