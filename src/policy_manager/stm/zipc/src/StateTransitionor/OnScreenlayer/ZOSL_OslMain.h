/************************************************************/
/*     ZOSL_OslMain.h                                       */
/*     OslMain State transition model header file           */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#ifndef ZHEADER_ZOSL_OSLMAIN_H
#define ZHEADER_ZOSL_OSLMAIN_H

/*State management variable access define*/
#define ZOSL_OSLMAIN ( 0U )
#define ZOSL_OSLMAINS0F ( 1U )
#define ZOSL_OSLMAINS1F ( 2U )
#define ZOSL_OSLMAINS0 ( 0U )
#define ZOSL_OSLRESTOFFS0 ( 0U )
#define ZOSL_OSLRESTOFFS1 ( 1U )
#define ZOSL_OSLRESTOFFS2 ( 2U )
#define ZOSL_OSLMAINS1 ( 1U )
#define ZOSL_OSLRESTONS0 ( 0U )
#define ZOSL_OSLRESTONS1 ( 1U )
#define ZOSL_OSLMAINSTATENOMAX ( 3U )

/*End state define*/
#define ZOSL_OSLMAINEND ( 3U )
/*Terminate state define*/
#define ZOSL_OSLMAINTERMINATE ( ZOSL_OSLMAINEND + 1U )

/*State no define*/
#define ZOSL_OSLMAINS0STATENO ( 0U )
#define ZOSL_OSLRESTOFFS0STATENO ( 0U )
#define ZOSL_OSLRESTOFFS1STATENO ( 1U )
#define ZOSL_OSLRESTOFFS2STATENO ( 2U )
#define ZOSL_OSLMAINS1STATENO ( 1U )
#define ZOSL_OSLRESTONS0STATENO ( 0U )
#define ZOSL_OSLRESTONS1STATENO ( 1U )

/*State serial no define*/
#define ZOSL_OSLMAINS0STATESERIALNO ( 0U )
#define ZOSL_OSLRESTOFFS0STATESERIALNO ( 1U )
#define ZOSL_OSLRESTOFFS1STATESERIALNO ( 2U )
#define ZOSL_OSLRESTOFFS2STATESERIALNO ( 3U )
#define ZOSL_OSLMAINS1STATESERIALNO ( 4U )
#define ZOSL_OSLRESTONS0STATESERIALNO ( 5U )
#define ZOSL_OSLRESTONS1STATESERIALNO ( 6U )

/*Event no define*/
#define ZOSL_OSLMAINE0EVENTNO ( 0U )
#define ZOSL_OSLMAINE1EVENTNO ( 1U )
#define ZOSL_OSLMAINE2EVENTNO ( 2U )
#define ZOSL_OSLRESTOFFE0EVENTNO ( 0U )
#define ZOSL_OSLRESTOFFE1EVENTNO ( 1U )
#define ZOSL_OSLRESTOFFE2EVENTNO ( 2U )
#define ZOSL_OSLRESTOFFE3EVENTNO ( 3U )
#define ZOSL_OSLRESTOFFE4EVENTNO ( 4U )
#define ZOSL_OSLRESTOFFE5EVENTNO ( 5U )
#define ZOSL_OSLRESTOFFE6EVENTNO ( 6U )
#define ZOSL_OSLRESTONE0EVENTNO ( 0U )
#define ZOSL_OSLRESTONE1EVENTNO ( 1U )
#define ZOSL_OSLRESTONE2EVENTNO ( 2U )
#define ZOSL_OSLRESTONE3EVENTNO ( 3U )

/*Event serial no define*/
#define ZOSL_OSLMAINE0EVENTSERIALNO ( 0U )
#define ZOSL_OSLMAINE1EVENTSERIALNO ( 1U )
#define ZOSL_OSLMAINE2EVENTSERIALNO ( 2U )
#define ZOSL_OSLRESTOFFE0EVENTNO ( 0U )
#define ZOSL_OSLRESTOFFE1EVENTNO ( 1U )
#define ZOSL_OSLRESTOFFE2EVENTNO ( 2U )
#define ZOSL_OSLRESTOFFE3EVENTNO ( 3U )
#define ZOSL_OSLRESTOFFE4EVENTNO ( 4U )
#define ZOSL_OSLRESTOFFE5EVENTNO ( 5U )
#define ZOSL_OSLRESTOFFE6EVENTNO ( 6U )
#define ZOSL_OSLRESTONE0EVENTNO ( 0U )
#define ZOSL_OSLRESTONE1EVENTNO ( 1U )
#define ZOSL_OSLRESTONE2EVENTNO ( 2U )
#define ZOSL_OSLRESTONE3EVENTNO ( 3U )

/*Extern function*/
extern void stm_osl_event_call( void );
extern void stm_osl_initialize( void );
extern void ZOSL_OslMainTerminate( void );

#endif
