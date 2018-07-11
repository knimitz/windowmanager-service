/************************************************************/
/*     ZREM_RestrictionMode.c                               */
/*     RestrictionMode State transition model source file   */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/* State management variable */
static uint8_t ZREM_RestrictionModeState[ZREM_RESTRICTIONMODESTATENOMAX];

static void ZREM_RestrictionModes0e1( void );
static void ZREM_RestrictionModes0e2( void );
static void ZREM_RestrictionModes1e0( void );
static void ZREM_RestrictionModes0Event( void );
static void ZREM_RestrictionModes1Event( void );
static void ZREM_RestrictionModes2Event( void );

/****************************************/
/* Action function                      */
/*   STM : RestrictionMode              */
/*   State : restriction_mode_off( No 0 ) */
/*   Event : evt_restriction_mode_1_on( No 1 ) */
/****************************************/
static void ZREM_RestrictionModes0e1( void )
{
    ZREM_RestrictionModeState[ZREM_RESTRICTIONMODE] = ( uint8_t )ZREM_RESTRICTIONMODES1;
    stm_rem_start_activity_restriction_mode_1_on();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionMode              */
/*   State : restriction_mode_off( No 0 ) */
/*   Event : evt_restriction_mode_2_on( No 2 ) */
/****************************************/
static void ZREM_RestrictionModes0e2( void )
{
    ZREM_RestrictionModeState[ZREM_RESTRICTIONMODE] = ( uint8_t )ZREM_RESTRICTIONMODES2;
    stm_rem_start_activity_restriction_mode_2_on();
}

/****************************************/
/* Action function                      */
/*   STM : RestrictionMode              */
/*   State : restriction_mode_1_on( No 1 ) */
/*   Event : evt_restriction_mode_off( No 0 ) */
/****************************************/
static void ZREM_RestrictionModes1e0( void )
{
    ZREM_RestrictionModeState[ZREM_RESTRICTIONMODE] = ( uint8_t )ZREM_RESTRICTIONMODES0;
    stm_rem_start_activity_restriction_mode_off();
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionMode              */
/*   State : restriction_mode_off( No 0 ) */
/****************************************/
static void ZREM_RestrictionModes0Event( void )
{
    /*evt_restriction_mode_1_on*/
    if( g_stm_event == StmEvtNoRestrictionMode1On )
    {
        ZREM_RestrictionModes0e1();
    }
    /*evt_restriction_mode_2_on*/
    else if( g_stm_event == StmEvtNoRestrictionMode2On )
    {
        ZREM_RestrictionModes0e2();
    }
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionMode              */
/*   State : restriction_mode_1_on( No 1 ) */
/****************************************/
static void ZREM_RestrictionModes1Event( void )
{
    /*evt_restriction_mode_off*/
    if( g_stm_event == StmEvtNoRestrictionModeOff )
    {
        ZREM_RestrictionModes1e0();
    }
    /*evt_restriction_mode_2_on*/
    else if( g_stm_event == StmEvtNoRestrictionMode2On )
    {
        ZREM_RestrictionModes0e2();
    }
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event appraisal function             */
/*   STM : RestrictionMode              */
/*   State : restriction_mode_2_on( No 2 ) */
/****************************************/
static void ZREM_RestrictionModes2Event( void )
{
    /*evt_restriction_mode_off*/
    if( g_stm_event == StmEvtNoRestrictionModeOff )
    {
        ZREM_RestrictionModes1e0();
    }
    /*evt_restriction_mode_1_on*/
    else if( g_stm_event == StmEvtNoRestrictionMode1On )
    {
        ZREM_RestrictionModes0e1();
    }
    else
    {
        /*Else and default design have not done.*/
        /*Please confirm the STM and design else and default.*/
    }
}

/****************************************/
/* Event call function                  */
/*   STM : RestrictionMode              */
/****************************************/
void stm_rem_event_call( void )
{
    stm_rem_start_stm();
    switch( ZREM_RestrictionModeState[ZREM_RESTRICTIONMODE] )
    {
    case ZREM_RESTRICTIONMODES0:
        ZREM_RestrictionModes0Event();
        break;
    case ZREM_RESTRICTIONMODES1:
        ZREM_RestrictionModes1Event();
        break;
    case ZREM_RESTRICTIONMODES2:
        ZREM_RestrictionModes2Event();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* Initial function                     */
/*   STM : RestrictionMode              */
/****************************************/
void stm_rem_initialize( void )
{
    ZREM_RestrictionModeState[ZREM_RESTRICTIONMODE] = ( uint8_t )ZREM_RESTRICTIONMODES0;
    stm_rem_start_activity_restriction_mode_off();
}

/****************************************/
/* Terminate function                   */
/*   STM : RestrictionMode              */
/****************************************/
void ZREM_RestrictionModeTerminate( void )
{
    ZREM_RestrictionModeState[ZREM_RESTRICTIONMODE] = ( uint8_t )ZREM_RESTRICTIONMODETERMINATE;
}

