#include "poly.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int	poly_alloc,poly_free;

void	polyResetStats()
{
	poly_alloc = poly_free = 0;
}

void	polyPrintStats()
{
	if (poly_alloc != poly_free)
		printf("!!!%d alloc, %d free\n",poly_alloc,poly_free);
}

void	polyPrintVerts(Polygon *p)
{
int		i;
Vec3	dv;

	for(i=0;i<p->nverts;i++)
	{
		subVec3(p->verts[i].pos,p->verts[(i+1)%p->nverts].pos,dv);
		printf("% -3.5f % -3.5f % -3.5f % -2.3f\n",
			p->verts[i].pos[0],p->verts[i].pos[1],p->verts[i].pos[2],
			lengthVec3(dv));
	}
	printf("\n");
}

Polygon	*polyClone(Polygon *poly)
{
Polygon	*clone;

	poly_alloc++;
	clone = malloc(sizeof(Polygon));
	*clone = *poly;
	clone->verts = malloc(poly->nverts * sizeof(Vertex));
	memcpy(clone->verts,poly->verts,poly->nverts * sizeof(Vertex));

	return clone;
}

Polygon	*polyFirstLink(Polygon *poly)
{
	while(poly && poly->prev) poly = poly->prev;
	return poly;
}

Polygon	*polyLastLink(Polygon *poly)
{
	while(poly && poly->next) poly = poly->next;
	return poly;
}

int		polyNumLinks(Polygon *poly)
{
int		count = 0;

	for(poly = polyFirstLink(poly);poly;poly = poly->next) count++;
	return count;
}

static void	calcFaceNormal(Polygon *poly)
{
Vec3	tvec1,tvec2;

	/* Calc face normal */
	subVec3(poly->verts[1].pos,poly->verts[0].pos,tvec1);
	subVec3(poly->verts[2].pos,poly->verts[1].pos,tvec2);
	crossVec3(tvec1,tvec2,poly->norm);
	normalVec3(poly->norm);
}


#define VEC_TOL	0.0001f

int		poly_err,poly_index;

int		polyLegal(Polygon *p)
{
int		i,j,i2,i3;
Vec3	dvx,dvy,dva,dvb,cp,cpx;
Vec3	zerovec = {0.f,0.f,0.f};
F32		angle;

	subVec3(p->verts[1].pos,p->verts[0].pos,dvx);
	subVec3(p->verts[2].pos,p->verts[0].pos,dvy);
	crossVec3(dvy,dvx,cp);
	if (nearSameVec3Tol(cp,zerovec,VEC_TOL))
	{
		poly_err = 10;
		return 0;
	}
	normalVec3(cp);

	for(i=0;i<p->nverts-1;i++)
	{
		for(j=i+1;j<p->nverts;j++)
		{
			if (nearSameVec3(p->verts[i].pos,p->verts[j].pos))
			{
				poly_err = 20;
				return 0;
			}
		}
	}
	for(i=0;i<p->nverts;i++)
	{
		poly_index = i;
		i2 = (i + 1) % p->nverts;
		i3 = (i + 2) % p->nverts;
		subVec3(p->verts[i2].pos,p->verts[i].pos,dva);
		if (nearSameVec3Tol(dva,zerovec,VEC_TOL))
		{
			poly_err = 1;
			return 0;
		}
		subVec3(p->verts[i3].pos,p->verts[i].pos,dvb);
		if (nearSameVec3Tol(dvb,zerovec,VEC_TOL))
		{
			poly_err = 2;
			return 0;
		}


		normalVec3(dva);
		normalVec3(dvb);
		angle = dotVec3(dva,dvb);
#if 0
		if (1 || angle < -0.9f)
		{
			printf("%f narrow\n",angle);
			polyPrintVerts(p);
		}
#endif
		crossVec3(dvb,dva,cpx);
		if (nearSameVec3Tol(cpx,zerovec,VEC_TOL))
		{
			poly_err = 3;
			return 0;
		}
		normalVec3(cpx);
		if (! nearSameVec3Tol(cp,cpx,0.01f))
		{
			poly_err = 4;
			return 0;
		}

	}
	return 1;
}

