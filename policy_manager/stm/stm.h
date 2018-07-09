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

#ifndef TMCAGLWM_STM_HPP
#define TMCAGLWM_STM_HPP

#define STM_TRUE 1
#define STM_FALSE 0

#define STM_CREATE_EVENT_ID(evt, ctg, area) \
    ((evt) | ((ctg) << 8) | ((area) << 16))

#define STM_GET_EVENT_FROM_ID(id) \
    ((id) & 0xFF)

#define STM_GET_CATEGORY_FROM_ID(id) \
    (((id) >> 8) & 0xFF)

#define STM_GET_AREA_FROM_ID(id) \
    (((id) >> 16) & 0xFF)

// Event number
enum StmEvtNo {
    StmEvtNoNone = 0,
    StmEvtNoActivate,
    StmEvtNoDeactivate,
    StmEvtNoRestrictionModeOff,
    StmEvtNoRestrictionMode1On,
    StmEvtNoRestrictionMode2On,
    StmEvtNoUndo,

    StmEvtNoNum,

    StmEvtNoMin = StmEvtNoNone,
    StmEvtNoMax = StmEvtNoNum - 1,
};

// Category number
enum StmCtgNo {
    StmCtgNoNone = 0,
    StmCtgNoHomescreen,
    StmCtgNoMap,
    StmCtgNoGeneral,
    StmCtgNoSplitable,
    StmCtgNoPopUp,
    StmCtgNoSystemAlert,
    StmCtgNoRestriction,
    StmCtgNoSystem,
    StmCtgNoSoftwareKeyboard,
    StmCtgNoDebug,

    StmCtgNoNum,

    StmCtgNoMin = StmCtgNoNone,
    StmCtgNoMax = StmCtgNoNum - 1,
};

// Area number
enum StmAreaNo {
    StmAreaNoNone = 0,
    StmAreaNoFullscreen,
    StmAreaNoNormal,
    StmAreaNoSplitMain,
    StmAreaNoSplitSub,
    StmAreaNoOnScreen,
    StmAreaNoRestrictionNormal,
    StmAreaNoRestrictionSplitMain,
    StmAreaNoRestrictionSplitSub,
    StmAreaNoSoftwareKyeboard,

    StmAreaNoNum,

    StmAreaNoMin = StmAreaNoNone,
    StmAreaNoMax = StmAreaNoNum - 1,
};

// Layer number
enum StmLayerNo {
    StmLayerNoHomescreen = 0,
    StmLayerNoApps,
    StmLayerNoNearHomescreen,
    StmLayerNoRestriction,
    StmLayerNoOnScreen,

    StmLayerNoNum,

    StmLayerNoMin = StmLayerNoHomescreen,
    StmLayerNoMax = StmLayerNoNum - 1,
};

// Layout kind number
enum StmLayoutNo {
    StmLayoutNoNone = 0,
    StmLayoutNoPopUp,
    StmLayoutNoSysAlt,
    StmLayoutNoMapNml,
    StmLayoutNoMapSpl,
    StmLayoutNoMapFll,
    StmLayoutNoSplNml,
    StmLayoutNoSplSpl,
    StmLayoutNoGenNml,
    StmLayoutNoHms,
    StmLayoutNoRstNml,
    StmLayoutNoRstSplMain,
    StmLayoutNoRstSplSub,
    StmLayoutNoSysNml,
    StmLayoutNoSftKbd,
    StmLayoutNoDbgNml,
    StmLayoutNoDbgSplMain,
    StmLayoutNoDbgSplSub,
    StmLayoutNoDbgFll,

    StmLayoutNoNum,

    StmLayoutNoMin = StmLayoutNoNone,
    StmLayoutNoMax = StmLayoutNoNum - 1,
};

// Mode kind number
enum StmModeNo {
    StmModeNoRestrictionMode = 0,

    StmModeNoNum,

    StmModeNoMin = StmModeNoRestrictionMode,
    StmModeNoMax = StmModeNoNum - 1,
};

// Enum for mode state
enum StmRestrictionModeSttNo {
    StmRestrictionModeSttNoOff = 0,
    StmRestrictionModeSttNo1On,
    StmRestrictionModeSttNo2On,
};

// String for state
extern const char* kStmEventName[];
extern const char* kStmCategoryName[];
extern const char* kStmAreaName[];
extern const char* kStmLayoutName[];
extern const char* kStmLayerName[];
extern const char* kStmModeName[];
extern const char** kStmModeStateNameList[];

// Struct for state
typedef struct StmBaseState {
    int changed;
    int state;
} StmBaseState;

typedef struct StmState {
    StmBaseState mode[StmModeNoNum];
    StmBaseState layer[StmLayerNoNum];
} StmState;

// API
void stmInitialize();
int stmTransitionState(int event_no, StmState* state);
void stmUndoState();


#endif  // TMCAGLWM_STM_HPP
