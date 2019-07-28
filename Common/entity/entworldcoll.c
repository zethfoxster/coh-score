#include "entworldcoll.h"
#include <string.h>
#include "mathutil.h"
#include "entity.h"
#include "gridcoll.h"
#include "varutils.h"
#include "motion.h"
#include "groupscene.h"
#include "group.h"
#include "utils.h"
#include "timing.h"
#ifdef SERVER
#include "entGameActions.h"
#include "entsend.h"
#else
#include "font.h"
#include "entDebug.h"
#endif
#define DEFAULT_RADIUS 1.0f

int coll_is_player; //Unused
int landed_on_ground; //Anytime in the last DoPhysics did you hit the ground? (if so, do done fall)

#define MDBG 0 && CLIENT

static CollInfo	last_surf;

static void printColls(CollInfo* coll,U32 flags)
{
	int		i;

	if (coll->coll_count)
		printf("\n%dclosest   %d\t%f %f %f   %f\n",flags ? 1 : 0,coll->tri_idx,vecParamsXYZ(coll->mat[3]),coll->mat[1][1]);
	for(i=0;i<coll->coll_count;i++)
	{
		F32		d = distance3(coll->mat[3],coll->tri_colls[i].mat[3]);
		F32		d2 = fabs(coll->mat[3][1] - coll->tri_colls[i].mat[3][1]);
		int j;

		printf(	"tri %3d:\t%f %f %f    %f %f   %f\n",
				coll->tri_colls[i].tri_idx,
				vecParamsXYZ(coll->tri_colls[i].mat[3]),
				d,
				d2,
				coll->tri_colls[i].mat[1][1]);

		for(j = 0; j < 3; j++)
		{
			printf("   vert %d:\t%f\t%f\t%f\n", j, vecParamsXYZ(coll->tri_colls[i].verts[j]));
		}
	}
}

static int BaseEditCollCallback(void *nd, int facing)
{
	DefTracker *tracker = nd;

	if(strstr(tracker->def->name, "_Wall_") || strstr(tracker->def->name, "_Ceiling_") 
		|| strstr(tracker->def->name, "_Floor_") || strnicmp(tracker->def->name, "grp", 3)==0)
 		return 1;
	else
		return 0;
}

static int collide(Vec3 start,Vec3 end,CollInfo *coll,F32 rad,U32 flags)
{
	extern int dump_grid_coll_info;

	int		ret;

	//coll->tri_colls = 0;
	//coll->coll_count = 0;
	//coll->coll_max = 0;
	//flags |= COLL_GATHERTRIS;

	flags |= COLL_NOTSELECTABLE; // wire fences
	//if (!coll_is_player)
	//	flags |= COLL_ENTBLOCKER;

	//if(dump_grid_coll_info)
	//{
	//	printf(	"start:\t(%1.8f,%1.8f,%1.8f)\n"
	//			"end:\t(%1.8f,%1.8f,%1.8f)\n"
	//			"flags: %d, radius: %1.8f\n",
	//			vecParamsXYZ(start),
	//			vecParamsXYZ(end),
	//			flags,
	//			rad);
	//}

	if(global_motion_state.noDetailCollisions)
	{
		coll->node_callback = BaseEditCollCallback;
		ret = collGrid(0,start,end,coll,rad,flags|COLL_NODECALLBACK);
	}
	else
	{
		ret = collGrid(0,start,end,coll,rad,flags);
	}

	//if(dump_grid_coll_info)
	//{
	//	printColls(coll,flags);
	//}

	return ret;
}

F32 HeightAtLoc(const Vec3 vec, F32 radius, F32 dist)
{
	CollInfo coll;
	Vec3 top;
	Vec3 bot;

	// Move the top up a tiny bit in case the point is coincident with the ground.
	copyVec3(vec, top);
	top[1] += 0.1f;

	copyVec3(vec, bot);
	bot[1] -= dist;

	if (!collide(top,bot,&coll,radius,COLL_DISTFROMSTART|COLL_CYLINDER))
		copyVec3(bot,coll.mat[3]);
	return top[1] - coll.mat[3][1];
}

F32 entHeight(Entity *e, F32 dist)
{
	return HeightAtLoc(ENTPOS(e), 1.0f, dist);
}

