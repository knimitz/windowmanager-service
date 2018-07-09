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
};

const char* kStmAreaName[] = {
    "none",
    "fullscreen",
    "normal",
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

static StmState g_crr_state;
static StmState g_prv_state;

void stmInitialize() {
    // Initialize previous state
    memset(&g_prv_state, 0, sizeof(g_prv_state));

    // Initialize current state
    g_crr_state = g_prv_state;
}

int stmTransitionState(int event, StmState* state) {
    int event_no, category_no;

    event_no    = STM_GET_EVENT_FROM_ID(event);
    category_no = STM_GET_CATEGORY_FROM_ID(event);

    // Backup previous state
    g_prv_state = g_crr_state;

    // Clear current state
    memset(&g_prv_state, 0, sizeof(g_prv_state));

    // -------------------------------------------------------
    // There is no policy table by default.
    // Therefore update each layers
    // to draw the applications in normal.full area
    // in accordance with inputed activate/deactivate events.
    // -------------------------------------------------------
    if (StmEvtNoActivate == event_no)
    {
        if (StmCtgNoHomescreen == category_no)
        {
            g_crr_state.layer[StmLayerNoHomescreen].state = StmLayoutNoHms;
            g_crr_state.layer[StmLayerNoHomescreen].changed = STM_TRUE;

            g_crr_state.layer[StmLayerNoApps].state = StmLayoutNoNone;
            g_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
        }
        else if (StmCtgNoMap == category_no)
        {
            g_crr_state.layer[StmLayerNoApps].state = StmLayoutNoMapNml;
            g_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
        }
        else if (StmCtgNoGeneral == category_no)
        {
            g_crr_state.layer[StmLayerNoApps].state = StmLayoutNoGenNml;
            g_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
        }
        else if (StmCtgNoSystem == category_no)
        {
            g_crr_state.layer[StmLayerNoApps].state = StmLayoutNoSysNml;
            g_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
        }
        else if (StmCtgNoSplitable == category_no)
        {
            g_crr_state.layer[StmLayerNoApps].state = StmLayoutNoSplNml;
            g_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
        }
        else if (StmCtgNoPopUp == category_no)
        {
            g_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoPopUp;
            g_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
        }
        else if (StmCtgNoSystemAlert == category_no)
        {
            g_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoSysAlt;
            g_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
        }
    }
    else if (StmEvtNoDeactivate == event_no)
    {
        if (StmCtgNoHomescreen == category_no)
        {
            g_crr_state.layer[StmLayerNoHomescreen].state = StmLayoutNoNone;
            g_crr_state.layer[StmLayerNoHomescreen].changed = STM_TRUE;
        }
        else if (StmCtgNoMap == category_no)
        {
            if (StmLayoutNoMapNml == g_prv_state.layer[StmLayerNoApps].state )
            {
                g_crr_state.layer[StmLayerNoApps].state = StmLayoutNoNone;
                g_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
            }
        }
        else if (StmCtgNoGeneral == category_no)
        {
            if (StmLayoutNoGenNml == g_prv_state.layer[StmLayerNoApps].state )
            {
                g_crr_state.layer[StmLayerNoApps].state = StmLayoutNoNone;
                g_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
            }
        }
        else if (StmCtgNoSystem == category_no)
        {
            if (StmLayoutNoSysNml == g_prv_state.layer[StmLayerNoApps].state )
            {
                g_crr_state.layer[StmLayerNoApps].state = StmLayoutNoNone;
                g_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
            }
        }
        else if (StmCtgNoSplitable == category_no)
        {
            if (StmLayoutNoSplNml == g_prv_state.layer[StmLayerNoApps].state )
            {
                g_crr_state.layer[StmLayerNoApps].state = StmLayoutNoNone;
                g_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
            }
        }
        else if (StmCtgNoPopUp == category_no)
        {
            if (StmLayoutNoPopUp == g_prv_state.layer[StmLayerNoOnScreen].state )
            {
                g_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoNone;
                g_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
            }
        }
        else if (StmCtgNoSystemAlert == category_no)
        {
            if (StmLayoutNoSysAlt == g_prv_state.layer[StmLayerNoOnScreen].state )
            {
                g_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoNone;
                g_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
            }
        }
    }

    // Copy current state for return
    memcpy(state, &g_crr_state, sizeof(g_crr_state));

    return 0;
}

void stmUndoState() {
    g_crr_state = g_prv_state;
}
