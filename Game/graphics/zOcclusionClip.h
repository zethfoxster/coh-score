#ifndef _ZOCCLUSIONCLIP_H_
#define _ZOCCLUSIONCLIP_H_

#include "zOcclusionTypes.h"
#include "SuperAssert.h"

void drawZTriangleClip(ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2, int clip_bit);

extern float zo_near_plane;
extern float zo_jitter_amount;

#define CLIP_EPSILON (1E-5)

enum {CLIP_MINX = 1, CLIP_MAXX = 1<<1, CLIP_MINY = 1<<2, CLIP_MAXY = 1<<3, CLIP_MINZ = 1<<4, CLIP_MAXZ = 1<<5};

static INLINEDBG int getClipCode(Vec4 v)
{
	float w;

	w = v[3] * (1.0 + CLIP_EPSILON);
	return	 (v[0] < -w) |
			((v[0] > w) << 1) |
			((v[1] < -w) << 2) |
			((v[1] > w) << 3) |
			((v[2] < -w) << 4) | 
			((v[2] > w) << 5) ;
}

static INLINEDBG void transformPoint2(ZBufferPoint *transformed)
{
	transformed->clipcode = getClipCode(transformed->hClipPos);

	if (!transformed->clipcode)
	{
		F32 scale;
		F32 x, y;
#if ZO_SAFE
		assert(transformed->hClipPos[3] != 0);
#endif

		scale = 1.f / transformed->hClipPos[3];
		x = 0.5f + transformed->hClipPos[0] * scale * 0.5f;
		y = 0.5f + transformed->hClipPos[1] * scale * 0.5f;

		transformed->x = round(x * ZB_DIM_MINUS_ONE);
		transformed->y = round(y * ZB_DIM_MINUS_ONE);
		transformed->z = transformed->hClipPos[2] * scale * ZMAX;

#if 1
		if (transformed->x >= ZB_DIM)
			transformed->x = ZB_DIM-1;
		if (transformed->y >= ZB_DIM)
			transformed->y = ZB_DIM-1;
#endif

#if ZO_SAFE
		assert(transformed->x >= 0);
		assert(transformed->x < ZB_DIM);
		assert(transformed->y >= 0);
		assert(transformed->y < ZB_DIM);
#endif
	}
}

static INLINEDBG void transformPoint(ZBufferPoint *transformed, Vec3 point, Mat44 toClipMat)
{
	float jitter;
	mulVecMat44(point, toClipMat, transformed->hClipPos);
	jitter = transformed->hClipPos[2];

	// HYPNOS_AKI: fabsf should be done on the FPU and should be faster than the if()
#if 0
	if (jitter < 0)
		jitter = (20000.f + jitter) * 0.00005f;
	else
		jitter = (20000.f - jitter) * 0.00005f;
#else
	jitter = 1.0f - (fabsf(jitter) * 0.00005f);
#endif

	jitter = zo_jitter_amount * jitter * jitter * 1.1f;
	transformed->hClipPos[0] += jitter;
	transformed->hClipPos[1] += jitter;
	transformPoint2(transformed);
}


#endif//_ZOCCLUSIONCLIP_H_
