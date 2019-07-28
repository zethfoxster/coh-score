#include "Cloth.h"
#include "ClothCollide.h"
#include "ClothMesh.h"
#include "ClothPrivate.h"

//============================================================================
// CLOTH COLLISION CODE
//============================================================================
// This code implements a series of FAST collision primitives with a sphere.
// Collisions are with a point sphere instead of a swept sphere (capsule)
// because they are MUCH faster.
// A side effect is that a fast moving particle can easily "pop through" a
// primitive. For this reason it is best to keep the primitives large,
// and the time delta between calls small.
// It is faster to call this code 2-3 times in a frame than it would be
// to use swept spheres.
//
// The primitives supported are:
//
// SPHERE: Simple Point-Point distance test (VERY FAST)
// PLANE:  Simple Point-Plane distance test (VERT FAST AND ONE SIDED)
// CYLINDER: Point-Plane distance test + projection + Point-Point distance test (FAST)
// BALLOON: Point-Line distance test (KIND OF FAST)
// BOX: Point-Plane distance test + projection + 2 dot products (FAST)
//
// Note: Because the PLANE test is ONE SIDED, it is an ideal solution
//   since points can not "pop through"

#ifndef NEAR_ZERO
#define NEAR_ZERO 0.000001f
#endif

#ifndef NEAR_ONE
#define NEAR_ONE 0.999999f
#endif

//============================================================================
// CLOTH COLLISION STRUCTURE CREATION & DELETION
//============================================================================

void ClothColInitialize(ClothCol *clothcol)
{
	clothcol->Type = CLOTH_COL_NONE;
	zeroVec3(clothcol->Center);
	setVec3(clothcol->Norm, 0, 1.f, 0);
	clothcol->PlaneD = 0;
	clothcol->HLength = 0;
	clothcol->Radius = 1.f;
	clothcol->MinSubLOD = 0;
	clothcol->MaxSubLOD = -1;
	clothcol->Mesh = 0;
}

void ClothColDelete(ClothCol *clothcol)
{
	if (clothcol->Mesh)
		ClothMeshDelete(clothcol->Mesh);
}

void ClothColSetSubLOD(ClothCol *clothcol, int min, int max)
{
	clothcol->MinSubLOD = min;
	clothcol->MaxSubLOD = max;
};

//============================================================================
// COLLISION UPDATE FUNCTIONS
//============================================================================

void ClothColUpdate(ClothCol *clothcol, Vec3 c, Vec3 n, F32 len, F32 rad, F32 frac)
{
	if (frac < .99f)
	{
		Vec3 tpos;
		scaleVec3(c, frac, tpos);
		scaleAddVec3(clothcol->Center, 1.0f-frac, tpos, c);
		scaleVec3(n, frac, tpos);
		scaleAddVec3(clothcol->Norm, 1.0f-frac, tpos, n);
	}
	copyVec3(c, clothcol->Center);
	normalVec3(n);
	copyVec3(n, clothcol->Norm);
	clothcol->PlaneD = -dotVec3(c,n);
	clothcol->Radius = rad;
	clothcol->HLength = len*.5f;
}

void ClothColSetSphere(ClothCol *clothcol, Vec3 p1, F32 r, F32 frac)
{
	Vec3 n;
	clothcol->Type = CLOTH_COL_SPHERE;
	setVec3(n, 0, 1.f, 0);
	ClothColUpdate(clothcol, p1, n, 0, r, frac);
}

void ClothColSetPlane(ClothCol *clothcol, Vec3 c, Vec3 dir, F32 frac)
{
	Vec3 n;
	clothcol->Type = CLOTH_COL_PLANE;
	copyVec3(dir, n);
	//normalVec3(n);
	ClothColUpdate(clothcol, c, n, 0, -1.0f, frac);
}

void ClothColSetCylinder(ClothCol *clothcol, Vec3 p1, Vec3 p2, F32 r, F32 exten1, F32 exten2, F32 frac)
{
	Vec3 n,c;
	F32 len,tlen;
	clothcol->Type = CLOTH_COL_CYLINDER;
	subVec3(p2, p1, n);
	len = normalVec3(n);
	tlen = (len+exten2-exten1)*.5f;
	scaleAddVec3(n,tlen,p1,c);
	len += (exten1 + exten2);
	ClothColUpdate(clothcol, c, n, len, r, frac);
}

void ClothColSetBalloon(ClothCol *clothcol, Vec3 p1, Vec3 p2, F32 r, F32 exten1, F32 exten2, F32 frac)
{
	ClothColSetCylinder(clothcol, p1, p2, r, exten1, exten2, frac);
	clothcol->Type = CLOTH_COL_BALLOON;
}

