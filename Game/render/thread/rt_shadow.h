#ifndef _RT_SHADOW_H
#define _RT_SHADOW_H

#include "rt_model_cache.h"

typedef struct VBO		VBO;
typedef struct Splat	Splat;

typedef struct
{
	VBO		*vbo;
	Mat4	mat;
	Vec3	offsetx;
	int		alpha;
	int		shadow_mask;
	F32		dist;
} RdrStencilShadow;

#ifdef RT_PRIVATE
typedef struct ShadowInfo ShadowInfo;

void shadowFinishSceneDirect(void);
void modelDrawShadowVolumeDirect(RdrStencilShadow *rs);
void modelDrawShadowObjectDirect( Splat * splat );
void shadowFreeDirect(ShadowInfo *shadow);

void rdrSetupStippleStencilDirect(void);

#endif

#endif
