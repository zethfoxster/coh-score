#include "GenericPoly.h"
#include "utils.h"
#include "mathutil.h"

//////////////////////////////////////////////////////////////////////////

void gpolySetSize(GPoly *poly, int count)
{
	dynArrayFitStructs(&poly->points, &poly->max, count);
	poly->count = count;
}

void gpolyAddPoint(GPoly *poly, const Vec3 p)
{
	F32 *newp = dynArrayAddStruct(&poly->points, &poly->count, &poly->max);
	copyVec3(p, newp);
}

void gpolyAddUniquePoint(GPoly *poly, const Vec3 p)
{
	int i;
	for (i = 0; i < poly->count; i++)
	{
		if (nearSameVec3(poly->points[i], p))
			return;
	}

	gpolyAddPoint(poly, p);
}

void gpolyRemovePoint(GPoly *poly, int idx)
{
	if (idx < poly->count)
	{
		if (idx < poly->count-1)
			CopyStructs(&poly->points[idx], &poly->points[idx+1], poly->count - idx - 1);
		poly->count--;
	}
}

void gpolyFreeData(GPoly *poly)
{
	SAFE_FREE(poly->points);
	ZeroStruct(poly);
}

void gpolyCopy(GPoly *dst, const GPoly *src)
{
	gpolySetSize(dst, src->count);
	CopyStructs(dst->points, src->points, src->count);
}