static int SlideWall(Entity *e,Vec3 top,Vec3 bot,Vec3 pos)
{
	int			i;
	CollInfo	coll;
	Vec3		dv,pt1,pt2;
	F32			d,h,rad = DEFAULT_RADIUS;

	//if (0)
	//{
	//	copyVec3(bot,pt1);
	//	copyVec3(bot,pt2);
	//	pt1[1] += 6.0;
	//	pt2[1] += 1.5;
	//	coll.tri_colls = 0;
	//	coll.coll_count = 0;
	//	coll.coll_max = 0;
	//	collide(pt1,pt2,&coll,rad,COLL_DISTFROMCENTER | COLL_CYLINDER | COLL_BOTHSIDES | COLL_GATHERTRIS);
	//	free(coll.tri_colls);
	//	if (coll.coll_count)
	//		printf("\n");
	//}

	h = bot[1];
	for(i=0;i<3;i++)
	{
		copyVec3(bot,pt1);
		copyVec3(bot,pt2);
		pt1[1] += 6.0;
		pt2[1] += 1.5;
		if (!collide(pt1,pt2,&coll,rad,COLL_DISTFROMCENTER | COLL_CYLINDER | COLL_BOTHSIDES))
		{
			#if MDBG
				if (!i)
					xyprintf(5,13,"NOSLIDE");
			#endif
			copyVec3(top,pt1);
			copyVec3(bot,pt2);
			pt1[1] += 3.5;
			pt2[1] += 3.5;

			subVec3(top,bot,dv);
			if (lengthVec3Squared(dv) < 1.f || !collGrid(0,pt1,pt2,&coll,rad * 0.667,COLL_DISTFROMCENTER))
			{
				last_surf.ctri = 0;
				copyVec3(bot,top);
				return 0;
			}
			else
			{
				#if MDBG
					xyprintf(5,19,"STUCK %f %f %f",coll.mat[3][0],coll.mat[3][1],coll.mat[3][2]);
					xyprintf(5,20,"  PT2 %f %f %f",bot[0],bot[1],bot[2]);
				#endif

				return 1;
			}
		}
		if (1)
		{
			last_surf = coll;
			e->motion->last_surf_type = SURFTYPE_WALL;
		}
		subVec3(coll.mat[3],bot,dv);
		dv[1] = 0;
		d = rad + 0.05 - normalVec3(dv);
		scaleVec3(dv,d,dv);
		subVec3(bot,dv,bot);

		#if MDBG
		{
			DefTracker *tracker;

			tracker = coll.node;
			xyprintf(5,14+i,"%08x %s  %d SLIDE: %f %f %f (%f %f %f) amt %f",
				tracker,tracker->def->name,i,coll.mat[3][0],coll.mat[3][1],coll.mat[3][2],
				-dv[0],-dv[1],-dv[2],d);
		}
		#endif
		//copyVec3(dv,e->last_slide);
	}
	if (e->motion->vel[1] > 0)
		e->motion->vel[1] = 0;

	return 2;
}

void testcoll(U32 flags)
{
	int		ret;
	Vec3	top = {584.92419,2.5+0.25978798,380.55411};
	Vec3	bot = {584.92419,0.25978798,380.55411};
	CollInfo	coll;

	coll.tri_colls = 0;
	coll.coll_count = 0;
	coll.coll_max = 0;

	ret = collide(top,bot,&coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_GATHERTRIS | flags);
	{
		int		i;

		if (coll.coll_count)
			printf("\n%dclosest   %f %f %f   %f\n",flags ? 1 : 0,coll.mat[3][0],coll.mat[3][1],coll.mat[3][2],coll.mat[1][1]);
		for(i=0;i<coll.coll_count;i++)
		{
			F32		d = distance3(coll.mat[3],coll.tri_colls[i].mat[3]), d2 = fabs(coll.mat[3][1] - coll.tri_colls[i].mat[3][1]);

			printf("%f %f %f    %f %f   %f\n",coll.tri_colls[i].mat[3][0],coll.tri_colls[i].mat[3][1],coll.tri_colls[i].mat[3][2],d,d2,coll.tri_colls[i].mat[1][1]);
		}
		if (0 && coll.mat[1][1] < 0.4)
		{
			coll.tri_colls = 0;
			coll.coll_count = 0;
			coll.coll_max = 0;
			ret = collide(top,bot,&coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_GATHERTRIS);
			coll.tri_colls = 0;
			coll.coll_count = 0;
			coll.coll_max = 0;
			ret = collide(top,bot,&coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_GATHERTRIS);
		}
	}
}

static int GroundHeight(const Vec3 pos,CollInfo *coll,Entity *e,SurfaceParams *surf)
{
	Vec3			top;
	Vec3			bot;
	int				ret;
	MotionState*	motion = e->motion;

#if 0
	testcoll(COLL_DISTFROMSTARTEXACT);
	testcoll(0);
#endif

	copyVec3(pos,top);
	copyVec3(pos,bot);
	top[1] += 2.5 - motion->vel[1] * e->timestep;
	bot[1] += motion->vel[1] * e->timestep - 0.5 * surf->gravity * SQR(e->timestep);
	coll->tri_colls = 0;
	coll->coll_count = 0;
	coll->coll_max = 0;
	ret = collide(top,bot,coll,DEFAULT_RADIUS,COLL_DISTFROMSTART);// | COLL_GATHERTRIS);
	if (ret)
	{
		last_surf = *coll;
		motion->last_surf_type = SURFTYPE_GROUND;
	}
	if (1 && coll->mat[1][1] < 0.7)
	{
		ret = collide(top,bot,coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_DISTFROMSTARTEXACT);// | COLL_GATHERTRIS);
		if (ret)
		{
			last_surf = *coll;
		}
	}

#if 0 && CLIENT
	{
		int		i;

		if (coll->coll_count)
			printf("\nclosest   %f %f %f   %f\n",coll->mat[3][0],coll->mat[3][1],coll->mat[3][2],coll->mat[1][1]);
		for(i=0;i<coll->coll_count;i++)
		{
			F32		d = distance3(coll->mat[3],coll->tri_colls[i].mat[3]), d2 = fabs(coll->mat[3][1] - coll->tri_colls[i].mat[3][1]);

			if (1 || d2 < 0.2)
				printf("%f %f %f    %f %f   %f\n",coll->tri_colls[i].mat[3][0],coll->tri_colls[i].mat[3][1],coll->tri_colls[i].mat[3][2],d,d2,coll->tri_colls[i].mat[1][1]);
		}
		if (coll->mat[1][1] < 0.4)
		{
			coll->tri_colls = 0;
			coll->coll_count = 0;
			coll->coll_max = 0;
			ret = collide(top,bot,coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_GATHERTRIS);
			coll->tri_colls = 0;
			coll->coll_count = 0;
			coll->coll_max = 0;
			ret = collide(top,bot,coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_GATHERTRIS);
		}
	}
#endif
	if(coll->tri_colls)
	{
		free(coll->tri_colls);
	}
	if (!ret)
	{
		F32 minHeight = min(-2000.0f, scene_info.minHeight);
		
		coll->mat[3][1] = minHeight;
		
		if (bot[1] < minHeight){
			static CTri bottom_tri;
	
			coll->mat[1][0] = 0;
			coll->mat[1][1] = 1;
			coll->mat[1][2] = 0;

			bottom_tri.norm[0] = 0;
			bottom_tri.norm[1] = 1;
			bottom_tri.norm[2] = 0;
			
			coll->ctri = &bottom_tri;
			last_surf = *coll;
			motion->last_surf_type = SURFTYPE_GROUND;

			ret = 1;
		}
	}
	return ret;
}

