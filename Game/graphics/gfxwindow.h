#ifndef _GFX_WINDOW_H
#define _GFX_WINDOW_H

#include "stdtypes.h"
#include "mathutil.h"

enum
{
	CLIP_TOP =		(1 << 0),
	CLIP_BOTTOM =	(1 << 1),
	CLIP_LEFT =		(1 << 2),
	CLIP_RIGHT =	(1 << 3),
	CLIP_NEAR =		(1 << 4),
	CLIP_FAR =		(1 << 5),
	CLIP_NONE =		(1 << 6),
};

enum
{
	FRUST_NEAR,
	FRUST_FAR,
	FRUST_LEFT,
	FRUST_RIGHT,
	FRUST_TOP,
	FRUST_BOTTOM,
};

typedef struct
{
	F32	znear,zfar;
	F32 hvam,vvam;
	F32 hcos,vcos;
	F32 fovy, fovx;
	int use_frustum;
	Vec4 frustum[6]; // planes face inward
} GfxWindow;

extern GfxWindow gfx_window;

static INLINEDBG int frustumSphereVisible(Vec3 pos, F32 rad)
{
	int			clip = CLIP_NONE;
	GfxWindow	*w = &gfx_window;
	F32			d;

	d = distanceToPlane(pos, w->frustum[FRUST_NEAR]);
	if (d <= -rad)
		return 0;
	else if (d < rad)
		clip |= CLIP_NEAR;

	d = distanceToPlane(pos, w->frustum[FRUST_FAR]);
	if (d <= -rad)
		return 0;
	else if (d < rad)
		clip |= CLIP_FAR;

	d = distanceToPlane(pos, w->frustum[FRUST_LEFT]);
	if (d <= -rad)
		return 0;
	else if (d < rad)
		clip |= CLIP_LEFT;

	d = distanceToPlane(pos, w->frustum[FRUST_RIGHT]);
	if (d <= -rad)
		return 0;
	else if (d < rad)
		clip |= CLIP_RIGHT;

	d = distanceToPlane(pos, w->frustum[FRUST_TOP]);
	if (d <= -rad)
		return 0;
	else if (d < rad)
		clip |= CLIP_TOP;

	d = distanceToPlane(pos, w->frustum[FRUST_BOTTOM]);
	if (d <= -rad)
		return 0;
	else if (d < rad)
		clip |= CLIP_BOTTOM;

	return clip;
}

static INLINEDBG int frustumPointVisible(Vec3 pos)
{
	int			clip = 0;
	GfxWindow	*w = &gfx_window;
	F32			d;

	d = distanceToPlane(pos, w->frustum[FRUST_NEAR]);
	if (d <= 0)
		clip |= CLIP_NEAR;

	d = distanceToPlane(pos, w->frustum[FRUST_FAR]);
	if (d <= 0)
		clip |= CLIP_FAR;

	d = distanceToPlane(pos, w->frustum[FRUST_LEFT]);
	if (d < 0)
		clip |= CLIP_LEFT;

	d = distanceToPlane(pos, w->frustum[FRUST_RIGHT]);
	if (d < 0)
		clip |= CLIP_RIGHT;

	d = distanceToPlane(pos, w->frustum[FRUST_TOP]);
	if (d < 0)
		clip |= CLIP_TOP;

	d = distanceToPlane(pos, w->frustum[FRUST_BOTTOM]);
	if (d < 0)
		clip |= CLIP_BOTTOM;

	return clip;
}

static INLINEDBG int gfxSphereVisible(Vec3 pos, F32 rad) 
{
	GfxWindow	*w;
	F32			cx,cy,cz,dist,hplane,vplane;
	int			clip = CLIP_NONE;

	w = &gfx_window;

	if (w->use_frustum)
		return frustumSphereVisible(pos, rad);

	// near plane
	cz = -pos[2];
	if ((cz+rad) <= -w->znear )	 	/* behind camera		*/
		return 0;
	else if (cz-rad < -w->znear)
		clip |= CLIP_NEAR;
	
	// far plane
	if ((cz-rad) >= -w->zfar )	 	/* too far		*/
		return 0;
	
	hplane = cz * w->hvam;
	cx = pos[0];
	// Left edge
	dist = (cx - (-hplane)) * w->hcos;
	if (-dist >= rad)					/* Left of view volume */
		return 0;
	else if (rad > dist)
		clip |= CLIP_LEFT;

	// Right edge
	dist = (hplane - cx) * w->hcos;
	if (-dist >= rad)
		return 0;
	else if (rad > dist)
		clip |= CLIP_RIGHT;

	vplane = cz * w->vvam;
	cy = pos[1];
	// Bottom edge
	dist = (cy - (-vplane)) * w->vcos;
	if (-dist >= rad)
		return 0;
	else if (rad > dist)
		clip |= CLIP_BOTTOM;

	// top edge
	dist = (vplane - cy) * w->vcos;
	if (-dist >= rad)
		return 0;
	else if (rad > dist)
		clip |= CLIP_TOP;

	return clip;
}

