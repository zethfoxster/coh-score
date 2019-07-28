#ifndef _GENERICPOLY_H_
#define _GENERICPOLY_H_

#include "stdtypes.h"


typedef struct GPoly
{
	int count, max;
	Vec3 *points;
} GPoly;

typedef struct GPolySet
{
	int count, max;
	GPoly *polys;
} GPolySet;

typedef struct ConvexHull
{
	int count, max;
	Vec4 *planes;
} ConvexHull;

//////////////////////////////////////////////////////////////////////////

void gpolySetSize(GPoly *poly, int count);
void gpolyAddPoint(GPoly *poly, const Vec3 p);
void gpolyAddUniquePoint(GPoly *poly, const Vec3 p);
void gpolyRemovePoint(GPoly *poly, int idx);
void gpolyFreeData(GPoly *poly);
void gpolyCopy(GPoly *dst, const GPoly *src);

int gpolyClipPlane(GPoly *poly, const Vec4 clipplane);

void gpolyBounds(const GPoly *poly, Vec3 min, Vec3 max);
void gpolyTransform(GPoly *poly, const Mat4 mat);
void gpolyTransformToBounds(const GPoly *poly, const Mat4 mat, Vec3 min, Vec3 max);
void gpolyTransformToBounds44(const GPoly *poly, const Mat44 mat, Vec3 min, Vec3 max);

void gpolyRemoveDuplicates(GPoly *poly);

//////////////////////////////////////////////////////////////////////////

void gpsetSetSize(GPolySet *set, int count);
GPoly *gpsetAddPoly(GPolySet *set);
GPoly *gpsetAddPolyCopy(GPolySet *set, const GPoly *src);
void gpsetRemovePoly(GPolySet *set, int idx);
void gpsetFreeData(GPolySet *set);
void gpsetCopy(GPolySet *dst, const GPolySet *src);

void gpsetMakeBox(GPolySet *set, const Vec3 boxmin, const Vec3 boxmax);
void gpsetMakeFrustum(GPolySet *set, const Mat4 cammat, F32 fovx, F32 fovy, F32 znear, F32 zfar);

void gpsetExtrudeConvexHull(GPolySet *set, const Vec3 extrude);

void gpsetInvertNormals(GPolySet *set);

void gpsetClipPlane(GPolySet *set, const Vec4 clipplane);
void gpsetClipBox(GPolySet *set, const Vec3 clipmin, const Vec3 clipmax);

void gpsetBounds(const GPolySet *set, Vec3 min, Vec3 max);
void gpsetTransform(GPolySet *set, const Mat4 mat);
void gpsetTransformToBounds(const GPolySet *set, const Mat4 mat, Vec3 min, Vec3 max);
void gpsetTransformToBounds44(const GPolySet *set, const Mat44 mat, Vec3 min, Vec3 max);

void gpsetCollapse(const GPolySet *set, GPoly *pointlist);

void gpsetToConvexHull(const GPolySet *set, ConvexHull *hull, int eyespace);

//////////////////////////////////////////////////////////////////////////

void hullSetSize(ConvexHull *hull, int count);
void hullAddPlane(ConvexHull *hull, const Vec4 plane);
void hullFreeData(ConvexHull *hull);
void hullCopy(ConvexHull *dst, const ConvexHull *src);

int hullIsPointInside(const ConvexHull *hull, const Vec3 point);
int hullIsSphereInside(const ConvexHull *hull, const Vec3 center, F32 radius);


#endif//_GENERICPOLY_H_

