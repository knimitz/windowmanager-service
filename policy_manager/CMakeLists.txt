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
pkg_check_modules(SD REQUIRED libsystemd>=222)

# Set name of STM
set(STM_DIR stub)
#set(STM_DIR zipc)

# Select roles.db
if(zipc MATCHES ${STM_DIR})
    # if trying to show split layout, change from OFF to ON
    set(TRY_SPLIT_LAYOUT OFF CACHE BOOL "Enable to show split layout")

    if(${TRY_SPLIT_LAYOUT})
        set(ROLES_DB_NAME roles.db.zipc.split)
    else()
        set(ROLES_DB_NAME roles.db.zipc)
    endif()
else()
    set(ROLES_DB_NAME roles.db)
endif()

# Add STM directory
add_subdirectory(stm/${STM_DIR})

set(TARGETS_PM ${PLUGIN_PM})

add_library(${TARGETS_PM}
    SHARED
        policy_manager.cpp
        stm/stm.c
        ../src/util.cpp
)

target_include_directories(${TARGETS_PM}
    PRIVATE
        ${AFB_INCLUDE_DIRS}
        ${SD_INCLUDE_DIRS}
        ../include
        ../src
        ./
        ./stm
        ./stm/${STM_DIR}
)

target_link_libraries(${TARGETS_PM}
    PRIVATE
        ${AFB_LIBRARIES}
        ${SD_LIBRARIES}
        pmstm
)

target_compile_definitions(${TARGETS_PM}
    PRIVATE
        _GNU_SOURCE
)

target_compile_options(${TARGETS_PM}
    PRIVATE
        -Wall -Wextra -Wno-unused-parameter -Wno-comment)

set_target_properties(${TARGETS_PM}
    PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON

        C_EXTENSIONS OFF
        C_STANDARD 99
        C_STANDARD_REQUIRED ON

        LINK_FLAGS "-Wl,-rpath,'$ORIGIN'"
)

if (LINK_LIBCXX)
    set_target_properties(${TARGETS_PM}
        PROPERTIES
            LINK_FLAGS "-lc++ -Wl,-rpath,'$ORIGIN'"
    )
endif()

add_custom_command(TARGET ${TARGETS_PM} POST_BUILD
   COMMAND mkdir -p ${PROJECT_BINARY_DIR}/package/root/lib
   COMMAND cp -f ${PROJECT_BINARY_DIR}/${PLUGIN_PM}/lib${PLUGIN_PM}.so ${PROJECT_BINARY_DIR}/package/root/lib
   COMMAND mkdir -p ${PROJECT_BINARY_DIR}/package/root/etc
   COMMAND cp -f ${PROJECT_SOURCE_DIR}/${PLUGIN_PM}/db/states.db ${PROJECT_BINARY_DIR}/package/root/etc
   COMMAND cp -f ${PROJECT_SOURCE_DIR}/${PLUGIN_PM}/db/${ROLES_DB_NAME} ${PROJECT_BINARY_DIR}/package/root/etc/roles.db
)
