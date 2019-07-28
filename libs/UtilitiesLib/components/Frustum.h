#ifndef _FRUSTUM_H_
#define _FRUSTUM_H_

#include "stdtypes.h"
#include "mathutil.h"
#include "fastmath.h"
#include "GenericPoly.h"

enum
{
	FRUSTUM_CLIP_TOP =		(1 << 0),
	FRUSTUM_CLIP_BOTTOM =	(1 << 1),
	FRUSTUM_CLIP_LEFT =		(1 << 2),
	FRUSTUM_CLIP_RIGHT =	(1 << 3),
	FRUSTUM_CLIP_NEAR =		(1 << 4),
	FRUSTUM_CLIP_FAR =		(1 << 5),
	FRUSTUM_CLIP_NONE =		(1 << 6),
};

typedef struct Frustum
{
	F32	znear,zfar;
	F32 hvam,vvam;
	F32 hcos,vcos;
	F32 fovy, fovx;
	ConvexHull hull; // in cameraspace
	Mat4 viewmat, inv_viewmat;
	U32 use_hull:1;
	Vec3 world_min, world_max; // bounds of stuff actually drawn, update by calling frustumUpdateBounds
} Frustum;


static INLINEDBG int frustumCheckSphere(const Frustum *w, const Vec3 pos_cameraspace, F32 rad)
{
	F32			cx,cy,cz,dist,hplane,vplane;
	int			clip = FRUSTUM_CLIP_NONE;

	if (w->use_hull)
		return hullIsSphereInside(&w->hull, pos_cameraspace, rad) ? FRUSTUM_CLIP_NONE : 0;

	// near plane
	cz = -pos_cameraspace[2];
	if ((cz+rad) <= -w->znear )	 	/* behind camera		*/
		return 0;
	else if (cz-rad < -w->znear)
		clip |= FRUSTUM_CLIP_NEAR;

	// far plane
	if ((cz-rad) >= -w->zfar )	 	/* too far		*/
		return 0;

	hplane = cz * w->hvam;
	cx = pos_cameraspace[0];
	// Left edge
	dist = (cx - (-hplane)) * w->hcos;
	if (-dist >= rad)					/* Left of view volume */
		return 0;
	else if (rad > dist)
		clip |= FRUSTUM_CLIP_LEFT;

	// Right edge
	dist = (hplane - cx) * w->hcos;
	if (-dist >= rad)
		return 0;
	else if (rad > dist)
		clip |= FRUSTUM_CLIP_RIGHT;

	vplane = cz * w->vvam;
	cy = pos_cameraspace[1];
	// Bottom edge
	dist = (cy - (-vplane)) * w->vcos;
	if (-dist >= rad)
		return 0;
	else if (rad > dist)
		clip |= FRUSTUM_CLIP_BOTTOM;

	// top edge
	dist = (vplane - cy) * w->vcos;
	if (-dist >= rad)
		return 0;
	else if (rad > dist)
		clip |= FRUSTUM_CLIP_TOP;

	return clip;
}

static INLINEDBG int frustumCheckSphereWorld(const Frustum *w, const Vec3 pos_worldspace, F32 rad)
{
	Vec3 pos_cameraspace;
	mulVecMat4(pos_worldspace, w->viewmat, pos_cameraspace);
	return frustumCheckSphere(w, pos_cameraspace, rad);
}

static INLINEDBG int frustumCheckPoint(const Frustum *w, const Vec3 pos_cameraspace)
{
	F32			cx,cy,cz,dist,hplane,vplane;
	int			clip = 0;

	if (w->use_hull)
		return hullIsPointInside(&w->hull, pos_cameraspace) ? 0 : FRUSTUM_CLIP_NEAR;

	cz = -pos_cameraspace[2];
	if ( cz <= -w->znear )
		clip |= FRUSTUM_CLIP_NEAR;

	if ( cz >= -w->zfar )
		clip |= FRUSTUM_CLIP_FAR;

	hplane = cz * w->hvam;
	cx = pos_cameraspace[0];
	dist = (cx - (-hplane)) * w->hcos;
	if (dist < 0)	
		clip |= FRUSTUM_CLIP_LEFT;

	dist = (hplane - cx) * w->hcos;
	if (dist < 0)
		clip |= FRUSTUM_CLIP_RIGHT;

	vplane = cz * w->vvam;
	cy = pos_cameraspace[1];
	dist = (cy - (-vplane)) * w->vcos;
	if (dist< 0)
		clip |= FRUSTUM_CLIP_BOTTOM;

	dist = (vplane - cy) * w->vcos;
	if (dist < 0)
		clip |= FRUSTUM_CLIP_TOP;

	return clip;
}

static INLINEDBG int frustumCheckPointWorld(const Frustum *w, const Vec3 pos_worldspace)
{
	Vec3 pos_cameraspace;
	mulVecMat4(pos_worldspace, w->viewmat, pos_cameraspace);
	return frustumCheckPoint(w, pos_cameraspace);
}

static INLINEDBG int frustumCheckBoxNearClipped(const Frustum *w, const Vec3 bounds_cameraspace[8])
{
	int		idx;
	int		clipped = 0;

	for (idx=0;idx<8;idx++)
	{
		if (-bounds_cameraspace[idx][2] <= -w->znear)
			clipped++;
	}

	if (clipped == 8)
		return 2;
	if (clipped)
		return 1;
	return 0;
}

static INLINEDBG int frustumCheckBoxWorld(const Frustum *w, int clip, const Vec3 min, const Vec3 max, const Mat4 local_to_world_mat)
{
	int		x, y, z;
	Vec3	extents[2];
	Mat4	local_to_camera_mat;

	if (local_to_world_mat)
		mulMat4Inline(w->viewmat, local_to_world_mat, local_to_camera_mat);
	else
		copyMat4(w->viewmat, local_to_camera_mat);

	copyVec3(min,extents[0]);
	copyVec3(max,extents[1]);

	for(x=0;x<2;x++)
	{
		for(y=0;y<2;y++)
		{
			for(z=0;z<2;z++)
			{
				Vec3	pos;
				Vec3	tpos;

				pos[0] = extents[x][0];
				pos[1] = extents[y][1];
				pos[2] = extents[z][2];
				mulVecMat4(pos,local_to_camera_mat,tpos);
				clip &= frustumCheckPoint(w, tpos);
				if (!clip)
					return 1;
			}
		}
	}
	return 0;
}

static INLINEDBG void frustumResetBounds(Frustum *w)
{
	setVec3(w->world_min, 8e16, 8e16, 8e16);
	setVec3(w->world_max, -8e16, -8e16, -8e16);
}
static INLINEDBG void frustumUpdateBounds(Frustum *w, const Vec3 world_min, const Vec3 world_max)
{
	vec3RunningMin(world_min, w->world_min);
	vec3RunningMax(world_max, w->world_max);
}

void frustumGetBounds(const Frustum *w, Vec3 min, Vec3 max);

void frustumSet(Frustum *w, F32 fovy, F32 fovx, F32 znear, F32 zfar);
void frustumGetScreenPosition(const Frustum *w, int screen_width, int screen_height, const Vec3 pos_cameraspace, Vec2 screen_pos);
void frustumGetWorldRay(const Frustum *w, int screen_width, int screen_height, const Vec2 screen_pos, F32 len, Vec3 start, Vec3 end);

#endif //_FRUSTUM_H_
