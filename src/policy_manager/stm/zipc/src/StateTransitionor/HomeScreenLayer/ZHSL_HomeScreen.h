/************************************************************/
/*     ZHSL_HomeScreen.h                                    */
/*     HomeScreen State transition model header file        */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#ifndef ZHEADER_ZHSL_HOMESCREEN_H
#define ZHEADER_ZHSL_HOMESCREEN_H

/*State management variable access define*/
#define ZHSL_HOMESCREEN ( 0U )
#define ZHSL_HOMESCREENS0 ( 0U )
#define ZHSL_HOMESCREENS1 ( 1U )
#define ZHSL_HOMESCREENSTATENOMAX ( 1U )

/*End state define*/
#define ZHSL_HOMESCREENEND ( 2U )
/*Terminate state define*/
#define ZHSL_HOMESCREENTERMINATE ( ZHSL_HOMESCREENEND + 1U )

/*State no define*/
#define ZHSL_HOMESCREENS0STATENO ( 0U )
#define ZHSL_HOMESCREENS1STATENO ( 1U )

/*State serial no define*/
#define ZHSL_HOMESCREENS0STATESERIALNO ( 0U )
#define ZHSL_HOMESCREENS1STATESERIALNO ( 1U )

/*Event no define*/
#define ZHSL_HOMESCREENE0EVENTNO ( 0U )

/*Event serial no define*/
#define ZHSL_HOMESCREENE0EVENTSERIALNO ( 0U )

/*Extern function*/
extern void stm_hsl_event_call( void );
extern void stm_hsl_initialize( void );
extern void ZHSL_HomeScreenTerminate( void );

#endif
