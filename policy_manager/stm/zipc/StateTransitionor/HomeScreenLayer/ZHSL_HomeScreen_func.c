/************************************************************/
/*     ZHSL_HomeScreen_func.c                               */
/*     Function and variable source file                    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/*************************************************************
    Function definition
*************************************************************/

/*
 * @name stm_hsl_start_activity_none
 */
void stm_hsl_start_activity_none() {
    g_stm_crr_state.layer[StmLayerNoHomescreen].state = StmLayoutNoNone;
    g_stm_crr_state.layer[StmLayerNoHomescreen].changed = STM_TRUE;
}

/*
 * @name stm_hsl_start_activity_homescreen
 */
void stm_hsl_start_activity_homescreen() {
    g_stm_crr_state.layer[StmLayerNoHomescreen].state = StmLayoutNoHms;
    g_stm_crr_state.layer[StmLayerNoHomescreen].changed = STM_TRUE;
}

/*
 * @name stm_hsl_initialize_variable
 */
void stm_hsl_initialize_variable() {
    g_stm_prv_state.layer[StmLayerNoHomescreen].state = StmLayoutNoNone;
    g_stm_prv_state.layer[StmLayerNoHomescreen].changed = STM_FALSE;

    g_stm_crr_state.layer[StmLayerNoHomescreen].state = StmLayoutNoNone;
    g_stm_crr_state.layer[StmLayerNoHomescreen].changed = STM_FALSE;
}

/*
 * @name stm_hsl_start_stm
 */
void stm_hsl_start_stm() {
	g_stm_prv_state.layer[StmLayerNoHomescreen].state = g_stm_crr_state.layer[StmLayerNoHomescreen].state;
	g_stm_crr_state.layer[StmLayerNoHomescreen].changed = STM_FALSE;
}