static void reflectOffPlane(Vec3 incident,Vec3 plane_norm,Vec3 reflect)
{
	Vec3	N;
	Vec3	nI;
	Vec3	I = {0};
	F32		dot;
	F32		mag;

	copyVec3(incident,I);
	normalVec3(I);
	subVec3(zerovec3,I,nI);
	mag = lengthVec3(incident);

	copyVec3(plane_norm,N);

	dot = dotVec3(nI,N);
	scaleVec3(N,2*dot,N);
	addVec3(N,I,reflect);
	scaleVec3(reflect,mag,reflect);
}

static void CheckFeet(Entity *e,SurfaceParams *surf)
{
	F32				new_height;
	F32				plr_height;
	CollInfo		coll;
	CollInfo		savecoll;
	int				first = 1;
	Vec3			dv;
	int				hit;
	MotionState*	motion = e->motion;

	savecoll = last_surf;
retry:
	hit = GroundHeight(ENTPOS(e),&coll,e,surf);

	new_height = coll.mat[3][1];
	plr_height = ENTPOSY(e);

	if(hit)
	{
		motion->heightILastTouchedTheGround = ENTPOSY(e);
	}

	// Is the entity more than 0.05 feet above the ground?
	
	if (/*motion->flying ||*/ (new_height - plr_height < -0.3 * e->timestep))
	{
		if(motion->input.flying || (plr_height > motion->highest_height))
			motion->highest_height = plr_height;

		// It should be considered as "falling".
		
		//printf("falling!\n");		
		
  		motion->falling = !motion->input.flying;
		zeroVec3(motion->surf_normal);
		last_surf = savecoll;

		// Don't have to do anything else if the entity is somewhere
		// in the air.
		return;
	}

	// The entity is less than 0.3 feet above the ground.  The entity
	// should be considered as "not falling."
	// If the entity thinks it's falling, it means it's time to make the entity
	// land.

	copyVec3(coll.mat[1],motion->surf_normal);

	if(!motion->input.flying && motion->falling && motion->surf_normal[1] > 0.3)
	{
		// Don't say the entity is falling anymore.
		
		//printf("not falling: %f\n", new_height - plr_height);
		
		motion->falling = 0;
		motion->jump_not_landed = 0;

		landed_on_ground = 1;
	}

	//t = -motion->vel[1] * e->timestep;

	if (new_height - plr_height > 1.5)
	{
		if (first)
		{
			Vec3 newpos;
			last_surf.ctri = 0;
			subVec3(ENTPOS(e),motion->last_pos,dv);
			scaleVec3(dv,0.5,dv);
			addVec3(motion->last_pos,dv,newpos);
			if (new_height - plr_height > 40)
				vecY(newpos) = new_height;
				//e->mat[3][1] -= 1.5;
			entUpdatePosInterpolated(e, newpos);
			first = 0;
			goto retry;
		}

#if MDBG
		xyprintf(5,12,"feet stuck: old height %f new height %f",plr_height,new_height);
#endif
		entUpdatePosInterpolated(e, motion->last_pos);

		return;
	}
	if (!motion->input.flying)
	{
		if (new_height - plr_height < 1.5)
		{
			Vec3 newpos;
			copyVec3(ENTPOS(e), newpos);
			vecY(newpos) = new_height;
			entUpdatePosInterpolated(e, newpos);
		}
	}
	else
	{
		Vec3 newpos;
		copyVec3(ENTPOS(e), newpos);
		vecY(newpos) = MAX(new_height, plr_height);
		entUpdatePosInterpolated(e, newpos);
	}
}