static INLINEDBG int pointVisible(Vec3 pos) 
{
	GfxWindow	*w;
	F32			cx,cy,cz,dist,hplane,vplane;
	int			clip = 0;

	w = &gfx_window;

	if (w->use_frustum)
		return frustumPointVisible(pos);

	cz = -pos[2];
	if ( cz <= -w->znear )
		clip |= CLIP_NEAR;
	
	if ( cz >= -w->zfar )
		clip |= CLIP_FAR;
	
	hplane = cz * w->hvam;
	cx = pos[0];
	dist = (cx - (-hplane)) * w->hcos;
	if (dist < 0)	
		clip |= CLIP_LEFT;

	dist = (hplane - cx) * w->hcos;
	if (dist < 0)
		clip |= CLIP_RIGHT;

	vplane = cz * w->vvam;
	cy = pos[1];
	dist = (cy - (-vplane)) * w->vcos;
	if (dist< 0)
		clip |= CLIP_BOTTOM;

	dist = (vplane - cy) * w->vcos;
	if (dist < 0)
		clip |= CLIP_TOP;

	return clip;
}

static INLINEDBG int isBoxNearClipped(Vec3 eye_bounds[8])
{
	int		idx;
	int		clipped = 0;

	if (gfx_window.use_frustum)
	{
		for(idx=0;idx<8;idx++)
		{
			if (distanceToPlane(eye_bounds[idx], gfx_window.frustum[FRUST_NEAR]) <= 0)
				clipped++;
		}
	}
	else
	{
		for(idx=0;idx<8;idx++)
		{
			if ( -eye_bounds[idx][2] <= -gfx_window.znear )
				clipped++;
		}
	}

	if (clipped == 8)
		return 2;
	if (clipped)
		return 1;
	return 0;
}

static INLINEDBG int checkBoxFrustum(int clip, Vec3 min, Vec3 max, Mat4 pmat)
{
#if 0
	// Original version
	// 9 multiplies in mulVecMat4 * 8 iterations = 72 multiplies
	int		x, y, z;
	Vec3	extents[2];

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
				mulVecMat4(pos,pmat,tpos);
				clip &= pointVisible(tpos);
				if (!clip)
					return 1;
			}
		}
	}
#elif 0
	// This version incorporates the logic from mulVecMat4() into the loop
	// 6 multiplies + 12 multiplies + 24 multiplies = 42 multiplies for replacement mulVecMat4 logic
	int		x, y, z;
	Vec3	extents[2];
	Vec3	tpos;
	Vec3	vTemp0;
	Vec3	vTemp1;
	F32		temp;

	copyVec3(min,extents[0]);
	copyVec3(max,extents[1]);

	for(x=0;x<2;x++)
	{
		temp = extents[x][0];
		vTemp0[0] = temp*pmat[0][0] + pmat[3][0];
		vTemp0[1] = temp*pmat[0][1] + pmat[3][1];
		vTemp0[2] = temp*pmat[0][2] + pmat[3][2];

		for(y=0;y<2;y++)
		{
			temp = extents[y][1];
			vTemp1[0] = vTemp0[0] + temp*pmat[1][0];
			vTemp1[1] = vTemp0[1] + temp*pmat[1][1];
			vTemp1[2] = vTemp0[2] + temp*pmat[1][2];

			for(z=0;z<2;z++)
			{
				temp = extents[z][2];
				tpos[0] = vTemp1[0] + temp*pmat[2][0];
				tpos[1] = vTemp1[1] + temp*pmat[2][1];
				tpos[2] = vTemp1[2] + temp*pmat[2][2];

				clip &= pointVisible(tpos);
				if (!clip)
					return 1;

			}
		}
	}
