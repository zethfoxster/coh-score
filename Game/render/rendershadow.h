#ifndef _RENDERSHADOW_H
#define _RENDERSHADOW_H

#include "model.h"
#include "gfxtree.h"
#include "splat.h"
#include "animtrackanimate.h"

#include "rt_shadow.h"

// stencil shadows
int shadowVolumeVisible(Vec3 mid,F32 radius,F32 shadow_dist);
void shadowStartScene(void);
void shadowFinishScene(void);
void modelDrawShadowVolume(Model *model,Mat4 mat,int alpha,int shadow_mask, GfxNode * node);


// splat shadows
void modelDrawShadowObject( Mat4 viewspace, Splat * splat );

#endif
