#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "clip.h"

#define PZERO 0.0001
#define MAXVERTEX 32

Vec3	FFVertex[MAXVERTEX],BBVertex[MAXVERTEX];
Vec3	FFNormal[MAXVERTEX],BBNormal[MAXVERTEX];
Vec2	FFst[MAXVERTEX],BBst[MAXVERTEX];
F32		PEQZ;
Vec3	PEQt1,PEQt2,PEQX;	/* temp values used to determine intersection */
Vec2	PEQst;
F32		PEQD;				/* Plane equation of plane */
F32		PEQ1,PEQ2;			/* result pf p1,p2 plugged into plane equation */
F32		*VV;				/* Vertex on plane */
F32		*NN;				/* Normal to plane */

F32	pointPlaneDist(Vec3 VV,Vec3 NN,Vec3 pt)
{
F32	PEQD;

    PEQD = -(NN[0]*VV[0] + NN[1]*VV[1] + NN[2]*VV[2]);
	return NN[0]*pt[0] + NN[1]*pt[1] + NN[2]*pt[2] + PEQD;
}

int	VWhichSide(Vec3 VV,Vec3 NN,Vec3 pt)
{
F32	s;

	s = pointPlaneDist(VV,NN,pt);
	if (s > PZERO) return POLY_FRONT;
	if (s < -PZERO) return POLY_BACK;
	return POLY_ON;
}

int PWhichSide(Vec3 pt,Vec3 norm,Polygon *poly)
{
    int i;
    F32 side,max;
	F32 s;
    int vn;

    NN = norm;
    VV = pt;
    PEQD = -(NN[0]*VV[0] + NN[1]*VV[1] + NN[2]*VV[2]);
	
    side = 0.0F;
	max = 0.0F;
    vn = poly->nverts;
    for (i=0;i < vn;i++)
    {
		VV = poly->verts[i].pos;
		s = NN[0]*VV[0] + NN[1]*VV[1] + NN[2]*VV[2] + PEQD;

		if ((side < -PZERO && s > PZERO) || (side > PZERO && s < -PZERO)) 
		{
			return POLY_CUT;
		}
		if (fabs(side) < PZERO)
			side = s;
		if (fabs(side) > fabs(max))
			max = side;
    }
    if (max > PZERO)
		return POLY_FRONT;
	if (max < -PZERO)
		return POLY_BACK;
	return POLY_ON;
}

int SplitPoly(Vec3 pos,Vec3 norm,Polygon *poly,Polygon **front,Polygon **back)
{
    int pn; /* Number of points (vertices) in polygon */
    int fvn,bvn; /* Number of vertices in front and back polys */
    int side;
    int i,j;
	Vertex	*v1,*v2;

    side = PWhichSide(pos,norm,poly);
    if (side != POLY_CUT) return side;
	
    fvn = bvn = 0;
	
    NN = norm;
    VV = pos;
    PEQD = -(NN[0]*VV[0] + NN[1]*VV[1] + NN[2]*VV[2]);

    pn = poly->nverts;
    v1 = &poly->verts[pn-1]; /* Start with last vertex ->first vertex */
    PEQ1 = NN[0]*v1->pos[0] + NN[1]*v1->pos[1] + NN[2]*v1->pos[2] + PEQD;
    for (i=0;i<pn;i++)
    {
		v2 = &poly->verts[i];
		PEQ2 = NN[0]*v2->pos[0] + NN[1]*v2->pos[1] + NN[2]*v2->pos[2] + PEQD;
		if ((PEQ1 > PZERO && PEQ2 < -PZERO) || (PEQ1 < -PZERO && PEQ2 > PZERO))
		{
			subVec3(VV,v1->pos,PEQt1);
			subVec3(v2->pos,v1->pos,PEQt2);
			PEQZ = dotVec3(PEQt1,NN) / dotVec3(PEQt2,NN);
			scaleVec3(PEQt2,PEQZ,PEQt1); /* Re-use t1 */
			addVec3(v1->pos,PEQt1,PEQX);

			for(j=0;j<2;j++)
				FFst[fvn][j] = BBst[bvn][j] = v1->st[j] + PEQZ*(v2->st[j]-v1->st[j]);
			for(j=0;j<3;j++)
			{
				FFNormal[fvn][j] = FFNormal[bvn][j] = v1->norm[j] + PEQZ*(v2->norm[j]-v1->norm[j]);
				FFVertex[fvn][j] = BBVertex[bvn][j] = PEQX[j];
			}
			fvn++;
			bvn++;
		}
		if (PEQ2 > PZERO) 
		{
			copyVec2(v2->st,FFst[fvn]);
			copyVec3(v2->norm,FFNormal[fvn]);
			copyVec3(v2->pos,FFVertex[fvn]);
			fvn++;
		}
		else if (PEQ2 < -PZERO) 
		{
			copyVec2(v2->st,BBst[bvn]);
			copyVec3(v2->norm,BBNormal[bvn]);
			copyVec3(v2->pos,BBVertex[bvn]);
			bvn++;
		}
		else /* Point on plane, in both front and back */
		{
			copyVec2(v2->st,FFst[fvn]);
			copyVec2(v2->st,BBst[bvn]);
			copyVec3(v2->norm,FFNormal[fvn]);
			copyVec3(v2->norm,BBNormal[bvn]);
			copyVec3(v2->pos,FFVertex[fvn]);
			copyVec3(v2->pos,BBVertex[bvn]);
			fvn++;
			bvn++;
		}
		v1 = v2;
		PEQ1 = PEQ2;
    }

	if (front)
	{
		*front	= polyCreate(fvn,FFVertex,FFNormal,FFst,poly,*front == poly);
		(*front)->idx = poly->idx;
	}
	if (back)
	{
		*back	= polyCreate(bvn,BBVertex,BBNormal,BBst,poly,*back == poly);
		(*back)->idx = poly->idx;
	}
	
	return POLY_CUT;
}


