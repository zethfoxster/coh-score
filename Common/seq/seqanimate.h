#ifndef _SEQANIMATE_H
#define _SEQANIMATE_H

typedef struct SeqInst SeqInst;

void animExpand8ByteQuat( S16 * compressed, Quat expanded );
void animPlayInst( SeqInst * seq );

void doHeadTurn(SeqInst * seq);
void calcHeadTurn(SeqInst * seq);

#endif