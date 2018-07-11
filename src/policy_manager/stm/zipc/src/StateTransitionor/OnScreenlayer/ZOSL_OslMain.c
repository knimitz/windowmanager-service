/************************************************************/
/*     ZOSL_OslMain.c                                       */
/*     OslMain State transition model source file           */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/* State management variable */
static uint8_t ZOSL_OslMainState[ZOSL_OSLMAINSTATENOMAX];

static void ZOSL_OslMains0StateEntry( void );
static void ZOSL_OslMains1StateEntry( void );
static void ZOSL_OslMains0e1( void );
static void ZOSL_OslMains0e2( void );
static void ZOSL_OslMains1e0( void );
static void ZOSL_OslRestOffs0e0( void );
static void ZOSL_OslRestOffs0e1( void );
static void ZOSL_OslRestOffs0e4( void );
static void ZOSL_OslRestOffs1e0( void );
static void ZOSL_OslRestOffs1e2( void );
static void ZOSL_OslRestOffs2e1( void );
static void ZOSL_OslRestOns0e0( void );
static void ZOSL_OslRestOns0e2( void );
static void ZOSL_OslRestOns1e0( void );
static void ZOSL_OslRestOns1e1( void );
static void ZOSL_OslMains0Event( void );
static void ZOSL_OslRestOffs0Event( void );
static void ZOSL_OslRestOffs1Event( void );
static void ZOSL_OslRestOffs2Event( void );
static void ZOSL_OslMains1Event( void );
static void ZOSL_OslRestOns0Event( void );
static void ZOSL_OslRestOns1Event( void );

