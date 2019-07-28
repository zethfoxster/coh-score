// this file is intended to be included ONLY once, by uvunwrap.c
#ifndef _COMBINEDPOLY_H_
#define _COMBINEDPOLY_H_

#ifdef EPSILON
#undef EPSILON
#endif
#define EPSILON 0.001
#define NEARZERO(x) (fabs(x) < EPSILON)

typedef struct
{
	Vec3 pt[2];
	float len;
	int flags;
} Line3D;

// needed this to make earrays of Vec3
typedef struct {
	Vec3 vec;
} Vec3D;


static int NextCPId = 0;

typedef struct _CombinedPoly
{
	Prim **primitives;
	Line3D **edges; // the perimeter of the cp
	int *corners;
	F32 *cornerDistances;
	int id;
	int minU, minV, maxU, maxV; // does not include offset.  these are the extents of the primitives.
	int offsetU, offsetV;
	int widthWithBorder, heightWithBorder;
	int rotated;
	int fails;
	float areaWithBorder;
	float perimeter;
	Rect **availableRects;
} CombinedPoly;

MP_DEFINE(CombinedPoly);

static CombinedPoly *cpCreate(void)
{
	CombinedPoly *cp;
	MP_CREATE(CombinedPoly, 5000);
	cp = MP_ALLOC(CombinedPoly);
	cp->id = NextCPId++;
	cp->fails = 0;
	eaCreate(&cp->primitives);
	eaCreate(&cp->availableRects);
	eaCreate(&cp->edges);
	eaiCreate(&cp->corners);
	eafCreate(&cp->cornerDistances);
	return cp;
}

static void cpCopy( CombinedPoly *dest, CombinedPoly *src )
{
	eaClear(&dest->primitives);
	eaClear(&dest->availableRects);
	eaClear(&dest->edges);
	eaCopy(&dest->primitives, &src->primitives);
	eaCopy(&dest->availableRects, &src->availableRects);
	eaCopy(&dest->edges, &src->edges);
	eaiCopy(&dest->corners, &src->corners);
	eafCopy(&dest->cornerDistances, &src->cornerDistances);
	dest->id = src->id;
	dest->minU = src->minU;
	dest->minV = src->minV;
	dest->maxU = src->maxU;
	dest->maxV = src->maxV;
	dest->offsetU = src->offsetU;
	dest->offsetV = src->offsetV;
	dest->widthWithBorder = src->widthWithBorder;
	dest->heightWithBorder = src->heightWithBorder;
	dest->rotated = src->rotated;
	dest->areaWithBorder = src->areaWithBorder;
	dest->perimeter = src->perimeter;
}

static void cpDestroy(CombinedPoly *cp)
{
	eaDestroyEx(&cp->availableRects, rectFree);
	eaDestroy(&cp->primitives);
	eaDestroy(&cp->edges);
	eaiDestroy(&cp->corners);
	eafDestroy(&cp->cornerDistances);
	MP_FREE(CombinedPoly, cp);
}

__inline static void cpPushPrim(CombinedPoly *cp, Prim *p)
{
	p->cp = (void*)cp;
	eaPush(&cp->primitives, p);
}

__inline static void cpRemovePrim(CombinedPoly *cp, int prim_idx)
{
	eaRemove(&cp->primitives, prim_idx);
}

__inline static void cpRotate(CombinedPoly *cp)
{
	int temp;

	temp = cp->minU;
	cp->minU = cp->minV;
	cp->minV = temp;

	temp = cp->maxU;
	cp->maxU = cp->maxV;
	cp->maxV = temp;

	temp = cp->widthWithBorder;
	cp->widthWithBorder = cp->heightWithBorder;
	cp->heightWithBorder = temp;

	cp->rotated = !cp->rotated;
}

// returns true if the cp is already oriented properly
__inline static int cpOrient(CombinedPoly *cp, int primarilyHorizontal)
{
	if (primarilyHorizontal)
	{
		if (cp->widthWithBorder >= cp->heightWithBorder)
			return true;
	}
	else
	{
		if (cp->heightWithBorder >= cp->widthWithBorder)
			return true;
	}

	cpRotate(cp);
	return false;
}

