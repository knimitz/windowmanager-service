/************************************************************/
/*     ZNHL_NearHomescreen.h                                */
/*     NearHomescreen State transition model header file    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#ifndef ZHEADER_ZNHL_NEARHOMESCREEN_H
#define ZHEADER_ZNHL_NEARHOMESCREEN_H

/*State management variable access define*/
#define ZNHL_NEARHOMESCREEN ( 0U )
#define ZNHL_NEARHOMESCREENS0 ( 0U )
#define ZNHL_NEARHOMESCREENS1 ( 1U )
#define ZNHL_NEARHOMESCREENSTATENOMAX ( 1U )

/*End state define*/
#define ZNHL_NEARHOMESCREENEND ( 2U )
/*Terminate state define*/
#define ZNHL_NEARHOMESCREENTERMINATE ( ZNHL_NEARHOMESCREENEND + 1U )

/*State no define*/
#define ZNHL_NEARHOMESCREENS0STATENO ( 0U )
#define ZNHL_NEARHOMESCREENS1STATENO ( 1U )

/*State serial no define*/
#define ZNHL_NEARHOMESCREENS0STATESERIALNO ( 0U )
#define ZNHL_NEARHOMESCREENS1STATESERIALNO ( 1U )

/*Event no define*/
#define ZNHL_NEARHOMESCREENE0EVENTNO ( 0U )
#define ZNHL_NEARHOMESCREENE1EVENTNO ( 1U )
#define ZNHL_NEARHOMESCREENE2EVENTNO ( 2U )
#define ZNHL_NEARHOMESCREENE3EVENTNO ( 3U )
#define ZNHL_NEARHOMESCREENE4EVENTNO ( 4U )
#define ZNHL_NEARHOMESCREENE5EVENTNO ( 5U )
#define ZNHL_NEARHOMESCREENE6EVENTNO ( 6U )
#define ZNHL_NEARHOMESCREENE7EVENTNO ( 7U )

/*Event serial no define*/
#define ZNHL_NEARHOMESCREENE0EVENTSERIALNO ( 0U )
#define ZNHL_NEARHOMESCREENE1EVENTSERIALNO ( 1U )
#define ZNHL_NEARHOMESCREENE2EVENTSERIALNO ( 2U )
#define ZNHL_NEARHOMESCREENE3EVENTSERIALNO ( 3U )
#define ZNHL_NEARHOMESCREENE4EVENTSERIALNO ( 4U )
#define ZNHL_NEARHOMESCREENE5EVENTSERIALNO ( 5U )
#define ZNHL_NEARHOMESCREENE6EVENTSERIALNO ( 6U )
#define ZNHL_NEARHOMESCREENE7EVENTSERIALNO ( 7U )

/*Extern function*/
extern void stm_nhl_event_call( void );
extern void stm_nhl_initialize( void );
extern void ZNHL_NearHomescreenTerminate( void );

#endif
