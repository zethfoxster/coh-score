// this file is intended to be included ONLY once, by uvunwrap.c
#ifndef _PRIM_H_
#define _PRIM_H_


typedef struct _Prim
{
	Vec3 pos[3];
	Vec2 st[3];
	Vec2 uv[3];
	Vec4 plane;
	float area;
	Mat3 projectionMat;
	struct _Prim *neighbors[3];
	U32 in_cp : 1;
	U32 in_vertex_graph : 1;
	void *cp; // the cp this prim is in
	U32 is_bad : 1;
	int debug_id;
} Prim;

MP_DEFINE(Prim);

__inline static void primCheck(Prim *p)
{
	assert(FINITE(p->uv[0][0]));
	assert(FINITE(p->uv[0][1]));
	assert(FINITE(p->uv[1][0]));
	assert(FINITE(p->uv[1][1]));
	assert(FINITE(p->uv[2][0]));
	assert(FINITE(p->uv[2][1]));
}


Prim *primCreate(Vec3 p0, Vec3 p1, Vec3 p2, Vec2 st0, Vec2 st1, Vec2 st2)
{
	Prim *p;
	MP_CREATE(Prim, 5000);
	p = MP_ALLOC(Prim);
	memset(p, 0, sizeof(*p));
	copyVec3(p0, p->pos[0]);
	copyVec3(p1, p->pos[1]);
	copyVec3(p2, p->pos[2]);
	p->area = makePlane(p0, p1, p2, p->plane);
	copyVec2(st0, p->st[0]);
	copyVec2(st1, p->st[1]);
	copyVec2(st2, p->st[2]);

	// calc projection matrix
	{
		Vec3 stransform, ttransform;

		// Z axis = plane normal
		copyVec3(p->plane, p->projectionMat[2]);

		// X axis = texcoord major axis
		calcTransformVectorsAccurate(p->pos, p->st, stransform, ttransform);
		if (lengthVec3Squared(ttransform) > lengthVec3Squared(stransform))
		{
			Vec3 temp;
			copyVec3(stransform, temp);
			copyVec3(ttransform, stransform);
			copyVec3(temp, ttransform);
		}
		if (!nearSameVec3(stransform, zerovec3))
			copyVec3(stransform, p->projectionMat[0]);
		else if (!nearSameVec3(ttransform, zerovec3))
			copyVec3(ttransform, p->projectionMat[0]);
		else
		{
			subVec3(p->pos[1], p->pos[0], p->projectionMat[0]);
			if (nearSameVec3(p->projectionMat[0], zerovec3))
			{
				subVec3(p->pos[2], p->pos[0], p->projectionMat[0]);
				if (nearSameVec3(p->projectionMat[0], zerovec3))
					p->is_bad = 1;
			}
		}
		normalVec3(p->projectionMat[0]);

		// Y axis = cross product
		crossVec3(p->projectionMat[2], p->projectionMat[0], p->projectionMat[1]);
		normalVec3(p->projectionMat[1]);

		if (!nearSameF32(dotVec3(p->projectionMat[0], p->projectionMat[1]), 0))
			p->is_bad = 1;
		if (!nearSameF32(dotVec3(p->projectionMat[0], p->projectionMat[2]), 0))
			p->is_bad = 1;
		if (!nearSameF32(dotVec3(p->projectionMat[1], p->projectionMat[2]), 0))
			p->is_bad = 1;

		transposeMat3(p->projectionMat);

		if (lengthVec3(p->projectionMat[0]) == 0)
			p->is_bad = 1;
		if (lengthVec3(p->projectionMat[1]) == 0)
			p->is_bad = 1;
		if (lengthVec3(p->projectionMat[2]) == 0)
			p->is_bad = 1;
	}

	return p;
}

void primDestroy(Prim *p)
{
	MP_FREE(Prim, p);
}

void primGetTexCoords(Prim *p, Vec2 texcoord0, Vec2 texcoord1, Vec2 texcoord2)
{
	assert(p->uv[0][0] >= 0);
	assert(p->uv[0][1] >= 0);
	assert(p->uv[1][0] >= 0);
	assert(p->uv[1][1] >= 0);
	assert(p->uv[2][0] >= 0);
	assert(p->uv[2][1] >= 0);

	copyVec2(p->uv[0], texcoord0);
	copyVec2(p->uv[1], texcoord1);
	copyVec2(p->uv[2], texcoord2);
}