#if CLOTH_SUPPORT_BOX_COL
void ClothColSetBox(ClothCol *clothcol, Vec3 p1, Vec3 norm, Vec3 xvec, Vec3 yvec, F32 height, F32 xrad, F32 yrad, F32 frac)
{
	clothcol->Type = CLOTH_COL_BOX;
	ClothColUpdate(clothcol, p1, norm, height, xrad, frac);
	if (frac == 1.0f)
	{
		copyVec3(xvec, clothcol->XVec);
		copyVec3(yvec, clothcol->YVec);
	}
	else
	{
		Vec3 tpos;
		scaleVec3(xvec, frac, tpos);
		scaleAddVec3(clothcol->XVec, 1.0f-frac, tpos, clothcol->XVec);
		normalVec3(clothcol->XVec);
		scaleVec3(yvec, frac, tpos);
		scaleAddVec3(clothcol->YVec, 1.0f-frac, tpos, clothcol->YVec);
		normalVec3(clothcol->YVec);
	}
	clothcol->YRadius = yrad; 
}
#endif

//============================================================================
// COLLISION FUNCTIONS
//============================================================================

// Collision routines return:
//  -1 if no collision
//  penetration amount otherwise
// NOTE: Modifies p to non penetrating position!!!

F32 ClothColCollideSphere(ClothCol *clothcol, Vec3 p, F32 rad)
{
	switch(clothcol->Type)
	{
	  default:
	  case CLOTH_COL_SPHERE:	return ClothColSphereSphereCol(clothcol, p, rad);
	  case CLOTH_COL_PLANE:		return ClothColSpherePlaneCol(clothcol, p, rad);
	  case CLOTH_COL_CYLINDER:	return ClothColSphereCylinderCol(clothcol, p, rad);
	  case CLOTH_COL_BALLOON:	return ClothColSphereBalloonCol(clothcol, p, rad);
#if CLOTH_SUPPORT_BOX_COL
	  case CLOTH_COL_BOX:		return ClothColSphereBoxCol(clothcol, p, rad);
#endif
	}
}

F32 ClothColSphereBalloonCol(ClothCol *clothcol, Vec3 p, F32 rad)
{
	// Project p on to balloon line segment (ipoint)
	Vec3 dvec,ipoint;
	F32 HLength = clothcol->HLength;
	F32 dist,dist2,delta,r2,dp,rtot;

	rtot = clothcol->Radius + rad;
	
	if (HLength > 0.0f)
	{
		subVec3(p, clothcol->Center, dvec);
		// early exit
		r2 = (HLength + rtot);
		dist2 = lengthVec3Squared(dvec);
		if (dist2 >= r2*r2)
			return -1.0f; // no collision

		dp = dotVec3(dvec,clothcol->Norm);
		dp = MINMAX(dp, -HLength, HLength);
		scaleAddVec3(clothcol->Norm, dp, clothcol->Center, ipoint);
		subVec3(p, ipoint, dvec);
	}
	else
	{
		subVec3(p, clothcol->Center, dvec);
	}
	// Sphere-Sphere collision
	dist2 = lengthVec3Squared(dvec);
	if (dist2 < rtot*rtot)
	{
		dist = sqrtf(dist2);
		delta = rtot - dist;
		delta *= (1.0f/dist);
		scaleAddVec3(dvec, delta, p, p);
		return delta;
	}
	else
	{
		return -1.0f; // no collision
	}
}

F32 ClothColSphereSphereCol(ClothCol *clothcol, Vec3 p, F32 rad)
{
	Vec3 dvec;
	F32 dist,dist2,delta,rtot;

	rtot = clothcol->Radius + rad;
	
	subVec3(p, clothcol->Center, dvec);
	dist2 = lengthVec3Squared(dvec);
	if (dist2 < rtot*rtot)
	{
		dist = sqrtf(dist2);
		delta = rtot - dist;
		delta *= (1.0f/dist);
		scaleAddVec3(dvec, delta, p, p);
		return delta;
	}
	else
	{
		return -1.0f; // no collision
	}
}

// NOTE: There is a slight error here when the sphere is near the
// top or bottom end-cap of the cylinder resulting in a false positive.
// The error however is slight (worst case .3 * rad units), and
// for reasons of speed probably not worth fixing for this purpose.
F32 ClothColSphereCylinderCol(ClothCol *clothcol, Vec3 p, F32 rad)
{
	Vec3 p2,dhoriz;
	F32 dhorizlen2,rtot,d,absd,deltavert;
	
	d = dotVec3(p,clothcol->Norm) + clothcol->PlaneD;
	absd = fabsf(d);
	deltavert = (clothcol->HLength + rad) - absd;
	
	if (deltavert <= 0)
		return -1.0f;
	
	scaleAddVec3(clothcol->Norm, -d, p, p2);
	subVec3(p2, clothcol->Center, dhoriz);
	dhorizlen2 = lengthVec3Squared(dhoriz);
	rtot = clothcol->Radius + rad;
	if (dhorizlen2 > rtot*rtot)
		return -1.0f;
	
	{
		F32 dhorizlen = sqrtf(dhorizlen2);
		F32 deltahoriz = rtot - dhorizlen;
		if (dhorizlen == 0.0f || deltavert < deltahoriz)
		{
			if (d >= 0.0f)
				scaleAddVec3(clothcol->Norm, deltavert, p, p);
			else
				scaleAddVec3(clothcol->Norm, -deltavert, p, p);
			return deltavert;
		}
		else
		{
			F32 s = deltahoriz / dhorizlen;
			scaleAddVec3(dhoriz, s, p, p);
			return deltahoriz;
		}
	}
}

