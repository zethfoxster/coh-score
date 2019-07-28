#ifndef RT_WATER_H
#define RT_WATER_H

#include "stdtypes.h"
#include "rt_pbuffer.h"

typedef struct VBO VBO;
typedef struct RdrTexList RdrTexList;
typedef struct RdrModelStruct RdrModel;
typedef struct ViewportInfo ViewportInfo;

typedef struct RdrFrameGrabParams
{
	int allocated_handles;
	int handle[MAX_SLI];
	int allocated_depth_handles;
	int depth_handle[MAX_SLI];
	int texture_width;
	int texture_height;
	int screen_width;
	int screen_height;
	F32 alpha;
	U32 noAlphaBlend:1;
	U32 copySubImage:1;
	U32 floatBuffer:1;
	U32 showDepth:1;
} RdrFrameGrabParams;

typedef struct RdrWaterParams
{
	int allocated_handles;
	int handle[MAX_SLI];
	int allocated_depth_handles;
	int depth_handle[MAX_SLI];
	PBuffer * water_reflection_pbuffer;
	PBuffer * generic_reflection_pbuffer;
	int refraction_texture_width;
	int refraction_texture_height;
	int screen_width;
	int screen_height;
} RdrWaterParams;

typedef struct RdrSetReflectionPlaneParams
{
	Vec4 generic_reflection_plane;
	Vec4 water_reflection_plane;
	bool generic_reflection_active;
	bool water_reflection_active;
} RdrSetReflectionPlaneParams;

#ifdef RT_PRIVATE
void rdrFrameGrabDirect(RdrFrameGrabParams *params);
void rdrFrameGrabShowDirect(RdrFrameGrabParams *params);
void rdrWaterSetParamsDirect(RdrWaterParams *params);
void rdrSetReflectionPlaneDirect(RdrSetReflectionPlaneParams *params);

void modelDrawWater( RdrModel *draw, U8 * rgbs, RdrTexList *texlist);
void rdrDebugCommandsDirect(void *data);
const Vec4 *rdrGetWaterReflectionPlane();

int  rdrGetBuildingReflectionTextureHandle();
const Vec4 *rdrGetBuildingReflectionPlane();
#endif

#endif