static int CheckHead(Entity *e,int ok_to_move)
{
	Vec3			top;
	Vec3			bot;
	int				ret;
	CollInfo		coll;
	F32				diff;
	MotionState*	motion = e->motion;

	copyVec3(ENTPOS(e),top);
	copyVec3(ENTPOS(e),bot);
	top[1] += 6.0;
	bot[1] += 1.5;
	ret = collide(bot,top,&coll,DEFAULT_RADIUS,COLL_DISTFROMSTART | COLL_CYLINDER);
	//if (0 && ret)
	//{
	//	last_surf = coll;
	//	last_surf_type = 11;
	//}
	if (ret)
	{
		if (!ok_to_move)
			return 1;
		diff = ENTPOSY(e) + 6 - coll.mat[3][1];
		if (diff > 1.5)
		{
			entUpdatePosInterpolated(e, motion->last_pos);
		}
		else
		{
			Vec3 newpos;
			copyVec3(ENTPOS(e), newpos);
			vecY(newpos) += diff;
			entUpdatePosInterpolated(e, newpos);
		}
		return 1;
	}
	return 0;
}

static void makeSteepSlopesSlippery(MotionState *motion,SurfaceParams *surf)
{
	F32		slope = 1 - motion->surf_normal[1];

	if (motion->on_poly_edge)
		return;
		
	// surf_normal[1]=0 means vertical
	#define SLOPE_START_SLIDE 0.3
	#define SLOPE_FULL_SLIDE 0.6
	if (slope >= SLOPE_START_SLIDE)
	{
		F32		ratio;

		ratio = (slope - SLOPE_START_SLIDE) / (SLOPE_FULL_SLIDE - SLOPE_START_SLIDE);
		if (ratio > 1)
			ratio = 1;
		if (!motion->input.no_slide_physics)
	 		surf->traction = (1-ratio)*surf->traction + ratio * 0.001;
		surf->friction = (1-ratio)*surf->friction + ratio * 0.001;
		//surf->bounce = (1-ratio)*surf->bounce + ratio * 0.6;
	}
}

#define SURFACE_GROUND	0
#define SURFACE_AIR		1
#define SURFACE_ICE		2

void entWorldGetSurface(Entity *e, SurfaceParams *surf)
{
	#define GRAVITY 0.065
	static SurfaceParams defaultSurfs[] =
	{
		{ 1.00f,	0.45f,	0.01,	GRAVITY,	1.00f }, // default SURFACE_GROUND
		{ 0.02f,	0.01f,	0,		GRAVITY,	1.00f }, // default SURFACE_AIR
	};
	
	if (e->motion->falling || e->motion->input.flying)
		*surf = defaultSurfs[SURFACE_AIR];
	else
		*surf = defaultSurfs[SURFACE_GROUND];

	if (!e->motion->falling && !e->motion->input.flying)
		makeSteepSlopesSlippery(e->motion,surf);
}

SurfaceParams *entWorldGetSurfaceModifier(Entity *e)
{
	static SurfaceParams surf;
	
	MotionState* motion = e->motion;
	int inAir = motion->falling || motion->input.flying;

	surf = motion->input.surf_mods[inAir];

	if(motion->jumping || motion->input.flying)
	{
		surf.gravity = 0;
	}
	else if(inAir)
	{
		surf.friction = 1.0f;

		if(!motion->falling)
		{
			surf.traction = 1.0f;
		}
	}

	return &surf;
}

void entWorldApplySurfMods(SurfaceParams *surf_mod,SurfaceParams *surf)
{
	surf->friction	*= surf_mod->friction;
	surf->traction	*= surf_mod->traction;
	surf->bounce	*= surf_mod->bounce;
	surf->gravity	*= surf_mod->gravity;
	surf->max_speed *= surf_mod->max_speed;
}															  

void entWorldApplyTextureOverrides( SurfaceParams *	surf, int lastSurfFlags )
{
	if( lastSurfFlags && surf )  
	{
		if( lastSurfFlags & COLL_SURFACESLICK )
		{
			surf->friction = 0.01;
			surf->traction = 0.2;
		}
		if( lastSurfFlags & COLL_SURFACEBOUNCY )
		{
			surf->bounce = 0.5;
		}
		if( lastSurfFlags & COLL_SURFACEICY )
		{
			surf->friction = 0.0;
			surf->traction = 0.05;
		}
	}
}

static void checkMaxHeight(Entity *e, SurfaceParams* surf)
{
	F32		max_fly_height = scene_info.maxHeight;

	if(ENTPOSY(e) > max_fly_height)
	{
		Vec3 newpos;

		copyVec3(ENTPOS(e), newpos);
		vecY(newpos) = max_fly_height;
		entUpdatePosInterpolated(e, newpos);
		
		if (e->motion->vel[1] > 0)
			e->motion->vel[1] = 0;

		if(e->motion->jumping)
		{
			e->motion->jumping = 0; // hit max world height, turn gravity on.
			e->motion->falling = 1;
		}
	}
}

