/************************************************************/
/*     ZST_StateTransitionor_var.c                          */
/*     Function and variable source file                    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "ZST_include.h"

/*************************************************************
    Variable definition
*************************************************************/

// Current state
StmState g_stm_crr_state;

// Previous state
StmState g_stm_prv_state;

/**
 *  g_stm_event
 */
int g_stm_event;

/**
 *  g_stm_category
 */
int g_stm_category;

/**
 *  g_stm_area
 */
int g_stm_area;

/**
 *  g_stm_map_is_activated
 */
int g_stm_map_is_activated;

/**
 *  g_prv_apps_state_rest_mode_1
 */
int g_prv_apps_state_rest_mode_1;

/**
 *  g_prv_near_homescreen_state_rest_mode_1
 */
int g_prv_near_homescreen_state_rest_mode_1;

/**
 *  g_prv_restriction_state_rest_mode_1
 */
int g_prv_restriction_state_rest_mode_1;

/**
 *  g_prv_on_screen_state_rest_mode_1
 */
int g_prv_on_screen_state_rest_mode_1;