__inline static void primCalcUVExtents(Prim *p, Vec2 min, Vec2 max)
{
	vec2MinMax(p->uv[0], p->uv[1], min, max);
	MINVEC2(min, p->uv[2], min);
	MAXVEC2(max, p->uv[2], max);
}

__inline static void primCalcIntegerUVExtents(Prim *p, int *minU, int *maxU, int *minV, int *maxV)
{
	Vec2 min, max;
	primCalcUVExtents(p, min, max);
	*minU = floor(min[0]);
	*maxU = ceil(max[0]);
	*minV = floor(min[1]);
	*maxV = ceil(max[1]);
}

__inline static void primCalcIntegerUVExtentsRect(Prim *p, Rect *r)
{
	Vec2 min, max;
	primCalcUVExtents(p, min, max);
	r->x = floor(min[0]);
	r->x2 = ceil(max[0]);
	r->y = floor(min[1]);
	r->y2 = ceil(max[1]);
}

__inline static void scaleMat3Row(Mat3 m, int row, float scale)
{
	if (row < 0 || row > 2)
		return;
	m[0][row] *= scale;
	m[1][row] *= scale;
	m[2][row] *= scale;
}

__inline static void primProjectUVs(Prim *p)
{
	int rescale, debugcount = 0;
	Vec3 mn, mx, center, pos[3];

	if (p->is_bad)
		return;

	findCenter(p->pos[0], p->pos[1], p->pos[2], center);
	subVec3(p->pos[0], center, pos[0]);
	subVec3(p->pos[1], center, pos[1]);
	subVec3(p->pos[2], center, pos[2]);

	do
	{
		Vec3 v;
		float difx, dify, dif=0;
		rescale = 0;

		mulVecMat3(pos[0], p->projectionMat, v);
		copyVec2(v, p->uv[0]);
		mulVecMat3(pos[1], p->projectionMat, v);
		copyVec2(v, p->uv[1]);
		mulVecMat3(pos[2], p->projectionMat, v);
		copyVec2(v, p->uv[2]);

		primCalcUVExtents(p, mn, mx);
		difx = mx[0]-mn[0];
		dify = mx[1]-mn[1];
		dif = MIN(difx, dify);

		if (dif < 1.f && difx < 4.f && dify < 4.f)
		{
			if (dif < 0.01)
				dif = 0.01;

			scaleMat3Row(p->projectionMat, 0, 1.5f / dif);
			scaleMat3Row(p->projectionMat, 1, 1.5f / dif);

			rescale = 1;
		}

		debugcount++;
	} while (rescale);
 

	subVec2(p->uv[0], mn, p->uv[0]);
	subVec2(p->uv[1], mn, p->uv[1]);
	subVec2(p->uv[2], mn, p->uv[2]);
}

__inline static void primScaleUVs(Prim *p, float divisor)
{
	p->uv[0][0] /= divisor;
	p->uv[0][1] /= divisor;
	p->uv[1][0] /= divisor;
	p->uv[1][1] /= divisor;
	p->uv[2][0] /= divisor;
	p->uv[2][1] /= divisor;

	primCheck(p);

	assert(p->uv[0][0] >= 0);
	assert(p->uv[0][1] >= 0);
	assert(p->uv[1][0] >= 0);
	assert(p->uv[1][1] >= 0);
	assert(p->uv[2][0] >= 0);
	assert(p->uv[2][1] >= 0);
}

__inline static int edge(int i, int j)
{
	if (i < j)
	{
		if (i == 0 && j == 1)
			return 0;
		if (i == 1 && j == 2)
			return 1;
		if (i == 0 && j == 2)
			return 2;
	}
	else
	{
		if (i == 1 && j == 0)
			return 0;
		if (i == 2 && j == 1)
			return 1;
		if (i == 2 && j == 0)
			return 2;
	}
	assertmsg(0, "Bad edge indices!");
	return 0;
}

__inline static int neighborSharedVerts( int neighbor, int *i, int *j )
{
	switch ( neighbor )
	{
	case 0:
		*i = 0;
		*j = 1;
		break;
	case 1:
		*i = 1;
		*j = 2;
		break;
	case 2:
		*i = 0;
		*j = 2;
		break;
	default:
		assert("Bad neighbor passed to neighborSharedVerts"==0);
		return 0;
	}
	return 1;
}

