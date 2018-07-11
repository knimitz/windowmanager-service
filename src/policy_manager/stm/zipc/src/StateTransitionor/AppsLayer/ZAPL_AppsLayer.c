/************************************************************/
/*     ZAPL_AppsLayer.c                                     */
/*     AppsLayer State transition model source file         */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/* State management variable */
static uint8_t ZAPL_AppsLayerState[ZAPL_APPSLAYERSTATENOMAX];

static void ZAPL_AppsLayers0StateEntry( void );
static void ZAPL_AppsLayers1StateEntry( void );
static void ZAPL_AppsLayers0e1( void );
static void ZAPL_AppsLayers1e0( void );
static void ZAPL_RestrictionModeOffs0e0( void );
static void ZAPL_RestrictionModeOffs0e2( void );
static void ZAPL_RestrictionModeOffs0e3( void );
static void ZAPL_RestrictionModeOffs0e6( void );
static void ZAPL_RestrictionModeOffs0e7( void );
static void ZAPL_RestrictionModeOffs0e13( void );
static void ZAPL_RestrictionModeOffs0e15( void );
static void ZAPL_RestrictionModeOffs0e18( void );
static void ZAPL_RestrictionModeOffs1e0( void );
static void ZAPL_RestrictionModeOffs1e8( void );
static void ZAPL_RestrictionModeOffs2e3( void );
static void ZAPL_RestrictionModeOffs3e2( void );
static void ZAPL_RestrictionModeOffs4e3( void );
static void ZAPL_RestrictionModeOffs5e3( void );
static void ZAPL_RestrictionModeOffs6e6( void );
static void ZAPL_RestrictionModeOffs7e7( void );
static void ZAPL_RestrictionModeOns0e1( void );
static void ZAPL_RestrictionModeOns0e2( void );
static void ZAPL_RestrictionModeOns1e0( void );
static void ZAPL_RestrictionModeOns1e3( void );
static void ZAPL_AppsLayers0Event( void );
static void ZAPL_RestrictionModeOffs0Event( void );
static void ZAPL_RestrictionModeOffs1Event( void );
static void ZAPL_RestrictionModeOffs2Event( void );
static void ZAPL_RestrictionModeOffs3Event( void );
static void ZAPL_RestrictionModeOffs4Event( void );
static void ZAPL_RestrictionModeOffs5Event( void );
static void ZAPL_RestrictionModeOffs6Event( void );
static void ZAPL_RestrictionModeOffs7Event( void );
static void ZAPL_AppsLayers1Event( void );
static void ZAPL_RestrictionModeOns0Event( void );
static void ZAPL_RestrictionModeOns1Event( void );

