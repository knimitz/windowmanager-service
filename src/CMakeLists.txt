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
pkg_check_modules(AFB REQUIRED afb-daemon)
pkg_check_modules(ILM REQUIRED ilmControl ilmCommon)
pkg_check_modules(SD REQUIRED libsystemd>=222)

# We do not want a prefix for our module
set(CMAKE_SHARED_MODULE_PREFIX "")

set(TARGETS_WM windowmanager-service)

add_library(${TARGETS_WM} MODULE
   main.cpp
   util.cpp
   json_helper.cpp
   applist.cpp
   request.cpp
   pm_wrapper.cpp
   window_manager.cpp
   wm_client.cpp
   wm_error.cpp
   wm_layer.cpp
   wm_layer_control.cpp)

target_include_directories(${TARGETS_WM}
    PRIVATE
        ${AFB_INCLUDE_DIRS}
        ${SD_INCLUDE_DIRS}
        ${ILM_INCLUDE_DIRS}
        ../include
        ../src
        ../${PLUGIN_PM})

target_link_libraries(${TARGETS_WM}
    PRIVATE
        ${AFB_LIBRARIES}
        ${WLC_LIBRARIES}
        ${ILM_LIBRARIES}
        ${SD_LIBRARIES}
        ${PLUGIN_PM})

target_compile_definitions(${TARGETS_WM}
    PRIVATE
        AFB_BINDING_VERSION=3
        # We do not want source location of messages
        AFB_BINDING_PRAGMA_NO_VERBOSE_DETAILS
        WINMAN_VERSION_STRING="${PACKAGE_VERSION}"
        _GNU_SOURCE)

if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
   target_compile_definitions(${TARGETS_WM}
       PRIVATE
           _GLIBCXX_DEBUG)
endif()

target_compile_options(${TARGETS_WM}
    PRIVATE
        -Wall -Wextra -Wno-unused-parameter -Wno-comment -Wno-missing-field-initializers)

set_target_properties(${TARGETS_WM}
    PROPERTIES
    # INTERPROCEDURAL_OPTIMIZATION ON

        CXX_EXTENSIONS OFF
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON

        C_EXTENSIONS OFF
        C_STANDARD 99
        C_STANDARD_REQUIRED ON

        LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../export.map -Wl,-rpath,'$ORIGIN'")

if (LINK_LIBCXX)
   set_target_properties(${TARGETS_WM}
           PROPERTIES
           LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../export.map -lc++ -Wl,-rpath,'$ORIGIN'")
endif()

if (NOT ${SANITIZER_MODE} STREQUAL "none" AND NOT ${SANITIZER_MODE} STREQUAL "")
   target_compile_options(${TARGETS_WM}
      PRIVATE
         -fsanitize=${SANITIZER_MODE} -g -fno-omit-frame-pointer)
   set_target_properties(${TARGETS_WM}
      PROPERTIES
         LINK_FLAGS "-fsanitize=${SANITIZER_MODE} -g")
endif()

if(NOT EXISTS ${PROJECT_BINARY_DIR}/package)
   add_custom_command(TARGET ${TARGETS_WM} POST_BUILD
      COMMAND cp -rf ${PROJECT_SOURCE_DIR}/package ${PROJECT_BINARY_DIR}
   )
endif()

add_custom_command(TARGET ${TARGETS_WM} POST_BUILD
   COMMAND mkdir -p ${PROJECT_BINARY_DIR}/package/root/lib
   COMMAND cp -rf ${PROJECT_BINARY_DIR}/src/${TARGETS_WM}.so ${PROJECT_BINARY_DIR}/package/root/lib
   COMMAND mkdir -p ${PROJECT_BINARY_DIR}/package/root/etc
   COMMAND cp -f ${CMAKE_SOURCE_DIR}/conf/layers.json ${PROJECT_BINARY_DIR}/package/root/etc
   COMMAND cp -f ${CMAKE_SOURCE_DIR}/conf/areas.json ${PROJECT_BINARY_DIR}/package/root/etc
)

add_custom_target(package DEPENDS ${PROJECT_BINARY_DIR}/package/root
   COMMAND wgtpkg-pack -f -o ${PROJECT_BINARY_DIR}/package/${TARGETS_WM}.wgt ${PROJECT_BINARY_DIR}/package/root
)