__inline static void cpCalcExtents(CombinedPoly *cp)
{
	int i, size = eaSize(&cp->primitives);
	for (i = 0; i < size; i++)
	{
		int	mnU, mxU, mnV, mxV;
		primCalcIntegerUVExtents(cp->primitives[i], &mnU, &mxU, &mnV, &mxV);

		if (!i || mnU < cp->minU)
			cp->minU = mnU;
		if (!i || mxU > cp->maxU)
			cp->maxU = mxU;
		if (!i || mnV < cp->minV)
			cp->minV = mnV;
		if (!i || mxV > cp->maxV)
			cp->maxV = mxV;
	}

	cp->widthWithBorder = cp->maxU - cp->minU + 1 + BORDER_SIZE + BORDER_SIZE;
	cp->heightWithBorder = cp->maxV - cp->minV + 1 + BORDER_SIZE + BORDER_SIZE;
	cp->areaWithBorder = cp->widthWithBorder * cp->heightWithBorder;

	cp->rotated = 0;
}
__inline static void cpCalcExtents2(CombinedPoly *cp, Vec2 mn, Vec2 mx)
{
	int i, size = eaSize(&cp->primitives);
	for (i = 0; i < size; i++)
	{
		Prim *p = cp->primitives[i];
		if (!i)
		{
			vec2MinMax(p->uv[0], p->uv[1], mn, mx);
		}
		else
		{
			MINVEC2(p->uv[0], mn, mn);
			MAXVEC2(p->uv[0], mx, mx);
			MINVEC2(p->uv[1], mn, mn);
			MAXVEC2(p->uv[1], mx, mx);
		}
		MINVEC2(p->uv[2], mn, mn);
		MAXVEC2(p->uv[2], mx, mx);
	}
}

// combine the src cp into the dest cp, keeping the src
static void cpAdd( CombinedPoly *dest, CombinedPoly *src )
{
	int i, numPrims = eaSize(&src->primitives);
	for ( i = 0; i < numPrims; ++i )
	{
		src->primitives[i]->cp = dest;
	}
	eaPushArray(&dest->primitives, &src->primitives);
	eaPushArray(&dest->availableRects, &src->availableRects);
	//cpCalcExtents(dest);
}

__inline static float distance2( Vec2 a, Vec2 b )
{
	return sqrt(SQR(a[0] - b[0]) + SQR(a[1] - b[1]));
}

__inline static void cpSetOffsets(CombinedPoly *cp, int offsetU, int offsetV)
{
	cp->offsetU = offsetU;
	cp->offsetV = offsetV;
}

__inline static void cpAddOffsets(CombinedPoly *cp, int offsetU, int offsetV)
{
	cp->offsetU += offsetU;
	cp->offsetV += offsetV;
}

__inline static void cpFinalize(CombinedPoly *cp)
{
	int i, size = eaSize(&cp->primitives);
	for (i = 0; i < size; i++)
	{
		Prim *p = cp->primitives[i];

		if (cp->rotated)
		{
			float temp;
			temp = p->uv[0][0];
			p->uv[0][0] = p->uv[0][1];
			p->uv[0][1] = temp;

			temp = p->uv[1][0];
			p->uv[1][0] = p->uv[1][1];
			p->uv[1][1] = temp;

			temp = p->uv[2][0];
			p->uv[2][0] = p->uv[2][1];
			p->uv[2][1] = temp;
		}

		p->uv[0][0] -= cp->minU;
		p->uv[0][0] += cp->offsetU + BORDER_SIZE;
		p->uv[0][1] -= cp->minV;
		p->uv[0][1] += cp->offsetV + BORDER_SIZE;

		p->uv[1][0] -= cp->minU;
		p->uv[1][0] += cp->offsetU + BORDER_SIZE;
		p->uv[1][1] -= cp->minV;
		p->uv[1][1] += cp->offsetV + BORDER_SIZE;

		p->uv[2][0] -= cp->minU;
		p->uv[2][0] += cp->offsetU + BORDER_SIZE;
		p->uv[2][1] -= cp->minV;
		p->uv[2][1] += cp->offsetV + BORDER_SIZE;

		assert(p->uv[0][0] >= 0);
		assert(p->uv[0][1] >= 0);
		assert(p->uv[1][0] >= 0);
		assert(p->uv[1][1] >= 0);
		assert(p->uv[2][0] >= 0);
		assert(p->uv[2][1] >= 0);
	}
}

