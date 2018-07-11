/************************************************************/
/*     ZHSL_HomeScreen.c                                    */
/*     HomeScreen State transition model source file        */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#include "../ZST_include.h"

/* State management variable */
static uint8_t ZHSL_HomeScreenState[ZHSL_HOMESCREENSTATENOMAX];

static void ZHSL_HomeScreens0e0( void );
static void ZHSL_HomeScreens1e0( void );
static void ZHSL_HomeScreens0Event( void );
static void ZHSL_HomeScreens1Event( void );

/****************************************/
/* Action function                      */
/*   STM : HomeScreen                   */
/*   State : none( No 0 )               */
/*   Event : ara_fullscreen( No 0 )     */
/****************************************/
static void ZHSL_HomeScreens0e0( void )
{
    ZHSL_HomeScreenState[ZHSL_HOMESCREEN] = ( uint8_t )ZHSL_HOMESCREENS1;
    stm_hsl_start_activity_homescreen();
}

/****************************************/
/* Action function                      */
/*   STM : HomeScreen                   */
/*   State : homescreen( No 1 )         */
/*   Event : ara_fullscreen( No 0 )     */
/****************************************/
static void ZHSL_HomeScreens1e0( void )
{
    stm_hsl_start_activity_homescreen();
}

/****************************************/
/* Event appraisal function             */
/*   STM : HomeScreen                   */
/*   State : none( No 0 )               */
/****************************************/
static void ZHSL_HomeScreens0Event( void )
{
    /*evt_activate*/
    if( g_stm_event == StmEvtNoActivate )
    {
        /*ctg_homescreen*/
        if( g_stm_category == StmCtgNoHomescreen )
        {
            /*ara_fullscreen*/
            if( g_stm_area == StmAreaNoFullscreen )
            {
                ZHSL_HomeScreens0e0();
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
/*   STM : HomeScreen                   */
/*   State : homescreen( No 1 )         */
/****************************************/
static void ZHSL_HomeScreens1Event( void )
{
    /*evt_activate*/
    if( g_stm_event == StmEvtNoActivate )
    {
        /*ctg_homescreen*/
        if( g_stm_category == StmCtgNoHomescreen )
        {
            /*ara_fullscreen*/
            if( g_stm_area == StmAreaNoFullscreen )
            {
                ZHSL_HomeScreens1e0();
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
/*   STM : HomeScreen                   */
/****************************************/
void stm_hsl_event_call( void )
{
    stm_hsl_start_stm();
    switch( ZHSL_HomeScreenState[ZHSL_HOMESCREEN] )
    {
    case ZHSL_HOMESCREENS0:
        ZHSL_HomeScreens0Event();
        break;
    case ZHSL_HOMESCREENS1:
        ZHSL_HomeScreens1Event();
        break;
    default:
        /*Not accessible to this else (default).*/
        break;
    }
}

/****************************************/
/* Initial function                     */
/*   STM : HomeScreen                   */
/****************************************/
void stm_hsl_initialize( void )
{
    ZHSL_HomeScreenState[ZHSL_HOMESCREEN] = ( uint8_t )ZHSL_HOMESCREENS0;
    stm_hsl_start_activity_none();
}

/****************************************/
/* Terminate function                   */
/*   STM : HomeScreen                   */
/****************************************/
void ZHSL_HomeScreenTerminate( void )
{
    ZHSL_HomeScreenState[ZHSL_HOMESCREEN] = ( uint8_t )ZHSL_HOMESCREENTERMINATE;
}