#else
	// This version incorporates the logic from mulVecMat4() into the loop
	// and unrolls the loop and precalculates the multiplies
	// TODO: This screams for SSE optimizations
	Vec3	tpos;
	F32		temp;
	Vec3	vTempMinX, vTempMinY, vTempMinZ;
	Vec3	vTempMaxX, vTempMaxY, vTempMaxZ;

	// precalculate multiplies
	// 12 multiplies for replacement mulVecMat4 logic
	temp = min[0];
	vTempMinX[0] = temp*pmat[0][0] + pmat[3][0];
	vTempMinX[1] = temp*pmat[0][1] + pmat[3][1];
	vTempMinX[2] = temp*pmat[0][2] + pmat[3][2];

	temp = min[1];
	vTempMinY[0] = temp*pmat[1][0];
	vTempMinY[1] = temp*pmat[1][1];
	vTempMinY[2] = temp*pmat[1][2];

	temp = min[2];
	vTempMinZ[0] = temp*pmat[2][0];
	vTempMinZ[1] = temp*pmat[2][1];
	vTempMinZ[2] = temp*pmat[2][2];

	temp = max[0];
	vTempMaxX[0] = temp*pmat[0][0] + pmat[3][0];
	vTempMaxX[1] = temp*pmat[0][1] + pmat[3][1];
	vTempMaxX[2] = temp*pmat[0][2] + pmat[3][2];

	temp = max[1];
	vTempMaxY[0] = temp*pmat[1][0];
	vTempMaxY[1] = temp*pmat[1][1];
	vTempMaxY[2] = temp*pmat[1][2];

	temp = max[2];
	vTempMaxZ[0] = temp*pmat[2][0];
	vTempMaxZ[1] = temp*pmat[2][1];
	vTempMaxZ[2] = temp*pmat[2][2];

	// Do adds to get each corner
	tpos[0] = vTempMinX[0] + vTempMinY[0] + vTempMinZ[0];
	tpos[1] = vTempMinX[1] + vTempMinY[1] + vTempMinZ[1];
	tpos[2] = vTempMinX[2] + vTempMinY[2] + vTempMinZ[2];
	clip &= pointVisible(tpos);
	if (!clip)
		return 1;

	tpos[0] = vTempMinX[0] + vTempMinY[0] + vTempMaxZ[0];
	tpos[1] = vTempMinX[1] + vTempMinY[1] + vTempMaxZ[1];
	tpos[2] = vTempMinX[2] + vTempMinY[2] + vTempMaxZ[2];
	clip &= pointVisible(tpos);
	if (!clip)
		return 1;

	tpos[0] = vTempMinX[0] + vTempMaxY[0] + vTempMinZ[0];
	tpos[1] = vTempMinX[1] + vTempMaxY[1] + vTempMinZ[1];
	tpos[2] = vTempMinX[2] + vTempMaxY[2] + vTempMinZ[2];
	clip &= pointVisible(tpos);
	if (!clip)
		return 1;

	tpos[0] = vTempMinX[0] + vTempMaxY[0] + vTempMaxZ[0];
	tpos[1] = vTempMinX[1] + vTempMaxY[1] + vTempMaxZ[1];
	tpos[2] = vTempMinX[2] + vTempMaxY[2] + vTempMaxZ[2];
	clip &= pointVisible(tpos);
	if (!clip)
		return 1;

	tpos[0] = vTempMaxX[0] + vTempMinY[0] + vTempMinZ[0];
	tpos[1] = vTempMaxX[1] + vTempMinY[1] + vTempMinZ[1];
	tpos[2] = vTempMaxX[2] + vTempMinY[2] + vTempMinZ[2];
	clip &= pointVisible(tpos);
	if (!clip)
		return 1;

	tpos[0] = vTempMaxX[0] + vTempMinY[0] + vTempMaxZ[0];
	tpos[1] = vTempMaxX[1] + vTempMinY[1] + vTempMaxZ[1];
	tpos[2] = vTempMaxX[2] + vTempMinY[2] + vTempMaxZ[2];
	clip &= pointVisible(tpos);
	if (!clip)
		return 1;

	tpos[0] = vTempMaxX[0] + vTempMaxY[0] + vTempMinZ[0];
	tpos[1] = vTempMaxX[1] + vTempMaxY[1] + vTempMinZ[1];
	tpos[2] = vTempMaxX[2] + vTempMaxY[2] + vTempMinZ[2];
	clip &= pointVisible(tpos);
	if (!clip)
		return 1;

	tpos[0] = vTempMaxX[0] + vTempMaxY[0] + vTempMaxZ[0];
	tpos[1] = vTempMaxX[1] + vTempMaxY[1] + vTempMaxZ[1];
	tpos[2] = vTempMaxX[2] + vTempMaxY[2] + vTempMaxZ[2];
	clip &= pointVisible(tpos);
	if (!clip)
		return 1;

#endif
 	return 0;
}

typedef struct ViewportInfo ViewportInfo;

// Generated by mkproto
void gfxWindowReshape();
void gfxWindowReshapeForViewport(ViewportInfo * viewport, int fovMagic);
void gfxWindowReshapeForHeadShot(int fovMagic );
void gfxWindowScreenPos(Vec3 pos,Vec2 pixel_pos);
void gfxWindowGetFrustumBounds(Vec3 min, Vec3 max, F32 znear, F32 zfar);
void gfxCursor3d(F32 x,F32 y,F32 len,Vec3 start,Vec3 end);
int gfxIsPointVisible(const Vec3 world_pos);
int gfxBoxIsVisible(Vec3 min, Vec3 max, Mat4 mat);
int gfxScreenBoxBestArea(Vec3 min, Vec3 max, Mat4 mat);
// End mkproto
#endif
