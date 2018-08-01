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

#include "util.hpp"

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <unistd.h>

#ifdef SCOPE_TRACING
thread_local int ScopeTrace::indent = 0;
ScopeTrace::ScopeTrace(char const *func) : f(func)
{
    fprintf(stderr, "%lu %*s%s -->\n", pthread_self(), 2 * indent++, "", this->f);
}
ScopeTrace::~ScopeTrace() { fprintf(stderr, "%lu %*s%s <--\n", pthread_self(), 2 * --indent, "", this->f); }
#endif

unique_fd::~unique_fd()
{
    if (this->fd != -1)
    {
        close(this->fd);
    }
}

void rectangle::fit(unsigned long to_width, unsigned long to_height)
{
    // fit rect within (to_width x to_height)

    if (to_width <= width()) {
        // scale to fit with
        set_bottom(top() + (static_cast<long>(to_width) * height() / width()) - 1);
        set_right(left() + to_width - 1);
    } else {
        // scale to fit height
        set_right(left() + (static_cast<long>(to_height) * width () / height()) - 1);
        set_bottom(top() + to_height - 1);
    }
}

void rectangle::center(unsigned long outer_w, unsigned long outer_h)
{
    long inner_w = width();
    long inner_h = height();

    set_left((outer_w - inner_w) / 2);
    set_right(left() + inner_w - 1);
    set_top((outer_h - inner_h) / 2);
    set_bottom(top() + inner_h - 1);
}

void rectangle::set_aspect(double ratio)
{
    // aspect ratio is width:height (= width/height)
    // e.g. Landscape of HD's ratio is 16:9 (= 1.777...)
    //      Portrait of HD's ratio is 9:16 (= 0.5625)
    //
    // width / height = ratio
    // width * height = area
    //
    // width = sqrt(ratio * area)
    // height = width / ratio

    long orig_w = width();
    long orig_h = height();

    if (ratio >= 1) {
        // width >= height
        // try to keep width
        set_right(left() + orig_w - 1);
        set_bottom(top() + static_cast<long>(orig_w / ratio + 0.5) - 1);
    } else {
        set_bottom(top() + orig_h - 1);
        set_right(left() + static_cast<long>(orig_h * ratio + 0.5) - 1);
    }
}

