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

//=================================
// API
//=================================
/**
 *  Initialize STM
 */
void stmInitializeInner() {
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
int stmTransitionStateInner(int event_id, StmState* state) {
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
void stmUndoStateInner() {
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

