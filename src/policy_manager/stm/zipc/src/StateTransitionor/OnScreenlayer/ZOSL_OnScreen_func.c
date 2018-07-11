/************************************************************/
/*     ZOSL_OnScreen_func.c                                 */
/*     Function and variable source file                    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/*************************************************************
    Function definition
*************************************************************/

/*
 * @name stm_osl_start_activity_none
 */
void stm_osl_start_activity_none() {
    g_stm_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoNone;
    g_stm_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
}

/*
 * @name stm_osl_start_activity_pop_up
 */
void stm_osl_start_activity_pop_up() {
    g_stm_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoPopUp;
    g_stm_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
}

/*
 * @name stm_osl_start_activity_system_alert
 */
void stm_osl_start_activity_system_alert() {
    g_stm_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoSysAlt;
    g_stm_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
}

/*
 * @name stm_osl_event_restriction_mode_off
 */
void stm_osl_event_restriction_mode_off() {
	 g_stm_crr_state.layer[StmLayerNoOnScreen].state = g_prv_on_screen_state_rest_mode_1;
	 g_stm_crr_state.layer[StmLayerNoOnScreen].changed = STM_TRUE;
}

/*
 * @name stm_osl_event_restriction_mode_2_on
 */
void stm_osl_event_restriction_mode_2_on() {
	g_prv_on_screen_state_rest_mode_1 = g_stm_prv_state.layer[StmLayerNoOnScreen].state;
}
/*
 * @name stm_osl_initialize_variable
 */
void stm_osl_initialize_variable() {
	g_stm_prv_state.layer[StmLayerNoOnScreen].state = StmLayoutNoNone;
	g_stm_prv_state.layer[StmLayerNoOnScreen].changed = STM_FALSE;

	g_stm_crr_state.layer[StmLayerNoOnScreen].state = StmLayoutNoNone;
	g_stm_crr_state.layer[StmLayerNoOnScreen].changed = STM_FALSE;
}

/*
 * @name stm_osl_start_stm
 */
void stm_osl_start_stm() {
	if (g_stm_event == StmEvtNoUndo) {
		// nop
	}
	else {
		g_stm_prv_state.layer[StmLayerNoOnScreen].state = g_stm_crr_state.layer[StmLayerNoOnScreen].state;
	}
	g_stm_crr_state.layer[StmLayerNoOnScreen].changed = STM_FALSE;
}
