#include "group.h"
#include "gridcoll.h"
#include "mathutil.h"
#include "gridcache.h"
#include "gridcollobj.h"
#include "utils.h"
#include "tricks.h"
#include "anim.h"
#include "grouptrack.h"

int dump_grid_coll_info;

static INLINEDBG F32 worldSlope(Vec3 n,F32 s,Mat3 wmat)
{
	if (n[1] > .999999) {
		return wmat[1][1];
	} else if (n[1] < -.999999) {
		return -wmat[1][1];
	} else {
		return n[0] * wmat[0][1] + n[1] * wmat[1][1] + n[2] * wmat[2][1];
	}
}

static INLINEDBG int checkCollCloser(DefTracker *node,int idx,CollInfo *coll,Vec3 col,F32 sqdist)
{
	Vec3		dv;
	Model	*model;

	mulVecMat4(col,node->mat,dv);
	copyVec3(dv,col);
	if (coll->flags & COLL_DISTFROMSTART)
	{
		F32 line_dist = sqdist;

		subVec3(col,coll->start,dv);
		sqdist = lengthVec3Squared(dv);
		sqdist -= line_dist;
		if (1) // give precedence to standing on floors instead of edges
		{
			CTri	*ctri = &node->def->model->ctris[idx];
			F32		t,slope = worldSlope(ctri->norm,ctri->scale,node->mat);

			if (sqdist < 0)
				sqdist = 0;
			t = sqrtf(sqdist) + (1-slope) / 64.f; // hack to give precedence to flatter surfaces
			sqdist = SQR(t);
		}
	}
	if (sqdist < coll->sqdist)
	{
		model = node->def->model;
		copyVec3(col,coll->mat[3]);
		coll->sqdist = sqdist;
		coll->ctri = &model->ctris[idx];
		coll->node = node;
		coll->tri_idx = idx;
		coll->backside = coll->tri_state.backside;
		coll->surfTex = model->ctris[idx].surfTex;
		if ((coll->flags & COLL_HITANY) && !coll->grid_cache)
		{
			return 1;
		}
	}
	return 0;
}

int checkTriColl(Model *model,int idx,CollInfo *coll,Vec3 obj_start,Vec3 obj_end,DefTracker *node)
{
	F32		sqdist;
	Vec3	col;

	// Allow the user to reject this particular triangle to be considered for collision.
	if(coll->flags & COLL_TRICALLBACK && !coll->tri_callback(node, &model->ctris[idx]))
		return 0;

	if( (coll->flags & COLL_CAMERA) && ( ( model->ctris[idx].flags & COLL_NOCAMERACOLLIDE ) || (model->ctris[idx].flags & COLL_PORTAL) && !node->def->water_volume ) )
		return 0;

	if (!(model->ctris[idx].flags & coll->flags))
		return 0;

	model->tags[idx] = model->grid.tag;

	sqdist = ctriCollideRad(&model->ctris[idx],obj_start,obj_end,col,coll->radius,&coll->tri_state);
	if (sqdist >= 0.f)
	{
		if (coll->flags & COLL_CYLINDER)
		{
			Vec3	tverts[3];
			F32		rad2;
			int		ret;

			expandCtriVerts(&model->ctris[idx],tverts);
			ret = ctriCylCheck(col,obj_start,obj_end,coll->radius,tverts,&rad2);
			if (!ret)
				return 0;
			if (ret > 0 && (coll->flags & COLL_DISTFROMSTART))
				sqdist = rad2;
		}
		if (coll->flags & COLL_DISTFROMSTARTEXACT)
		{
			Vec3	tverts[3];

			expandCtriVerts(&model->ctris[idx],tverts);
			sqdist = ctriGetPointClosestToStart(col,obj_start,obj_end,coll->radius,tverts,model->ctris[idx].norm,node->mat);
		}
		if (coll->flags & COLL_GATHERTRIS)
		{
			TriCollInfo *tri;
			Mat4		m;
			Vec3		tverts[3];
			int			i;

			tri = dynArrayAdd(&coll->tri_colls,sizeof(coll->tri_colls[0]),&coll->coll_count,&coll->coll_max,1);
			tri->tracker = node;
			tri->tri_idx = idx;
			mulVecMat4(col,node->mat,tri->mat[3]);
			normScaleToMat(model->ctris[idx].norm,model->ctris[idx].scale,m);
			mulMat3(node->mat,m,tri->mat);
			expandCtriVerts(&model->ctris[idx],tverts);
			for(i=0;i<3;i++)
				mulVecMat4(tverts[i],node->mat,tri->verts[i]);
		}
		return checkCollCloser(node,idx,coll,col,sqdist);
	}
	return 0;
}

