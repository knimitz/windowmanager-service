#
# Copyright (c) 2017 TOYOTA MOTOR CORPORATION
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include(FindPkgConfig)

pkg_check_modules(AFB REQUIRED libafbwsc)
pkg_check_modules(SD REQUIRED libsystemd>=222)

set(TARGET_LIBWM windowmanager)

add_library(${TARGET_LIBWM} SHARED
   libwindowmanager.cpp
   libwindowmanager.h)

target_include_directories(${TARGET_LIBWM}
    PRIVATE
        ${AFB_INCLUDE_DIRS}
        ${SD_INCLUDE_DIRS})

target_link_libraries(${TARGET_LIBWM}
   PUBLIC
        -lpthread
        ${AFB_LIBRARIES}
        ${SD_LIBRARIES})

target_compile_definitions(${TARGET_LIBWM}
    PRIVATE
        _GNU_SOURCE)  # XXX should I define this here?!

if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
   target_compile_definitions(${TARGET_LIBWM}
           PRIVATE
           _GLIBCXX_DEBUG)
endif()

target_compile_options(${TARGET_LIBWM}
    PRIVATE
        -Wall -Wextra -Wno-unused-parameter -Wno-comment)

set_target_properties(${TARGET_LIBWM}
    PROPERTIES
    # INTERPROCEDURAL_OPTIMIZATION ON
        CXX_EXTENSIONS OFF
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON)

if (LINK_LIBCXX)
   set_target_properties(${TARGET_LIBWM}
           PROPERTIES
           LINK_FLAGS "-lc++")
endif()

if (NOT ${SANITIZER_MODE} STREQUAL "none" AND NOT ${SANITIZER_MODE} STREQUAL "")
   target_compile_options(${TARGET_LIBWM}
      PRIVATE
         -fsanitize=${SANITIZER_MODE} -g -fno-omit-frame-pointer)
   set_target_properties(${TARGET_LIBWM}
      PROPERTIES
         LINK_FLAGS "-fsanitize=${SANITIZER_MODE} -g")
endif()

install(
   TARGETS ${TARGET_LIBWM}
   DESTINATION ${CMAKE_INSTALL_LIBDIR}
   COMPONENT "runtime")

install(
   FILES libwindowmanager.h
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   COMPONENT "development")

add_subdirectory(doc)
