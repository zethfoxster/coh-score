#ifndef _SEQSKELETON_H
#define _SEQSKELETON_H

#include "seq.h"

void animSetHeader(SeqInst *seq, int preserveOldAnimation );
int animCheckForLoadingObjects( GfxNode * node, int seqHandle );
void animCalcObjAndBoneUse( GfxNode * pNode, int seqHandle );


#endif