Polygon	*polyCreateSimple(Vec3 v1,Vec3 v2,Vec3 v3,Vec3 v4)
{
Polygon	*poly;
Vertex	*v;
int		i;
Vec2	fake_st = {0.f,0.f};
Vec3	fake_rgb = {255.f,255.f,255.f};
Vec3	fake_norm = {0.f,0.f,1.f};

	poly_alloc++;
	poly = malloc(sizeof(Polygon));
	poly->nverts = 3;
	if (v4) poly->nverts = 4;
	poly->verts = malloc(sizeof(Vertex) * poly->nverts);
	for(i=0;i<poly->nverts;i++)
	{
		v = &poly->verts[i];
		copyVec2(fake_st,v->st);
		copyVec3(fake_rgb,v->rgb);
		copyVec3(fake_norm,v->norm);
//		v->blended	= 0;
	}
	copyVec3(v1,poly->verts[0].pos);
	copyVec3(v2,poly->verts[1].pos);
	copyVec3(v3,poly->verts[2].pos);
	if (v4) copyVec3(v4,poly->verts[3].pos);

	calcFaceNormal(poly);

	poly->matidx	= 0;
	poly->tag		= 0;
	return poly;
}

Polygon	*polyCreate(int numv,Vec3 *verts,Vec3 *norms,Vec2 *sts,Polygon *parent,int reuse)
{
Polygon	*poly;
Vertex	*v;
int		i;
Vec3	fake_rgb = {1.f,1.f,1.f};
Vec3	fake_norm = {0.f,0.f,1.f};

	if (reuse)
	{
		free(parent->verts);
		poly = parent;
	}
	else
	{
		poly_alloc++;
		poly = malloc(sizeof(Polygon));
	}
	poly->verts = malloc(sizeof(Vertex) * numv);
	for(i=0;i<numv;i++)
	{
		v = &poly->verts[i];
		copyVec2(sts[i],v->st);
		copyVec3(fake_rgb,v->rgb);
		copyVec3(norms[i],v->norm);
		copyVec3(verts[i],v->pos);
	}

	calcFaceNormal(poly);

	poly->matidx	= parent->matidx;
	poly->tag		= parent->tag;
	poly->nverts	= numv;
	return poly;
}

void	polyFree(Polygon *poly)
{
	if (! poly)
		return;
	poly_free++;
	free(poly->verts);
	free(poly);
}

Polygon *polyLink(Polygon *list,Polygon *poly)
{
#if 0
	if (!polyLegal(poly))
	{
		polyLegal(poly);
		poly->next = poly->prev = 0;
		return list;
	}
#endif
	if (!list)
	{
		poly->next = 0;
		poly->prev = 0;
		return poly;
	}
	poly->next = list->next;
	poly->prev = list;
	if (list->next) list->next->prev = poly;
	list->next = poly;
	return poly;
}

Polygon	*polyUnlink(Polygon *poly)
{
Polygon	*legal;

	legal = poly->prev;
	if (poly->prev)
		poly->prev->next = poly->next;
	if (poly->next)
	{
		poly->next->prev = poly->prev;
		legal = poly->next;
	}
	poly->next = 0;
	poly->prev = 0;
	return legal;
}

Polygon	*polyLinkList(Polygon *parent,Polygon *child)
{
Polygon	*last,*first;

	if (!parent)	return child;
	if (!child)		return parent;

	last	= polyLastLink(parent);
	first	= polyFirstLink(child);
	last->next = first;
	first->prev = last;

	return last;
}

void	polyFreeList(Polygon *poly)
{
Polygon	*next;
	
	for(poly = polyFirstLink(poly);poly;)
	{
		next = poly->next;
		polyUnlink(poly);
		polyFree(poly);
		poly = next;
	}
}

Polygon	*polyTriangulateList(Polygon *list)
{
Polygon	*poly,*tri = 0,*next;
int		i;

	for(poly = polyFirstLink(list),next = poly;next;poly = next)
	{
		next = poly->next;
		if (poly->nverts == 3) continue;

		for(i=1;i<poly->nverts-1;i++)
		{
			tri = malloc(sizeof(Polygon));
			*tri = *poly;
			tri->nverts = 3;
			tri->verts = malloc(sizeof(Vertex) * tri->nverts);
			tri->verts[0] = poly->verts[0];
			tri->verts[1] = poly->verts[i];
			tri->verts[2] = poly->verts[i+1];
			polyLink(poly,tri);
		}
		polyUnlink(poly);
		polyFree(poly);
	}
	if (tri) return tri;
	return list;
}