typedef struct
{
	Model	    *model;
	CollInfo	*coll;
	Vec3		obj_start,obj_end;
	Vec3		dir;
	DefTracker	*node;
} ObjGridState;

int all_in_count,not_all_in_count;

static int INLINEDBG objClipLineToBox(Vec3 box,F32 r,Vec3 start,Vec3 end,Vec3 lna,Vec3 lnb)
{
	Vec3	dv;
	F32		fst = 0,fet = 1;
	int		i;

	for(i=0;i<3;i++)
	{
		F32		st,et,inv_di,maxbi,minbi;

		maxbi = box[i] + r;
		minbi = box[i] - r;
		if (start[i] < end[i])
		{
			if (start[i] > maxbi || end[i] < minbi)
				return 0;
			//st = (start[i] < minB[i])? (minB[i] - start[i]) / di: 0;
			//et = (end[i] > maxB[i])? (maxB[i] - start[i]) / di: 1;
			st = minbi - start[i];
			if (st > 0)
			{
				inv_di = 1.f / (end[i] - start[i]);
				st *= inv_di;
				if (end[i] > maxbi)
				{
					et = (maxbi - start[i]) * inv_di;
				}
				else
				{
					et = 1;
				}
			}
			else
			{
				st = 0;

				if (end[i] > maxbi)
				{
					et = (maxbi - start[i]) / (end[i] - start[i]);
				}
				else
				{
					et = 1;
				}
			}
		}
		else
		{
			if (end[i] > maxbi || start[i] < minbi)
				return 0;
			//st = (start[i] > maxB[i])? (maxB[i] - start[i]) / di: 0;
			//et = (end[i] < minB[i])? (minB[i] - start[i]) / di: 1;
			st = maxbi - start[i];
			if (st < 0)
			{
				inv_di = 1.f / (end[i] - start[i]);
				st *= inv_di;
				if (end[i] < minbi)
				{
					et = (minbi - start[i]) * inv_di;
				}
				else
				{
					et = 1;
				}
			}
			else
			{
				st = 0;

				if (end[i] < minbi)
				{
					et = (minbi - start[i]) / (end[i] - start[i]);
				}
				else
				{
					et = 1;
				}
			}
		}
		if (st > fst)	fst = st;
		if (et < fet)	fet = et;
		if (fet < fst)
			return 0;
	}
	subVec3(end,start,dv);
	scaleAddVec3(dv,fst,start,lna);
	scaleAddVec3(dv,fet,start,lnb);
	return 1;
}