__inline static void cpFinalizeRotation(CombinedPoly *cp)
{
	int i, size = eaSize(&cp->primitives);
	if (cp->rotated)
	{
		for (i = 0; i < size; i++)
		{
			Prim *p = cp->primitives[i];
			float temp;

			temp = p->uv[0][0];
			p->uv[0][0] = p->uv[0][1];
			p->uv[0][1] = temp;

			temp = p->uv[1][0];
			p->uv[1][0] = p->uv[1][1];
			p->uv[1][1] = temp;

			temp = p->uv[2][0];
			p->uv[2][0] = p->uv[2][1];
			p->uv[2][1] = temp;
		}
		cp->rotated = 0;
	}
}

__inline static void cpFinalizePosition(CombinedPoly *cp)
{
	int i, size = eaSize(&cp->primitives);
	for (i = 0; i < size; i++)
	{
		Prim *p = cp->primitives[i];

		//p->uv[0][0] -= cp->minU;
		p->uv[0][0] += cp->offsetU;// + BORDER_SIZE;
		//p->uv[0][1] -= cp->minV;
		p->uv[0][1] += cp->offsetV;// + BORDER_SIZE;

		//p->uv[1][0] -= cp->minU;
		p->uv[1][0] += cp->offsetU;// + BORDER_SIZE;
		//p->uv[1][1] -= cp->minV;
		p->uv[1][1] += cp->offsetV;// + BORDER_SIZE;

		//p->uv[2][0] -= cp->minU;
		p->uv[2][0] += cp->offsetU;// + BORDER_SIZE;
		//p->uv[2][1] -= cp->minV;
		p->uv[2][1] += cp->offsetV;// + BORDER_SIZE;

		if ( NEARZERO(p->uv[0][0]) )
			p->uv[0][0] = 0.0f;
		if ( NEARZERO(p->uv[0][1]) )
			p->uv[0][1] = 0.0f;
		if ( NEARZERO(p->uv[1][0]) )
			p->uv[1][0] = 0.0f;
		if ( NEARZERO(p->uv[1][1]) )
			p->uv[1][1] = 0.0f;
		if ( NEARZERO(p->uv[2][0]) )
			p->uv[2][0] = 0.0f;
		if ( NEARZERO(p->uv[2][1]) )
			p->uv[2][1] = 0.0f;

		assert(p->uv[0][0] >= 0);
		assert(p->uv[0][1] >= 0);
		assert(p->uv[1][0] >= 0);
		assert(p->uv[1][1] >= 0);
		assert(p->uv[2][0] >= 0);
		assert(p->uv[2][1] >= 0);
	}

	cpCalcExtents(cp);
	//cp->offsetU = BORDER_SIZE;
	//cp->offsetV = BORDER_SIZE;
}

// sets the offset to a value that would bring the bounding rect of the cp to the origin and finalizes it
__inline static void cpNormalizeOffsetsFinalize( CombinedPoly *cp )
{
	cp->offsetU = -cp->minU;
	cp->offsetV = -cp->minV;
	cpFinalizePosition( cp );
	cp->offsetU = 0;
	cp->offsetV = 0;
}

// sets the offset to a value that would bring the bounding rect of the cp to the origin
__inline static void cpNormalizeOffsets( CombinedPoly *cp )
{
	cp->offsetU = -cp->minU;
	cp->offsetV = -cp->minV;
}

static void vec2Center(Vec2 uv[3], Vec2 center)
{
	addVec2(uv[0], uv[1], center);
	addVec2(center, uv[2], center);
	scaleVec2(center, 0.333333f, center);
}

static int shareOneVert(Prim *p1, Prim *p2)
{
	if (nearSameVec3(p1->pos[0], p2->pos[0]) || nearSameVec3(p1->pos[0], p2->pos[1]) || nearSameVec3(p1->pos[0], p2->pos[2]))
		return 1;
	if (nearSameVec3(p1->pos[1], p2->pos[0]) || nearSameVec3(p1->pos[1], p2->pos[1]) || nearSameVec3(p1->pos[1], p2->pos[2]))
		return 1;
	if (nearSameVec3(p1->pos[2], p2->pos[0]) || nearSameVec3(p1->pos[2], p2->pos[1]) || nearSameVec3(p1->pos[2], p2->pos[2]))
		return 1;
	return 0;
}