static int share2verts(Prim *p0, Prim *p1, int *edge0, int *edge1)
{
	int i, j, found = 0, iidxs[9], jidxs[9];
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (sameVec3(p0->pos[i], p1->pos[j]))
			{
				iidxs[found] = i;
				jidxs[found] = j;
				found++;
			}
		}
	}

	if (found != 2)
		return 0;

	*edge0 = edge(iidxs[0], iidxs[1]);
	*edge1 = edge(jidxs[0], jidxs[1]);

	return 1;
}

// returns 0 if no edge is shared.
// returns the end points of the edge shared in a and b
static int sharedEdge(Prim *p0, Prim *p1, Vec3 *a, Vec3 *b)
{
	int i, j, found = 0, iidxs[9];
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (sameVec3(p0->pos[i], p1->pos[j]))
			{
				iidxs[found] = i;
				found++;
			}
		}
	}

	if (found != 2)
		return 0;

	memcpy((*a), p0->pos[iidxs[0]], sizeof(Vec3));
	memcpy((*b), p0->pos[iidxs[1]], sizeof(Vec3));

	return 1;
}

int vertIsCorner(Prim *p, int vert)
{
	int i, j, k, v1, v2;
	int exists = 0;
	static Prim **primsContainingVert = NULL, **nonCPNeighbors = NULL;

	if ( primsContainingVert == NULL )
		eaCreate(&primsContainingVert);
	if ( nonCPNeighbors == NULL )
		eaCreate(&nonCPNeighbors);

	eaSetSize(&primsContainingVert, 0);
	eaSetSize(&nonCPNeighbors, 0);

	eaPush(&primsContainingVert, p);

	// find all of the primitives that contain this vert.
	for ( i = 0; i < eaSize(&primsContainingVert); ++i )
	{
		for ( j = 0; j < 3; ++j )
		{
			if ( primsContainingVert[i]->neighbors[j] && 
				primsContainingVert[i]->neighbors[j]->cp == p->cp &&
				neighborSharedVerts(j, &v1, &v2) && 
				(sameVec3(p->pos[vert], primsContainingVert[i]->neighbors[j]->pos[v1]) ||
				sameVec3(p->pos[vert], primsContainingVert[i]->neighbors[j]->pos[v2]) ) 
				)
			{
				exists = 0;

				for ( k = 0; k < eaSize(&primsContainingVert); ++k )
				{
					if ( primsContainingVert[k] == primsContainingVert[i]->neighbors[j] )
					{
						exists = 1;
						break;
					}
				}

				if ( !exists )
					eaPush(&primsContainingVert, primsContainingVert[i]->neighbors[j]);
			}
		}
	}

	// find all of the neighbors that border this vert
	for ( i = 0; i < eaSize(&primsContainingVert); ++i )
	{
		int numNullNeighbors = 0;
		int numNonCPNeighbors = 0;
		for ( j = 0; j < 3; ++j )
		{
			exists = 0;

			neighborSharedVerts(j, &v1, &v2);

			if ( primsContainingVert[i]->neighbors[j] &&
				(sameVec3(p->pos[vert], primsContainingVert[i]->neighbors[j]->pos[v1]) ||
				sameVec3(p->pos[vert], primsContainingVert[i]->neighbors[j]->pos[v1])) )
			{
				for ( k = 0; k < eaSize(&nonCPNeighbors); ++k )
				{
					if ( primsContainingVert[i]->neighbors[j]->cp != p->cp )
					{
						++numNonCPNeighbors;
						if ( primsContainingVert[i]->neighbors[j] == nonCPNeighbors[k] )
							exists = 1;
						if ( primsContainingVert[i]->neighbors[j] == NULL )
							++numNullNeighbors;
					}
				}
				
				if ( !exists )
					eaPush(&nonCPNeighbors, primsContainingVert[i]->neighbors[j]);
			}
			else if ( primsContainingVert[i]->neighbors[j] == NULL )
				++numNullNeighbors;
		}

		// this vertex borders 2 null or non-cp neighbors (or more?), and we have no way to know
		// whether its a corner or not, so we err on the side of safety and assume it is
		if ( numNullNeighbors > 1 || numNonCPNeighbors > 1 )
			return 1;
	}

	return eaSize(&nonCPNeighbors) > 1;
}

#endif

