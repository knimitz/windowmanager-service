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

#ifndef WM_UTIL_HPP
#define WM_UTIL_HPP

#include <functional>
#include <thread>
#include <vector>
#include <string>

#include <sys/poll.h>

#define CONCAT_(X, Y) X##Y
#define CONCAT(X, Y) CONCAT_(X, Y)

#ifdef __GNUC__
#define ATTR_FORMAT(stringindex, firsttocheck) \
    __attribute__((format(printf, stringindex, firsttocheck)))
#define ATTR_NORETURN __attribute__((noreturn))
#else
#define ATTR_FORMAT(stringindex, firsttocheck)
#define ATTR_NORETURN
#endif

#ifdef AFB_BINDING_VERSION
#define lognotice(...) AFB_NOTICE(__VA_ARGS__)
#define logerror(...) AFB_ERROR(__VA_ARGS__)
#define fatal(...)              \
    do                          \
    {                           \
        AFB_ERROR(__VA_ARGS__); \
        abort();                \
    } while (0)
#else
#define lognotice(...)
#define logerror(...)
#define fatal(...) \
    do             \
    {              \
        abort();   \
    } while (0)
#endif

#ifdef DEBUG_OUTPUT
#ifdef AFB_BINDING_VERSION
#define logdebug(...) AFB_DEBUG(__VA_ARGS__)
#else
#define logdebug(...)
#endif
#else
#define logdebug(...)
#endif

#ifndef SCOPE_TRACING
#define ST()
#define STN(N)
#else
#define ST() \
    ScopeTrace __attribute__((unused)) CONCAT(trace_scope_, __LINE__)(__func__)
#define STN(N) \
    ScopeTrace __attribute__((unused)) CONCAT(named_trace_scope_, __LINE__)(#N)

struct ScopeTrace
{
    thread_local static int indent;
    char const *f{};
    explicit ScopeTrace(char const *func);
    ~ScopeTrace();
};
#endif

/**
 * @struct unique_fd
 */
struct unique_fd
{
    int fd{-1};
    unique_fd() = default;
    explicit unique_fd(int f) : fd{f} {}
    operator int() const { return fd; }
    ~unique_fd();
    unique_fd(unique_fd const &) = delete;
    unique_fd &operator=(unique_fd const &) = delete;
    unique_fd(unique_fd &&o) : fd(o.fd) { o.fd = -1; }
    unique_fd &operator=(unique_fd &&o)
    {
        std::swap(this->fd, o.fd);
        return *this;
    }
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

// Configuration file path helper
std::string get_file_path(const char *file_name, const char *log_category = "wm");

#endif // !WM_UTIL_HPP