static int cpTestOverlapAndMap(CombinedPoly *cp, Prim *pin, Vec2 uv[3])
{
	Vec2 center, dv, smalltest[3], bigtest[3];
	int i, size = eaSize(&cp->primitives);

	vec2Center(uv, center);

	subVec2(uv[0], center, dv);
	normalVec2(dv);
	scaleAddVec2(dv, -0.1f, uv[0], smalltest[0]);
	scaleAddVec2(dv, 0.5f, uv[0], bigtest[0]);

	subVec2(uv[1], center, dv);
	normalVec2(dv);
	scaleAddVec2(dv, -0.1f, uv[1], smalltest[1]);
	scaleAddVec2(dv, 0.5f, uv[1], bigtest[1]);

	subVec2(uv[2], center, dv);
	normalVec2(dv);
	scaleAddVec2(dv, -0.1f, uv[2], smalltest[2]);
	scaleAddVec2(dv, 0.5f, uv[2], bigtest[2]);

	for (i = 0; i < size; i++)
	{
		Prim *p = cp->primitives[i];
		if (shareOneVert(p, pin))
		{
			if (tri_tri_overlap_test_2d(p->uv[0], p->uv[1], p->uv[2], smalltest[0], smalltest[1], smalltest[2]))
				return 0;
		}
		else
		{
			if (tri_tri_overlap_test_2d(p->uv[0], p->uv[1], p->uv[2], bigtest[0], bigtest[1], bigtest[2]))
				return 0;
		}
	}

	copyVec2(uv[0], pin->uv[0]);
	copyVec2(uv[1], pin->uv[1]);
	copyVec2(uv[2], pin->uv[2]);
	return 1;
}

static void cpScale(CombinedPoly *cp, float scale)
{
	int i, size = eaSize(&cp->primitives);
	for (i = 0; i < size; i++)
	{
		Prim *p = cp->primitives[i];
		scaleVec2(p->uv[0], scale, p->uv[0]);
		scaleVec2(p->uv[1], scale, p->uv[1]);
		scaleVec2(p->uv[2], scale, p->uv[2]);
	}
}

// returns the perimeter length
float cpPerimeter( CombinedPoly *cp )
{
	int i, numPrims = eaSize(&cp->primitives), idxA, idxB;
	float len = 0.0f;
	Vec3 a, b;

	for ( i = 0; i < numPrims; ++i )
	{
		if ( !cp->primitives[i]->neighbors[0] )
		{
			neighborSharedVerts(0, &idxA, &idxB);
			len += distance3Squared( cp->primitives[i]->pos[idxA], cp->primitives[i]->pos[idxB] );
		}
		else if ( cp->primitives[i]->cp != cp->primitives[i]->neighbors[0]->cp )
		{
			sharedEdge(cp->primitives[i], cp->primitives[i]->neighbors[0], &a, &b);
			len += distance3Squared(a, b);
		}

		if ( !cp->primitives[i]->neighbors[1] )
		{
			neighborSharedVerts(1, &idxA, &idxB);
			len += distance3Squared( cp->primitives[i]->pos[idxA], cp->primitives[i]->pos[idxB] );
		}
		else if ( cp->primitives[i]->cp != cp->primitives[i]->neighbors[1]->cp )
		{
			sharedEdge(cp->primitives[i], cp->primitives[i]->neighbors[1], &a, &b);
			len += distance3Squared(a, b);
		}

		if ( !cp->primitives[i]->neighbors[2] )
		{
			neighborSharedVerts(2, &idxA, &idxB);
			len += distance3Squared( cp->primitives[i]->pos[idxA], cp->primitives[i]->pos[idxB] );
		}
		else if ( cp->primitives[i]->cp != cp->primitives[i]->neighbors[2]->cp )
		{
			sharedEdge(cp->primitives[i], cp->primitives[i]->neighbors[2], &a, &b);
			len += distance3Squared(a, b);
		}
	}

	return len;
}

float cpArea(CombinedPoly *cp)
{
	int i, numPrims = eaSize(&cp->primitives);
	float ret = 0.0f;

	for ( i = 0; i < numPrims; ++i )
	{
		ret += cp->primitives[i]->area;
	}

	return ret;
}

