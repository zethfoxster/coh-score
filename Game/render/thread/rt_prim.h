#ifndef _RENDERDISPATCH_H
#define _RENDERDISPATCH_H

#include "rt_queue.h"

void rdrSetFogDirect(RdrFog *vals);
void rdrSetFogDirect2(F32 near_dist, F32 far_dist, Vec3 color);
void rdrSetBgColorDirect(Vec3 vals);
void winMakeCurrentDirect();

void drawLine2DDirect(RdrLineBox *line);
void drawLine3DDirect(RdrLineBox *line);
void drawFilledBoxDirect(RdrLineBox *box);
void drawTriangle3DDirect(RdrLineBox *tri);

void rdrGetFrameBufferDirect(RdrGetFrameBuffer *get);

void rdrSetStencilDirect(RdrStencil *rs);
void rdrSetClipPlanesDirect(RdrClipPlane *rcp);
void rdrDynamicQuad(RdrDynamicQuad* d);
#endif
