/************************************************************/
/*     ZST_StateTransitionor_func.h                         */
/*     Function and variable header file                    */
/*     ZIPC Designer Version 1.2.0                          */
/************************************************************/
#ifndef ZHEADER_ZST_STATETRANSITIONOR_FUNC_H
#define ZHEADER_ZST_STATETRANSITIONOR_FUNC_H

extern void stmInitializeInner();
extern int stmTransitionStateInner(int event_id, StmState* state);
extern void stmUndoStateInner();

#endif