float uvArea(Vec2 *uvs)
{
	Vec3 a, b, c;
	a[0] = uvs[0][0]; a[1] = uvs[0][1]; a[2] = 0.0f;
	b[0] = uvs[1][0]; b[1] = uvs[1][1]; b[2] = 0.0f;
	c[0] = uvs[2][0]; c[1] = uvs[2][1]; c[2] = 0.0f;
	return triArea(a, b, c);
	//subVec3(b, a, a);
	//subVec3(c, a, b);
	//crossVec3(a, b, c);
	//return 0.5f * lengthVec3(c);
}

float cpUVArea(CombinedPoly *cp)
{
	int i, numPrims = eaSize(&cp->primitives);
	float ret = 0.0f;

	for ( i = 0; i < numPrims; ++i )
	{
		ret += uvArea(cp->primitives[i]->uv);
	}

	return ret;
}

void printEdges(Line3D **edges)
{
	int i;

	for ( i = 0; i < eaSize(&edges); ++i )
	{
		printf( "%-4.2f, %-4.2f, %-4.2f\t->\t%-4.2f, %-4.2f, %-4.2f\n", 
			edges[i]->pt[0][0], edges[i]->pt[0][1], edges[i]->pt[0][2],
			edges[i]->pt[1][0], edges[i]->pt[1][1], edges[i]->pt[1][2] );
	}
}

// sort the edges so that each index position joins the next by one vertex
void sortEdges(Line3D **edges)
{
	int i, j;

	for ( i = 0; i < eaSize(&edges)-1; ++i )
	{
		for ( j = i+1; j < eaSize(&edges); ++j )
		{
			if ( sameVec3(edges[i]->pt[1], edges[j]->pt[0]) )
			{
				if ( j != i+1 )
					eaSwap(&edges, j, i+1);
				break;
			}
			else if ( sameVec3(edges[i]->pt[1], edges[j]->pt[1]) )
			{
				SWAPF32(edges[j]->pt[1][0], edges[j]->pt[0][0]);
				SWAPF32(edges[j]->pt[1][1], edges[j]->pt[0][1]);
				SWAPF32(edges[j]->pt[1][2], edges[j]->pt[0][2]);
				
				if ( j != i+1 )
					eaSwap(&edges, j, i+1);
				break;
			}
		}
	}
}

Line3D **freeLines = NULL;

__forceinline Line3D *newLine()
{
	Line3D *line = eaPop(&freeLines);
	if ( !line )
		line = malloc(sizeof(Line3D));
	return line;
}

