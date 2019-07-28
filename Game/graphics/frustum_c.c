#include <assert.h>
#include "frustum_c.h"

#include "mathutil.h"

// build a view frustum based on the supplied arguments
// this function builds the frustum planes as follows:
//  * in 'view space' i.e. for camera at origin
//  * FOV should be supplied in radians
//  * plane normals face inward
//  * plane normals are normalized
//  * horizontal field of view is supplied
//  * aspect ratio is height/width
//	* OpenGL camera convention, right handed view looking down the -Z axis 
//  * z_near_abs, z_far_abs are distance quantities from frustum origin
//	  to which we apply the view convention (e.g., -z view direction)
void frustum_build( GfxFrustum* frustum, float fov_x, float aspect, float z_near_abs, float z_far_abs )
{
	float focal = 1.0f/ftan(fov_x*0.5f);
	float norm1 = 1.0f/fsqrt( focal*focal + 1.0f );
	float norm2 = 1.0f/fsqrt( focal*focal + aspect*aspect );

	// we can write the plane equations down directly for camera space

	// OpenGL camera convention, right handed view looking down the -Z axis
	float u			= focal*norm1;
	float v			= -norm1;
	float s			= focal*norm2;
	float t			= -aspect*norm2;

	setVec4( frustum->planes[FRUST_PLANE_NEAR].m128_f32,  0.0f, 0.0f, -1.0f, -z_near_abs );
	setVec4( frustum->planes[FRUST_PLANE_FAR].m128_f32,   0.0f, 0.0f,  1.0f,  z_far_abs  );
	setVec4( frustum->planes[FRUST_PLANE_LEFT].m128_f32,     u, 0.0f,     v,  0.0f   );
	setVec4( frustum->planes[FRUST_PLANE_RIGHT].m128_f32,   -u, 0.0f,     v,  0.0f   );
	setVec4( frustum->planes[FRUST_PLANE_BOTTOM].m128_f32, 0.0f,   s,     t,  0.0f   );
	setVec4( frustum->planes[FRUST_PLANE_TOP].m128_f32,    0.0f,  -s,     t,  0.0f   );

}

/*
 Capsule Visibility

 This test will test a capsule (i.e. swept sphere) against the frustum.
 This is useful for testing shadow casters during shadow map rendering.

 * 'direction' must use its magnitude to indicate the length of the capsule segment.
 * We assume normal vectors in plane specs are normalized
 * We assume normal vectors face 'inward' to visible volume
 * @todo In view space a lot of the dot product components from the planes
		are going to be 0. May want to optimize if not using SIMD
 * @todo can do optimization for symmetry and temporal coherence on intersections
*/

static INLINEDBG bool cull_test_plane_capsule( Vec4 plane, Vec3 origin, Vec3 direction, float radius )
{
	// which side of the plane is the origin point on?
	// if one or both endpoints of the segment are on the positive side of the plane
	// then the capsule is not culled
	float d1 = dotVec3( plane, origin ) + plane[3];
	if (d1 < 0.0f)
	{
		// if it is outside the plane then check the other end of the capsule
		float d2 = d1 + dotVec3( plane, direction );
		if (d2 < 0.0f)
		{
			// If that is outside then we need to check the radius of the closest
			// endpoint to the plane.
			// recall these are negative signed distances so the comparison is
			// opposite of what you might intuit
			if (d1 >= d2)
			{
				return d1 <= -radius;
			}
			else
			{
				return d2 <= -radius;
			}
		}
	}
	return false;
}

// This is the easy to understand/debug culling code.
bool frustum_is_capsule_culled_nosse( GfxFrustum* frustum, Vec3 origin, Vec3 direction, float radius )
{
	int i;

	for (i=0; i < 6; ++i)
	{
		if (cull_test_plane_capsule( frustum->planes[i].m128_f32, origin, direction, radius ) )
			return true;
	}
	return false;
}

static INLINEDBG __m128 sse2_dot4(__m128 a, __m128 b)
{
	register __m128 t, u;
	t = _mm_mul_ps(a, b);
	u = _mm_movehl_ps(t, t);
	u = _mm_add_ps(u, t);
	t = _mm_shuffle_ps(u, u, 0x55);
	t = _mm_add_ps(u, t);
	#ifdef _FULLDEBUG
	{
		// compare SSE results with 24 bit fpu precision
		float d;
		unsigned int saved_fpu_control;
		_controlfp_s(&saved_fpu_control, 0, 0);	// retrieve current control word
		_controlfp_s(NULL, _PC_24, _MCW_PC);	// set the fpu to 24 bit precision
		d = dotVec4(a.m128_f32, b.m128_f32);
		_controlfp_s(NULL, saved_fpu_control, _MCW_PC);	// restore fpu precision
		assert(t.m128_f32[0] == d);
	}
	#endif
	return t;
}

bool frustum_is_capsule_culled_sse2( GfxFrustum* frustum, Vec3 origin, Vec3 direction, float radius )
{
	unsigned int i;

	register __m128 zero = _mm_setzero_ps();
	register __m128 nrad = _mm_set_ss(-radius);
	register __m128 pos = _mm_set_ps(1.0f, origin[2], origin[1], origin[0]);
	register __m128 dir = _mm_set_ps(0.0f, direction[2], direction[1], direction[0]);

	for (i=0; i < 6; ++i)
	{		
		register __m128 d1;
		register __m128 d2;

		register __m128 plane = _mm_load_ps(frustum->planes[i].m128_f32);
 
		d1 = sse2_dot4(plane, pos); // plane . pos
		if(_mm_comige_ss(d1, zero)) { // d1 >= zero
			__nop();
		} else {
			d2 = sse2_dot4(plane, dir); // plane . dir
			d2 = _mm_add_ps(d1, d2); // d1 + d2
			if(_mm_comige_ss(d2, zero)) { // d2 >= zero
				__nop();
			} else {
				if(_mm_comige_ss(d1, d2)) { // d1 >= d2
					if(_mm_comile_ss(d1, nrad)) { // d1 <= -radius
						#ifdef _FULLDEBUG
						assert(frustum_is_capsule_culled_nosse(frustum, origin, direction, radius) == true);
						#endif
						return true;
					}
				} else {
					if(_mm_comile_ss(d2, nrad)) { // d1 <= -radius
						#ifdef _FULLDEBUG
						assert(frustum_is_capsule_culled_nosse(frustum, origin, direction, radius) == true);
						#endif
						return true;
					}
				}
			}
		}
	}

	#ifdef _FULLDEBUG
	assert(frustum_is_capsule_culled_nosse(frustum, origin, direction, radius) == false);
	#endif
	return false;
}

// the general version checks each plane in turn
// @todo use SSE
bool frustum_is_sphere_culled_general( GfxFrustum* frustum, Vec3 origin, float radius ) 
{
	int i;

	for (i=0; i < 6; ++i)
	{
		// If the sphere origin is on the 'outside' of the plane by more than the
		// sphere radius then it is completely out
		float d = dotVec3( frustum->planes[i].m128_f32, origin ) + frustum->planes[i].m128_f32[3];
		if (d < -radius)
			return true;
	}
	return false;
}

// when clipping against view space frustum we can leverage sparse coefficients
// @todo use SSE?
//bool frustum_is_sphere_culled_vieww( GfxFrustum* frustum, Vec3 origin, float radius ) 
