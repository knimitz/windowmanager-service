/************************************************************/
/*     ZAPL_Apps_func.c                                     */
/*     Function and variable source file                    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/*************************************************************
    Function definition
*************************************************************/

/*
 * @name stm_apl_start_activity_none
 */
void stm_apl_start_activity_none() {
    g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoNone;
    g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
}

/*
 * @name stm_apl_start_activity_map
 */
void stm_apl_start_activity_map() {
    g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoMapNml;
    g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;

    if ((g_stm_event == StmEvtNoActivate)
    		&& (g_stm_category == StmCtgNoMap)) {
    	g_stm_map_is_activated = STM_TRUE;
    }
}

/*
 * @name stm_apl_start_activity_map_split
 */
void stm_apl_start_activity_map_split() {
    g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoMapSpl;
    g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;

    if ((g_stm_event == StmEvtNoActivate)
    		&& (g_stm_category == StmCtgNoMap)) {
    	g_stm_map_is_activated = STM_TRUE;
    }
}

/*
 * @name stm_apl_start_activity_map_fullscreen
 */
void stm_apl_start_activity_map_fullscreen() {
    g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoMapFll;
    g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;

    if ((g_stm_event == StmEvtNoActivate)
    		&& (g_stm_category == StmCtgNoMap)) {
    	g_stm_map_is_activated = STM_TRUE;
    }
}

/*
 * @name stm_apl_start_activity_splitable_normal
 */
void stm_apl_start_activity_splitable_normal() {
    g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoSplNml;
    g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
}

/*
 * @name stm_apl_start_activity_splitable_split
 */
void stm_apl_start_activity_splitable_split() {
    g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoSplSpl;
    g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
}

/*
 * @name stm_apl_start_activity_general
 */
void stm_apl_start_activity_general() {
    g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoGenNml;
    g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
}

/*
 * @name stm_apl_start_activity_system
 */
void stm_apl_start_activity_system() {
    g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoSysNml;
    g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
}

/*
 * @name stm_apl_event_restriction_mode_off
 */
void stm_apl_event_restriction_mode_off() {
	 g_stm_crr_state.layer[StmLayerNoApps].state = g_prv_apps_state_rest_mode_1;
	 g_stm_crr_state.layer[StmLayerNoApps].changed = STM_TRUE;
}

/*
 * @name stm_apl_event_restriction_mode_2_on
 */
void stm_apl_event_restriction_mode_2_on() {
	g_prv_apps_state_rest_mode_1 = g_stm_prv_state.layer[StmLayerNoApps].state;
}

/*
 * @name stm_apl_initialize_variable
 */
void stm_apl_initialize_variable() {
	g_stm_prv_state.layer[StmLayerNoApps].state = StmLayoutNoNone;
	g_stm_prv_state.layer[StmLayerNoApps].changed = STM_FALSE;

	g_stm_crr_state.layer[StmLayerNoApps].state = StmLayoutNoNone;
	g_stm_crr_state.layer[StmLayerNoApps].changed = STM_FALSE;
}

/*
 * @name stm_apl_start_stm
 */
void stm_apl_start_stm() {
	if (g_stm_event == StmEvtNoUndo) {
		// nop
	}
	else {
		g_stm_prv_state.layer[StmLayerNoApps].state = g_stm_crr_state.layer[StmLayerNoApps].state;
	}
	g_stm_crr_state.layer[StmLayerNoApps].changed = STM_FALSE;
}
