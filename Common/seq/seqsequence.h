#ifndef _SEQSEQUENCE_H
#define _SEQSEQUENCE_H

#include "stdtypes.h"
#include "seq.h"

#ifdef CLIENT
int seqSetMoveTrigger(FutureMoveTracker fmt_array[], TriggeredMove tm_array[], int * tm_count);
int seqCheckMoveTriggers(FutureMoveTracker fmt_array[] );
#endif

int sparseRequiresMet(const SparseBits *seq_state, const U32 *curr_state);
int requiresMet(U32 *seq_state,U32 *curr_state);
bool seqAInterruptsB( const SeqMove * a, const SeqMove * b );
void seqListAllSetStates( int allstates[MAXSTATES], int * allcnt, U32 * state );
int isThisDebugEnt( const SeqInst * seq ); //this is the wrong file for this, what's the right file?
F32 seqGetAnimScale( const SeqInst * seq );
const SeqMove * seqForceGetMove( SeqInst * seq, U32 state[], U32 requiresThisBit );

#endif
