/************************************************************/
/*     ZREL_RelMain.h                                       */
/*     RelMain State transition model header file           */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#ifndef ZHEADER_ZREL_RELMAIN_H
#define ZHEADER_ZREL_RELMAIN_H

/*State management variable access define*/
#define ZREL_RELMAIN ( 0U )
#define ZREL_RELMAINS0F ( 1U )
#define ZREL_RELMAINS1F ( 2U )
#define ZREL_RELMAINS0 ( 0U )
#define ZREL_RELRESTOFFS0 ( 0U )
#define ZREL_RELRESTOFFS1 ( 1U )
#define ZREL_RELRESTOFFS2 ( 2U )
#define ZREL_RELRESTOFFS3 ( 3U )
#define ZREL_RELMAINS1 ( 1U )
#define ZREL_RELRESTONS0 ( 0U )
#define ZREL_RELMAINSTATENOMAX ( 3U )

/*End state define*/
#define ZREL_RELMAINEND ( 4U )
/*Terminate state define*/
#define ZREL_RELMAINTERMINATE ( ZREL_RELMAINEND + 1U )

/*State no define*/
#define ZREL_RELMAINS0STATENO ( 0U )
#define ZREL_RELRESTOFFS0STATENO ( 0U )
#define ZREL_RELRESTOFFS1STATENO ( 1U )
#define ZREL_RELRESTOFFS2STATENO ( 2U )
#define ZREL_RELRESTOFFS3STATENO ( 3U )
#define ZREL_RELMAINS1STATENO ( 1U )
#define ZREL_RELRESTONS0STATENO ( 0U )

/*State serial no define*/
#define ZREL_RELMAINS0STATESERIALNO ( 0U )
#define ZREL_RELRESTOFFS0STATESERIALNO ( 1U )
#define ZREL_RELRESTOFFS1STATESERIALNO ( 2U )
#define ZREL_RELRESTOFFS2STATESERIALNO ( 3U )
#define ZREL_RELRESTOFFS3STATESERIALNO ( 4U )
#define ZREL_RELMAINS1STATESERIALNO ( 5U )
#define ZREL_RELRESTONS0STATESERIALNO ( 6U )

/*Event no define*/
#define ZREL_RELMAINE0EVENTNO ( 0U )
#define ZREL_RELMAINE1EVENTNO ( 1U )
#define ZREL_RELRESTOFFE0EVENTNO ( 0U )
#define ZREL_RELRESTOFFE1EVENTNO ( 1U )
#define ZREL_RELRESTOFFE2EVENTNO ( 2U )
#define ZREL_RELRESTOFFE3EVENTNO ( 3U )
#define ZREL_RELRESTOFFE4EVENTNO ( 4U )
#define ZREL_RELRESTOFFE5EVENTNO ( 5U )
#define ZREL_RELRESTOFFE6EVENTNO ( 6U )
#define ZREL_RELRESTOFFE7EVENTNO ( 7U )
#define ZREL_RELRESTOFFE8EVENTNO ( 8U )
#define ZREL_RELRESTOFFE9EVENTNO ( 9U )
#define ZREL_RELRESTOFFE10EVENTNO ( 10U )
#define ZREL_RELRESTOFFE11EVENTNO ( 11U )
#define ZREL_RELRESTOFFE12EVENTNO ( 12U )
#define ZREL_RELRESTOFFE13EVENTNO ( 13U )
#define ZREL_RELRESTOFFE14EVENTNO ( 14U )
#define ZREL_RELRESTOFFE15EVENTNO ( 15U )
#define ZREL_RELRESTOFFE16EVENTNO ( 16U )
#define ZREL_RELRESTONE0EVENTNO ( 0U )

/*Event serial no define*/
#define ZREL_RELMAINE0EVENTSERIALNO ( 0U )
#define ZREL_RELMAINE1EVENTSERIALNO ( 1U )
#define ZREL_RELRESTOFFE0EVENTNO ( 0U )
#define ZREL_RELRESTOFFE1EVENTNO ( 1U )
#define ZREL_RELRESTOFFE2EVENTNO ( 2U )
#define ZREL_RELRESTOFFE3EVENTNO ( 3U )
#define ZREL_RELRESTOFFE4EVENTNO ( 4U )
#define ZREL_RELRESTOFFE5EVENTNO ( 5U )
#define ZREL_RELRESTOFFE6EVENTNO ( 6U )
#define ZREL_RELRESTOFFE7EVENTNO ( 7U )
#define ZREL_RELRESTOFFE8EVENTNO ( 8U )
#define ZREL_RELRESTOFFE9EVENTNO ( 9U )
#define ZREL_RELRESTOFFE10EVENTNO ( 10U )
#define ZREL_RELRESTOFFE11EVENTNO ( 11U )
#define ZREL_RELRESTOFFE12EVENTNO ( 12U )
#define ZREL_RELRESTOFFE13EVENTNO ( 13U )
#define ZREL_RELRESTOFFE14EVENTNO ( 14U )
#define ZREL_RELRESTOFFE15EVENTNO ( 15U )
#define ZREL_RELRESTOFFE16EVENTNO ( 16U )
#define ZREL_RELRESTONE0EVENTNO ( 0U )

/*Extern function*/
extern void stm_rel_event_call( void );
extern void stm_rel_initialize( void );
extern void ZREL_RelMainTerminate( void );

#endif
