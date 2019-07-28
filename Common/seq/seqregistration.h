#ifndef SEQ_REGISTRATION_H__
#define SEQ_REGISTRATION_H__

typedef struct GfxNode GfxNode;
typedef struct SeqInst SeqInst;

void scaleBone(GfxNode * node, F32 x, F32 y, F32 z);
void unscaleBone(GfxNode * node, F32 x, F32 y, F32 z);

void adjustLimbs(SeqInst *seq);

#endif