/****************************************/
/* State start activity function        */
/*   STM : OslMain                      */
/*   State : restriction_mode_off( No 0 ) */
/****************************************/
static void ZOSL_OslMains0StateEntry( void )
{
    switch( ZOSL_OslMainState[ZOSL_OSLMAINS0F] )
    {
    case ZOSL_OSLRESTOFFS0:
        stm_osl_start_activity_none();
        break;
    case ZOSL_OSLRESTOFFS1:
        stm_osl_start_activity_pop_up();
        break;
    case ZOSL_OSLRESTOFFS2:
        stm_osl_start_activity_system_alert();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* State start activity function        */
/*   STM : OslMain                      */
/*   State : restriction_mode_2_on( No 1 ) */
/****************************************/
static void ZOSL_OslMains1StateEntry( void )
{
    switch( ZOSL_OslMainState[ZOSL_OSLMAINS1F] )
    {
    case ZOSL_OSLRESTONS0:
        stm_osl_start_activity_none();
        break;
    case ZOSL_OSLRESTONS1:
        stm_osl_start_activity_system_alert();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* Action function                      */
/*   STM : OslMain                      */
/*   State : restriction_mode_off( No 0 ) */
/*   Event : stt_crr_layer_on_screen_pop( No 1 ) */
/****************************************/
static void ZOSL_OslMains0e1( void )
{
    ZOSL_OslMainState[ZOSL_OSLMAIN] = ( uint8_t )ZOSL_OSLMAINS1;
    ZOSL_OslMainState[ZOSL_OSLMAINS1F] = ( uint8_t )ZOSL_OSLRESTONS0;
    ZOSL_OslMains1StateEntry();
}

/****************************************/
/* Action function                      */
/*   STM : OslMain                      */
/*   State : restriction_mode_off( No 0 ) */
/*   Event : stt_crr_layer_on_screen_sys( No 2 ) */
/****************************************/
static void ZOSL_OslMains0e2( void )
{
    ZOSL_OslMainState[ZOSL_OSLMAIN] = ( uint8_t )ZOSL_OSLMAINS1;
    ZOSL_OslMainState[ZOSL_OSLMAINS1F] = ( uint8_t )ZOSL_OSLRESTONS1;
    ZOSL_OslMains1StateEntry();
}

/****************************************/
/* Action function                      */
/*   STM : OslMain                      */
/*   State : restriction_mode_2_on( No 1 ) */
/*   Event : stt_restriction_mode_off( No 0 ) */
/****************************************/
static void ZOSL_OslMains1e0( void )
{
    ZOSL_OslMainState[ZOSL_OSLMAIN] = ( uint8_t )ZOSL_OSLMAINS0;
    ZOSL_OslMains0StateEntry();
}

/****************************************/
/* Action function                      */
/*   STM : OslRestOff                   */
/*   State : none( No 0 )               */
/*   Event : ara_onscreen( No 0 )       */
/****************************************/
static void ZOSL_OslRestOffs0e0( void )
{
    ZOSL_OslMainState[ZOSL_OSLMAINS0F] = ( uint8_t )ZOSL_OSLRESTOFFS1;
    stm_osl_start_activity_pop_up();
}

/****************************************/
/* Action function                      */
/*   STM : OslRestOff                   */
/*   State : none( No 0 )               */
/*   Event : ara_onscreen( No 1 )       */
/****************************************/
static void ZOSL_OslRestOffs0e1( void )
{
    ZOSL_OslMainState[ZOSL_OSLMAINS0F] = ( uint8_t )ZOSL_OSLRESTOFFS2;
    stm_osl_start_activity_system_alert();
}

/****************************************/
/* Action function                      */
/*   STM : OslRestOff                   */
/*   State : none( No 0 )               */
/*   Event : stt_prv_layer_on_screen_none( No 4 ) */
/****************************************/
static void ZOSL_OslRestOffs0e4( void )
{
    stm_osl_start_activity_none();
}

/****************************************/
/* Action function                      */
/*   STM : OslRestOff                   */
/*   State : popup( No 1 )              */
/*   Event : ara_onscreen( No 0 )       */
/****************************************/
static void ZOSL_OslRestOffs1e0( void )
{
    stm_osl_start_activity_pop_up();
}

/****************************************/
/* Action function                      */
/*   STM : OslRestOff                   */
/*   State : popup( No 1 )              */
/*   Event : ctg_popup( No 2 )          */
/****************************************/
static void ZOSL_OslRestOffs1e2( void )
{
    ZOSL_OslMainState[ZOSL_OSLMAINS0F] = ( uint8_t )ZOSL_OSLRESTOFFS0;
    stm_osl_start_activity_none();
}

/****************************************/
/* Action function                      */
/*   STM : OslRestOff                   */
/*   State : system_alert( No 2 )       */
/*   Event : ara_onscreen( No 1 )       */
/****************************************/
static void ZOSL_OslRestOffs2e1( void )
{
    stm_osl_start_activity_system_alert();
}

/****************************************/
/* Action function                      */
/*   STM : OslRestOn                    */
/*   State : none( No 0 )               */
/*   Event : ara_onscreen( No 0 )       */
/****************************************/
static void ZOSL_OslRestOns0e0( void )
{
    ZOSL_OslMainState[ZOSL_OSLMAINS1F] = ( uint8_t )ZOSL_OSLRESTONS1;
    stm_osl_start_activity_system_alert();
}

/****************************************/
/* Action function                      */
/*   STM : OslRestOn                    */
/*   State : none( No 0 )               */
/*   Event : stt_prv_layer_on_screen_none( No 2 ) */
/****************************************/
static void ZOSL_OslRestOns0e2( void )
{
    stm_osl_start_activity_none();
}

/****************************************/
/* Action function                      */
/*   STM : OslRestOn                    */
/*   State : system_alert( No 1 )       */
/*   Event : ara_onscreen( No 0 )       */
/****************************************/
static void ZOSL_OslRestOns1e0( void )
{
    stm_osl_start_activity_system_alert();
}

/****************************************/
/* Action function                      */
/*   STM : OslRestOn                    */
/*   State : system_alert( No 1 )       */
/*   Event : ctg_systemalert( No 1 )    */
/****************************************/
static void ZOSL_OslRestOns1e1( void )
{
    ZOSL_OslMainState[ZOSL_OSLMAINS1F] = ( uint8_t )ZOSL_OSLRESTONS0;
    stm_osl_start_activity_none();
}

/****************************************/
/* Event appraisal function             */
/*   STM : OslMain                      */
/*   State : restriction_mode_off( No 0 ) */
/****************************************/
static void ZOSL_OslMains0Event( void )
{
    /*stt_restriction_mode_2_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo2On )
    {
        stm_rel_event_restriction_mode_2_on();
        /*stt_crr_layer_on_screen_pop*/
        if( g_stm_crr_state.layer[StmLayerNoOnScreen].state == StmLayoutNoPopUp )
        {
            ZOSL_OslMains0e1();
        }
        /*stt_crr_layer_on_screen_sys*/
        else if( g_stm_crr_state.layer[StmLayerNoOnScreen].state == StmLayoutNoSysAlt )
        {
            ZOSL_OslMains0e2();
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
/*   STM : OslRestOff                   */
/*   State : none( No 0 )               */
/****************************************/
static void ZOSL_OslRestOffs0Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_popup*/
            if( g_stm_category == StmCtgNoPopUp )
            {
                /*ara_onscreen*/
                if( g_stm_area == StmAreaNoOnScreen )
                {
                    ZOSL_OslRestOffs0e0();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_systemalert*/
            else if( g_stm_category == StmCtgNoSystemAlert )
            {
                /*ara_onscreen*/
                if( g_stm_area == StmAreaNoOnScreen )
                {
                    ZOSL_OslRestOffs0e1();
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
            /*stt_prv_layer_on_screen_none*/
            if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoNone )
            {
                ZOSL_OslRestOffs0e4();
            }
            /*stt_prv_layer_on_screen_pop_up*/
            else if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoPopUp )
            {
                ZOSL_OslRestOffs0e0();
            }
            /*stt_prv_layer_on_screen_sys_alt*/
            else if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoSysAlt )
            {
                ZOSL_OslRestOffs0e1();
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
/*   STM : OslRestOff                   */
/*   State : popup( No 1 )              */
/****************************************/
static void ZOSL_OslRestOffs1Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_popup*/
            if( g_stm_category == StmCtgNoPopUp )
            {
                /*ara_onscreen*/
                if( g_stm_area == StmAreaNoOnScreen )
                {
                    ZOSL_OslRestOffs1e0();
                }
                else
                {
                    /*Else and default design have not done.*/
                    /*Please confirm the STM and design else and default.*/
                }
            }
            /*ctg_systemalert*/
            else if( g_stm_category == StmCtgNoSystemAlert )
            {
                /*ara_onscreen*/
                if( g_stm_area == StmAreaNoOnScreen )
                {
                    ZOSL_OslRestOffs0e1();
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
            /*ctg_popup*/
            if( g_stm_category == StmCtgNoPopUp )
            {
                ZOSL_OslRestOffs1e2();
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
            /*stt_prv_layer_on_screen_none*/
            if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoNone )
            {
                ZOSL_OslRestOffs1e2();
            }
            /*stt_prv_layer_on_screen_pop_up*/
            else if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoPopUp )
            {
                ZOSL_OslRestOffs1e0();
            }
            /*stt_prv_layer_on_screen_sys_alt*/
            else if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoSysAlt )
            {
                ZOSL_OslRestOffs0e1();
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
/*   STM : OslRestOff                   */
/*   State : system_alert( No 2 )       */
/****************************************/
static void ZOSL_OslRestOffs2Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_systemalert*/
            if( g_stm_category == StmCtgNoSystemAlert )
            {
                /*ara_onscreen*/
                if( g_stm_area == StmAreaNoOnScreen )
                {
                    ZOSL_OslRestOffs2e1();
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
            /*ctg_systemalert*/
            if( g_stm_category == StmCtgNoSystemAlert )
            {
                ZOSL_OslRestOffs1e2();
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
            /*stt_prv_layer_on_screen_none*/
            if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoNone )
            {
                ZOSL_OslRestOffs1e2();
            }
            /*stt_prv_layer_on_screen_pop_up*/
            else if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoPopUp )
            {
                ZOSL_OslRestOffs0e0();
            }
            /*stt_prv_layer_on_screen_sys_alt*/
            else if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoSysAlt )
            {
                ZOSL_OslRestOffs2e1();
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
/*   STM : OslMain                      */
/*   State : restriction_mode_2_on( No 1 ) */
/****************************************/
static void ZOSL_OslMains1Event( void )
{
    /*stt_restriction_mode_off*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNoOff )
    {
        stm_rel_event_restriction_mode_off();
        ZOSL_OslMains1e0();
    }
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : OslRestOn                    */
/*   State : none( No 0 )               */
/****************************************/
static void ZOSL_OslRestOns0Event( void )
{
    /*stt_restriction_mode_2_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo2On )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_systemalert*/
            if( g_stm_category == StmCtgNoSystemAlert )
            {
                /*ara_onscreen*/
                if( g_stm_area == StmAreaNoOnScreen )
                {
                    ZOSL_OslRestOns0e0();
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
            /*stt_prv_layer_on_screen_none*/
            if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoNone )
            {
                ZOSL_OslRestOns0e2();
            }
            /*stt_prv_layer_on_screen_sys_alt*/
            else if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoSysAlt )
            {
                ZOSL_OslRestOns0e0();
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
/*   STM : OslRestOn                    */
/*   State : system_alert( No 1 )       */
/****************************************/
static void ZOSL_OslRestOns1Event( void )
{
    /*stt_restriction_mode_2_on*/
    if( g_stm_crr_state.mode[StmModeNoRestrictionMode].state == StmRestrictionModeSttNo2On )
    {
        /*evt_activate*/
        if( g_stm_event == StmEvtNoActivate )
        {
            /*ctg_systemalert*/
            if( g_stm_category == StmCtgNoSystemAlert )
            {
                /*ara_onscreen*/
                if( g_stm_area == StmAreaNoOnScreen )
                {
                    ZOSL_OslRestOns1e0();
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
            /*ctg_systemalert*/
            if( g_stm_category == StmCtgNoSystemAlert )
            {
                ZOSL_OslRestOns1e1();
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
            /*stt_prv_layer_on_screen_none*/
            if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoNone )
            {
                ZOSL_OslRestOns1e1();
            }
            /*stt_prv_layer_on_screen_sys_alt*/
            else if( g_stm_prv_state.layer[StmLayerNoOnScreen].state == StmLayoutNoSysAlt )
            {
                ZOSL_OslRestOns1e0();
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
/*   STM : OslMain                      */
/****************************************/
void stm_osl_event_call( void )
{
    stm_osl_start_stm();
    switch( ZOSL_OslMainState[ZOSL_OSLMAIN] )
    {
    case ZOSL_OSLMAINS0:
        switch( ZOSL_OslMainState[ZOSL_OSLMAINS0F] )
        {
        case ZOSL_OSLRESTOFFS0:
            ZOSL_OslRestOffs0Event();
            break;
        case ZOSL_OSLRESTOFFS1:
            ZOSL_OslRestOffs1Event();
            break;
        case ZOSL_OSLRESTOFFS2:
            ZOSL_OslRestOffs2Event();
            break;
        default:
            /*Not accessible to this else (default).*/
            break;
        }
        ZOSL_OslMains0Event();
        break;
    case ZOSL_OSLMAINS1:
        switch( ZOSL_OslMainState[ZOSL_OSLMAINS1F] )
        {
        case ZOSL_OSLRESTONS0:
            ZOSL_OslRestOns0Event();
            break;
        case ZOSL_OSLRESTONS1:
            ZOSL_OslRestOns1Event();
            break;
        default:
            /*Not accessible to this else (default).*/
            break;
        }
        ZOSL_OslMains1Event();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* Initial function                     */
/*   STM : OslMain                      */
/****************************************/
void stm_osl_initialize( void )
{
    ZOSL_OslMainState[ZOSL_OSLMAIN] = ( uint8_t )ZOSL_OSLMAINS0;
    ZOSL_OslMainState[ZOSL_OSLMAINS0F] = ( uint8_t )ZOSL_OSLRESTOFFS0;
    ZOSL_OslMainState[ZOSL_OSLMAINS1F] = ( uint8_t )ZOSL_OSLRESTONS0;
    ZOSL_OslMains0StateEntry();
}

/****************************************/
/* Terminate function                   */
/*   STM : OslMain                      */
/****************************************/
void ZOSL_OslMainTerminate( void )
{
    ZOSL_OslMainState[ZOSL_OSLMAIN] = ( uint8_t )ZOSL_OSLMAINTERMINATE;
}

