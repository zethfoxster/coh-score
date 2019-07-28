#include "model.h"
#include <stdio.h>
#include "gfxwindow.h"
#include "memcheck.h" 
#include "render.h"
#include "camera.h"
#include "rendershadow.h"
#include "assert.h"
#include "cmdgame.h"
#include "model.h"
#include "model_cache.h"
#include "gridcoll.h"
#include "font.h"
#include "splat.h"
#include "cmdgame.h"
#include "fxutil.h"
#include "tex.h"
#include "gfx.h"
#include "groupdraw.h"
#include "renderWater.h"
#include "rt_water.h"
#include "win_init.h"
#include "GenericPoly.h"
#include "sun.h"


void modelDrawShadowVolume(Model *model,Mat4 mat,int alpha,int shadow_mask, GfxNode * node)
{
	RdrStencilShadow	*rs;

	//assert(0); //this is disabled right now
	if ( !model || game_state.shadowvol==2 || scene_info.stippleFadeMode || g_sun.shadowcolor[3] < MIN_SHADOW_WORTH_DRAWING )
		return;
	if( node ) //Player shadows are no longer stenciled (easy enough to reenable )
		return;
	if ((model->loadstate & LOADED) == 0) //background loader isn't ready.
		return;

	modelSetupVertexObject(model, VBOS_DONTUSE, BlendMode(0,0));

	//makeShadowVolumeFromPlaneObject(Vec3 vbuf, int tris[], int * tri_count_ptr, Mat4 mat, ObjectInfo * model)
	//(If it's something you want to orient to face the sun like a tree shadow)

	//#####Orient the shadow object toward the sun if required
	if (model->trick && model->trick->flags1 & TRICK_LIGHTFACE)
	{
		F32		yaw,yaw0;
		Mat4	matx;
		Vec3	dv;

		copyVec3(g_sun.shadow_direction,dv);
		dv[2] = -dv[2];
		mulMat3(cam_info.inv_viewmat,mat,matx);
		yaw0 = getVec3Yaw(matx[2]);
		yaw = -yaw0 - getVec3Yaw(dv);
		yawMat3(yaw, mat);
	}
	rs = rdrQueueAlloc(DRAWCMD_STENCILSHADOW,sizeof(*rs));
	//##### Extrude the shadow model
	if (model->trick && model->trick->flags2 & TRICK2_CASTSHADOW) 
		rs->dist = model->trick->info->shadow_dist;
	else
		rs->dist = 2.0 * model->radius; 

	{
		Mat4	matx;
		Vec3	offset;

		scaleVec3(g_sun.shadow_direction,rs->dist,offset);
		mulMat4(cam_info.inv_viewmat,mat,matx);
		transposeMat3(matx);
		mulVecMat3(offset,matx,rs->offsetx);
	}

	rs->vbo			= model->vbo;
	rs->alpha		= alpha;
	rs->shadow_mask	= shadow_mask;
	copyMat4(mat,rs->mat);
	rdrQueueSend();
}

int shadowVolumeVisible(Vec3 mid,F32 radius,F32 shadow_dist)
{
Vec3	dv,dvx;
F32		d;

	if (game_state.shadowvol==2 || scene_info.stippleFadeMode)
		return 0;
	scaleVec3(g_sun.shadow_direction_in_viewspace,radius,dvx);
	addVec3(dvx,mid,dv);
	for(d=0;d<shadow_dist;d+=radius)
	{
		if (gfxSphereVisible(dv,radius))
			return 1;
		addVec3(dvx,dv,dv);
	}
	return 0;
}

void shadowStartScene()
{
Vec3	dir;
F32		mag;

#define MAX_Y 0.7f

	subVec3(zerovec3,g_sun.direction,dir);
	mag = sqrt(dir[0]*dir[0] + dir[2]*dir[2]);
	if (mag > MAX_Y)
		mag = MAX_Y;
	if (fabs(dir[1]) < MAX_Y)
	{
		if (dir[1] < 0)
			dir[1] = -MAX_Y;
		else
			dir[1] = MAX_Y;
	}
	if (dir[1] < 0)
		dir[1] = -mag;
	else
		dir[1] = mag;

	normalVec3(dir);
	copyVec3(dir,g_sun.shadow_direction);
	mulVecMat3(g_sun.shadow_direction,cam_info.viewmat,g_sun.shadow_direction_in_viewspace);
}



void shadowFinishScene()
{
	if ( game_state.shadowvol==2 || scene_info.stippleFadeMode || g_sun.shadowcolor[3] < MIN_SHADOW_WORTH_DRAWING )
		return;
	rdrQueueCmd(DRAWCMD_SHADOWFINISHFRAME);
}

// ##########################################################################
// ############ Draw splats

int splatShadowsDrawn;

void modelDrawShadowObject( Mat4 viewspace, Splat * splat )
{
	Splat	*pkg;
	U8		*mem;
	int		tri_bytes,vert_bytes,st_bytes,color_bytes;
	int		tex_ids[2];

	splatShadowsDrawn++;
#ifndef FINAL
	if( game_state.simpleShadowDebug )
	{
		xyprintf( 40, splatShadowsDrawn + 10, "Tris %d", splat->triCnt ); 
		xyprintf( 55, splatShadowsDrawn + 10, "Verts %d", splat->vertCnt );  
	}
#endif

	if( splat->invertedSplat )
	{	
		modelDrawShadowObject( viewspace, splat->invertedSplat );
	}
	tri_bytes	= splat->triCnt * sizeof(splat->tris[0]);
	vert_bytes	= splat->vertCnt * sizeof(splat->verts[0]);
	st_bytes	= splat->vertCnt * sizeof(splat->sts[0]);
	color_bytes	= splat->vertCnt * 4 * sizeof(splat->colors[0]);

	tex_ids[0] = texDemandLoad(splat->texture1);
	tex_ids[1] = texDemandLoad(splat->texture2);
	pkg = rdrQueueAlloc(DRAWCMD_SPLAT,sizeof(Splat) + tri_bytes + vert_bytes + color_bytes + st_bytes * 2 + sizeof(viewspace[3][2]) + sizeof(splatShadowsDrawn));
	*pkg = *splat;
	pkg->texid1 = tex_ids[0];
	pkg->texid2 = tex_ids[1];

	mem = (U8*) (pkg + 1);

	memcpy(mem,splat->tris,tri_bytes);
	mem += tri_bytes;

	memcpy(mem,splat->verts,vert_bytes);
	mem += vert_bytes;

	memcpy(mem,splat->sts,st_bytes);
	mem += st_bytes;

	memcpy(mem,splat->sts2,st_bytes);
	mem += st_bytes;

	memcpy(mem,splat->colors,color_bytes);
	mem += color_bytes;

	*((F32 *)mem) = viewspace[3][2];
	mem += sizeof(viewspace[3][2]);

	*((int *)mem) = splatShadowsDrawn;

	rdrQueueSend();
}