static int objSplitPlanes(ObjGridState *state,PolyCell *cell,Vec3 lnap,Vec3 lnbp,Vec3 ppos,F32 size)
{
	Vec3		pos,lna,lnb;
	F32			ta,tb,r;
	int			i;
	U8			mask;
	static		U8	masks[3] = {XLO,YLO,ZLO};
	Model		*model;
	int			idx;
	int			tri_count;
	U16			*tri_idxs;

	if (!cell)
		return 0;

	if (!(state->model->ctriflags_setonsome & state->coll->flags)) // Only if some of the triangles match the flags
	{
		return 0;
	}


	if (!objClipLineToBox(ppos,state->coll->radius + size,lnap,lnbp,lna,lnb))
		return 0;
	tri_count = cell->tri_count;
	tri_idxs = cell->tri_idxs;
	
	for(i=0;i<tri_count;i++)
	{
		idx = tri_idxs[i];
		model = state->model; 
		if (model->tags[idx] == model->grid.tag)
			continue;
		if (checkTriColl(model,idx,state->coll,state->obj_start,state->obj_end,state->node))
			return 1; // return 1 for early exit
	}

	if (!cell->children)
		return 0;


	mask = 0xff;
	size *= 0.5;
	r = state->coll->radius;
	for(i=0;i<3;i++)
	{
		ta = lna[i] - ppos[i];
		tb = lnb[i] - ppos[i];

		if (ta > r && tb > r) // neg side
			mask &= ~masks[i];

		if (ta < -r && tb < -r) // pos side
			mask &= masks[i];
	}
	if (!mask)
		return 0;

	if (mask & YLO)
	{
		pos[1] = ppos[1] - size;
		if (mask & XLO)
		{
			pos[0] = ppos[0] - size;
			if (mask & ZLO)
			{
				pos[2] = ppos[2] - size;
				if (objSplitPlanes(state,cell->children[0],lna,lnb,pos,size))
					return 1;
			}
			if (mask & ~ZLO)
			{
				pos[2] = ppos[2] + size;
				if (objSplitPlanes(state,cell->children[2],lna,lnb,pos,size))
					return 1;
			}
		}
		if (mask & ~XLO)
		{
			pos[0] = ppos[0] + size;
			if (mask & ZLO)
			{
				pos[2] = ppos[2] - size;
				if (objSplitPlanes(state,cell->children[1],lna,lnb,pos,size))
					return 1;
			}
			if (mask & ~ZLO)
			{
				pos[2] = ppos[2] + size;
				if (objSplitPlanes(state,cell->children[3],lna,lnb,pos,size))
					return 1;
			}
		}
	}
	if (mask & ~YLO)
	{
		pos[1] = ppos[1] + size;
		if (mask & XLO)
		{
			pos[0] = ppos[0] - size;
			if (mask & ZLO)
			{
				pos[2] = ppos[2] - size;
				if (objSplitPlanes(state,cell->children[4],lna,lnb,pos,size))
					return 1;
			}
			if (mask & ~ZLO)
			{
				pos[2] = ppos[2] + size;
				if (objSplitPlanes(state,cell->children[6],lna,lnb,pos,size))
					return 1;
			}
		}
		if (mask & ~XLO)
		{
			pos[0] = ppos[0] + size;
			if (mask & ZLO)
			{
				pos[2] = ppos[2] - size;
				if (objSplitPlanes(state,cell->children[5],lna,lnb,pos,size))
					return 1;
			}
			if (mask & ~ZLO)
			{
				pos[2] = ppos[2] + size;
				if (objSplitPlanes(state,cell->children[7],lna,lnb,pos,size))
					return 1;
			}
		}
	}
	return 0;
}