void cpUpdateEdges( CombinedPoly *cp )
{
	int i, numPrims = eaSize(&cp->primitives), numEdges = eaSize(&cp->edges);
	Vec3 a, b;
	int idxA, idxB;
	Line3D *line;

	if ( numEdges )
	{
		if ( !freeLines )
			eaCreate(&freeLines);

		for ( i = 0; i < numEdges; ++i )
		{
			free(cp->edges[i]);
			//eaPush(&freeLines, cp->edges[i]);
		}
		eaClear(&cp->edges);
		eaSetSize(&cp->edges, 0);
	}

	if ( numPrims == 2 )
	{
		int num = 0;
	}

PERFINFO_AUTO_START("find_edges",1);
	for ( i = 0; i < numPrims; ++i )
	{
		//assert ( cp->primitives[i]->cp == cp );

		// neighbor 0
		if ( !cp->primitives[i]->neighbors[0] )
		{
			neighborSharedVerts(0, &idxA, &idxB);
			line = malloc(sizeof(Line3D));
			//line = newLine();
			memcpy( &((*line).pt[0]), &cp->primitives[i]->pos[idxA], sizeof(Vec3) );
			memcpy( &((*line).pt[1]), &cp->primitives[i]->pos[idxB], sizeof(Vec3) );
			//line->len = distance3(line->pt[0], line->pt[1]);
			eaPush(&cp->edges, line);
		}
		else if ( cp->primitives[i]->cp != cp->primitives[i]->neighbors[0]->cp )
		{
			sharedEdge(cp->primitives[i], cp->primitives[i]->neighbors[0], &a, &b);
			line = malloc(sizeof(Line3D));
			//line = newLine();
			memcpy( &((*line).pt[0]), &a, sizeof(Vec3) );
			memcpy( &((*line).pt[1]), &b, sizeof(Vec3) );
			//line->len = distance3(line->pt[0], line->pt[1]);
			eaPush(&cp->edges, line);
		}

		// neighbor 1
		if ( !cp->primitives[i]->neighbors[1] )
		{
			neighborSharedVerts(1, &idxA, &idxB);
			line = malloc(sizeof(Line3D));
			//line = newLine();
			memcpy( &((*line).pt[0]), &cp->primitives[i]->pos[idxA], sizeof(Vec3) );
			memcpy( &((*line).pt[1]), &cp->primitives[i]->pos[idxB], sizeof(Vec3) );
			//line->len = distance3(line->pt[0], line->pt[1]);
			eaPush(&cp->edges, line);
		}
		else if ( cp->primitives[i]->cp != cp->primitives[i]->neighbors[1]->cp )
		{
			sharedEdge(cp->primitives[i], cp->primitives[i]->neighbors[1], &a, &b);
			line = malloc(sizeof(Line3D));
			//line = newLine();
			memcpy( &((*line).pt[0]), &a, sizeof(Vec3) );
			memcpy( &((*line).pt[1]), &b, sizeof(Vec3) );
			//line->len = distance3(line->pt[0], line->pt[1]);
			eaPush(&cp->edges, line);
		}

		// neighbor 2
		if ( !cp->primitives[i]->neighbors[2] )
		{
			neighborSharedVerts(2, &idxA, &idxB);
			line = malloc(sizeof(Line3D));
			//line = newLine();
			memcpy( &((*line).pt[0]), &cp->primitives[i]->pos[idxA], sizeof(Vec3) );
			memcpy( &((*line).pt[1]), &cp->primitives[i]->pos[idxB], sizeof(Vec3) );
			//line->len = distance3(line->pt[0], line->pt[1]);
			eaPush(&cp->edges, line);
		}
		else if ( cp->primitives[i]->cp != cp->primitives[i]->neighbors[2]->cp )
		{
			sharedEdge(cp->primitives[i], cp->primitives[i]->neighbors[2], &a, &b);
			line = malloc(sizeof(Line3D));
			//line = newLine();
			memcpy( &((*line).pt[0]), &a, sizeof(Vec3) );
			memcpy( &((*line).pt[1]), &b, sizeof(Vec3) );
			//line->len = distance3(line->pt[0], line->pt[1]);
			eaPush(&cp->edges, line);
		}
	}
PERFINFO_AUTO_STOP();

PERFINFO_AUTO_START("sortEdges",1);
	sortEdges(cp->edges);
PERFINFO_AUTO_STOP();
}

