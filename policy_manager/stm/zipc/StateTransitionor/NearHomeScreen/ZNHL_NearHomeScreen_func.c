/************************************************************/
/*     ZNHL_NearHomeScreen_func.c                           */
/*     Function and variable source file                    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/*************************************************************
    Function definition
*************************************************************/

/*
 * @name stm_nhl_start_activity_none
 */
void stm_nhl_start_activity_none() {
    g_stm_crr_state.layer[StmLayerNoNearHomescreen].state = StmLayoutNoNone;
    g_stm_crr_state.layer[StmLayerNoNearHomescreen].changed = STM_TRUE;
}

/*
 * @name stm_nhl_start_activity_software_keyboard
 */
void stm_nhl_start_activity_software_keyboard() {
    g_stm_crr_state.layer[StmLayerNoNearHomescreen].state = StmLayoutNoSftKbd;
    g_stm_crr_state.layer[StmLayerNoNearHomescreen].changed = STM_TRUE;
}

/*
 * @name stm_nhl_event_restriction_mode_off
 */
void stm_nhl_event_restriction_mode_off() {
	 g_stm_crr_state.layer[StmLayerNoNearHomescreen].state = g_prv_near_homescreen_state_rest_mode_1;
	 g_stm_crr_state.layer[StmLayerNoNearHomescreen].changed = STM_TRUE;
}

/*
 * @name stm_nhl_event_restriction_mode_2_on
 */
void stm_nhl_event_restriction_mode_on() {
	g_prv_near_homescreen_state_rest_mode_1 = g_stm_prv_state.layer[StmLayerNoNearHomescreen].state;
}

/*
 * @name stm_nhl_initialize_variable
 */
void stm_nhl_initialize_variable() {
	g_stm_prv_state.layer[StmLayerNoNearHomescreen].state = StmLayoutNoNone;
	g_stm_prv_state.layer[StmLayerNoNearHomescreen].changed = STM_FALSE;

	g_stm_crr_state.layer[StmLayerNoNearHomescreen].state = StmLayoutNoNone;
	g_stm_crr_state.layer[StmLayerNoNearHomescreen].changed = STM_FALSE;
}

/*
 * @name stm_nhl_start_stm
 */
void stm_nhl_start_stm() {
	if (g_stm_event == StmEvtNoUndo) {
		// nop
	}
	else {
		g_stm_prv_state.layer[StmLayerNoNearHomescreen].state = g_stm_crr_state.layer[StmLayerNoNearHomescreen].state;
	}
	g_stm_crr_state.layer[StmLayerNoNearHomescreen].changed = STM_FALSE;
}
