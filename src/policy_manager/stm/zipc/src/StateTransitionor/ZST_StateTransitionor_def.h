/************************************************************/
/*     ZST_StateTransitionor_def.h                          */
/*     Define header file                                   */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#ifndef ZHEADER_ZST_STATETRANSITIONOR_DEF_H
#define ZHEADER_ZST_STATETRANSITIONOR_DEF_H

/*************************************************************
    Define definition
*************************************************************/


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



#endif
