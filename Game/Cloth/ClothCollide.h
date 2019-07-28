#ifndef _CLOTHCOLLIDE_H
#define _CLOTHCOLLIDE_H

// Box collision code works, but has not proven useful
// and requires some additional memory, so disable for now
#define CLOTH_SUPPORT_BOX_COL 1
#include "Cloth.h"

typedef enum CLOTH_COLTYPE
{
	CLOTH_COL_NONE,
	CLOTH_COL_SPHERE,
	CLOTH_COL_PLANE,
	CLOTH_COL_CYLINDER,
	CLOTH_COL_BALLOON,
#if CLOTH_SUPPORT_BOX_COL
	CLOTH_COL_BOX,
#endif
	CLOTH_COL_SKIP = 0x1000
} CLOTH_COLTYPE;

struct ClothCol
{
	CLOTH_COLTYPE Type;
	S16 MinSubLOD;
	S16 MaxSubLOD;
	Vec3 Center;
	Vec3 Norm;
#if CLOTH_SUPPORT_BOX_COL
	Vec3 XVec; // Box Only
	Vec3 YVec; // Box Only
#endif
	F32 PlaneD;
	F32 HLength; // Half length
	F32 Radius;  // X radius for Box
#if CLOTH_SUPPORT_BOX_COL
	F32 YRadius; // Box Only
#endif
	// debug
	ClothMesh *Mesh;
};


void ClothColUpdate(ClothCol *clothcol, Vec3 c, Vec3 n, F32 len, F32 rad, F32 frac);
	
void ClothColSetSphere(ClothCol *clothcol, Vec3 c, F32 rad, F32 frac);
void ClothColSetPlane(ClothCol *clothcol, Vec3 c, Vec3 dir, F32 frac);
void ClothColSetCylinder(ClothCol *clothcol, Vec3 p1, Vec3 p2, F32 rad, F32 exten1, F32 exten2, F32 frac);
void ClothColSetBalloon(ClothCol *clothcol, Vec3 p1, Vec3 p2, F32 rad, F32 exten1, F32 exten2, F32 frac);
#if CLOTH_SUPPORT_BOX_COL
void ClothColSetBox(ClothCol *clothcol, Vec3 p1, Vec3 norm, Vec3 xvec, Vec3 yvec, F32 height, F32 xrad, F32 yrad, F32 frac);
#endif

void ClothColSetSubLOD(ClothCol *clothcol, int min, int max);
F32 ClothColSphereBalloonCol(ClothCol *clothcol, Vec3 p, F32 rad);
F32 ClothColSphereSphereCol(ClothCol *clothcol, Vec3 p, F32 rad);
F32 ClothColSphereCylinderCol(ClothCol *clothcol, Vec3 p, F32 rad);
F32 ClothColSpherePlaneCol(ClothCol *clothcol, Vec3 p, F32 rad);
#if CLOTH_SUPPORT_BOX_COL
F32 ClothColSphereBoxCol(ClothCol *clothcol, Vec3 p, F32 rad);
#endif

F32 ClothColCollideSphere(ClothCol *clothcol, Vec3 p, F32 rad);

void ClothColGetMatrix(ClothCol *clothcol, Mat4 mat);

// Debug
void ClothColCreateMesh(ClothCol *clothcol, int detail);

#endif // _CLOTHCOLLIDE_H
