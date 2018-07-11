/************************************************************/
/*     ZREL_Restriction_func.c                              */
/*     Function and variable source file                    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/*************************************************************
    Function definition
*************************************************************/

/*
 * @name stm_rel_start_activity_none
 */
void stm_rel_start_activity_none() {
    g_stm_crr_state.layer[StmLayerNoRestriction].state = StmLayoutNoNone;
    g_stm_crr_state.layer[StmLayerNoRestriction].changed = STM_TRUE;
}

/*
 * @name stm_rel_start_activity_restriction_normal
 */
void stm_rel_start_activity_restriction_normal() {
    g_stm_crr_state.layer[StmLayerNoRestriction].state = StmLayoutNoRstNml;
    g_stm_crr_state.layer[StmLayerNoRestriction].changed = STM_TRUE;
}

/*
 * @name stm_rel_start_activity_restriction_split_main
 */
void stm_rel_start_activity_restriction_split_main() {
    g_stm_crr_state.layer[StmLayerNoRestriction].state = StmLayoutNoRstSplMain;
    g_stm_crr_state.layer[StmLayerNoRestriction].changed = STM_TRUE;
}

/*
 * @name stm_rel_start_activity_restriction_split_sub
 */
void stm_rel_start_activity_restriction_split_sub() {
    g_stm_crr_state.layer[StmLayerNoRestriction].state = StmLayoutNoRstSplSub;
    g_stm_crr_state.layer[StmLayerNoRestriction].changed = STM_TRUE;
}

/*
 * @name stm_rel_event_restriction_mode_off
 */
void stm_rel_event_restriction_mode_off() {
	 g_stm_crr_state.layer[StmLayerNoRestriction].state = g_prv_restriction_state_rest_mode_1;
	 g_stm_crr_state.layer[StmLayerNoRestriction].changed = STM_TRUE;
}

/*
 * @name stm_rel_event_restriction_mode_2_on
 */
void stm_rel_event_restriction_mode_2_on() {
	g_prv_restriction_state_rest_mode_1 = g_stm_prv_state.layer[StmLayerNoRestriction].state;
}

/*
 * @name stm_rel_initialize_variable
 */
void stm_rel_initialize_variable() {
    g_stm_prv_state.layer[StmLayerNoRestriction].state = StmLayoutNoNone;
    g_stm_prv_state.layer[StmLayerNoRestriction].changed = STM_FALSE;

    g_stm_crr_state.layer[StmLayerNoRestriction].state = StmLayoutNoNone;
    g_stm_crr_state.layer[StmLayerNoRestriction].changed = STM_FALSE;
}

/*
 * @name stm_rel_start_stm
 */
void stm_rel_start_stm() {
	if (g_stm_event == StmEvtNoUndo) {
		// nop
	}
	else {
		g_stm_prv_state.layer[StmLayerNoRestriction].state = g_stm_crr_state.layer[StmLayerNoRestriction].state;
	}
    g_stm_crr_state.layer[StmLayerNoRestriction].changed = STM_FALSE;
}
