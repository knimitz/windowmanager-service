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

# We do not want a prefix for our module
set(CMAKE_SHARED_MODULE_PREFIX "")

set(TARGETS_PM lib${PLUGIN_PM})

add_library(${TARGETS_PM} MODULE
   policy_manager.cpp
)

target_include_directories(${TARGETS_PM}
    PRIVATE
        ../../include
        ../
        ./
        ./stm/${USE_STM_NAME}/src/include
)

target_link_libraries(${TARGETS_PM}
    PRIVATE
        ${CMAKE_BINARY_DIR}/src/${PLUGIN_PM}/stm/${USE_STM_NAME}/src/libstm.so
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
)

install(
   TARGETS ${TARGET_PM}
   DESTINATION ${CMAKE_INSTALL_LIBDIR}
   COMPONENT "runtime"
)
