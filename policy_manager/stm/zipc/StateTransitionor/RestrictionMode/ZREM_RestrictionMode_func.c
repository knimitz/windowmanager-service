/************************************************************/
/*     ZREM_RestrictionMode_func.c                          */
/*     Function and variable source file                    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/*************************************************************
    Function definition
*************************************************************/

/*
 * @name stm_rem_start_activity_restriction_mode_1_on
 */
void stm_rem_start_activity_restriction_mode_1_on() {
    g_stm_crr_state.mode[StmModeNoRestrictionMode].state = StmRestrictionModeSttNo1On;
    g_stm_crr_state.mode[StmModeNoRestrictionMode].changed = STM_TRUE;
}

/*
 * @name stm_rem_start_activity_restriction_mode_2_on
 */
void stm_rem_start_activity_restriction_mode_2_on() {
    g_stm_crr_state.mode[StmModeNoRestrictionMode].state = StmRestrictionModeSttNo2On;
    g_stm_crr_state.mode[StmModeNoRestrictionMode].changed = STM_TRUE;
}

/*
 * @name stm_rem_start_activity_restriction_mode_off
 */
void stm_rem_start_activity_restriction_mode_off() {
    g_stm_crr_state.mode[StmModeNoRestrictionMode].state = StmRestrictionModeSttNoOff;
    g_stm_crr_state.mode[StmModeNoRestrictionMode].changed = STM_TRUE;
}

/*
 * @name stm_rem_initialize_variable
 */
void stm_rem_initialize_variable() {
	g_stm_prv_state.mode[StmModeNoRestrictionMode].state = StmRestrictionModeSttNoOff;
	g_stm_prv_state.mode[StmModeNoRestrictionMode].changed = STM_FALSE;

	g_stm_crr_state.mode[StmModeNoRestrictionMode].state = StmRestrictionModeSttNoOff;
	g_stm_crr_state.mode[StmModeNoRestrictionMode].changed = STM_FALSE;
}

/*
 * @name stm_rem_start_stm
 */
void stm_rem_start_stm() {
	g_stm_prv_state.mode[StmModeNoRestrictionMode].state = g_stm_crr_state.mode[StmModeNoRestrictionMode].state;
	g_stm_crr_state.mode[StmModeNoRestrictionMode].changed = STM_FALSE;
}