void entWorldMoveMe(Entity* e, SurfaceParams* surf)
{
	MotionState*	motion = e->motion;
	int				try_count = 0;
	Vec3			top;
	Vec3			bot;
	Vec3			test;
	int				ret;
	
	checkMaxHeight(e, surf);

	retry:
	try_count++;
	copyVec3(motion->last_pos, top);
	copyVec3(ENTPOS(e), bot);
	ret = SlideWall(e, top, bot, top);
	if(ret)
	{
		motion->stuck_head = STUCK_COMPLETELY;
		entUpdatePosInterpolated(e, motion->last_pos);
	}
	else
	{
		motion->stuck_head = 0;
		entUpdatePosInterpolated(e, top);
	}

	copyVec3(ENTPOS(e), test);

	CheckFeet(e, surf);
	
	if (fabs(ENTPOSY(e) - test[1]) > 0.00001)
	{
		if (CheckHead(e,0))
		{
			if (!surf->gravity && try_count == 1)
			{
				Vec3 newpos;

				copyVec3(ENTPOS(e), newpos);
				vecY(newpos) = motion->last_pos[1];
				entUpdatePosInterpolated(e, newpos);
				goto retry;
			}
			else
			{
 				if (motion->vel[1] > 0)
 				{
					motion->vel[1] = 0;
				}
				motion->stuck_head = STUCK_SLIDE;
			}
			entUpdatePosInterpolated(e, motion->last_pos);
		}
	}
	else if (motion->stuck_head == STUCK_COMPLETELY)
	{
		if (CheckHead(e,1))
		{
			copyVec3(ENTPOS(e),test);

			CheckFeet(e,surf);

			if (fabs(ENTPOSY(e) - test[1]) > 0.00001)
			{
				entUpdatePosInterpolated(e, motion->last_pos);
			}
		}
	}
}