static int collVolume(const Vec3 min, const Vec3 max, CollInfo *coll, const ObjGridState *state, DefTracker *node)
{
	F32 distSq = 10e10;
	Vec4 clipplane;
	Vec3 intersection, col={0,0,0}, col_temp;

	setVec4(clipplane, 1, 0, 0, min[0]);
	if (intersectPlane(state->obj_start, state->obj_end, clipplane, intersection))
	{
		F32 d = distanceToBoxSquared(min, max, intersection);
		if (d <= coll->radius)
		{
			mulVecMat4(intersection, node->mat, col_temp);
			if (coll->flags & COLL_DISTFROMSTART)
			{
				Vec3 dv;
				subVec3(col_temp,coll->start,dv);
				d = lengthVec3Squared(dv) - d;
			}
			if (d < distSq)
			{
				copyVec3(col_temp, col);
				distSq = d;
			}
		}
	}

	setVec4(clipplane, -1, 0, 0, -max[0]);
	if (intersectPlane(state->obj_start, state->obj_end, clipplane, intersection))
	{
		F32 d = distanceToBoxSquared(min, max, intersection);
		if (d <= coll->radius)
		{
			mulVecMat4(intersection, node->mat, col_temp);
			if (coll->flags & COLL_DISTFROMSTART)
			{
				Vec3 dv;
				subVec3(col_temp,coll->start,dv);
				d = lengthVec3Squared(dv) - d;
			}
			if (d < distSq)
			{
				copyVec3(col_temp, col);
				distSq = d;
			}
		}
	}

	setVec4(clipplane, 0, 1, 0, min[1]);
	if (intersectPlane(state->obj_start, state->obj_end, clipplane, intersection))
	{
		F32 d = distanceToBoxSquared(min, max, intersection);
		if (d <= coll->radius)
		{
			mulVecMat4(intersection, node->mat, col_temp);
			if (coll->flags & COLL_DISTFROMSTART)
			{
				Vec3 dv;
				subVec3(col_temp,coll->start,dv);
				d = lengthVec3Squared(dv) - d;
			}
			if (d < distSq)
			{
				copyVec3(col_temp, col);
				distSq = d;
			}
		}
	}

	setVec4(clipplane, 0, -1, 0, -max[1]);
	if (intersectPlane(state->obj_start, state->obj_end, clipplane, intersection))
	{
		F32 d = distanceToBoxSquared(min, max, intersection);
		if (d <= coll->radius)
		{
			mulVecMat4(intersection, node->mat, col_temp);
			if (coll->flags & COLL_DISTFROMSTART)
			{
				Vec3 dv;
				subVec3(col_temp,coll->start,dv);
				d = lengthVec3Squared(dv) - d;
			}
			if (d < distSq)
			{
				copyVec3(col_temp, col);
				distSq = d;
			}
		}
	}

	setVec4(clipplane, 0, 0, 1, min[2]);
	if (intersectPlane(state->obj_start, state->obj_end, clipplane, intersection))
	{
		F32 d = distanceToBoxSquared(min, max, intersection);
		if (d <= coll->radius)
		{
			mulVecMat4(intersection, node->mat, col_temp);
			if (coll->flags & COLL_DISTFROMSTART)
			{
				Vec3 dv;
				subVec3(col_temp,coll->start,dv);
				d = lengthVec3Squared(dv) - d;
			}
			if (d < distSq)
			{
				copyVec3(col_temp, col);
				distSq = d;
			}
		}
	}

	setVec4(clipplane, 0, 0, -1, -max[2]);
	if (intersectPlane(state->obj_start, state->obj_end, clipplane, intersection))
	{
		F32 d = distanceToBoxSquared(min, max, intersection);
		if (d <= coll->radius)
		{
			mulVecMat4(intersection, node->mat, col_temp);
			if (coll->flags & COLL_DISTFROMSTART)
			{
				Vec3 dv;
				subVec3(col_temp,coll->start,dv);
				d = lengthVec3Squared(dv) - d;
			}
			if (d < distSq)
			{
				copyVec3(col_temp, col);
				distSq = d;
			}
		}
	}

	if (distSq < coll->sqdist)
	{
		copyVec3(col,coll->mat[3]);
		coll->sqdist = distSq;
		coll->ctri = 0;
		trackerOpen(node);
		coll->node = node->count?&node->entries[0]:node;
		coll->tri_idx = 0;
		coll->backside = pointBoxCollision(state->obj_start, min, max);
		coll->surfTex = 0;
		if ((coll->flags & COLL_HITANY) && !coll->grid_cache)
			return 1;
	}

	return 0;
}

