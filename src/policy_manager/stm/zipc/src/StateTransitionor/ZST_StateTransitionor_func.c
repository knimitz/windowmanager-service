/************************************************************/
/*     ZST_StateTransitionor_func.c                         */
/*     Function and variable source file                    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "ZST_include.h"

/*************************************************************
    Function definition
*************************************************************/

#include <string.h>


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


//=================================
// API
//=================================
/**
 *  Initialize STM
 */
void stmInitialize() {
    // Initialize previous state
    memset(&g_stm_prv_state, 0, sizeof(g_stm_prv_state));

    // Initialize current state
    g_stm_crr_state = g_stm_prv_state;

	/* Initialize restriction mode state */
	stm_rem_initialize();
	stm_rem_initialize_variable();

	// Initialize homecsreen layer
	stm_hsl_initialize();
	stm_hsl_initialize_variable();

	// Initialize apps layer
	stm_apl_initialize();
	stm_apl_initialize_variable();

	// Initialize near_homecsreen layer
	stm_nhl_initialize();
	stm_nhl_initialize_variable();

	/* Initialize restriction layer */
	stm_rel_initialize();
	stm_rel_initialize_variable();

	g_stm_map_is_activated = STM_FALSE;
}

/**
 *  Transition State
 */
int stmTransitionState(int event_id, StmState* state) {
    g_stm_event    = STM_GET_EVENT_FROM_ID(event_id);
    g_stm_category = STM_GET_CATEGORY_FROM_ID(event_id);
    g_stm_area     = STM_GET_AREA_FROM_ID(event_id);

    // restriction mode
    stm_rem_event_call();

    // homescreen layer
    stm_hsl_event_call();

    // apps layer
    stm_apl_event_call();

    // near_homecsreen layer
    stm_nhl_event_call();

    // restriction layer
    stm_rel_event_call();

    // on_screen layer
    stm_osl_event_call();

    // Copy current state for return
    memcpy(state, &g_stm_crr_state, sizeof(g_stm_crr_state));

    return 0;
}

/**
 *  Undo State
 */
void stmUndoState() {
    g_stm_event = StmEvtNoUndo;

    // apps layer
    stm_apl_event_call();

    // near_homecsreen layer
    stm_nhl_event_call();

    // restriction layer
    stm_rel_event_call();

    // on_screen layer
    stm_osl_event_call();

	g_stm_crr_state = g_stm_prv_state;
}


