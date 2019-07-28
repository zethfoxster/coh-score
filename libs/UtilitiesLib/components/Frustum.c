#include "Frustum.h"

void frustumSet(Frustum *w, F32 fovy, F32 fovx, F32 znear, F32 zfar)
{
	F32			viewang,cosval,hang,vang, aspect = fovx / fovy;

	w->fovy = fovy;
	w->fovx = fovx;

	vang = RAD(fovy);
	hang = 2 * (fatan(aspect * ftan(vang/2.0)));

	viewang	= hang * .5;
	cosval	= cosf(viewang);
	w->hvam = (sinf(viewang)/cosval);
	w->hcos	= cosval;

	viewang	= vang * .5 ;
	cosval	= cosf(viewang);
	w->vvam	= (sinf(viewang)/cosval);
	w->vcos	= cosval;

	w->znear= -znear;
	w->zfar	= -zfar;
}

void frustumGetScreenPosition(const Frustum *w, int screen_width, int screen_height, const Vec3 pos_cameraspace, Vec2 screen_pos)
{
	screen_width >>= 1;
	screen_height >>= 1;
	screen_pos[0] = -pos_cameraspace[0] * screen_width  * (1.f/w->hvam) / pos_cameraspace[2] + screen_width;
	screen_pos[1] = pos_cameraspace[1] * screen_height * (1.f/w->vvam) / pos_cameraspace[2] + screen_height;
}

// len is how far to move endpoint
// start and end are outputs
void frustumGetWorldRay(const Frustum *w, int screen_width, int screen_height, const Vec2 screen_pos, F32 len, Vec3 start, Vec3 end)
{
	Mat4	mat;
	Vec3	dv;
	F32		yaw,pitch,aspect = w->fovx / w->fovy;

	copyMat4(w->inv_viewmat,mat);

	dv[0] = (screen_pos[0] - screen_width * 0.5f) / (screen_width * 0.5f);
	dv[1] = (screen_pos[1] - screen_height * 0.5f) / (screen_height * aspect * 0.5f);
	dv[2] = 1.f / w->hvam;

	getVec3YP(dv,&yaw,&pitch);
	yawMat3(yaw,mat);
	pitchMat3(pitch,mat);

	copyVec3(mat[3],start);
	copyVec3(mat[3],end);
	moveVinZ(start,mat,1.f);
	moveVinZ(end,mat,len);
}

void frustumGetBounds(const Frustum *w, Vec3 min, Vec3 max)
{
	Vec3 p1, p;
	F32 scale = w->zfar / w->znear;
	F32 aspect = w->fovx / w->fovy;
	F32 top = w->znear * ftan(RAD(w->fovy * 0.5f));
	F32 bottom = -top;
	F32 left = bottom * aspect;
	F32 right = top * aspect;

	if (w->world_max[0] > -8e15)
	{
		copyVec3(w->world_min, min);
		copyVec3(w->world_max, max);
		return;
	}

	copyVec3(w->inv_viewmat[3], min);
	copyVec3(w->inv_viewmat[3], max);

	setVec3(p1, left, top, -w->znear);
	mulVecMat4(p1, w->inv_viewmat, p);
	vec3RunningMinMax(p, min, max);

	setVec3(p1, right, top, -w->znear);
	mulVecMat4(p1, w->inv_viewmat, p);
	vec3RunningMinMax(p, min, max);

	setVec3(p1, right, bottom, -w->znear);
	mulVecMat4(p1, w->inv_viewmat, p);
	vec3RunningMinMax(p, min, max);

	setVec3(p1, left, bottom, -w->znear);
	mulVecMat4(p1, w->inv_viewmat, p);
	vec3RunningMinMax(p, min, max);

	setVec3(p1, left * scale, top * scale, -w->zfar);
	mulVecMat4(p1, w->inv_viewmat, p);
	vec3RunningMinMax(p, min, max);

	setVec3(p1, right * scale, top * scale, -w->zfar);
	mulVecMat4(p1, w->inv_viewmat, p);
	vec3RunningMinMax(p, min, max);

	setVec3(p1, right * scale, bottom * scale, -w->zfar);
	mulVecMat4(p1, w->inv_viewmat, p);
	vec3RunningMinMax(p, min, max);

	setVec3(p1, left * scale, bottom * scale, -w->zfar);
	mulVecMat4(p1, w->inv_viewmat, p);
	vec3RunningMinMax(p, min, max);
}

