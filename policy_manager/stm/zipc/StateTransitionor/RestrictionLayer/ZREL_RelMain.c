/************************************************************/
/*     ZREL_RelMain.c                                       */
/*     RelMain State transition model source file           */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/* State management variable */
static uint8_t ZREL_RelMainState[ZREL_RELMAINSTATENOMAX];

static void ZREL_RelMains0StateEntry( void );
static void ZREL_RelMains1StateEntry( void );
static void ZREL_RelMains0e1( void );
static void ZREL_RelMains1e0( void );
static void ZREL_RelRestOffs0e0( void );
static void ZREL_RelRestOffs0e1( void );
static void ZREL_RelRestOffs0e8( void );
static void ZREL_RelRestOffs0e13( void );
static void ZREL_RelRestOffs1e1( void );
static void ZREL_RelRestOffs1e4( void );
static void ZREL_RelRestOffs2e15( void );
static void ZREL_RelRestOffs3e0( void );
static void ZREL_RelMains0Event( void );
static void ZREL_RelRestOffs0Event( void );
static void ZREL_RelRestOffs1Event( void );
static void ZREL_RelRestOffs2Event( void );
static void ZREL_RelRestOffs3Event( void );
static void ZREL_RelMains1Event( void );
static void ZREL_RelRestOns0Event( void );

