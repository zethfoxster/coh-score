#ifndef _GFX_FRUSTUM_C_H
#define _GFX_FRUSTUM_C_H

#include "stdtypes.h"
#include "ssemath.h"

enum
{
	FRUST_PLANE_NEAR,
	FRUST_PLANE_FAR,
	FRUST_PLANE_LEFT,
	FRUST_PLANE_RIGHT,
	FRUST_PLANE_TOP,
	FRUST_PLANE_BOTTOM,
	FRUST_PLANE_COUNT
};

// simple frustum container for culling operations
typedef struct GfxFrustum
{
	sseVec4	planes[FRUST_PLANE_COUNT];
//	float fov_x, fov_y;
//	float z_near, z_far;
} GfxFrustum;

void frustum_build( GfxFrustum* frustum, float fov_x, float aspect, float z_near_abs, float z_far_abs );
bool frustum_is_capsule_culled_nosse( GfxFrustum* frustum, Vec3 origin, Vec3 direction, float radius );
bool frustum_is_capsule_culled_sse2( GfxFrustum* frustum, Vec3 origin, Vec3 direction, float radius );
bool frustum_is_sphere_culled_general( GfxFrustum* frustum, Vec3 origin, float radius );

static bool INLINEDBG frustum_is_capsule_culled( GfxFrustum* frustum, Vec3 origin, Vec3 direction, float radius )
{
	if(sseAvailable & SSE_2) {
		return frustum_is_capsule_culled_sse2(frustum, origin, direction, radius);
	} else {
		return frustum_is_capsule_culled_nosse(frustum, origin, direction, radius);
	}
}


#endif // _GFX_FRUSTUM_C_H
