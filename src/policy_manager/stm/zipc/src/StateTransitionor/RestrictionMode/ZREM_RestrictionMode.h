/************************************************************/
/*     ZREM_RestrictionMode.h                               */
/*     RestrictionMode State transition model header file   */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#ifndef ZHEADER_ZREM_RESTRICTIONMODE_H
#define ZHEADER_ZREM_RESTRICTIONMODE_H

/*State management variable access define*/
#define ZREM_RESTRICTIONMODE ( 0U )
#define ZREM_RESTRICTIONMODES0 ( 0U )
#define ZREM_RESTRICTIONMODES1 ( 1U )
#define ZREM_RESTRICTIONMODES2 ( 2U )
#define ZREM_RESTRICTIONMODESTATENOMAX ( 1U )

/*End state define*/
#define ZREM_RESTRICTIONMODEEND ( 3U )
/*Terminate state define*/
#define ZREM_RESTRICTIONMODETERMINATE ( ZREM_RESTRICTIONMODEEND + 1U )

/*State no define*/
#define ZREM_RESTRICTIONMODES0STATENO ( 0U )
#define ZREM_RESTRICTIONMODES1STATENO ( 1U )
#define ZREM_RESTRICTIONMODES2STATENO ( 2U )

/*State serial no define*/
#define ZREM_RESTRICTIONMODES0STATESERIALNO ( 0U )
#define ZREM_RESTRICTIONMODES1STATESERIALNO ( 1U )
#define ZREM_RESTRICTIONMODES2STATESERIALNO ( 2U )

/*Event no define*/
#define ZREM_RESTRICTIONMODEE0EVENTNO ( 0U )
#define ZREM_RESTRICTIONMODEE1EVENTNO ( 1U )
#define ZREM_RESTRICTIONMODEE2EVENTNO ( 2U )

/*Event serial no define*/
#define ZREM_RESTRICTIONMODEE0EVENTSERIALNO ( 0U )
#define ZREM_RESTRICTIONMODEE1EVENTSERIALNO ( 1U )
#define ZREM_RESTRICTIONMODEE2EVENTSERIALNO ( 2U )

/*Extern function*/
extern void stm_rem_event_call( void );
extern void stm_rem_initialize( void );
extern void ZREM_RestrictionModeTerminate( void );

#endif