/****************************************/
/* State start activity function        */
/*   STM : RelMain                      */
/*   State : restriction_mode_off( No 0 ) */
/****************************************/
static void ZREL_RelMains0StateEntry( void )
{
    switch( ZREL_RelMainState[ZREL_RELMAINS0F] )
    {
    case ZREL_RELRESTOFFS0:
        stm_rel_start_activity_none();
        break;
    case ZREL_RELRESTOFFS1:
        stm_rel_start_activity_restriction_normal();
        break;
    case ZREL_RELRESTOFFS2:
        stm_rel_start_activity_restriction_split_main();
        break;
    case ZREL_RELRESTOFFS3:
        stm_rel_start_activity_restriction_split_sub();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* State start activity function        */
/*   STM : RelMain                      */
/*   State : restriction_mode_2_on( No 1 ) */
/****************************************/
static void ZREL_RelMains1StateEntry( void )
{
    switch( ZREL_RelMainState[ZREL_RELMAINS1F] )
    {
    case ZREL_RELRESTONS0:
        stm_rel_start_activity_none();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* Action function                      */
/*   STM : RelMain                      */
/*   State : restriction_mode_off( No 0 ) */
/*   Event : stt_restriction_mode_2_on( No 1 ) */
/****************************************/
static void ZREL_RelMains0e1( void )
{
    ZREL_RelMainState[ZREL_RELMAIN] = ( uint8_t )ZREL_RELMAINS1;
    ZREL_RelMainState[ZREL_RELMAINS1F] = ( uint8_t )ZREL_RELRESTONS0;
    ZREL_RelMains1StateEntry();
}

/****************************************/
/* Action function                      */
/*   STM : RelMain                      */
/*   State : restriction_mode_2_on( No 1 ) */
/*   Event : stt_restriction_mode_off( No 0 ) */
/****************************************/
static void ZREL_RelMains1e0( void )
{
    ZREL_RelMainState[ZREL_RELMAIN] = ( uint8_t )ZREL_RELMAINS0;
    ZREL_RelMains0StateEntry();
}

/****************************************/
/* Action function                      */
/*   STM : RelRestOff                   */
/*   State : none( No 0 )               */
/*   Event : stt_crr_layer_apps_map_spl( No 0 ) */
/****************************************/
static void ZREL_RelRestOffs0e0( void )
{
    ZREL_RelMainState[ZREL_RELMAINS0F] = ( uint8_t )ZREL_RELRESTOFFS3;
    stm_rel_start_activity_restriction_split_sub();
}

/****************************************/
/* Action function                      */
/*   STM : RelRestOff                   */
/*   State : none( No 0 )               */
/*   Event : stt_crr_layer_apps_spl_nml( No 1 ) */
/****************************************/
static void ZREL_RelRestOffs0e1( void )
{
    ZREL_RelMainState[ZREL_RELMAINS0F] = ( uint8_t )ZREL_RELRESTOFFS1;
    stm_rel_start_activity_restriction_normal();
}

/****************************************/
/* Action function                      */
/*   STM : RelRestOff                   */
/*   State : none( No 0 )               */
/*   Event : ara_restriction_split_main( No 8 ) */
/****************************************/
static void ZREL_RelRestOffs0e8( void )
{
    ZREL_RelMainState[ZREL_RELMAINS0F] = ( uint8_t )ZREL_RELRESTOFFS2;
    stm_rel_start_activity_restriction_split_main();
}

/****************************************/
/* Action function                      */
/*   STM : RelRestOff                   */
/*   State : none( No 0 )               */
/*   Event : stt_prv_layer_rst_none( No 13 ) */
/****************************************/
static void ZREL_RelRestOffs0e13( void )
{
    stm_rel_start_activity_none();
}

/****************************************/
/* Action function                      */
/*   STM : RelRestOff                   */
/*   State : restriction_normal( No 1 ) */
/*   Event : stt_crr_layer_apps_spl_nml( No 1 ) */
/****************************************/
static void ZREL_RelRestOffs1e1( void )
{
    stm_rel_start_activity_restriction_normal();
}

/****************************************/
/* Action function                      */
/*   STM : RelRestOff                   */
/*   State : restriction_normal( No 1 ) */
/*   Event : ELSE( No 4 )               */
/****************************************/
static void ZREL_RelRestOffs1e4( void )
{
    ZREL_RelMainState[ZREL_RELMAINS0F] = ( uint8_t )ZREL_RELRESTOFFS0;
    stm_rel_start_activity_none();
}

/****************************************/
/* Action function                      */
/*   STM : RelRestOff                   */
/*   State : restriction_split_main( No 2 ) */
/*   Event : stt_prv_layer_rst_rst_spl_main( No 15 ) */
/****************************************/
static void ZREL_RelRestOffs2e15( void )
{
    stm_rel_start_activity_restriction_split_main();
}

/****************************************/
/* Action function                      */
/*   STM : RelRestOff                   */
/*   State : restriction_split_sub( No 3 ) */
/*   Event : stt_crr_layer_apps_map_spl( No 0 ) */
/****************************************/
static void ZREL_RelRestOffs3e0( void )
{
    stm_rel_start_activity_restriction_split_sub();
}

/****************************************/
/* Event appraisal function             */
/*   STM : RelMain                      */
/*   State : restriction_mode_off( No 0 ) */
/****************************************/
static void ZREL_RelMains0Event( void )
{
    /*stt_restriction_mode_2_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo2On )
    {
        stm_rel_event_restriction_mode_2_on();
        ZREL_RelMains0e1();
    }
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RelRestOff                   */
/*   State : none( No 0 )               */
/****************************************/
static void ZREL_RelRestOffs0Event( void )
{
    /*stt_restriction_mode_1_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo1On )
    {
        /*stt_crr_layer_apps_changed*/
        if( g_stm_crr_state.layer[StmLayerNoApps].changed == STM_TRUE )
        {
            /*stt_crr_layer_apps_map_spl*/
            if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZREL_RelRestOffs0e0();
            }
            /*stt_crr_layer_apps_spl_nml*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZREL_RelRestOffs0e1();
            }
            /*stt_crr_layer_apps_spl_spl*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZREL_RelRestOffs0e1();
            }
            /*stt_crr_layer_apps_gen_nml*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZREL_RelRestOffs0e1();
            }
            else
            {
                /*Else and default design have not done.*/
                /*Please confirm the STM and design else and default.*/
            }
        }
        /*evt_activate*/
        else if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_restriction*/
            if( g_stm_category == StmCtgNoRestriction )
            {
                /*ara_restriction_normal*/
                if( g_stm_area == StmAreaNoRestrictionNormal )
                {
                    /*stt_crr_layer_apps_spl_nml*/
                    if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
                    {
                        ZREL_RelRestOffs0e1();
                    }
                    /*stt_crr_layer_apps_map_spl*/
                    else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
                    {
                        ZREL_RelRestOffs0e0();
                    }
                    else
                    {
                        /*Else and default design have not done.*/
                        /*Please confirm the STM and design else and default.*/
                    }
                }
                /*ara_restriction_split_main*/
                else if( g_stm_area == StmAreaNoRestrictionSplitMain )
                {
                    ZREL_RelRestOffs0e8();
                }
                /*ara_restriction_split_sub*/
                else if( g_stm_area == StmAreaNoRestrictionSplitSub )
                {
                    ZREL_RelRestOffs0e0();
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
    /*stt_restriction_mode_off*/
    else if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_undo*/
        if( g_stm_event == StmEvtNoUndo )
        {
            /*stt_prv_layer_rst_none*/
            if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoNone )
            {
                ZREL_RelRestOffs0e13();
            }
            /*stt_prv_layer_rst_rst_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstNml )
            {
                ZREL_RelRestOffs0e1();
            }
            /*stt_prv_layer_rst_rst_spl_main*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstSplMain )
            {
                ZREL_RelRestOffs0e8();
            }
            /*stt_prv_layer_rst_rst_spl_sub*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstSplSub )
            {
                ZREL_RelRestOffs0e0();
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
/*   STM : RelRestOff                   */
/*   State : restriction_normal( No 1 ) */
/****************************************/
static void ZREL_RelRestOffs1Event( void )
{
    /*stt_restriction_mode_1_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo1On )
    {
        /*stt_crr_layer_apps_changed*/
        if( g_stm_crr_state.layer[StmLayerNoApps].changed == STM_TRUE )
        {
            /*stt_crr_layer_apps_map_spl*/
            if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZREL_RelRestOffs0e0();
            }
            /*stt_crr_layer_apps_spl_nml*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZREL_RelRestOffs1e1();
            }
            /*stt_crr_layer_apps_spl_spl*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZREL_RelRestOffs1e1();
            }
            /*stt_crr_layer_apps_gen_nml*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZREL_RelRestOffs1e1();
            }
            else
            {
                ZREL_RelRestOffs1e4();
            }
        }
        /*stt_crr_layer_hs_changed*/
        else if( g_stm_crr_state.layer[StmLayerNoHomescreen].changed == STM_TRUE )
        {
            /*stt_crr_layer_hs_hms*/
            if( g_stm_crr_state.layer[StmLayerNoHomescreen].state == StmLayoutNoHms )
            {
                ZREL_RelRestOffs1e4();
            }
            else
            {
                /*Else and default design have not done.*/
                /*Please confirm the STM and design else and default.*/
            }
        }
        /*evt_activate*/
        else if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_restriction*/
            if( g_stm_category == StmCtgNoRestriction )
            {
                /*ara_restriction_normal*/
                if( g_stm_area == StmAreaNoRestrictionNormal )
                {
                    /*stt_crr_layer_apps_map_spl*/
                    if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
                    {
                        ZREL_RelRestOffs0e0();
                    }
                    else
                    {
                        /*Else and default design have not done.*/
                        /*Please confirm the STM and design else and default.*/
                    }
                }
                /*ara_restriction_split_main*/
                else if( g_stm_area == StmAreaNoRestrictionSplitMain )
                {
                    ZREL_RelRestOffs0e8();
                }
                /*ara_restriction_split_sub*/
                else if( g_stm_area == StmAreaNoRestrictionSplitSub )
                {
                    ZREL_RelRestOffs0e0();
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
                    ZREL_RelRestOffs1e4();
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
            /*ctg_restriction*/
            if( g_stm_category == StmCtgNoRestriction )
            {
                ZREL_RelRestOffs1e4();
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
    /*stt_restriction_mode_off*/
    else if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*stt_prv_layer_rst_not_none*/
        if( g_stm_prv_state.layer[StmLayerNoRestriction].state != StmLayoutNoNone )
        {
            ZREL_RelRestOffs1e4();
        }
        /*evt_undo*/
        else if( g_stm_event == StmEvtNoUndo )
        {
            /*stt_prv_layer_rst_none*/
            if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoNone )
            {
                ZREL_RelRestOffs1e4();
            }
            /*stt_prv_layer_rst_rst_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstNml )
            {
                ZREL_RelRestOffs1e1();
            }
            /*stt_prv_layer_rst_rst_spl_main*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstSplMain )
            {
                ZREL_RelRestOffs0e8();
            }
            /*stt_prv_layer_rst_rst_spl_sub*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstSplSub )
            {
                ZREL_RelRestOffs0e0();
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
/*   STM : RelRestOff                   */
/*   State : restriction_split_main( No 2 ) */
/****************************************/
static void ZREL_RelRestOffs2Event( void )
{
    /*stt_restriction_mode_1_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo1On )
    {
        /*stt_crr_layer_apps_changed*/
        if( g_stm_crr_state.layer[StmLayerNoApps].changed == STM_TRUE )
        {
            /*stt_crr_layer_apps_map_spl*/
            if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZREL_RelRestOffs0e0();
            }
            /*stt_crr_layer_apps_spl_nml*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZREL_RelRestOffs0e1();
            }
            /*stt_crr_layer_apps_spl_spl*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZREL_RelRestOffs0e1();
            }
            /*stt_crr_layer_apps_gen_nml*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZREL_RelRestOffs0e1();
            }
            else
            {
                ZREL_RelRestOffs1e4();
            }
        }
        /*stt_crr_layer_hs_changed*/
        else if( g_stm_crr_state.layer[StmLayerNoHomescreen].changed == STM_TRUE )
        {
            /*stt_crr_layer_hs_hms*/
            if( g_stm_crr_state.layer[StmLayerNoHomescreen].state == StmLayoutNoHms )
            {
                ZREL_RelRestOffs1e4();
            }
            else
            {
                /*Else and default design have not done.*/
                /*Please confirm the STM and design else and default.*/
            }
        }
        /*evt_activate*/
        else if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_restriction*/
            if( g_stm_category == StmCtgNoRestriction )
            {
                /*ara_restriction_normal*/
                if( g_stm_area == StmAreaNoRestrictionNormal )
                {
                    /*stt_crr_layer_apps_spl_nml*/
                    if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
                    {
                        ZREL_RelRestOffs0e1();
                    }
                    /*stt_crr_layer_apps_map_spl*/
                    else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
                    {
                        ZREL_RelRestOffs0e0();
                    }
                    else
                    {
                        /*Else and default design have not done.*/
                        /*Please confirm the STM and design else and default.*/
                    }
                }
                /*ara_restriction_split_sub*/
                else if( g_stm_area == StmAreaNoRestrictionSplitSub )
                {
                    ZREL_RelRestOffs0e1();
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
                    ZREL_RelRestOffs1e4();
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
            /*ctg_restriction*/
            if( g_stm_category == StmCtgNoRestriction )
            {
                ZREL_RelRestOffs1e4();
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
    /*stt_restriction_mode_off*/
    else if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*stt_prv_layer_rst_not_none*/
        if( g_stm_prv_state.layer[StmLayerNoRestriction].state != StmLayoutNoNone )
        {
            ZREL_RelRestOffs1e4();
        }
        /*evt_undo*/
        else if( g_stm_event == StmEvtNoUndo )
        {
            /*stt_prv_layer_rst_none*/
            if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoNone )
            {
                ZREL_RelRestOffs1e4();
            }
            /*stt_prv_layer_rst_rst_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstNml )
            {
                ZREL_RelRestOffs0e1();
            }
            /*stt_prv_layer_rst_rst_spl_main*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstSplMain )
            {
                ZREL_RelRestOffs2e15();
            }
            /*stt_prv_layer_rst_rst_spl_sub*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstSplSub )
            {
                ZREL_RelRestOffs0e0();
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
/*   STM : RelRestOff                   */
/*   State : restriction_split_sub( No 3 ) */
/****************************************/
static void ZREL_RelRestOffs3Event( void )
{
    /*stt_restriction_mode_1_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo1On )
    {
        /*stt_crr_layer_apps_changed*/
        if( g_stm_crr_state.layer[StmLayerNoApps].changed == STM_TRUE )
        {
            /*stt_crr_layer_apps_map_spl*/
            if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoMapSpl )
            {
                ZREL_RelRestOffs3e0();
            }
            /*stt_crr_layer_apps_spl_nml*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
            {
                ZREL_RelRestOffs0e1();
            }
            /*stt_crr_layer_apps_spl_spl*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplSpl )
            {
                ZREL_RelRestOffs0e1();
            }
            /*stt_crr_layer_apps_gen_nml*/
            else if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoGenNml )
            {
                ZREL_RelRestOffs0e1();
            }
            else
            {
                ZREL_RelRestOffs1e4();
            }
        }
        /*stt_crr_layer_hs_changed*/
        else if( g_stm_crr_state.layer[StmLayerNoHomescreen].changed == STM_TRUE )
        {
            /*stt_crr_layer_hs_hms*/
            if( g_stm_crr_state.layer[StmLayerNoHomescreen].state == StmLayoutNoHms )
            {
                ZREL_RelRestOffs1e4();
            }
            else
            {
                /*Else and default design have not done.*/
                /*Please confirm the STM and design else and default.*/
            }
        }
        /*evt_activate*/
        else if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_restriction*/
            if( g_stm_category == StmCtgNoRestriction )
            {
                /*ara_restriction_normal*/
                if( g_stm_area == StmAreaNoRestrictionNormal )
                {
                    /*stt_crr_layer_apps_spl_nml*/
                    if( g_stm_crr_state.layer[StmLayerNoApps].state == StmLayoutNoSplNml )
                    {
                        ZREL_RelRestOffs0e1();
                    }
                    else
                    {
                        /*Else and default design have not done.*/
                        /*Please confirm the STM and design else and default.*/
                    }
                }
                /*ara_restriction_split_main*/
                else if( g_stm_area == StmAreaNoRestrictionSplitMain )
                {
                    ZREL_RelRestOffs0e1();
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
                    ZREL_RelRestOffs1e4();
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
            /*ctg_restriction*/
            if( g_stm_category == StmCtgNoRestriction )
            {
                ZREL_RelRestOffs1e4();
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
    /*stt_restriction_mode_off*/
    else if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*stt_prv_layer_rst_not_none*/
        if( g_stm_prv_state.layer[StmLayerNoRestriction].state != StmLayoutNoNone )
        {
            ZREL_RelRestOffs1e4();
        }
        /*evt_undo*/
        else if( g_stm_event == StmEvtNoUndo )
        {
            /*stt_prv_layer_rst_none*/
            if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoNone )
            {
                ZREL_RelRestOffs1e4();
            }
            /*stt_prv_layer_rst_rst_nml*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstNml )
            {
                ZREL_RelRestOffs0e1();
            }
            /*stt_prv_layer_rst_rst_spl_main*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstSplMain )
            {
                ZREL_RelRestOffs0e8();
            }
            /*stt_prv_layer_rst_rst_spl_sub*/
            else if( g_stm_prv_state.layer[StmLayerNoRestriction].state == StmLayoutNoRstSplSub )
            {
                ZREL_RelRestOffs3e0();
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
/*   STM : RelMain                      */
/*   State : restriction_mode_2_on( No 1 ) */
/****************************************/
static void ZREL_RelMains1Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        stm_rel_event_restriction_mode_off();
        ZREL_RelMains1e0();
    }
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RelRestOn                    */
/*   State : none( No 0 )               */
/****************************************/
static void ZREL_RelRestOns0Event( void )
{
}

/****************************************/
/* Event call function                  */
/*   STM : RelMain                      */
/****************************************/
void stm_rel_event_call( void )
{
    stm_rel_start_stm();
    switch( ZREL_RelMainState[ZREL_RELMAIN] )
    {
    case ZREL_RELMAINS0:
        switch( ZREL_RelMainState[ZREL_RELMAINS0F] )
        {
        case ZREL_RELRESTOFFS0:
            ZREL_RelRestOffs0Event();
            break;
        case ZREL_RELRESTOFFS1:
            ZREL_RelRestOffs1Event();
            break;
        case ZREL_RELRESTOFFS2:
            ZREL_RelRestOffs2Event();
            break;
        case ZREL_RELRESTOFFS3:
            ZREL_RelRestOffs3Event();
            break;
        default:
            /*Not accessible to this else (default).*/
            break;
        }
        ZREL_RelMains0Event();
        break;
    case ZREL_RELMAINS1:
        switch( ZREL_RelMainState[ZREL_RELMAINS1F] )
        {
        case ZREL_RELRESTONS0:
            ZREL_RelRestOns0Event();
            break;
        default:
            /*Not accessible to this else (default).*/
            break;
        }
        ZREL_RelMains1Event();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* Initial function                     */
/*   STM : RelMain                      */
/****************************************/
void stm_rel_initialize( void )
{
    ZREL_RelMainState[ZREL_RELMAIN] = ( uint8_t )ZREL_RELMAINS0;
    ZREL_RelMainState[ZREL_RELMAINS0F] = ( uint8_t )ZREL_RELRESTOFFS0;
    ZREL_RelMainState[ZREL_RELMAINS1F] = ( uint8_t )ZREL_RELRESTONS0;
    ZREL_RelMains0StateEntry();
}

/****************************************/
/* Terminate function                   */
/*   STM : RelMain                      */
/****************************************/
void ZREL_RelMainTerminate( void )
{
    ZREL_RelMainState[ZREL_RELMAIN] = ( uint8_t )ZREL_RELMAINTERMINATE;
}