F32 ClothColSpherePlaneCol(ClothCol *clothcol, Vec3 p, F32 rad)
{
	F32 d = dotVec3(p,clothcol->Norm) + clothcol->PlaneD;
	F32 deltavert = rad - d;
	if (deltavert <= 0)
		return -1.0f;
	scaleAddVec3(clothcol->Norm, deltavert, p, p);
	return deltavert;
}

#if CLOTH_SUPPORT_BOX_COL
F32 ClothColSphereBoxCol(ClothCol *clothcol, Vec3 p, F32 rad)
{
	Vec3 p2,dpos;
	F32 dx,dy,dz;
	F32 dxa,dya,dza;
	F32 xrad,yrad,zrad;
	F32 delta;
	
	zrad = clothcol->HLength + rad;
	dz = dotVec3(p,clothcol->Norm) + clothcol->PlaneD;
	if (dz >= zrad || dz <= -zrad)
		return -1.0f;
	
	scaleAddVec3(clothcol->Norm, -dz, p, p2);
	subVec3(p2,clothcol->Center,dpos);

	xrad = clothcol->Radius + rad;
	dx = dotVec3(dpos, clothcol->XVec);
	if (dx >= xrad || dx <= -xrad)
		return -1.0f;
	
	yrad = clothcol->YRadius + rad;
	dy = dotVec3(dpos, clothcol->YVec);
	if (dy >= yrad || dy <= -yrad)
		return -1.0f;

	if (dz >= 0) { dz = zrad - dz; dza = dz; }
	else { dz = -zrad - dz; dza = -dz; }
	if (dx >= 0) { dx = xrad - dx; dxa = dx; }
	else { dx = -xrad - dx; dxa = -dx; }
	if (dy >= 0) { dy = yrad - dy; dya = dy; }
	else { dy = -yrad - dy; dya = -dy; }

	if (dza <= dxa && dza <= dya)
	{
		scaleAddVec3(clothcol->Norm, dz, p, p);
		delta = dza;
	}
	else if (dya <= dxa)
	{
		scaleAddVec3(clothcol->YVec, dy, p, p);
		delta = dya;
	}
	else
	{
		scaleAddVec3(clothcol->XVec, dx, p, p);
		delta = dxa;
	}
	assert(delta > 0);
	return delta;
}
#endif

//============================================================================
// UTILITY FUNCTIONS FOR RENDERING (DEBUG)
//============================================================================

void ClothColGetMatrix(ClothCol *clothcol, Mat4 mat)
{
#if CLOTH_SUPPORT_BOX_COL
	if (clothcol->Type == CLOTH_COL_BOX)
		orientMat3Yvec(mat, clothcol->Norm, clothcol->YVec);
	else
#endif
		orientMat3(mat, clothcol->Norm);
	copyVec3(clothcol->Center, mat[3]);
}

void ClothColCreateMesh(ClothCol *clothcol, int detail)
{
	if (clothcol->Mesh)
		ClothMeshDelete(clothcol->Mesh);
	clothcol->Mesh = ClothMeshCreate();
	switch(clothcol->Type)
	{
	  default:
	  case CLOTH_COL_SPHERE:
		ClothMeshPrimitiveCreateSphere(clothcol->Mesh, clothcol->Radius, detail);
		break;
	  case CLOTH_COL_PLANE:
		ClothMeshPrimitiveCreatePlane(clothcol->Mesh, clothcol->Radius, detail);
		break;
	  case CLOTH_COL_CYLINDER:
		ClothMeshPrimitiveCreateCylinder(clothcol->Mesh, clothcol->Radius, clothcol->HLength, detail);
		break;
	  case CLOTH_COL_BALLOON:
		ClothMeshPrimitiveCreateBalloon(clothcol->Mesh, clothcol->Radius, clothcol->HLength, detail);
		break;
#if CLOTH_SUPPORT_BOX_COL
	  case CLOTH_COL_BOX:
		ClothMeshPrimitiveCreateBox(clothcol->Mesh, clothcol->Radius, clothcol->YRadius, clothcol->HLength, detail);
		break;
#endif
	}
}

//////////////////////////////////////////////////////////////////////////////