void gpolyRemoveDuplicates(GPoly *poly)
{
	int i, j;
	for (i = 0; i < poly->count; i++)
	{
		for (j = i+1; j < poly->count; j++)
		{
			if (nearSameVec3(poly->points[i], poly->points[j]))
			{
				gpolyRemovePoint(poly, j);
				j--;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void gpsetSetSize(GPolySet *set, int count)
{
	int i;
	dynArrayFitStructs(&set->polys, &set->max, count);
	for (i = count; i < set->count; i++)
		gpolyFreeData(&set->polys[i]);
	set->count = count;
}

GPoly *gpsetAddPoly(GPolySet *set)
{
	GPoly *dst = dynArrayAddStruct(&set->polys, &set->count, &set->max);
	ZeroStruct(dst);
	return dst;
}

GPoly *gpsetAddPolyCopy(GPolySet *set, const GPoly *src)
{
	GPoly *dst = gpsetAddPoly(set);
	gpolySetSize(dst, src->count);
	CopyStructs(dst->points, src->points, src->count);
	return dst;
}

void gpsetRemovePoly(GPolySet *set, int idx)
{
	if (idx < set->count)
	{
		gpolyFreeData(&set->polys[idx]);
		if (idx < set->count-1)
			CopyStructs(&set->polys[idx], &set->polys[idx+1], set->count - idx - 1);
		set->count--;
		ZeroStruct(&set->polys[set->count]);
	}
}

void gpsetFreeData(GPolySet *set)
{
	int i;
	for (i = 0; i < set->count; i++)
	{
		SAFE_FREE(set->polys[i].points);
	}

	SAFE_FREE(set->polys);
	ZeroStruct(set);
}

void gpsetCopy(GPolySet *dst, const GPolySet *src)
{
	int i;
	gpsetSetSize(dst, src->count);
	for (i = 0; i < src->count; i++)
		gpolyCopy(&dst->polys[i], &src->polys[i]);
}

//////////////////////////////////////////////////////////////////////////

void gpsetMakeBox(GPolySet *set, const Vec3 boxmin, const Vec3 boxmax)
{
	Vec3 points[8];
	//gpsetFreeData(set);
	set->count = 0;

	setVec3(points[0], boxmin[0], boxmin[1], boxmin[2]);
	setVec3(points[1], boxmax[0], boxmin[1], boxmin[2]);
	setVec3(points[2], boxmax[0], boxmax[1], boxmin[2]);
	setVec3(points[3], boxmin[0], boxmax[1], boxmin[2]);
	setVec3(points[4], boxmin[0], boxmin[1], boxmax[2]);
	setVec3(points[5], boxmax[0], boxmin[1], boxmax[2]);
	setVec3(points[6], boxmax[0], boxmax[1], boxmax[2]);
	setVec3(points[7], boxmin[0], boxmax[1], boxmax[2]);

	gpsetSetSize(set, 6);

	// make polys face inward

	// near
	gpolySetSize(&set->polys[0], 4);
	copyVec3(points[0], set->polys[0].points[0]);
	copyVec3(points[1], set->polys[0].points[1]);
	copyVec3(points[2], set->polys[0].points[2]);
	copyVec3(points[3], set->polys[0].points[3]);

	// far
	gpolySetSize(&set->polys[1], 4);
	copyVec3(points[7], set->polys[1].points[0]);
	copyVec3(points[6], set->polys[1].points[1]);
	copyVec3(points[5], set->polys[1].points[2]);
	copyVec3(points[4], set->polys[1].points[3]);

	// left
	gpolySetSize(&set->polys[2], 4);
	copyVec3(points[0], set->polys[2].points[0]);
	copyVec3(points[3], set->polys[2].points[1]);
	copyVec3(points[7], set->polys[2].points[2]);
	copyVec3(points[4], set->polys[2].points[3]);

	// top
	gpolySetSize(&set->polys[3], 4);
	copyVec3(points[1], set->polys[3].points[0]);
	copyVec3(points[0], set->polys[3].points[1]);
	copyVec3(points[4], set->polys[3].points[2]);
	copyVec3(points[5], set->polys[3].points[3]);

	// right
	gpolySetSize(&set->polys[4], 4);
	copyVec3(points[1], set->polys[4].points[0]);
	copyVec3(points[5], set->polys[4].points[1]);
	copyVec3(points[6], set->polys[4].points[2]);
	copyVec3(points[2], set->polys[4].points[3]);

	// bottom
	gpolySetSize(&set->polys[5], 4);
	copyVec3(points[2], set->polys[5].points[0]);
	copyVec3(points[6], set->polys[5].points[1]);
	copyVec3(points[7], set->polys[5].points[2]);
	copyVec3(points[3], set->polys[5].points[3]);
}

void gpsetMakeFrustum(GPolySet *set, const Mat4 cammat, F32 fovx, F32 fovy, F32 znear, F32 zfar)
{
	Vec3 p, points[8];
	F32 t, b, l, r;
	F32 aspect = fovx / fovy;

	//gpsetFreeData(set);
	set->count = 0;

	znear = ABS(znear);
	zfar = ABS(zfar);

	t = znear * tanf(fovy * PI / 360.0);
	b = -t;
	l = b * aspect;
	r = t * aspect;

	setVec3(p, l, t, -znear);
	mulVecMat4(p, cammat, points[0]);

	setVec3(p, r, t, -znear);
	mulVecMat4(p, cammat, points[1]);

	setVec3(p, r, b, -znear);
	mulVecMat4(p, cammat, points[2]);

	setVec3(p, l, b, -znear);
	mulVecMat4(p, cammat, points[3]);

	t = zfar * tanf(fovy * PI / 360.0);
	b = -t;
	l = b * aspect;
	r = t * aspect;

	setVec3(p, l, t, -zfar);
	mulVecMat4(p, cammat, points[4]);

	setVec3(p, r, t, -zfar);
	mulVecMat4(p, cammat, points[5]);

	setVec3(p, r, b, -zfar);
	mulVecMat4(p, cammat, points[6]);

	setVec3(p, l, b, -zfar);
	mulVecMat4(p, cammat, points[7]);

	gpsetSetSize(set, 6);

	// make polys face inward

	// near
	gpolySetSize(&set->polys[0], 4);
	copyVec3(points[0], set->polys[0].points[0]);
	copyVec3(points[1], set->polys[0].points[1]);
	copyVec3(points[2], set->polys[0].points[2]);
	copyVec3(points[3], set->polys[0].points[3]);

	// far
	gpolySetSize(&set->polys[1], 4);
	copyVec3(points[7], set->polys[1].points[0]);
	copyVec3(points[6], set->polys[1].points[1]);
	copyVec3(points[5], set->polys[1].points[2]);
	copyVec3(points[4], set->polys[1].points[3]);

	// left
	gpolySetSize(&set->polys[2], 4);
	copyVec3(points[0], set->polys[2].points[0]);
	copyVec3(points[3], set->polys[2].points[1]);
	copyVec3(points[7], set->polys[2].points[2]);
	copyVec3(points[4], set->polys[2].points[3]);

	// top
	gpolySetSize(&set->polys[3], 4);
	copyVec3(points[1], set->polys[3].points[0]);
	copyVec3(points[0], set->polys[3].points[1]);
	copyVec3(points[4], set->polys[3].points[2]);
	copyVec3(points[5], set->polys[3].points[3]);

	// right
	gpolySetSize(&set->polys[4], 4);
	copyVec3(points[1], set->polys[4].points[0]);
	copyVec3(points[5], set->polys[4].points[1]);
	copyVec3(points[6], set->polys[4].points[2]);
	copyVec3(points[2], set->polys[4].points[3]);

	// bottom
	gpolySetSize(&set->polys[5], 4);
	copyVec3(points[2], set->polys[5].points[0]);
	copyVec3(points[6], set->polys[5].points[1]);
	copyVec3(points[7], set->polys[5].points[2]);
	copyVec3(points[3], set->polys[5].points[3]);
}

static GPoly *clipcollecter = 0;
int gpolyClipPlane(GPoly *poly, const Vec4 clipplane)
{
	Vec3 p;
	int idx, previdx;
	GPoly clipped = {0};
	F32 *distances = _alloca(sizeof(F32) * poly->count);

	for (idx = 0; idx < poly->count; idx++)
		distances[idx] = distanceToPlane(poly->points[idx], clipplane);

	idx = 0;
	previdx = poly->count - 1;
	while (idx < poly->count)
	{
		if ((distances[previdx] < -0.001 && distances[idx] > 0.001) || (distances[previdx] > 0.001 && distances[idx] < -0.001))
		{
			intersectPlane2(poly->points[previdx], poly->points[idx], clipplane, distances[previdx], distances[idx], p);
			gpolyAddPoint(&clipped, p);
			if (clipcollecter)
				gpolyAddPoint(clipcollecter, p);
		}
		if (distances[idx] >= -0.001)
		{
			gpolyAddPoint(&clipped, poly->points[idx]);
		}

		previdx = idx;
		idx++;
	}

	gpolyRemoveDuplicates(&clipped);

	gpolyFreeData(poly);
	CopyStructs(poly, &clipped, 1);

	if (poly->count < 3 && clipcollecter)
	{
		for (idx = 0; idx < poly->count; idx++)
			gpolyAddPoint(clipcollecter, poly->points[idx]);
	}

	return poly->count > 2;
}

void gpsetClipPlane(GPolySet *set, const Vec4 clipplane)
{
	int i;
	GPoly newpoly={0};

	clipcollecter = &newpoly;
	for (i = 0; i < set->count; i++)
	{
		GPoly *poly = &set->polys[i];
		if (!gpolyClipPlane(poly, clipplane))
		{
			gpsetRemovePoly(set, i);
			i--;
		}
	}
	clipcollecter = 0;

	// remove any duplicate clipped points
	gpolyRemoveDuplicates(&newpoly);

	// order the clipped points so that they create a polygon facing the direction of the clipplane
	do
	{
		Vec4 plane;
		for (i = 2; i < newpoly.count; i++)
		{
			makePlane(newpoly.points[0], newpoly.points[i-1], newpoly.points[i], plane);
			if (dotVec3(plane, clipplane) < 0)
			{
				Vec3 temp;
				copyVec3(newpoly.points[i-1], temp);
				copyVec3(newpoly.points[i], newpoly.points[i-1]);
				copyVec3(temp, newpoly.points[i]);
				break;
			}
		}
	} while (i < newpoly.count);

	// add the new poly to the set
	if (newpoly.count > 2)
		gpsetAddPolyCopy(set, &newpoly);

	gpolyFreeData(&newpoly);
}

void gpsetClipBox(GPolySet *set, const Vec3 clipmin, const Vec3 clipmax)
{
	Vec4 clipplane;

	setVec4(clipplane, 1, 0, 0, clipmin[0]);
	gpsetClipPlane(set, clipplane);

	setVec4(clipplane, -1, 0, 0, -clipmax[0]);
	gpsetClipPlane(set, clipplane);

	setVec4(clipplane, 0, 1, 0, clipmin[1]);
	gpsetClipPlane(set, clipplane);

	setVec4(clipplane, 0, -1, 0, -clipmax[1]);
	gpsetClipPlane(set, clipplane);

	setVec4(clipplane, 0, 0, 1, clipmin[2]);
	gpsetClipPlane(set, clipplane);

	setVec4(clipplane, 0, 0, -1, -clipmax[2]);
	gpsetClipPlane(set, clipplane);
}

void gpolyBounds(const GPoly *poly, Vec3 min, Vec3 max)
{
	int i;
	setVec3(min, 8e16, 8e16, 8e16);
	setVec3(max, -8e16, -8e16, -8e16);

	for (i = 0; i < poly->count; i++)
		vec3RunningMinMax(poly->points[i], min, max);
}

void gpolyTransform(GPoly *poly, const Mat4 mat)
{
	int i;
	Vec3 v;
	for (i = 0; i < poly->count; i++)
	{
		mulVecMat4(poly->points[i], mat, v);
		copyVec3(v, poly->points[i]);
	}
}

void gpolyTransformToBounds(const GPoly *poly, const Mat4 mat, Vec3 min, Vec3 max)
{
	int i;
	Vec3 v;
	setVec3(min, 8e16, 8e16, 8e16);
	setVec3(max, -8e16, -8e16, -8e16);

	for (i = 0; i < poly->count; i++)
	{
		mulVecMat4(poly->points[i], mat, v);
		vec3RunningMinMax(v, min, max);
	}
}

void gpolyTransformToBounds44(const GPoly *poly, const Mat44 mat, Vec3 min, Vec3 max)
{
	int i;
	Vec4 v;
	setVec3(min, 8e16, 8e16, 8e16);
	setVec3(max, -8e16, -8e16, -8e16);

	for (i = 0; i < poly->count; i++)
	{
		mulVecMat44(poly->points[i], mat, v);
		if (v[3] > 0)
		{
			F32 scale = 1.f / v[3];
			scaleVec3(v, scale, v);
			vec3RunningMinMax(v, min, max);
		}
	}
}

void gpsetBounds(const GPolySet *set, Vec3 min, Vec3 max)
{
	int i, j;
	setVec3(min, 8e16, 8e16, 8e16);
	setVec3(max, -8e16, -8e16, -8e16);

	for (i = 0; i < set->count; i++)
	{
		for (j = 0; j < set->polys[i].count; j++)
			vec3RunningMinMax(set->polys[i].points[j], min, max);
	}
}

void gpsetTransform(GPolySet *set, const Mat4 mat)
{
	int i;
	for (i = 0; i < set->count; i++)
		gpolyTransform(&set->polys[i], mat);
}

void gpsetTransformToBounds(const GPolySet *set, const Mat4 mat, Vec3 min, Vec3 max)
{
	int i, j;
	Vec3 v;
	setVec3(min, 8e16, 8e16, 8e16);
	setVec3(max, -8e16, -8e16, -8e16);

	for (i = 0; i < set->count; i++)
	{
		for (j = 0; j < set->polys[i].count; j++)
		{
			mulVecMat4(set->polys[i].points[j], mat, v);
			vec3RunningMinMax(v, min, max);
		}
	}
}

void gpsetTransformToBounds44(const GPolySet *set, const Mat44 mat, Vec3 min, Vec3 max)
{
	int i, j;
	Vec4 v;
	setVec3(min, 8e16, 8e16, 8e16);
	setVec3(max, -8e16, -8e16, -8e16);

	for (i = 0; i < set->count; i++)
	{
		for (j = 0; j < set->polys[i].count; j++)
		{
			mulVecMat44(set->polys[i].points[j], mat, v);
			if (v[3] > 0)
			{
				F32 scale = 1.f / v[3];
				scaleVec3(v, scale, v);
				vec3RunningMinMax(v, min, max);
			}
		}
	}
}

void gpsetCollapse(const GPolySet *set, GPoly *pointlist)
{
	int i, j;
	gpolyFreeData(pointlist);
	for (i = 0; i < set->count; i++)
	{
		for (j = 0; j < set->polys[i].count; j++)
			gpolyAddUniquePoint(pointlist, set->polys[i].points[j]);
	}
}

static int gpolyContainsPoints(const GPoly *poly, const Vec3 p0, const Vec3 p1)
{
	int i;
	int pcount = 0;
	for (i = 0; i < poly->count; i++)
	{
		if (nearSameVec3(poly->points[i], p0))
			pcount++;
		if (nearSameVec3(poly->points[i], p1))
			pcount++;
	}

	return pcount >= 2;
}

void gpsetExtrudeConvexHull(GPolySet *set, const Vec3 extrude)
{
	Vec4 plane;
	int i, j, k;
	Vec3 extrude_dir;
	GPolySet output={0};
	F32 *cosGammas = _alloca(sizeof(F32) * set->count);

	copyVec3(extrude, extrude_dir);
	normalVec3(extrude_dir);

	for (i = 0; i < set->count; i++)
	{
		GPoly *poly = &set->polys[i];
		GPoly *polyOut;

		if (poly->count < 3)
			continue;
		makePlane(poly->points[0], poly->points[1], poly->points[2], plane);
		cosGammas[i] = -dotVec3(extrude_dir, plane); // negative because convex hull plane normals point inward
		polyOut = gpsetAddPolyCopy(&output, poly);

		if (cosGammas[i] > 0.001f)
		{
			// facing in extrude direction, move it
			for (j = 0; j < polyOut->count; j++)
			{
				addVec3(polyOut->points[j], extrude, polyOut->points[j]);
			}
		}
	}

	// now fill in the separated edges with quads
	for (i = 0; i < set->count; i++)
	{
		int prevJ;
		GPoly *poly = &set->polys[i];
		if (poly->count < 3 || cosGammas[i] <= 0.001f)
			continue;

		prevJ = poly->count-1;
		for (j = 0; j < poly->count; j++)
		{
			// find edges that were extruded
			for (k = 0; k < set->count; k++)
			{
				// only find neighbor polys that were moved
				if (i == k || poly->count < 3 || cosGammas[k] > 0.001f)
					continue;

				if (gpolyContainsPoints(&set->polys[k], poly->points[prevJ], poly->points[j]))
				{
					GPoly *polyOut = gpsetAddPoly(&output);
					gpolySetSize(polyOut, 4);
					copyVec3(poly->points[j], polyOut->points[3]);
					copyVec3(poly->points[prevJ], polyOut->points[2]);
					addVec3(poly->points[prevJ], extrude, polyOut->points[1]);
					addVec3(poly->points[j], extrude, polyOut->points[0]);
					break;
				}
			}

			prevJ = j;
		}
	}
	gpsetFreeData(set);
	CopyStructs(set, &output, 1);
}

void gpsetToConvexHull(const GPolySet *set, ConvexHull *hull, int eyespace)
{
	int i;
	Vec4 plane;

	hullFreeData(hull);

	for (i = 0; i < set->count; i++)
	{
		GPoly *poly = &set->polys[i];
		if (poly->count < 3)
			continue;
		if (eyespace)
			makePlane(poly->points[2], poly->points[1], poly->points[0], plane);
		else
			makePlane(poly->points[0], poly->points[1], poly->points[2], plane);
		hullAddPlane(hull, plane);
	}
}

void gpsetInvertNormals(GPolySet *set)
{
	int i, j;
	for (i = 0; i < set->count; i++)
	{
		GPoly *poly = &set->polys[i];
		int half_count = poly->count / 2;
		if (poly->count < 3)
			continue;
		for (j = 0; j < half_count; j++)
		{
			Vec3 temp;
			copyVec3(poly->points[j], temp);
			copyVec3(poly->points[poly->count - j - 1], poly->points[j]);
			copyVec3(temp, poly->points[poly->count - j - 1]);
		}
	}
}



//////////////////////////////////////////////////////////////////////////

void hullSetSize(ConvexHull *hull, int count)
{
	dynArrayFitStructs(&hull->planes, &hull->max, count);
	hull->count = count;
}

void hullAddPlane(ConvexHull *hull, const Vec4 plane)
{
	F32 *newp = dynArrayAddStruct(&hull->planes, &hull->count, &hull->max);
	copyVec4(plane, newp);
}

void hullFreeData(ConvexHull *hull)
{
	SAFE_FREE(hull->planes);
	ZeroStruct(hull);
}

void hullCopy(ConvexHull *dst, const ConvexHull *src)
{
	hullFreeData(dst);
	hullSetSize(dst, src->count);
	CopyStructs(dst->planes, src->planes, dst->count);
}

int hullIsPointInside(const ConvexHull *hull, const Vec3 point)
{
	int i;
	for (i = 0; i < hull->count; i++)
	{
		F32 dist = distanceToPlane(point, hull->planes[i]);
		if (dist < 0)
            return 0;
	}

	return 1;
}

int hullIsSphereInside(const ConvexHull *hull, const Vec3 center, F32 radius)
{
	int i;
	for (i = 0; i < hull->count; i++)
	{
		F32 dist = distanceToPlane(center, hull->planes[i]);
		if (dist < -radius)
			return 0;
	}

	return 1;
}



