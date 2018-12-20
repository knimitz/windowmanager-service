/*
 * Copyright (c) 2017 TOYOTA MOTOR CORPORATION
 * Copyright (c) 2018 Konsulko Group
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

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fstream>

#include <unistd.h>

static char ERROR_FLAG[6][20] = {"NONE", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};

void rectangle::fit(unsigned long to_width, unsigned long to_height)
{
    // fit rect within (to_width x to_height)

    if (to_width <= (unsigned long)width()) {
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

void _HMI_LOG(enum LOG_LEVEL level, const char* file, const char* func, const int line, const char* prefix, const char* log, ...)
{
    const int log_level = (getenv("USE_HMI_DEBUG") == NULL)?LOG_LEVEL_ERROR:atoi(getenv("USE_HMI_DEBUG"));
    if(log_level < level)
    {
        return;
    }

    char *message;
    struct timespec tp;
    unsigned int time;

    clock_gettime(CLOCK_REALTIME, &tp);
    time = (tp.tv_sec * 1000000L) + (tp.tv_nsec / 1000);

    va_list args;
    va_start(args, log);
    if (log == NULL || vasprintf(&message, log, args) < 0)
        message = NULL;
    fprintf(stderr,  "[%10.3f] [%s %s] [%s, %s(), Line:%d] >>> %s \n", time / 1000.0, prefix, ERROR_FLAG[level], file, func, line, message);
    va_end(args);
    free(message);
}

void _HMI_SEQ_LOG(enum LOG_LEVEL level, const char* file, const char* func, const int line, unsigned seq_num, const char* log, ...){
    const int log_level = (getenv("USE_HMI_DEBUG") == NULL) ? LOG_LEVEL_ERROR:atoi(getenv("USE_HMI_DEBUG"));
    if(log_level < level)
    {
        return;
    }

    char *message;
    struct timespec tp;
    unsigned int time;

    clock_gettime(CLOCK_REALTIME, &tp);
	time = (tp.tv_sec * 1000000L) + (tp.tv_nsec / 1000);

	va_list args;
	va_start(args, log);
	if (log == NULL || vasprintf(&message, log, args) < 0)
        message = NULL;
    fprintf(stderr,  "[%10.3f] [wm %s] [%s, %s(), Line:%d] >>> req %d: %s \n", time / 1000.0, ERROR_FLAG[level], file, func, line, seq_num, message);
    va_end(args);
	free(message);
}

void _DUMP(enum LOG_LEVEL level, const char *log, ...)
{
    const int log_level = (getenv("USE_HMI_DEBUG") == NULL) ? LOG_LEVEL_ERROR : atoi(getenv("USE_HMI_DEBUG"));
    if (log_level < level)
    {
        return;
    }
    char *message;
    va_list args;
    va_start(args, log);
    if (log == NULL || vasprintf(&message, log, args) < 0)
        message = NULL;
    fprintf(stderr, "%s \n", message);
    va_end(args);
    free(message);
}

std::string get_file_path(const char *file_name, const char *root_path)
{
    char const *default_base_path = root_path;
    std::string path("");

    if(!file_name) {
        return path;
    }

    if (!default_base_path) {
        default_base_path = getenv("AFM_APP_INSTALL_DIR");
        if (!default_base_path) {
            HMI_ERROR("AFM_APP_INSTALL_DIR is not defined");
        }
    }
    if (default_base_path) {
        path = default_base_path;
        path.append("/etc/");
        path.append(file_name);
    }

    // Check for over-ride in /etc/xdg/windowmanager
    std::string override_path("/etc/xdg/windowmanager/");
    override_path.append(file_name);
    std::ifstream i(override_path);
    if (i.good()) {
        path = override_path;
    }
    i.close();

    HMI_INFO("Using %s", path.c_str());
    return path;
}

void ChangeAreaReq::dump()
{
    DUMP("=== change request dump ===");
    DUMP("request from : %s", this->appname.c_str());
    DUMP("save : %s", this->save ? "true" : "false");
    for(const auto& req : this->area_req)
    {
        DUMP("area change req : %s", req.first.c_str());
        DUMP("  x:%d y:%d w:%d h:%d",req.second.x, req.second.y, req.second.w, req.second.h);
    }
    for(const auto& req : this->update_app2area)
    {
        DUMP("update change app : %s, area:%s", req.first.c_str(), req.second.c_str());
    }
    DUMP("======== dump end =========");
}