int collObj(DefTracker *node,const Vec3 start,CollInfo *coll)
{
	Vec3			pos,grid_start,grid_end;
	F32				size;
	Mat4			inv_mat;
	ObjGridState	state;
	Model			*model;
	CollInfo		collx;
	int				ret = 0;

	if (!node || node->no_collide)
		return 0;
	if (node->no_coll && !(coll->flags & COLL_EDITONLY))
		return 0;
	model = node->def->has_volume?0:node->def->model;
	
	if (!(node->def->has_volume || (model && model->grid.size)))
		return 0;

	if (!node->def->has_volume)
	{
		model->grid.tag++;
		if (!model->ctris)
			modelCreateCtris(model);

		//TO DO : COLL_IGNOREINVISIBLE should also work per tri.
		if( coll->flags & COLL_IGNOREINVISIBLE )
		{
			if( model->flags & OBJ_HIDE )
				return 0;
			if( model->trick && model->trick->flags1 & TRICK_HIDE )
				return 0;
		}

		if (coll->flags & COLL_DENSITYREJECT)
		{
			F32		density = model->tri_count / (4.f * PI * SQR(model->radius));

			if (density > coll->max_density)
				return 0;
		}
	}

	// Put original line into object space coords for poly collision
	transposeMat4Copy(node->mat,inv_mat);
	if (coll->flags & COLL_FINDINSIDE)
	{
		Vec3	dv,end;

		if (coll->flags & COLL_FINDINSIDE_ANY)
		{
			F32		sqdist;

			subVec3(node->mid,coll->start,dv);
			sqdist = lengthVec3Squared(dv);
			if (sqdist < coll->sqdist)
			{
				coll->sqdist = sqdist;
				coll->node = node;
				return 0;
			}
		}
		subVec3(coll->end,coll->start,dv);
		if (model)
			scaleVec3(dv,model->radius*2,dv);
		else
			scaleVec3(dv,node->radius*2,dv);
		addVec3(coll->end,dv,end);
		mulVecMat4(coll->start,inv_mat,state.obj_start);
		mulVecMat4(end,inv_mat,state.obj_end);
		collx = *coll;
		coll->sqdist = GRID_MAXDIST;
	}
	else
	{
		mulVecMat4(coll->start,inv_mat,state.obj_start);
		mulVecMat4(coll->end,inv_mat,state.obj_end);
	}

	if (node->def->has_volume)
	{
		if (coll->flags & (COLL_EDITONLY|COLL_FINDINSIDE))
			ret = collVolume(node->def->min, node->def->max, coll, &state, node);
	}
	else
	{
		// Put grid line into grid-space coords
		subVec3(state.obj_start,model->grid.pos,grid_start);
		subVec3(state.obj_end,model->grid.pos,grid_end);

		state.node = node;
		subVec3(grid_end,grid_start,state.dir);
		normalVec3(state.dir);
		size = model->grid.size * 0.5f;
		setVec3(pos,size,size,size);
		state.coll = coll;
		state.model = model;
		ret = objSplitPlanes(&state,model->grid.cell,grid_start,grid_end,pos,size);
	}

	if (!(coll->flags & COLL_FINDINSIDE))
		return ret;
	if (coll->sqdist >= GRID_MAXDIST || !coll->node_callback(node,coll->backside) || collx.sqdist < coll->sqdist)
	{
		*coll = collx;
		return 0;
	}
	if (!(coll->flags & COLL_HITANY))
		ret = 0;
	return ret;
}


void setTriCollState(CollInfo *coll)
{
	F32		invmag,mag;

	subVec3(coll->end,coll->start,coll->dir);

	mag = fsqrt(SQR(coll->dir[0]) + SQR(coll->dir[1]) + SQR(coll->dir[2]));
	if (mag > 0)
		invmag = 1.f/mag;
	else
		invmag = 1.f;
	coll->tri_state.inv_linemag = invmag;

	coll->line_len	= normalVec3(coll->dir);
	coll->tri_state.linelen = coll->line_len;
	coll->tri_state.linelen2 = coll->line_len * 0.5;

	if (coll->flags & (COLL_BOTHSIDES | COLL_DISTFROMCENTER))
		coll->tri_state.early_exit = 0;
	else
		coll->tri_state.early_exit = 1;
}

int collGridObj(DefTracker *tracker,const Vec3 start,const Vec3 end,CollInfo *coll)
{
	int		ret;

	coll->sqdist	= GRID_MAXDIST;
	coll->ctri		= 0;
	coll->radius	= 0;
	coll->flags		= COLL_DISTFROMSTART | COLL_PORTAL | COLL_BOTHSIDES | COLL_NORMALTRI;// | COLL_HITANY;
	copyVec3(start,coll->start);
	copyVec3(end,coll->end);
	coll->surfTex	= 0;
	setTriCollState(coll);
	ret = collObj(tracker,start,coll);
	return (coll->sqdist < GRID_MAXDIST);
}
