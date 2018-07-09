/*
 * Copyright (c) 2018 TOYOTA MOTOR CORPORATION
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

#include <string.h>
#include "stm.h"
#include "stm_inner.h"

const char* kStmEventName[] = {
    "none",
    "activate",
    "deactivate",
    "restriction_mode_off",
    "restriction_mode_1_on",
    "restriction_mode_2_on",
    "undo",
};

const char* kStmCategoryName[] = {
    "none",
    "homescreen",
    "map",
    "general",
    "splitable",
    "pop_up",
    "system_alert",
    "restriction",
    "system",
    "software_keyboard",
    "debug",
};

const char* kStmAreaName[] = {
    "none",
    "fullscreen",
    "normal.full",
    "split.main",
    "split.sub",
    "on_screen",
    "restriction.normal",
    "restriction.split.main",
    "restriction.split.sub",
    "software_keyboard",
};

const char* kStmLayoutName[] = {
    "none",
    "pop_up",
    "system_alert",
    "map.normal",
    "map.split",
    "map.fullscreen",
    "splitable.normal",
    "splitable.split",
    "general.normal",
    "homescreen",
    "restriction.normal",
    "restriction.split.main",
    "restriction.split.sub",
    "system.normal",
    "software_keyboard",
    "debug.normal",
    "debug.split.main",
    "debug.split.sub",
    "debug.fullscreen",
};

const char* kStmLayerName[] = {
    "homescreen",
    "apps",
    "near_homescreen",
    "restriction",
    "on_screen",
};

const char* kStmModeName[] = {
    "trans_gear",
    "parking_brake",
    "accel_pedal",
    "running",
    "lamp",
    "lightstatus_brake",
    "restriction_mode",
};

const char* kStmRestrictionModeStateName[] = {
    "off",
    "1on",
    "2on",
};

const char** kStmModeStateNameList[] = {
    kStmRestrictionModeStateName,
};

void stmInitialize() {
    stmInitializeInner();
}

int stmTransitionState(int event, StmState* state) {
    return stmTransitionStateInner(event, state);
}

void stmUndoState() {
    stmUndoStateInner();
}