/****************************************/
/* State start activity function        */
/*   STM : AppsLayer                    */
/*   State : restriction_mode_off( No 0 ) */
/****************************************/
static void ZAPL_AppsLayers0StateEntry( void )
{
    switch( ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] )
    {
    case ZAPL_RESTRICTIONMODEOFFS0:
        stm_apl_start_activity_none();
        break;
    case ZAPL_RESTRICTIONMODEOFFS1:
        stm_apl_start_activity_map();
        break;
    case ZAPL_RESTRICTIONMODEOFFS2:
        stm_apl_start_activity_map_split();
        break;
    case ZAPL_RESTRICTIONMODEOFFS3:
        stm_apl_start_activity_map_fullscreen();
        break;
    case ZAPL_RESTRICTIONMODEOFFS4:
        stm_apl_start_activity_splitable_normal();
        break;
    case ZAPL_RESTRICTIONMODEOFFS5:
        stm_apl_start_activity_splitable_split();
        break;
    case ZAPL_RESTRICTIONMODEOFFS6:
        stm_apl_start_activity_general();
        break;
    case ZAPL_RESTRICTIONMODEOFFS7:
        stm_apl_start_activity_system();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* State start activity function        */
/*   STM : AppsLayer                    */
/*   State : restriction_mode_2_on( No 1 ) */
/****************************************/
static void ZAPL_AppsLayers1StateEntry( void )
{
    switch( ZAPL_AppsLayerState[ZAPL_APPSLAYERS1F] )
    {
    case ZAPL_RESTRICTIONMODEONS0:
        stm_apl_start_activity_map();
        break;
    case ZAPL_RESTRICTIONMODEONS1:
        stm_apl_start_activity_map_fullscreen();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* Action function                      */
/*   STM : AppsLayer                    */
/*   State : restriction_mode_off( No 0 ) */
/*   Event : stt_prv_layer_apps_not_sys_nml( No 1 ) */
/****************************************/
static void ZAPL_AppsLayers0e1( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYER] = ( uint8_t )ZAPL_APPSLAYERS1;
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS1F] = ( uint8_t )ZAPL_RESTRICTIONMODEONS0;
    ZAPL_AppsLayers1StateEntry();
}

/****************************************/
/* Action function                      */
/*   STM : AppsLayer                    */
/*   State : restriction_mode_2_on( No 1 ) */
/*   Event : stt_restriction_mode_off( No 0 ) */
/****************************************/
static void ZAPL_AppsLayers1e0( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYER] = ( uint8_t )ZAPL_APPSLAYERS0;
    ZAPL_AppsLayers0StateEntry();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : none( No 0 )               */
/*   Event : ara_normal( No 0 )         */
/****************************************/
static void ZAPL_RestrictionModeOffs0e0( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] = ( uint8_t )ZAPL_RESTRICTIONMODEOFFS1;
    stm_apl_start_activity_map();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : none( No 0 )               */
/*   Event : ara_fullscreen( No 2 )     */
/****************************************/
static void ZAPL_RestrictionModeOffs0e2( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] = ( uint8_t )ZAPL_RESTRICTIONMODEOFFS3;
    stm_apl_start_activity_map_fullscreen();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : none( No 0 )               */
/*   Event : ara_normal( No 3 )         */
/****************************************/
static void ZAPL_RestrictionModeOffs0e3( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] = ( uint8_t )ZAPL_RESTRICTIONMODEOFFS4;
    stm_apl_start_activity_splitable_normal();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : none( No 0 )               */
/*   Event : ara_normal( No 6 )         */
/****************************************/
static void ZAPL_RestrictionModeOffs0e6( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] = ( uint8_t )ZAPL_RESTRICTIONMODEOFFS6;
    stm_apl_start_activity_general();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : none( No 0 )               */
/*   Event : ara_normal( No 7 )         */
/****************************************/
static void ZAPL_RestrictionModeOffs0e7( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] = ( uint8_t )ZAPL_RESTRICTIONMODEOFFS7;
    stm_apl_start_activity_system();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : none( No 0 )               */
/*   Event : stt_prv_layer_apps_none( No 13 ) */
/****************************************/
static void ZAPL_RestrictionModeOffs0e13( void )
{
    stm_apl_start_activity_none();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : none( No 0 )               */
/*   Event : stt_prv_layer_apps_map_spl( No 15 ) */
/****************************************/
static void ZAPL_RestrictionModeOffs0e15( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] = ( uint8_t )ZAPL_RESTRICTIONMODEOFFS2;
    stm_apl_start_activity_map_split();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : none( No 0 )               */
/*   Event : stt_prv_layer_apps_spl_spl( No 18 ) */
/****************************************/
static void ZAPL_RestrictionModeOffs0e18( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] = ( uint8_t )ZAPL_RESTRICTIONMODEOFFS5;
    stm_apl_start_activity_splitable_split();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : map( No 1 )                */
/*   Event : ara_normal( No 0 )         */
/****************************************/
static void ZAPL_RestrictionModeOffs1e0( void )
{
    stm_apl_start_activity_map();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : map( No 1 )                */
/*   Event : ara_fullscreen( No 8 )     */
/****************************************/
static void ZAPL_RestrictionModeOffs1e8( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] = ( uint8_t )ZAPL_RESTRICTIONMODEOFFS0;
    stm_apl_start_activity_none();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : map_split( No 2 )          */
/*   Event : ara_normal( No 3 )         */
/****************************************/
static void ZAPL_RestrictionModeOffs2e3( void )
{
    stm_apl_start_activity_map_split();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : map_fullscreen( No 3 )     */
/*   Event : ara_fullscreen( No 2 )     */
/****************************************/
static void ZAPL_RestrictionModeOffs3e2( void )
{
    stm_apl_start_activity_map_fullscreen();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : splitable_normal( No 4 )   */
/*   Event : ara_normal( No 3 )         */
/****************************************/
static void ZAPL_RestrictionModeOffs4e3( void )
{
    stm_apl_start_activity_splitable_normal();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : splitable_split( No 5 )    */
/*   Event : ara_normal( No 3 )         */
/****************************************/
static void ZAPL_RestrictionModeOffs5e3( void )
{
    stm_apl_start_activity_splitable_split();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : general( No 6 )            */
/*   Event : ara_normal( No 6 )         */
/****************************************/
static void ZAPL_RestrictionModeOffs6e6( void )
{
    stm_apl_start_activity_general();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOff           */
/*   State : system( No 7 )             */
/*   Event : ara_normal( No 7 )         */
/****************************************/
static void ZAPL_RestrictionModeOffs7e7( void )
{
    stm_apl_start_activity_system();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOn            */
/*   State : map( No 0 )                */
/*   Event : ara_fullscreen( No 1 )     */
/****************************************/
static void ZAPL_RestrictionModeOns0e1( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS1F] = ( uint8_t )ZAPL_RESTRICTIONMODEONS1;
    stm_apl_start_activity_map_fullscreen();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOn            */
/*   State : map( No 0 )                */
/*   Event : stt_prv_layer_apps_map_nml( No 2 ) */
/****************************************/
static void ZAPL_RestrictionModeOns0e2( void )
{
    stm_apl_start_activity_map();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOn            */
/*   State : map_fullscreen( No 1 )     */
/*   Event : ara_normal( No 0 )         */
/****************************************/
static void ZAPL_RestrictionModeOns1e0( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS1F] = ( uint8_t )ZAPL_RESTRICTIONMODEONS0;
    stm_apl_start_activity_map();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionModeOn            */
/*   State : map_fullscreen( No 1 )     */
/*   Event : stt_prv_layer_apps_map_fll( No 3 ) */
/****************************************/
static void ZAPL_RestrictionModeOns1e3( void )
{
    stm_apl_start_activity_map_fullscreen();
}

/****************************************/
/* Event appraisal function             */
/*   STM : AppsLayer                    */
/*   State : restriction_mode_off( No 0 ) */
/****************************************/
static void ZAPL_AppsLayers0Event( void )
{
    /*stt_restriction_mode_2_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo2On )
    {
        stm_apl_event_restriction_mode_2_on();
        /*stt_map_is_activated*/
        if( g_stm_map_is_activated == STM_TRUE )
        {
            /*stt_prv_layer_apps_not_sys_nml*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state != StmLayoutNoSysNml )
            {
                ZAPL_AppsLayers0e1();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionModeOff           */
/*   State : none( No 0 )               */
/****************************************/
static void ZAPL_RestrictionModeOffs0Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e0();
                }
                /*ara_fullscreen*/
                else if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs0e2();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_splitable*/
            else if( g_stm_category == StmCtgNoSplitable )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e3();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_general*/
            else if( g_stm_category == StmCtgNoGeneral )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e6();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_system*/
            else if( g_stm_category == StmCtgNoSystem )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e7();
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
        /*evt_undo*/
        else if( g_stm_event == StmEvtNoUndo )
        {
            /*stt_prv_layer_apps_none*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoNone )
            {
                ZAPL_RestrictionModeOffs0e13();
            }
            /*stt_prv_layer_apps_map_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapNml )
            {
                ZAPL_RestrictionModeOffs0e0();
            }
            /*stt_prv_layer_apps_map_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZAPL_RestrictionModeOffs0e15();
            }
            /*stt_prv_layer_apps_map_fll*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapFll )
            {
                ZAPL_RestrictionModeOffs0e2();
            }
            /*stt_prv_layer_apps_spl_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZAPL_RestrictionModeOffs0e3();
            }
            /*stt_prv_layer_apps_spl_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZAPL_RestrictionModeOffs0e18();
            }
            /*stt_prv_layer_apps_gen_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZAPL_RestrictionModeOffs0e6();
            }
            /*stt_prv_layer_apps_sys_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSysNml )
            {
                ZAPL_RestrictionModeOffs0e7();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionModeOff           */
/*   State : map( No 1 )                */
/****************************************/
static void ZAPL_RestrictionModeOffs1Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs1e0();
                }
                /*ara_fullscreen*/
                else if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs0e2();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_splitable*/
            else if( g_stm_category == StmCtgNoSplitable )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e15();
                }
                /*ara_split_sub*/
                else if( g_stm_area == StmAreaNoSplitSub )
                {
                    ZAPL_RestrictionModeOffs0e15();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_general*/
            else if( g_stm_category == StmCtgNoGeneral )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e6();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_system*/
            else if( g_stm_category == StmCtgNoSystem )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e7();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_homescreen*/
            else if( g_stm_category == StmCtgNoHomescreen )
            {
                /*ara_fullscreen*/
                if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs1e8();
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
        /*evt_deactivate*/
        else if( g_stm_event == StmEvtNoDeactivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                ZAPL_RestrictionModeOffs1e8();
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
            /*stt_prv_layer_apps_none*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoNone )
            {
                ZAPL_RestrictionModeOffs1e8();
            }
            /*stt_prv_layer_apps_map_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapNml )
            {
                ZAPL_RestrictionModeOffs1e0();
            }
            /*stt_prv_layer_apps_map_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZAPL_RestrictionModeOffs0e15();
            }
            /*stt_prv_layer_apps_map_fll*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapFll )
            {
                ZAPL_RestrictionModeOffs0e2();
            }
            /*stt_prv_layer_apps_spl_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZAPL_RestrictionModeOffs0e3();
            }
            /*stt_prv_layer_apps_spl_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZAPL_RestrictionModeOffs0e18();
            }
            /*stt_prv_layer_apps_gen_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZAPL_RestrictionModeOffs0e6();
            }
            /*stt_prv_layer_apps_sys_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSysNml )
            {
                ZAPL_RestrictionModeOffs0e7();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionModeOff           */
/*   State : map_split( No 2 )          */
/****************************************/
static void ZAPL_RestrictionModeOffs2Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e0();
                }
                /*ara_fullscreen*/
                else if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs0e2();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_splitable*/
            else if( g_stm_category == StmCtgNoSplitable )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs2e3();
                }
                /*ara_split_sub*/
                else if( g_stm_area == StmAreaNoSplitSub )
                {
                    ZAPL_RestrictionModeOffs2e3();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_general*/
            else if( g_stm_category == StmCtgNoGeneral )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e6();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_system*/
            else if( g_stm_category == StmCtgNoSystem )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e7();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_homescreen*/
            else if( g_stm_category == StmCtgNoHomescreen )
            {
                /*ara_fullscreen*/
                if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs1e8();
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
        /*evt_deactivate*/
        else if( g_stm_event == StmEvtNoDeactivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                ZAPL_RestrictionModeOffs0e3();
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
            /*stt_prv_layer_apps_none*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoNone )
            {
                ZAPL_RestrictionModeOffs1e8();
            }
            /*stt_prv_layer_apps_map_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapNml )
            {
                ZAPL_RestrictionModeOffs0e0();
            }
            /*stt_prv_layer_apps_map_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZAPL_RestrictionModeOffs2e3();
            }
            /*stt_prv_layer_apps_map_fll*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapFll )
            {
                ZAPL_RestrictionModeOffs0e2();
            }
            /*stt_prv_layer_apps_spl_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZAPL_RestrictionModeOffs0e3();
            }
            /*stt_prv_layer_apps_spl_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZAPL_RestrictionModeOffs0e18();
            }
            /*stt_prv_layer_apps_gen_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZAPL_RestrictionModeOffs0e6();
            }
            /*stt_prv_layer_apps_sys_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSysNml )
            {
                ZAPL_RestrictionModeOffs0e7();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionModeOff           */
/*   State : map_fullscreen( No 3 )     */
/****************************************/
static void ZAPL_RestrictionModeOffs3Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e0();
                }
                /*ara_fullscreen*/
                else if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs3e2();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_splitable*/
            else if( g_stm_category == StmCtgNoSplitable )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e15();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_general*/
            else if( g_stm_category == StmCtgNoGeneral )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e6();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_system*/
            else if( g_stm_category == StmCtgNoSystem )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e7();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_homescreen*/
            else if( g_stm_category == StmCtgNoHomescreen )
            {
                /*ara_fullscreen*/
                if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs1e8();
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
        /*evt_deactivate*/
        else if( g_stm_event == StmEvtNoDeactivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                ZAPL_RestrictionModeOffs1e8();
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
            /*stt_prv_layer_apps_none*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoNone )
            {
                ZAPL_RestrictionModeOffs1e8();
            }
            /*stt_prv_layer_apps_map_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapNml )
            {
                ZAPL_RestrictionModeOffs0e0();
            }
            /*stt_prv_layer_apps_map_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZAPL_RestrictionModeOffs0e15();
            }
            /*stt_prv_layer_apps_map_fll*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapFll )
            {
                ZAPL_RestrictionModeOffs3e2();
            }
            /*stt_prv_layer_apps_spl_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZAPL_RestrictionModeOffs0e3();
            }
            /*stt_prv_layer_apps_spl_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZAPL_RestrictionModeOffs0e18();
            }
            /*stt_prv_layer_apps_gen_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZAPL_RestrictionModeOffs0e6();
            }
            /*stt_prv_layer_apps_sys_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSysNml )
            {
                ZAPL_RestrictionModeOffs0e7();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionModeOff           */
/*   State : splitable_normal( No 4 )   */
/****************************************/
static void ZAPL_RestrictionModeOffs4Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e15();
                }
                /*ara_split_main*/
                else if( g_stm_area == StmAreaNoSplitMain )
                {
                    ZAPL_RestrictionModeOffs0e15();
                }
                /*ara_fullscreen*/
                else if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs0e2();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_splitable*/
            else if( g_stm_category == StmCtgNoSplitable )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs4e3();
                }
                /*ara_split_main*/
                else if( g_stm_area == StmAreaNoSplitMain )
                {
                    ZAPL_RestrictionModeOffs0e18();
                }
                /*ara_split_sub*/
                else if( g_stm_area == StmAreaNoSplitSub )
                {
                    ZAPL_RestrictionModeOffs0e18();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_general*/
            else if( g_stm_category == StmCtgNoGeneral )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e6();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_system*/
            else if( g_stm_category == StmCtgNoSystem )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e7();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_homescreen*/
            else if( g_stm_category == StmCtgNoHomescreen )
            {
                /*ara_fullscreen*/
                if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs1e8();
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
        /*evt_deactivate*/
        else if( g_stm_event == StmEvtNoDeactivate )
        {
            /*ctg_splitable*/
            if( g_stm_category == StmCtgNoSplitable )
            {
                ZAPL_RestrictionModeOffs1e8();
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
            /*stt_prv_layer_apps_none*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoNone )
            {
                ZAPL_RestrictionModeOffs1e8();
            }
            /*stt_prv_layer_apps_map_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapNml )
            {
                ZAPL_RestrictionModeOffs0e0();
            }
            /*stt_prv_layer_apps_map_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZAPL_RestrictionModeOffs0e15();
            }
            /*stt_prv_layer_apps_map_fll*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapFll )
            {
                ZAPL_RestrictionModeOffs0e2();
            }
            /*stt_prv_layer_apps_spl_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZAPL_RestrictionModeOffs4e3();
            }
            /*stt_prv_layer_apps_spl_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZAPL_RestrictionModeOffs0e18();
            }
            /*stt_prv_layer_apps_gen_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZAPL_RestrictionModeOffs0e6();
            }
            /*stt_prv_layer_apps_sys_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSysNml )
            {
                ZAPL_RestrictionModeOffs0e7();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionModeOff           */
/*   State : splitable_split( No 5 )    */
/****************************************/
static void ZAPL_RestrictionModeOffs5Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e15();
                }
                /*ara_split_main*/
                else if( g_stm_area == StmAreaNoSplitMain )
                {
                    ZAPL_RestrictionModeOffs0e15();
                }
                /*ara_fullscreen*/
                else if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs0e2();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_splitable*/
            else if( g_stm_category == StmCtgNoSplitable )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs5e3();
                }
                /*ara_split_main*/
                else if( g_stm_area == StmAreaNoSplitMain )
                {
                    ZAPL_RestrictionModeOffs5e3();
                }
                /*ara_split_sub*/
                else if( g_stm_area == StmAreaNoSplitSub )
                {
                    ZAPL_RestrictionModeOffs5e3();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_general*/
            else if( g_stm_category == StmCtgNoGeneral )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e6();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_system*/
            else if( g_stm_category == StmCtgNoSystem )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e7();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_homescreen*/
            else if( g_stm_category == StmCtgNoHomescreen )
            {
                /*ara_fullscreen*/
                if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs1e8();
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
        /*evt_deactivate*/
        else if( g_stm_event == StmEvtNoDeactivate )
        {
            /*ctg_splitable*/
            if( g_stm_category == StmCtgNoSplitable )
            {
                ZAPL_RestrictionModeOffs0e3();
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
            /*stt_prv_layer_apps_none*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoNone )
            {
                ZAPL_RestrictionModeOffs1e8();
            }
            /*stt_prv_layer_apps_map_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapNml )
            {
                ZAPL_RestrictionModeOffs0e0();
            }
            /*stt_prv_layer_apps_map_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZAPL_RestrictionModeOffs0e15();
            }
            /*stt_prv_layer_apps_map_fll*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapFll )
            {
                ZAPL_RestrictionModeOffs0e2();
            }
            /*stt_prv_layer_apps_spl_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZAPL_RestrictionModeOffs0e3();
            }
            /*stt_prv_layer_apps_spl_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZAPL_RestrictionModeOffs5e3();
            }
            /*stt_prv_layer_apps_gen_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZAPL_RestrictionModeOffs0e6();
            }
            /*stt_prv_layer_apps_sys_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSysNml )
            {
                ZAPL_RestrictionModeOffs0e7();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionModeOff           */
/*   State : general( No 6 )            */
/****************************************/
static void ZAPL_RestrictionModeOffs6Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e0();
                }
                /*ara_fullscreen*/
                else if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs0e2();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_splitable*/
            else if( g_stm_category == StmCtgNoSplitable )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e3();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_general*/
            else if( g_stm_category == StmCtgNoGeneral )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs6e6();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_system*/
            else if( g_stm_category == StmCtgNoSystem )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e7();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_homescreen*/
            else if( g_stm_category == StmCtgNoHomescreen )
            {
                /*ara_fullscreen*/
                if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs1e8();
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
        /*evt_deactivate*/
        else if( g_stm_event == StmEvtNoDeactivate )
        {
            /*ctg_general*/
            if( g_stm_category == StmCtgNoGeneral )
            {
                ZAPL_RestrictionModeOffs1e8();
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
            /*stt_prv_layer_apps_none*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoNone )
            {
                ZAPL_RestrictionModeOffs1e8();
            }
            /*stt_prv_layer_apps_map_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapNml )
            {
                ZAPL_RestrictionModeOffs0e0();
            }
            /*stt_prv_layer_apps_map_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZAPL_RestrictionModeOffs0e15();
            }
            /*stt_prv_layer_apps_map_fll*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapFll )
            {
                ZAPL_RestrictionModeOffs0e2();
            }
            /*stt_prv_layer_apps_spl_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZAPL_RestrictionModeOffs0e3();
            }
            /*stt_prv_layer_apps_spl_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZAPL_RestrictionModeOffs0e18();
            }
            /*stt_prv_layer_apps_gen_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZAPL_RestrictionModeOffs6e6();
            }
            /*stt_prv_layer_apps_sys_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSysNml )
            {
                ZAPL_RestrictionModeOffs0e7();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionModeOff           */
/*   State : system( No 7 )             */
/****************************************/
static void ZAPL_RestrictionModeOffs7Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e0();
                }
                /*ara_fullscreen*/
                else if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs0e2();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_splitable*/
            else if( g_stm_category == StmCtgNoSplitable )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e3();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_general*/
            else if( g_stm_category == StmCtgNoGeneral )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs0e6();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_system*/
            else if( g_stm_category == StmCtgNoSystem )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOffs7e7();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_homescreen*/
            else if( g_stm_category == StmCtgNoHomescreen )
            {
                /*ara_fullscreen*/
                if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOffs1e8();
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
        /*evt_deactivate*/
        else if( g_stm_event == StmEvtNoDeactivate )
        {
            /*ctg_system*/
            if( g_stm_category == StmCtgNoSystem )
            {
                ZAPL_RestrictionModeOffs1e8();
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
            /*stt_prv_layer_apps_none*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoNone )
            {
                ZAPL_RestrictionModeOffs1e8();
            }
            /*stt_prv_layer_apps_map_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapNml )
            {
                ZAPL_RestrictionModeOffs0e0();
            }
            /*stt_prv_layer_apps_map_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZAPL_RestrictionModeOffs0e15();
            }
            /*stt_prv_layer_apps_map_fll*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapFll )
            {
                ZAPL_RestrictionModeOffs0e2();
            }
            /*stt_prv_layer_apps_spl_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZAPL_RestrictionModeOffs0e3();
            }
            /*stt_prv_layer_apps_spl_spl*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZAPL_RestrictionModeOffs0e18();
            }
            /*stt_prv_layer_apps_gen_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZAPL_RestrictionModeOffs0e6();
            }
            /*stt_prv_layer_apps_sys_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoSysNml )
            {
                ZAPL_RestrictionModeOffs7e7();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : AppsLayer                    */
/*   State : restriction_mode_2_on( No 1 ) */
/****************************************/
static void ZAPL_AppsLayers1Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        stm_apl_event_restriction_mode_off();
        ZAPL_AppsLayers1e0();
    }
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionModeOn            */
/*   State : map( No 0 )                */
/****************************************/
static void ZAPL_RestrictionModeOns0Event( void )
{
    /*stt_restriction_mode_2_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo2On )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                /*ara_fullscreen*/
                if( g_stm_area == StmAreaNoFullscreen )
                {
                    ZAPL_RestrictionModeOns0e1();
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
        /*evt_undo*/
        else if( g_stm_event == StmEvtNoUndo )
        {
            /*stt_prv_layer_apps_map_nml*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapNml )
            {
                ZAPL_RestrictionModeOns0e2();
            }
            /*stt_prv_layer_apps_map_fll*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapFll )
            {
                ZAPL_RestrictionModeOns0e1();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionModeOn            */
/*   State : map_fullscreen( No 1 )     */
/****************************************/
static void ZAPL_RestrictionModeOns1Event( void )
{
    /*stt_restriction_mode_2_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo2On )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_map*/
            if( g_stm_category == StmCtgNoMap )
            {
                /*ara_normal*/
                if( g_stm_area == StmAreaNoNormal )
                {
                    ZAPL_RestrictionModeOns1e0();
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
        /*evt_undo*/
        else if( g_stm_event == StmEvtNoUndo )
        {
            /*stt_prv_layer_apps_map_nml*/
            if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapNml )
            {
                ZAPL_RestrictionModeOns1e0();
            }
            /*stt_prv_layer_apps_map_fll*/
            else if( g_stm_prv_state.layer[StmLayerNoApps].state == StmLayoutNoMapFll )
            {
                ZAPL_RestrictionModeOns1e3();
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
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event call function                  */
/*   STM : AppsLayer                    */
/****************************************/
void stm_apl_event_call( void )
{
    stm_apl_start_stm();
    switch( ZAPL_AppsLayerState[ZAPL_APPSLAYER] )
    {
    case ZAPL_APPSLAYERS0:
        switch( ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] )
        {
        case ZAPL_RESTRICTIONMODEOFFS0:
            ZAPL_RestrictionModeOffs0Event();
            break;
        case ZAPL_RESTRICTIONMODEOFFS1:
            ZAPL_RestrictionModeOffs1Event();
            break;
        case ZAPL_RESTRICTIONMODEOFFS2:
            ZAPL_RestrictionModeOffs2Event();
            break;
        case ZAPL_RESTRICTIONMODEOFFS3:
            ZAPL_RestrictionModeOffs3Event();
            break;
        case ZAPL_RESTRICTIONMODEOFFS4:
            ZAPL_RestrictionModeOffs4Event();
            break;
        case ZAPL_RESTRICTIONMODEOFFS5:
            ZAPL_RestrictionModeOffs5Event();
            break;
        case ZAPL_RESTRICTIONMODEOFFS6:
            ZAPL_RestrictionModeOffs6Event();
            break;
        case ZAPL_RESTRICTIONMODEOFFS7:
            ZAPL_RestrictionModeOffs7Event();
            break;
        default:
            /*Not accessible to this else (default).*/
            break;
        }
        ZAPL_AppsLayers0Event();
        break;
    case ZAPL_APPSLAYERS1:
        switch( ZAPL_AppsLayerState[ZAPL_APPSLAYERS1F] )
        {
        case ZAPL_RESTRICTIONMODEONS0:
            ZAPL_RestrictionModeOns0Event();
            break;
        case ZAPL_RESTRICTIONMODEONS1:
            ZAPL_RestrictionModeOns1Event();
            break;
        default:
            /*Not accessible to this else (default).*/
            break;
        }
        ZAPL_AppsLayers1Event();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* Initial function                     */
/*   STM : AppsLayer                    */
/****************************************/
void stm_apl_initialize( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYER] = ( uint8_t )ZAPL_APPSLAYERS0;
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS0F] = ( uint8_t )ZAPL_RESTRICTIONMODEOFFS0;
    ZAPL_AppsLayerState[ZAPL_APPSLAYERS1F] = ( uint8_t )ZAPL_RESTRICTIONMODEONS0;
    ZAPL_AppsLayers0StateEntry();
}

/****************************************/
/* Terminate function                   */
/*   STM : AppsLayer                    */
/****************************************/
void ZAPL_AppsLayerTerminate( void )
{
    ZAPL_AppsLayerState[ZAPL_APPSLAYER] = ( uint8_t )ZAPL_APPSLAYERTERMINATE;
}

