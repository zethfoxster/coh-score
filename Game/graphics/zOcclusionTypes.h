#ifndef _ZOCCLUSIONTYPES_H_
#define _ZOCCLUSIONTYPES_H_

#include "stdtypes.h"

#define ZO_SAFE				0

#if 0

#define ZB_HALF_DIM 64
#define ZB_DIM 128
#define ZB_DIM_MINUS_ONE 127
#define ZB_ONE_OVER_DIM 0.0078125f
// use 8x8 - 32x32
#define ZO_START_HOFFSET 3
#define ZO_END_HOFFSET 2

#define ZO_USE_JITTER 1

#elif 1

#define ZB_HALF_DIM 128
#define ZB_DIM 256
#define ZB_DIM_MINUS_ONE 255
#define ZB_ONE_OVER_DIM 0.00390625f
// use 8x8 - 32x32
#define ZO_START_HOFFSET 3
#define ZO_END_HOFFSET 3

#define ZO_USE_JITTER 1

#else

#define ZB_HALF_DIM 256
#define ZB_DIM 512
#define ZB_DIM_MINUS_ONE 511
#define ZB_ONE_OVER_DIM 0.001953125
// use 8x8 - 32x32
#define ZO_START_HOFFSET 3
#define ZO_END_HOFFSET 4

#define ZO_USE_JITTER 0

#endif



#define ZMAX 1.0f

#define ZINIT ZMAX
#define ZTEST(newZ, existingZ) ((newZ) <= (existingZ))

#define ZTOR(val)		(val>=0.9?2550*val-2295:0)
#define ZTOG(val)		(0)
#define ZTOB(val)		(0)
#define ZTOALPHA(val)	(val < ZMAX ? 255 : 0)

#define MAX_OCCLUDERS 200
#define MAX_OCCLUDER_TRIS 2000
#define MAX_OCCLUDER_VERTS 8000
#define MIN_OCCLUDER_SCREEN_SPACE 0.0

// the smaller the number, the more rejections will happen
// the larger the number, the fewer false rejections will happen
#define TEST_Z_OFFSET (1E-5)

////////////////////////////////////////////////////
////////////////////////////////////////////////////

typedef float ZType;

typedef struct ZBufferPoint
{
	U8 cached;
	U8 clipcode;
	S16 x,y;     // integer coordinates in the zbuffer
	ZType z;
	Vec4 hClipPos;
} ZBufferPoint, *ZBufferPointPtr;

typedef struct Occluder
{
	float screenSpace;
	Model *m;
	Mat4 modelToWorld;
	int uid;
} Occluder, *OccluderPtr;

typedef struct HierarchicalZBuffer
{
	int dim;
	ZType *minZ;
	ZType *maxZ;
} HierarchicalZBuffer;

#endif//_ZOCCLUSIONTYPES_H_