void cpUpdateCorners( CombinedPoly *cp )
{
	int i, j, /**/numPrims = eaSize(&cp->primitives), numCorners = eaiSize(&cp->corners);
	Vec3 v1, v2;
	float dotThreshold = 0.5f;
	float dot, dist;
	int mapToCircle = 0;

	if ( cp->fails > 1 )
	{
		// we have failed at least twice, which likely means we are missing corners
		// and they are collapsing into degenerate triangles, so increase the dot threshold
		dotThreshold += (cp->fails - 1)/10.0f;
		// we have failed too many times, something about this CP is screwey, so just map to circle
		if ( dotThreshold > 1.0f )
			mapToCircle = 1;
	}

	if ( numCorners )
	{
		eaiClear(&cp->corners);
		eaiSetSize(&cp->corners, 0);
		eafClear(&cp->cornerDistances);
		eafSetSize(&cp->cornerDistances, 0);
	}

	while ( !mapToCircle && eaiSize( &cp->corners ) < 3 && dotThreshold <= 1.0f )
	{
		int idx, prevIdx = 0;

		eaiClear(&cp->corners);
		eaiSetSize(&cp->corners, 0);
		eafClear(&cp->cornerDistances);
		eafSetSize(&cp->cornerDistances, 0);
		
		for ( i = 0; i < eaSize(&cp->edges); ++i )
		{
			idx = i;

			cp->edges[i]->len = distance3(cp->edges[i]->pt[0], cp->edges[i]->pt[1]);
			
			if ( i == 0 )
			{
				subVec3(cp->edges[i]->pt[0], cp->edges[i]->pt[1], v1);
				normalVec3(v1);
				prevIdx = 0;
				cp->edges[i]->flags = 1;
				eaiPush(&cp->corners, i);
				eafPush(&cp->cornerDistances, 0.0f);
				continue;
			}
			else if ( i == eaSize(&cp->edges) - 1 )
				idx = 0;
			subVec3(cp->edges[idx]->pt[0], cp->edges[idx]->pt[1], v2);
			normalVec3(v2);

			if ( (dot = dotVec3(v1, v2)) < dotThreshold )
			{
				dist = 0.0f;
				for ( j = prevIdx; j < i; ++j )
				{
					dist += cp->edges[j]->len;//distance3(cp->edges[j]->pt[0],cp->edges[j]->pt[1]);
				}
				cp->edges[i]->flags = 1;
				eafPush(&cp->cornerDistances, dist);
				eaiPush(&cp->corners, i);
				subVec3(cp->edges[i]->pt[0], cp->edges[i]->pt[1], v1);
				prevIdx = i;
			}
			else
				cp->edges[i]->flags = 0;
		}

		dotThreshold += 0.1f;
	}

	// we failed, just add all the edge points
	if ( mapToCircle || eaiSize( &cp->corners ) < 3 )
	{
		eaiClear(&cp->corners);
		eaiSetSize(&cp->corners, 0);
		eafClear(&cp->cornerDistances);
		eafSetSize(&cp->cornerDistances, 0);

		for ( i = 0; i < eaSize(&cp->edges); ++i )
		{
			cp->edges[i]->len = distance3(cp->edges[i]->pt[0], cp->edges[i]->pt[1]);
			eaiPush(&cp->corners, i);
			cp->edges[i]->flags = 1;
			eafPush(&cp->cornerDistances, cp->edges[i]->len);
		}
	}
	else
	{
		// fix cornerDistances[0], which was initially set to 0
		for ( i = eaSize(&cp->edges)-1; i >= 0 ; --i )
		{
			if ( cp->edges[i]->flags == 1 )
			{
				float dist = 0.0f;
				while ( i < eaSize(&cp->edges) )
				{
					dist += cp->edges[i]->len;
					++i;
				}
				cp->cornerDistances[0] = dist;
				break;
			}
		}
	}

}

// given a current line and point, find the other line that connects to it, or -1 if none are found
// the next vert (the one in the found line that is different than the one given) is returned in pt
int nextLine( Line3D **lineList, int idx, int *pt )
{
	int i, numLines = eaSize(&lineList);
	Vec3 *cur = &lineList[idx]->pt[*pt];

	for ( i = 0; i < numLines; ++i )
	{
		if ( i != idx )
		{
			if ( sameVec3( *cur, lineList[i]->pt[0] ) )
			{
				// this returns the other vert of the line found
				*pt = 1;
				return i;
			}
			else if ( sameVec3( *cur, lineList[i]->pt[1] ) )
			{
				// this returns the other vert of the line found
				*pt = 0;
				return i;
			}
		}
	}
	return -1;
}

int cpIsLegal(CombinedPoly *cp)
{
	int i, j, numPrims = eaSize(&cp->primitives), numLines, found_once;
	int idx, pt;
	Vec3 *cur;

	PERFINFO_AUTO_START("cpUpdateEdges", 1);
	cpUpdateEdges(cp);
	PERFINFO_AUTO_STOP();
	numLines = eaSize(&cp->edges);

	if ( numLines < 3 )
		return 0;

	// if a vertex appears more than twice in the list of edge lines, that means the cp is invalid.
	for ( i = 0; i < numLines; ++i )
	{
		cur = &cp->edges[i]->pt[0];
		found_once = 0;
		for ( j = i + 1; j < numLines; ++j )
		{
			if ( sameVec3(cp->edges[j]->pt[0], *cur) )
			{
				if ( found_once == 1 )
					return 0;
				found_once = 1;
			}
			if ( sameVec3(cp->edges[j]->pt[1], *cur) )
			{
				if ( found_once == 1 )
					return 0;
				found_once = 1;
			}
		}

		cur = &cp->edges[i]->pt[1];
		found_once = 0;
		for ( j = i + 1; j < numLines; ++j )
		{
			if ( sameVec3(cp->edges[j]->pt[0], *cur) )
			{
				if ( found_once == 1 )
					return 0;
				found_once = 1;
			}
			if ( sameVec3(cp->edges[j]->pt[1], *cur) )
			{
				if ( found_once == 1 )
					return 0;
				found_once = 1;
			}
		}
	}

	// also, if a path from a vertex back to itself does not pass through all points, the cp is invalid.
	idx = pt = 0;
	cur = &cp->edges[idx]->pt[pt];
	for ( i = 0; i < numLines; ++i )
	{
		 idx = nextLine(cp->edges, idx, &pt);
		 if ( idx == -1 )
			 return 0;
		 else if ( idx == 0 && pt == 0 )
		 {
			 if ( i < numLines - 1 )
				return 0;
			 return 1;
		 }
	}

	return 1;
}