void entWorldCollide(Entity* e, const Mat3 control_mat)
{
	Vec3			gravity_vec = {0};
	Vec3			last_slope;
	MotionState*	motion = e->motion;
	SurfaceParams	surf;
	SurfaceParams*	surf_mod;
	F32				max_speed;
	F32				friction_scale;
	F32				traction_scale;

	PERFINFO_AUTO_START("entWorldCollideTop", 1);

	copyVec3(motion->surf_normal, last_slope);

	entWorldGetSurface(e, &surf);
	surf_mod = entWorldGetSurfaceModifier(e);
	entWorldApplySurfMods(surf_mod, &surf);
	entWorldApplyTextureOverrides(&surf, motion->lastSurfFlags);
	
	if(surf.friction > 1)
		surf.friction = 1;

	if(surf.traction > 1)
		surf.traction = 1;
		
	// Check for forced low-traction.
	
	if(motion->low_traction_steps_remaining > 0)
	{
		if(surf.traction > 0.1)
		{
			surf.traction = 0.1;
		}

		if(surf.friction > 0.3)
		{
			surf.friction = 0.3;
		}
	}

	//TL! entWorldCollide Apply per frame traction loss and gravity gain
	if( motion->hitStumbleTractionLoss )      
	{
		F32 t = 1.0 - motion->hitStumbleTractionLoss;
		t = MINMAX( t, 0.0 , 1.0 );
		surf.traction *= t;

		if( !surf.gravity ) 
		{
			surf.gravity = 0.02 * motion->hitStumbleTractionLoss ;
		} 
	}
	if( !motion->falling &&  motion->hit_stumble_kill_velocity_on_impact )        
	{
		//FLYING OR RUNNING 
		scaleVec3( motion->vel, 0.2 * motion->hitStumbleTractionLoss, motion->vel );//TO DO base on strength of hit
		//zeroVec3( motion->vel );
		motion->hit_stumble_kill_velocity_on_impact = 0; //TO DO does this slow me down
	}
	


	// Integrate friction and traction over time.
	
	friction_scale = 1 - powf(1 - surf.friction, e->timestep);
	traction_scale = 1 - powf(1 - surf.traction, e->timestep);

	// Get the max speed.

	max_speed = surf.max_speed;
	
	if(*(int*)&motion->input.max_speed_scale)
	{
		max_speed *= motion->input.max_speed_scale;
	}

	// Get the gravity vector for this timestep.

	if(surf.gravity)
	{
		F32 gravity = 1.0 * (motion->vel[1] <= 0 ? surf.gravity * 1.5 : surf.gravity);
		F32	fall = 0.5 * gravity * SQR(e->timestep);

		//e->mat[3][1] -= fall;
		gravity_vec[1] = -gravity * e->timestep;
	}
	PERFINFO_AUTO_STOP();
	if(motion->on_surf)
	{
		Vec3	dv;
		Vec3	inp_vel = {0,0,0};
		F32*	surf_norm = motion->last_surf_normal;
		
		PERFINFO_AUTO_START("entWorldCollide if(motion->on_surf", 1);
		if (motion->last_surf_type == SURFTYPE_GROUND && surf_norm[1] < last_slope[1] && motion->vel[1] <= 0)
		{
			surf_norm = last_slope;
		}
		
		if(dotVec3(motion->prev_surf_normal, surf_norm) < -0.7)
		{
			// Make me able to jump out of the wedge.
			
			motion->jump_held = 1;
			motion->falling = 0;
			
			traction_scale = 1;
		}
		
		copyVec3(surf_norm, motion->prev_surf_normal);

		// Calculate the gravity component vector parallel to the surface.
		
		reflectOffPlane(gravity_vec, surf_norm, dv);
		addVec3(dv, gravity_vec, gravity_vec);
		
		if(	!motion->input.flying &&
			surf_norm[1] > 0.0f)
		{
			F32 scale;
			
			// Vertically project the input velocity onto the surface plane,
			//   scaled by the surface normal y-component.
			
			inp_vel[0] = motion->input.vel[0] * surf_norm[1];
			inp_vel[1] = -(surf_norm[0] * motion->input.vel[0] + surf_norm[2] * motion->input.vel[2]);
			inp_vel[2] = motion->input.vel[2] * surf_norm[1];
			
			scale = lengthVec3XZ(motion->input.vel) * traction_scale;
			
			if(inp_vel[1] < 0.0f)
			{
				F32 inp_vel_len = lengthVec3(inp_vel);
				F32 input_vel_len = lengthVec3XZ(motion->input.vel);
				
				// Going downhill, so scale the inp_vel to the length of the input.
				
				scale *= input_vel_len / inp_vel_len;

				if(	!vec3IsZero(inp_vel) &&
					!vec3IsZero(gravity_vec))
				{
					// Speed up when going downhill.
					
					F32 diff = 1 - surf_norm[1];
					
					diff *= dotVec3(gravity_vec, inp_vel) / (lengthVec3(gravity_vec) * inp_vel_len);
					
					max_speed /= max(0.1, 1 - diff);
				}
			}

			scaleVec3(inp_vel, scale, inp_vel);
		}
		else
		{
			F32 scale = lengthVec3(motion->input.vel) * traction_scale;

			scaleVec3(motion->input.vel, scale, inp_vel);
		}

		scaleVec3(gravity_vec, 0.5 * min(1, 4 * (1 - traction_scale)), gravity_vec);
		
		// Project the current velocity onto the surface, and multiply by bounce.
		
		{
			Mat3 surf_mat;
			Mat3 surf_mat_inv;
			Vec3 xvel;
			Vec3 dvx;
			Vec3 up = {0,0,1};

			// Create matrix with z pointing out of the surface and x-vector on the xz-plane.
			
			camLookAt(surf_norm, surf_mat);
			
			// Create the transpose to convert from world-space to surf_mat-space.
			
			transposeMat3Copy(surf_mat, surf_mat_inv);
			
			// Get the velocity in surf_mat space.
			
			mulVecMat3(motion->vel, surf_mat_inv, xvel);
			
			if(xvel[2] <= 0)
			{
				// Reflect the velocity off the surface.
				
				reflectOffPlane(xvel, up, dvx);
				
				// Multiply by the bounce.
				dvx[2] *= surf.bounce; 

				//If friction is low, don't lose the velocity.  I doubt (1.0 - friction) is the right rate of loss, but
				//it will do for now...
				if( 0 && !nearSameVec3( unitmat[3], motion->vel ) )
				{
					F32 oldVel, newVel, lostVel, velToRestore, scaleToRestore;
 
					oldVel = lengthVec3( motion->vel );
					newVel = lengthVec3( dvx );
					lostVel = oldVel - newVel;
					velToRestore = lostVel * (1.0 - surf.friction);
					scaleToRestore = (newVel + velToRestore) / newVel;
					scaleVec3( dvx, scaleToRestore, dvx );
				}

				// Convert the reflected velocity back to world space.
				
				mulVecMat3(dvx, surf_mat, dv);
			}
			else
			{
				copyVec3(motion->vel, dv);
			}
		}
		
		// Calculate the friction and traction.
		
		{
			S32		no_input = vec3IsZero(inp_vel);
			F32		scale = no_input ? 0 : dotVec3(inp_vel, dv);
			Vec3	friction_dir;
			
			if(!motion->falling && !motion->input.flying)
			{
				motion->lastGroundSurfFlags = motion->lastSurfFlags;
				motion->was_on_surf = 1;
			}

			if(!no_input && scale >= 0)
			{
				F32		inp_vel_mag_SQR = lengthVec3Squared(inp_vel);
				Vec3	projected;
				Vec3	perp_friction_dir;
				int		clamp_max;
				F32		clamp_max_value;
				F32		perp_friction_scale;
				
				// Get the perpedicular friction direction.
				
				perp_friction_scale = friction_scale + traction_scale * 0.5;
				if(perp_friction_scale > 1)
					perp_friction_scale = 1;
				
				scaleVec3(dv, -perp_friction_scale, friction_dir);
				scale = dotVec3(inp_vel, friction_dir) / inp_vel_mag_SQR;
				scaleVec3(inp_vel, scale, projected);
				subVec3(friction_dir, projected, perp_friction_dir);
				addVec3(dv, perp_friction_dir, dv);
				
				// Get the projected current velocity after friction was applied.
				
				scale = dotVec3(inp_vel, dv) / inp_vel_mag_SQR;
				scaleVec3(inp_vel, scale, projected);
				
				// If projected current velocity is faster than max_speed, slow me down.
				
				if(lengthVec3Squared(projected) <= SQR(max_speed))
				{
					clamp_max = 1;
				}
				else
				{
					clamp_max = 0;
					clamp_max_value = lengthVec3(projected);
				}
				
				// Add in the input velocity.
				
				addVec3(dv, inp_vel, dv);

				// Get the projected current velocity again.
				
				scale = dotVec3(inp_vel, dv) / inp_vel_mag_SQR;
				scaleVec3(inp_vel, scale, projected);
				
				scale = lengthVec3Squared(projected);
				
				if(scale > SQR(max_speed))
				{
					// Remove the current projected velocity.
					
					subVec3(dv, projected, dv);
					
					scale = sqrt(scale);

					if(clamp_max)
					{
						// Wasn't going fast enough before, so clamp speed.
						
						scale = max_speed / scale;
					}
					else
					{
						// Reduce the current overflow-velocity by the friction coefficient.
						
						scale = ((clamp_max_value - max_speed) * (1 - friction_scale) + max_speed) / scale;
					}

					// Add the projected velocity back in.
					
					scaleVec3(projected, scale, projected);
					addVec3(dv, projected, dv);
				}
			}
			else
			{
				if(!no_input)
				{
					friction_scale += traction_scale * 0.5;
					if(friction_scale > 1)
						friction_scale = 1;
				}
				
				scaleVec3(dv, -friction_scale, friction_dir);
				addVec3(dv, inp_vel, dv);
				addVec3(dv, friction_dir, dv);
			}
			
			addVec3(dv, gravity_vec, dv);
		}

		//#if CLIENT
		//{
		//	// Draw some stuff.
		//	
		//	Vec3 target1;
		//	addVec3(motion->last_pos, gravity_vec, target1);
		//	entDebugAddLine(motion->last_pos, 0xffffffff, target1, 0xffff7700);
		//	addVec3(motion->last_pos, inp_vel, target1);
		//	entDebugAddLine(motion->last_pos, 0xffffffff, target1, 0xff0077ff);
		//	addVec3(motion->last_pos, motion->inp_vel, target1);
		//	entDebugAddLine(motion->last_pos, 0xffffffff, target1, 0xff00ff77);
		//	addVec3(motion->last_pos, motion->vel, target1);
		//	entDebugAddLine(motion->last_pos, 0xffffffff, target1, 0xffff0000);
		//}
		//#endif

		copyVec3(dv, motion->vel);

		//#if MDBG
		//	xyprintf(5,17,"vel %f %f %f",motion->vel[0],motion->vel[1],motion->vel[2]);
		//#endif

		//if(0){
		//	int moved = fabs(e->mat[3][1] - motion->last_pos[1]) > 0.00001;

		//	if (last_slope[1] == surf_norm[1] && (1-last_slope[1]) >= SLOPE_START_SLIDE && !moved && motion->vel[1] < -0.001)
		//	{
		//		motion->on_poly_edge = 1;
		//	}
		//	else if ((1-last_slope[1]) < SLOPE_START_SLIDE)
		//	{
		//		motion->on_poly_edge = 0;
		//	}
		//}
		PERFINFO_AUTO_STOP();
	}
	else
	{
		#define IGNORED_SURFACES (COLL_SURFACESLICK | COLL_SURFACEBOUNCY | COLL_SURFACEICY)
		
		static int debugBranchBitfield; // temporary debugging information to track down the crazy velocity bug.
		Vec3	dv;
		F32		scale;
		Vec3	friction_dir;
		F32		dv1;
		Vec3	inp_vel = {0,0,0};

		debugBranchBitfield = 0;

		PERFINFO_AUTO_START("entWorldCollide else", 1);
		if(motion->was_on_surf && !(motion->lastGroundSurfFlags & IGNORED_SURFACES) && motion->vel[1] > 0)
		{
			debugBranchBitfield |= 1 << 0;

			if (!motion->jump_held)
			{
				debugBranchBitfield |= 1 << 1;
				motion->vel[1] = 0;
			}
			else if (motion->vel[1] > 1)
			{
				debugBranchBitfield |= 1 << 2;
				motion->vel[1] = 1;
			}
		}
		
		motion->was_on_surf = 0;

		zeroVec3(motion->prev_surf_normal);
		
		motion->on_poly_edge = 0;
		
		if(motion->input.flying)
		{
			debugBranchBitfield |= 1 << 3;
			scale = lengthVec3(motion->input.vel) * traction_scale;
			scaleVec3(motion->input.vel, scale, inp_vel);

			copyVec3(motion->vel, dv);
			dv1 = 0;
		}
		else
		{
			debugBranchBitfield |= 1 << 4;
			scale = lengthVec3XZ(motion->input.vel) * traction_scale;
			scaleVec3(motion->input.vel, scale, inp_vel);

			dv1 = motion->vel[1];

			dv[0] = motion->vel[0];
			dv[1] = 0;
			dv[2] = motion->vel[2];
			
			inp_vel[1] = 0;
		}
		
		// Calculate the friction and traction.
		
		{
			S32		no_input = vec3IsZero(inp_vel);
			
			scale = no_input ? 0 : dotVec3(inp_vel, dv);
		
			if(!no_input && scale >= 0)
			{
				F32		inp_vel_mag_SQR = lengthVec3Squared(inp_vel);
				Vec3	projected;
				Vec3	perp_friction_dir;
				int		clamp_max;
				F32		clamp_max_value;
				F32		perp_friction_scale;
				
				debugBranchBitfield |= 1 << 5;

				// Get the perpedicular friction direction.
				
				perp_friction_scale = friction_scale + traction_scale * 0.5;
				if (perp_friction_scale > 1)
				{
					debugBranchBitfield |= 1 << 6;
					perp_friction_scale = 1;
				}

				scaleVec3(dv, -perp_friction_scale, friction_dir);
				scale = dotVec3(inp_vel, friction_dir) / inp_vel_mag_SQR;
				scaleVec3(inp_vel, scale, projected);
				subVec3(friction_dir, projected, perp_friction_dir);
				addVec3(dv, perp_friction_dir, dv);
				
				// Get the projected current velocity after friction was applied.
				
				scale = dotVec3(inp_vel, dv) / inp_vel_mag_SQR;
				scaleVec3(inp_vel, scale, projected);
				
				// If projected current velocity is faster than max_speed, slow me down.
				
				if(lengthVec3Squared(projected) <= SQR(max_speed))
				{
					debugBranchBitfield |= 1 << 7;
					clamp_max = 1;
				}
				else
				{
					debugBranchBitfield |= 1 << 8;
					clamp_max = 0;
					clamp_max_value = lengthVec3(projected);
				}
				
				// Add in the input velocity.
				
				addVec3(dv, inp_vel, dv);

				// Get the projected current velocity again.
				
				scale = dotVec3(inp_vel, dv) / inp_vel_mag_SQR;
				scaleVec3(inp_vel, scale, projected);
				
				scale = lengthVec3Squared(projected);
				
				if(scale > SQR(max_speed))
				{
					// Remove the current projected velocity.
					
					debugBranchBitfield |= 1 << 9;

					subVec3(dv, projected, dv);
					
					scale = sqrt(scale);

					if(clamp_max)
					{
						// Wasn't going fast enough before, so clamp speed.
						
						debugBranchBitfield |= 1 << 10;
						scale = max_speed / scale;
					}
					else
					{
						// Reduce the current overflow-velocity by the friction coefficient.
						
						debugBranchBitfield |= 1 << 11;
						scale = ((clamp_max_value - max_speed) * (1 - friction_scale) + max_speed) / scale;
					}

					// Add the projected velocity back in.
					
					scaleVec3(projected, scale, projected);
					addVec3(dv, projected, dv);
				}
			}
			else
			{
				debugBranchBitfield |= 1 << 12;
				if(!no_input)
				{
					debugBranchBitfield |= 1 << 13;
					friction_scale += traction_scale * 0.5;
					if (friction_scale > 1)
					{
						debugBranchBitfield |= 1 << 14;
						friction_scale = 1;
					}
				}
				
				scaleVec3(dv, -friction_scale, friction_dir);
				addVec3(dv, inp_vel, dv);
				addVec3(dv, friction_dir, dv);
			}
		}
				
		// Add in the gravity vector.

		addVec3(dv, gravity_vec, dv);
		
		// Copy back to vel.

		dv[1] += dv1;
		copyVec3(dv, motion->vel);
		PERFINFO_AUTO_STOP();
	}
	PERFINFO_AUTO_START("entWorldCollideBottom", 1);
	//printf("%f %f speed %f slope %f  type %d  d %f\n",e->mat[3][0],e->mat[3][1],lengthVec3(motion->vel),last_surf.ctri ? last_surf.mat[1][1] : 2,last_surf_type,distance3(e->motion->last_pos,e->mat[3]));
	//printf("gravity_vec[1]: %f: %f + %f = %f\n", e->timestep, gravity_vec[1], motion->vel[1], motion->vel[1] + gravity_vec[1]);

	#if CLIENT
	{
		Vec3 target1;
		addVec3(motion->last_pos, gravity_vec, target1);
		//entDebugAddLine(motion->last_pos, 0xffffffff, target1, 0xff0077ff);
	}
	#endif

	if(motion->jumping)
	{
		motion->vel[1] = motion->input.vel[1];
	}

	// Do entity collisions.

	{
		Vec3 add_vel, newpos;
		scaleVec3(motion->vel, e->timestep, add_vel);
		addVec3(motion->last_pos, add_vel, newpos);
		entUpdatePosInterpolated(e, newpos);
	}

	PERFINFO_AUTO_START("checkEntColl",1);
	checkEntColl(e, 1, control_mat);
	PERFINFO_AUTO_STOP();
	//printf("moving: %f\t(%1.3f\t%1.3f\t%1.3f)\n", lengthVec3(add_vel), vecParamsXYZ(add_vel));
	PERFINFO_AUTO_START("entWorldMoveMe",1);
	entWorldMoveMe(e, &surf);
	PERFINFO_AUTO_STOP();
	if(!motion->input.flying && motion->vel[1] < 0 && sameVec3(ENTPOS(e), motion->last_pos) && motion->last_surf_normal[1] < 0.5)
	{
		motion->jump_held = 0;
		setVec3(motion->surf_normal, 0, 1, 0);
		motion->on_surf = 1;
		motion->falling = 0;
		motion->vel[1] = 0;
	}
	else
	{
		motion->on_surf = last_surf.ctri ? 1 : 0;
		copyVec3(last_surf.mat[1], motion->last_surf_normal);
	}
		
	copyVec3(ENTPOS(e), motion->last_pos);

	//if(!motion->jump_not_landed && motion->vel[1] > 0)
	//	motion->vel[1] = 0;

	if(last_surf.ctri)
	{
		motion->lastSurfFlags = last_surf.ctri->flags;
		
		//printf(	"tri: (%f,%f,%f), (%f,%f), (%f,%f)\n",
		//		vecParamsXYZ(last_surf.ctri->V1),
		//		last_surf.ctri->v2[0], last_surf.ctri->v2[1], 
		//		last_surf.ctri->v3[0], last_surf.ctri->v3[1]);
	}
	else
	{
		motion->lastSurfFlags = 0;
	}

	#if MDBG
		xyprintf(4,18,"fall %d gravity_vec %f %f %f",e->motion->falling,gravity_vec[0],gravity_vec[1],gravity_vec[2]);
	#endif

	PERFINFO_AUTO_STOP();
}
