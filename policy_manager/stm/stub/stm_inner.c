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

static StmState g_stm_crr_state;
static StmState g_stm_prv_state;

void stmInitializeInner() {
    // Initialize previous state
    memset(&g_stm_prv_state, 0, sizeof(g_stm_prv_state));

    // Initialize current state
    g_stm_crr_state = g_stm_prv_state;
}

int stmTransitionStateInner(int event, StmState* state) {
    int event_no, category_no, area_no;

    event_no    = STM_GET_EVENT_FROM_ID(event);
    category_no = STM_GET_CATEGORY_FROM_ID(event);
    area_no     = STM_GET_AREA_FROM_ID(event);

    // Backup previous state
    g_stm_prv_state = g_stm_crr_state;

    // -------------------------------------------------------
    // There is no policy table by default.
    // Therefore update each layers
    // to draw the applications in requested area
    // in accordance with inputed activate/deactivate events.
    // -------------------------------------------------------
    if (StmEvtNoActivate == event_no)
    {
        if (StmCtgNoHomescreen == category_no)
        {
            g_stm_crr_state.layer[StmLayerNoHomescreen].state = StmLayoutNoHms;
            g_stm_crr_state.layer[StmLayerNoHomescreen].changed = STM_TRUE;

            // For AGL JIRA SPEC-1407
            // Apps layer is invisibled only when Homescreen app is started already
            if (StmLayoutNoHms == g_stm_prv_state.layer[StmLayerNoHomescreen].state)
            {
                g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoNone;
                g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
            }
        }
        else if (StmCtgNoDebug == category_no)
        {
            if (StmAreaNoNormal == area_no)
            {
                g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoDbgNml;
            }
            else if (StmAreaNoSplitMain == area_no)
            {
                g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoDbgSplMain;
            }
            else if (StmAreaNoSplitSub == area_no)
            {
                g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoDbgSplSub;
            }
            else if (StmAreaNoFullscreen == area_no)
            {
                g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoDbgFll;
            }
            g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
        }
        else if (StmCtgNoSoftwareKeyboard == category_no)
        {
            g_stm_crr_state.layer[StmLayerNoNearHomescreen].state = StmLayoutNoSftKbd;
            g_stm_crr_state.layer[StmLayerNoNearHomescreen].changed = STM_TRUE;
        }
        else if (StmCtgNoPopUp == category_no)
        {
            g_stm_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoPopUp;
            g_stm_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
        }
        else if (StmCtgNoSystemAlert == category_no)
        {
            g_stm_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoSysAlt;
            g_stm_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
        }
    }
    else if (StmEvtNoDeactivate == event_no)
    {
        if (StmCtgNoHomescreen == category_no)
        {
            g_stm_crr_state.layer[StmLayerNoHomescreen].state = StmLayoutNoNone;
            g_stm_crr_state.layer[StmLayerNoHomescreen].changed = STM_TRUE;
        }
        else if (StmCtgNoDebug == category_no)
        {
            if ((StmLayoutNoDbgNml     == g_stm_prv_state.layer[StmLayerNoApps].state) ||
                (StmLayoutNoDbgSplMain == g_stm_prv_state.layer[StmLayerNoApps].state) ||
                (StmLayoutNoDbgSplSub  == g_stm_prv_state.layer[StmLayerNoApps].state) ||
                (StmLayoutNoDbgFll     == g_stm_prv_state.layer[StmLayerNoApps].state))
            {
                g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoNone;
                g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
            }
        }
        else if (StmCtgNoSoftwareKeyboard == category_no)
        {
            if (StmLayoutNoSftKbd == g_stm_prv_state.layer[StmLayerNoNearHomescreen].state )
            {
                g_stm_crr_state.layer[StmLayerNoNearHomescreen].state = StmLayoutNoNone;
                g_stm_crr_state.layer[StmLayerNoNearHomescreen].changed = STM_TRUE;
            }
        }
        else if (StmCtgNoPopUp == category_no)
        {
            if (StmLayoutNoPopUp == g_stm_prv_state.layer[StmLayerNoOnScreen].state )
            {
                g_stm_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoNone;
                g_stm_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
            }
        }
        else if (StmCtgNoSystemAlert == category_no)
        {
            if (StmLayoutNoSysAlt == g_stm_prv_state.layer[StmLayerNoOnScreen].state )
            {
                g_stm_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoNone;
                g_stm_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
            }
        }
    }

    // Copy current state for return
    memcpy(state, &g_stm_crr_state, sizeof(g_stm_crr_state));

    return 0;
}

void stmUndoStateInner() {
    g_stm_crr_state = g_stm_prv_state;
}
