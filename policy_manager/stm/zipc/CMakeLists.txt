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

set(TARGETS_STM pmstm)

set(ST_DIR     StateTransitionor)
set(ST_DIR_CMN Common)
set(ST_DIR_AL  ${ST_DIR}/AppsLayer)
set(ST_DIR_HS  ${ST_DIR}/HomeScreenLayer)
set(ST_DIR_NHS ${ST_DIR}/NearHomeScreen)
set(ST_DIR_OS  ${ST_DIR}/OnScreenlayer)
set(ST_DIR_RL  ${ST_DIR}/RestrictionLayer)
set(ST_DIR_RM  ${ST_DIR}/RestrictionMode)

add_library(${TARGETS_STM}
    STATIC
        ${ST_DIR}/ZST_StateTransitionor_func.c
        ${ST_DIR}/ZST_StateTransitionor_var.c
        ${ST_DIR_AL}/ZAPL_AppsLayer.c
        ${ST_DIR_AL}/ZAPL_Apps_func.c
        ${ST_DIR_HS}/ZHSL_HomeScreen.c
        ${ST_DIR_HS}/ZHSL_HomeScreen_func.c
        ${ST_DIR_NHS}/ZNHL_NearHomescreen.c
        ${ST_DIR_NHS}/ZNHL_NearHomeScreen_func.c
        ${ST_DIR_OS}/ZOSL_OslMain.c
        ${ST_DIR_OS}/ZOSL_OnScreen_func.c
        ${ST_DIR_RL}/ZREL_RelMain.c
        ${ST_DIR_RL}/ZREL_Restriction_func.c
        ${ST_DIR_RM}/ZREM_RestrictionMode.c
        ${ST_DIR_RM}/ZREM_RestrictionMode_func.c
)

target_include_directories(${TARGETS_STM}
    PRIVATE
        ./
        ./${ST_DIR}
        ./${ST_DIR_AL}
        ./${ST_DIR_HS}
        ./${ST_DIR_NHS}
        ./${ST_DIR_OS}
        ./${ST_DIR_RL}
        ./${ST_DIR_RM}
        ./${ST_DIR_CMN}
)

target_compile_definitions(${TARGETS_STM}
    PRIVATE
        _GNU_SOURCE
)

target_compile_options(${TARGETS_STM}
    PRIVATE
        -Wall -Wextra -Wno-unused-parameter -Wno-comment)

set_target_properties(${TARGETS_STM}
    PROPERTIES
        C_EXTENSIONS OFF
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
)