void cpAverageNormal( CombinedPoly *cp, Vec3 *avgNorm )
{
	int i, cp_size = eaSize(&cp->primitives);
	Vec3 norm;

	for ( i = 0; i < cp_size; ++i )
	{
		Prim * prim = cp->primitives[i];
		calcFaceNormal(prim->pos[0], prim->pos[1], prim->pos[2], norm );
		(*avgNorm)[0] += norm[0] * prim->area;
		(*avgNorm)[1] += norm[1] * prim->area;
		(*avgNorm)[2] += norm[2] * prim->area;
	}

	(*avgNorm)[0] /= cp_size;
	(*avgNorm)[1] /= cp_size;
	(*avgNorm)[2] /= cp_size;
}

CombinedPoly *cpFind( CombinedPoly ***cpList, int id )
{
	int i, cpListSize = eaSize(cpList);

	for ( i = 0; i < cpListSize; ++i )
	{
		if ( (*cpList)[i]->id == id )
			return (*cpList)[i];
	}

	return NULL;
}

int cpFind2( CombinedPoly ***cpList, int id1, int id2, CombinedPoly **cpA, CombinedPoly **cpB )
{
	int i, cpListSize = eaSize(cpList), found = 0;
	(*cpA) = (*cpB) = NULL;

	for ( i = 0; i < cpListSize; ++i )
	{
		if ( (*cpList)[i]->id == id1 )
		{
			(*cpA) = (*cpList)[i];
			++found;
		}
		if ( (*cpList)[i]->id == id2 )
		{
			(*cpB) = (*cpList)[i];
			++found;
		}
		if ( found == 2 )
			break;
	}

	return found;
}

int cpFindIdx( CombinedPoly ***cpList, int id )
{
	int i, cpListSize = eaSize(cpList);

	for ( i = 0; i < cpListSize; ++i )
	{
		if ( (*cpList)[i]->id == id )
			return i;
	}

	return -1;
}

void cpCorrectPrimPointers( CombinedPoly *cp )
{
	int i, numPrims = eaSize(&cp->primitives);
	for ( i = 0; i < numPrims; ++i )
	{
		cp->primitives[i]->cp = cp;
	}
}

void cpProject( CombinedPoly *cp )
{
	int i, numPrims = eaSize(&cp->primitives);
	Vec3 norm;
	Mat3 mat;

	cpAverageNormal(cp, &norm);
	orientMat3(mat, norm);

	for ( i = 0; i < numPrims; ++i )
	{
		Vec3 out;
		mulVecMat3(cp->primitives[i]->pos[0], mat, out);
		cp->primitives[i]->uv[0][0] = out[0];
		cp->primitives[i]->uv[0][1] = out[1];
		mulVecMat3(cp->primitives[i]->pos[1], mat, out);
		cp->primitives[i]->uv[1][0] = out[0];
		cp->primitives[i]->uv[1][1] = out[1];
		mulVecMat3(cp->primitives[i]->pos[2], mat, out);
		cp->primitives[i]->uv[2][0] = out[0];
		cp->primitives[i]->uv[2][1] = out[1];
	}

	{
		//float area = cpArea(cp);
		//if ( area < MIN_CP_AREA )
		//{
		//	while ( (area = cpUVArea(cp)) < MIN_CP_AREA )
		//		cpScale(cp, 1.25);
		//}
	}

	cpCalcExtents(cp);
	cp->offsetU = cp->minU;
	cp->offsetV = cp->minV;
}

#endif
