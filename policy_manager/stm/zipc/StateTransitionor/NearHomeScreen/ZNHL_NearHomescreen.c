/************************************************************/
/*     ZNHL_NearHomescreen.c                                */
/*     NearHomescreen State transition model source file    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/* State management variable */
static uint8_t ZNHL_NearHomescreenState[ZNHL_NEARHOMESCREENSTATENOMAX];

static void ZNHL_NearHomescreens0e0( void );
static void ZNHL_NearHomescreens0e3( void );
static void ZNHL_NearHomescreens1e0( void );
static void ZNHL_NearHomescreens1e2( void );
static void ZNHL_NearHomescreens0Event( void );
static void ZNHL_NearHomescreens1Event( void );

/****************************************/
/* Action function                      */
/*   STM : NearHomescreen               */
/*   State : none( No 0 )               */
/*   Event : ara_software_keyboard( No 0 ) */
/****************************************/
static void ZNHL_NearHomescreens0e0( void )
{
    ZNHL_NearHomescreenState[ZNHL_NEARHOMESCREEN] = ( uint8_t )ZNHL_NEARHOMESCREENS1;
    stm_nhl_start_activity_software_keyboard();
}

/****************************************/
/* Action function                      */
/*   STM : NearHomescreen               */
/*   State : none( No 0 )               */
/*   Event : stt_prv_layer_near_homescreen_none( No 3 ) */
/****************************************/
static void ZNHL_NearHomescreens0e3( void )
{
    stm_nhl_start_activity_none();
}

/****************************************/
/* Action function                      */
/*   STM : NearHomescreen               */
/*   State : software_keyboard( No 1 )  */
/*   Event : ara_software_keyboard( No 0 ) */
/****************************************/
static void ZNHL_NearHomescreens1e0( void )
{
    stm_nhl_start_activity_software_keyboard();
}

/****************************************/
/* Action function                      */
/*   STM : NearHomescreen               */
/*   State : software_keyboard( No 1 )  */
/*   Event : ctg_software_keyboard( No 2 ) */
/****************************************/
static void ZNHL_NearHomescreens1e2( void )
{
    ZNHL_NearHomescreenState[ZNHL_NEARHOMESCREEN] = ( uint8_t )ZNHL_NEARHOMESCREENS0;
    stm_nhl_start_activity_none();
}

/****************************************/
/* Event appraisal function             */
/*   STM : NearHomescreen               */
/*   State : none( No 0 )               */
/****************************************/
static void ZNHL_NearHomescreens0Event( void )
{
    /*evt_activate*/
    if( g_stm_event == StmEvtNoActivate )
    {
        /*ctg_software_keyboard*/
        if( g_stm_category == StmCtgNoSoftwareKeyboard )
        {
            /*ara_software_keyboard*/
            if( g_stm_area == StmAreaNoSoftwareKyeboard )
            {
                ZNHL_NearHomescreens0e0();
            }
            else
            {
                ZNHL_NearHomescreens0e0();
            }
        }
        else
        {
            /*Else and default design have not done.*/
            /*Please confirm the STM and design else and default.*/
        }
    }
    /*evt_undo*/
    else if( g_stm_event == StmEvtNoUndo )
    {
        /*stt_prv_layer_near_homescreen_none*/
        if( g_stm_prv_state.layer[StmLayerNoNearHomescreen].state == StmLayoutNoNone )
        {
            ZNHL_NearHomescreens0e3();
        }
        /*stt_prv_layer_near_homescreen_sft_kbd*/
        else if( g_stm_prv_state.layer[StmLayerNoNearHomescreen].state == StmLayoutNoSftKbd )
        {
            ZNHL_NearHomescreens0e0();
        }
        else
        {
            /*Else and default design have not done.*/
            /*Please confirm the STM and design else and default.*/
        }
    }
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : NearHomescreen               */
/*   State : software_keyboard( No 1 )  */
/****************************************/
static void ZNHL_NearHomescreens1Event( void )
{
    /*evt_activate*/
    if( g_stm_event == StmEvtNoActivate )
    {
        /*ctg_software_keyboard*/
        if( g_stm_category == StmCtgNoSoftwareKeyboard )
        {
            /*ara_software_keyboard*/
            if( g_stm_area == StmAreaNoSoftwareKyeboard )
            {
                ZNHL_NearHomescreens1e0();
            }
            else
            {
                ZNHL_NearHomescreens1e0();
            }
        }
        else
        {
            /*Else and default design have not done.*/
            /*Please confirm the STM and design else and default.*/
        }
    }
    /*evt_deactivate*/
    else if( g_stm_event == StmEvtNoDeactivate )
    {
        /*ctg_software_keyboard*/
        if( g_stm_category == StmCtgNoSoftwareKeyboard )
        {
            ZNHL_NearHomescreens1e2();
        }
        else
        {
            /*Else and default design have not done.*/
            /*Please confirm the STM and design else and default.*/
        }
    }
    /*evt_undo*/
    else if( g_stm_event == StmEvtNoUndo )
    {
        /*stt_prv_layer_near_homescreen_none*/
        if( g_stm_prv_state.layer[StmLayerNoNearHomescreen].state == StmLayoutNoNone )
        {
            ZNHL_NearHomescreens1e2();
        }
        /*stt_prv_layer_near_homescreen_sft_kbd*/
        else if( g_stm_prv_state.layer[StmLayerNoNearHomescreen].state == StmLayoutNoSftKbd )
        {
            ZNHL_NearHomescreens1e0();
        }
        else
        {
            /*Else and default design have not done.*/
            /*Please confirm the STM and design else and default.*/
        }
    }
    /*stt_restriction_mode_1_on*/
    else if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo1On )
    {
        ZNHL_NearHomescreens1e2();
    }
    /*stt_crr_layer_apps_changed*/
    else if( g_stm_crr_state.layer[StmLayerNoApps].changed == STM_TRUE )
    {
        ZNHL_NearHomescreens1e2();
    }
    /*stt_crr_layer_hs_changed*/
    else if( g_stm_crr_state.layer[StmLayerNoHomescreen].changed == STM_TRUE )
    {
        ZNHL_NearHomescreens1e2();
    }
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event call function                  */
/*   STM : NearHomescreen               */
/****************************************/
void stm_nhl_event_call( void )
{
    stm_nhl_start_stm();
    switch( ZNHL_NearHomescreenState[ZNHL_NEARHOMESCREEN] )
    {
    case ZNHL_NEARHOMESCREENS0:
        ZNHL_NearHomescreens0Event();
        break;
    case ZNHL_NEARHOMESCREENS1:
        ZNHL_NearHomescreens1Event();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* Initial function                     */
/*   STM : NearHomescreen               */
/****************************************/
void stm_nhl_initialize( void )
{
    ZNHL_NearHomescreenState[ZNHL_NEARHOMESCREEN] = ( uint8_t )ZNHL_NEARHOMESCREENS0;
    stm_nhl_start_activity_none();
}

/****************************************/
/* Terminate function                   */
/*   STM : NearHomescreen               */
/****************************************/
void ZNHL_NearHomescreenTerminate( void )
{
    ZNHL_NearHomescreenState[ZNHL_NEARHOMESCREEN] = ( uint8_t )ZNHL_NEARHOMESCREENTERMINATE;
}