void SplitPolyList(Vec3 pos,Vec3 norm,Polygon *list,Polygon **front_p,Polygon **back_p)
{
Polygon	*poly,*clipped = 0,*next;
Polygon	*back,*front,*back_list=0,*front_list=0;
int		res;

	for(poly = polyFirstLink(list);poly;poly=next)
	{
		next = poly->next;
		back=front=0;
		res = SplitPoly(pos,norm,poly,&front,&back);

		if (res == POLY_FRONT)
			front = polyClone(poly);
		else if (res == POLY_BACK)
			back = polyClone(poly);
		else if (res == POLY_ON)
		{
			front = polyClone(poly);
			back = polyClone(poly);
		}
		else
			;//polyUnlink(poly);

		if (back)
			back_list = polyLink(back_list,back);
		if (front)
			front_list = polyLink(front_list,front);
	}
	polyFreeList(list);
	*front_p = front_list;
	*back_p = back_list;
}

Polygon	*ClipPoly(Vec3 pos,Vec3 norm,Polygon *poly,int reuse)
{
Polygon	*back = 0;
int		res;

	if (reuse) back = poly;
	res = SplitPoly(pos,norm,poly,0,&back);

	if (res == POLY_FRONT) return 0;
	if (reuse) return poly;

	if (res ==POLY_BACK || res == POLY_ON)
		back = polyClone(poly);
	return back;
}

Polygon	*ClipPolyList(Vec3 pos,Vec3 norm,Polygon *list)
{
Polygon	*p,*poly,*clipped = 0;

	for(poly = polyFirstLink(list);poly;poly=poly->next)
	{
		p = ClipPoly(pos,norm,poly,0);
		if (p)
			clipped = polyLink(clipped,p);
	}
	return clipped;
}

Polygon	*clipPolyToPlanes(Plane *planes,int count,Polygon *poly,int alloc,int except)
{
int		i,j;
Polygon	*clip,*curr_clip;
static int	clip_order[] = {3,2,5,4,0,1};

	clip = poly;
	if (!alloc) poly = 0;
	for(j=0;j<count;j++)
	{
		if (count == 6) i = clip_order[j];
		else i = j;
		if (i == except) continue;
		curr_clip = ClipPoly(planes[i].pos,planes[i].norm,clip,clip != poly);
#if 0
		if (except >= 0 && !curr_clip)
		{
			printf("%d removed by %d\n",except,i);
		}
#endif
		if (curr_clip)
			clip = curr_clip;
		else
		{
			if (clip != poly) polyFree(clip);
			return 0;
		}
	}
	return clip;
}

void boxToPlanes(Vec3 min,Vec3 max,Plane *planes)
{
int		i;

	if (max[1]-min[1] < 0.1f) max[1] = min[1] + 0.1f;
	for(i=0;i<3;i++)
	{
		copyVec3(min,planes[i].pos);
		zeroVec3(planes[i].norm);
		planes[i].norm[i] = -1.f;

		copyVec3(max,planes[3+i].pos);
		zeroVec3(planes[3+i].norm);
		planes[3+i].norm[i] = 1.f;
	}
}

int triInBox(Vec3 min,F32 cube_size,Vec3 v1,Vec3 v2,Vec3 v3)
{
Polygon *poly,*clip;
Plane	planes[6];
Vec3	max,dv;

	setVec3(dv,cube_size,cube_size,cube_size);
	addVec3(dv,min,max);
	boxToPlanes(min,max,planes);
	poly = polyCreateSimple(v1,v2,v3,0);
	if (!poly)
		return 0;
	clip = clipPolyToPlanes(planes,6,poly,1,-1);
	polyFree(poly);
	if (clip)
	{
		polyFree(clip);
		return 1;
	}
	return 0;
}
