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

#ifndef WM_UTIL_HPP
#define WM_UTIL_HPP

#include <functional>
#include <thread>
#include <sys/poll.h>
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define HMI_ERROR(args,...) _HMI_LOG(LOG_LEVEL_ERROR, __FILENAME__, __FUNCTION__, __LINE__,"wm",args, ##__VA_ARGS__)
#define HMI_WARNING(args,...) _HMI_LOG(LOG_LEVEL_WARNING, __FILENAME__, __FUNCTION__,__LINE__, "wm", args,##__VA_ARGS__)
#define HMI_NOTICE(args,...) _HMI_LOG(LOG_LEVEL_NOTICE, __FILENAME__, __FUNCTION__,__LINE__, "wm", args,##__VA_ARGS__)
#define HMI_INFO(args,...)  _HMI_LOG(LOG_LEVEL_INFO, __FILENAME__, __FUNCTION__,__LINE__, "wm", args,##__VA_ARGS__)
#define HMI_DEBUG(args,...) _HMI_LOG(LOG_LEVEL_DEBUG, __FILENAME__, __FUNCTION__,__LINE__, "wm", args,##__VA_ARGS__)

#define HMI_SEQ_ERROR(seq_num, args,...) _HMI_SEQ_LOG(LOG_LEVEL_ERROR, __FILENAME__, __FUNCTION__, __LINE__, seq_num, args, ##__VA_ARGS__)
#define HMI_SEQ_WARNING(seq_num, args,...) _HMI_SEQ_LOG(LOG_LEVEL_WARNING, __FILENAME__, __FUNCTION__, __LINE__, seq_num, args, ##__VA_ARGS__)
#define HMI_SEQ_NOTICE(seq_num, args,...) _HMI_SEQ_LOG(LOG_LEVEL_NOTICE, __FILENAME__, __FUNCTION__, __LINE__, seq_num, args, ##__VA_ARGS__)
#define HMI_SEQ_INFO(seq_num, args,...) _HMI_SEQ_LOG(LOG_LEVEL_INFO, __FILENAME__, __FUNCTION__, __LINE__, seq_num, args, ##__VA_ARGS__)
#define HMI_SEQ_DEBUG(seq_num, args,...) _HMI_SEQ_LOG(LOG_LEVEL_DEBUG, __FILENAME__, __FUNCTION__, __LINE__, seq_num, args, ##__VA_ARGS__)

#define DUMP(args, ...) _DUMP(LOG_LEVEL_INFO, args, ##__VA_ARGS__)

enum LOG_LEVEL{
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_MAX = LOG_LEVEL_DEBUG
};

void _HMI_LOG(enum LOG_LEVEL level, const char* file, const char* func, const int line, const char* prefix, const char* log, ...);
void _HMI_SEQ_LOG(enum LOG_LEVEL level, const char* file, const char* func, const int line, unsigned seq_num, const char* log, ...);
void _DUMP(enum LOG_LEVEL level, const char *log, ...);

struct rect
{
    int32_t w, h;
    int32_t x, y;
};

struct size
{
    uint32_t w, h;
};

class rectangle
{
  public:
    explicit rectangle(long wd, long ht) : _right(wd - 1), _bottom(ht - 1) {};

    void set_left(long l) {
        _left = l;
    }
    long left() const { return _left; };

    void set_right(long r) {
        _right = r;
    }
    long right() const { return _right; };

    void set_top(long t) {
        _top = t;
    }
    long top() const { return _top; };

    void set_bottom(long b) {
        _bottom = b;
    }
    long bottom() const { return _bottom; }

    long width() const {
        if (is_valid())
            return 0;
        else
            return _right - _left + 1;
    }

    long height() const {
        if (is_valid())
            return 0;
        else
            return _bottom - _top + 1;
    }

    void set_aspect(double ratio);
    void fit(unsigned long to_width, unsigned long to_height);
    void center(unsigned long outer_w, unsigned long outer_h);

  private:
    bool is_valid() const {
        return (_top > _bottom || _left > _right);
    }

    long _left = 0;
    long _top = 0;
    long _right;
    long _bottom;
};

#endif // !WM_UTIL_HPP
