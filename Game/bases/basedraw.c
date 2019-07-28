/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "mathutil.h"
#include "bases.h"
#include "group.h"
#include "camera.h"
#include "utils.h"
#include "basedraw.h"
#include "timing.h"
#include "renderprim.h"
#include "gfx.h"
#include "groupdraw.h"
#include "StashTable.h"
#include "model_cache.h"
#include "groupdrawutil.h"
#include "baseedit.h"
#include "basedata.h"
#include "rendertree.h"
#include "groupThumbnail.h"
#include "grouptrack.h"
#include "cmdgame.h"
#include "groupdrawinline.h"
#include "groupMiniTrackers.h"
#include "uiBaseInput.h"

static int clip_height[2];
static int fade_timer[2];

static BaseRoom *curr_room;
static GroupDef *curr_room_def;

static DefTracker light_cache[32];

int getEdgeInfo(Mat4 mat,int *left_side,GroupDef *def,BaseRoom *room)
{
	int xedges[2],zedges[2],trimsize=2;
	int xedge,zedge,cam_edge[2] = {-1,-1}, obj_edge[2] = {-1,-1};
	Mat4 abs_mat;
	Vec3 room_pos,abs_pos,cam_pos;

	*left_side = 1;
	if (!room)
		return 0;
	mulMat4(cam_info.inv_viewmat,mat,abs_mat);
	copyVec3(abs_mat[3],abs_pos);
	copyVec3(cam_info.cammat[3],cam_pos);

	room_pos[0] = abs_pos[0] - room->pos[0];
	room_pos[2] = abs_pos[2] - room->pos[2];

	xedges[0] = zedges[0] = trimsize;
	xedges[1] = room->size[0] * room->blockSize - trimsize;
	zedges[1] = room->size[1] * room->blockSize - trimsize;

	xedge = room_pos[0] >= xedges[1] || room_pos[0] <= xedges[0];
	zedge = room_pos[2] >= zedges[1] || room_pos[2] <= zedges[0];
	if (!xedge && !zedge)
		return 0;
	obj_edge[0] = room_pos[0] >= xedges[1];
	obj_edge[1] = room_pos[2] >= zedges[1];
	if (cam_pos[0] > room->pos[0] + xedges[1] + trimsize)
	{
		if (room_pos[0] >= xedges[1])
		{
			if (abs_mat[2][2] < 0.5)
				*left_side = 0;
			cam_edge[0] = 1;
		}
	}
	if (cam_pos[0] < room->pos[0])
	{
		if (room_pos[0] <= xedges[0])
		{
			if (abs_mat[2][2] > -0.5)
				*left_side = 0;
			cam_edge[0] = 0;
		}
	}
	if (cam_pos[2] > room->pos[2] + zedges[1] + trimsize)
	{
		if (room_pos[2] >= zedges[1])
		{
			if (abs_mat[0][0] < -0.5)
				*left_side = 0;
			cam_edge[1] = 1;
		}
	}
	if (cam_pos[2] < room->pos[2])
	{
		if (room_pos[2] <= zedges[0])
		{
			if (abs_mat[0][0] > 0.5)
				*left_side = 0;
			cam_edge[1] = 0;
		}
	}
	if (cam_edge[0] >= 0 || cam_edge[1] >= 0)
	{
		if (cam_edge[0] == obj_edge[0] && cam_edge[1] == obj_edge[1])
			*left_side = 2;
		return 1;
	}
	return 0;
}

int rejectByCameraHeight(GroupDef *def,int *which)
{
	if (cam_info.cammat[3][1] < 0)
	{
		*which = 0;
		if (strstriConst(def->name,"trimlow"))
			return 1;
		if (strstriConst(def->name,"wall_lower"))
			return 1;
	}
	if (cam_info.cammat[3][1] > 48)
	{
		*which = 1;
		if (strstriConst(def->name,"trimhi"))
			return 1;
		if (strstriConst(def->name,"wall_upper"))
			return 1;
	}
	return 0;
}

BaseRoom *roomFindByName(const char *name)
{
	int i,id;

	id = atoi(name + strlen("grproom___"));
	for(i=0;i<eaSize(&g_base.rooms);i++)
	{
		if (g_base.rooms[i]->id == id)
			return g_base.rooms[i];
	}
	return 0;
}

void baseHackeryFreeCachedRgbs(void)
{
	int i;
	for(i=0;i<ARRAY_SIZE(light_cache);i++)
	{
		DefTracker *curr = &light_cache[i];
		if (curr->src_rgbs)
		{
			int j;
			for (j=0;j<eaSize(&curr->src_rgbs);j++)
				modelFreeRgbBuffer(curr->src_rgbs[j]);
			eaDestroy(&curr->src_rgbs);
			curr->src_rgbs = 0;
		}
	}
}

GroupDef *baseHackery(GroupDef *def,Mat4 mat,Vec3 mid,int *alpha,DefTracker *light_trkr,DefTracker **tracker_p)
{
	int which;
	GroupDef  *room_def;
#define FADETIME 0.3

	if (!light_trkr)
		goto returndef;

	room_def = light_trkr->def;
	if (room_def != curr_room_def)
	{
		curr_room_def = room_def;
		curr_room = roomFindByName(room_def->name);
	}
	if (rejectByCameraHeight(def,&which))
	{
		F32 elapsed;

		clip_height[which] = 1;
		elapsed = timerElapsed(fade_timer[which]);
		if (elapsed >= FADETIME)
		{
			def = 0;
			goto returndef;
		}
		*alpha = (FADETIME - elapsed)/FADETIME * 255;
	}
	if (strstriConst(def->name,"trim"))
	{
		int left_side;

		if (getEdgeInfo(mat,&left_side,def,curr_room))
		{
			char buf[200],*s,endchars[60];

			strcpy(buf,def->name);
			s = strstri(buf,"90in");
			if (s && left_side < 2)
			{
				strcpy(endchars,s+4);
				if (!left_side)
				{
					strcpy(s,"Str_EndR");
					strcat(s,endchars);
					yawMat3(RAD(-90),mat);
				}
				else
				{
					strcpy(s,"Str_EndL");
					strcat(s,endchars);
				}
				def = groupDefFind(buf);
				if (!*tracker_p)
					goto returndef;
				{
					U32 i;
					U32 hash;
					DefTracker *curr,*tracker = *tracker_p;

					hash = 0;//hashCalc((void*)&tracker, sizeof(tracker), 0xa74d94e3);

					for(i=0;i<ARRAY_SIZE(light_cache);i++)
					{
						curr = &light_cache[(i + hash) & ARRAY_SIZE(light_cache)-1];
						if (curr->src_rgbs && curr->def == def && sameVec3(curr->mat[3],tracker->mat[3])
							&& curr->mod_time == group_info.ref_mod_time)
							goto success;
						if (!curr->def_mod_time)
							break;
					}
					if (i >= ARRAY_SIZE(light_cache))
						goto returndef; // light cache full
					if (curr->src_rgbs)
					{
						int i;
						for (i=0;i<eaSize(&curr->src_rgbs);i++)
							modelFreeRgbBuffer(curr->src_rgbs[i]);
						eaDestroy(&curr->src_rgbs);
						curr->src_rgbs = 0;
					}

					*curr = *tracker;
					curr->def = def;
					curr->mod_time = group_info.ref_mod_time;
					colorTracker(curr,mat,light_trkr,1, 1.f);

					success:
					curr->def_mod_time = 2;
					*tracker_p = curr;
					goto returndef;
				}
			}
			def = 0;
			goto returndef;
		}
	}

returndef:
	// Draw the currently selected room decor
	if(def && g_iDecor>=0
		&& g_refCurRoom.p == curr_room 
		&& strstriConst(def->name,roomdecor_names[g_iDecor]))
	{
		F32 elapsed = timerElapsed(g_iDecorFadeTimer);
		
		if(elapsed < DECOR_FADE_TOTAL)
		{
			if(strnicmp(def->name, "_base_",6)==0)
			{
				char *pos;
				pos = strchr(def->name+6, '_');
				if(pos)
				{
					GroupDef *def;
					char selector_name[256];

					strcpy(selector_name, "_base_select");
					strcat(selector_name, pos);

					def = groupDefFind(selector_name);
					if(def)
					{
						int alphaSelect = *alpha;

						if(elapsed<DECOR_FADE_IN)
							alphaSelect = 255 - ((DECOR_FADE_IN - elapsed)/DECOR_FADE_IN * 255);
						else if(elapsed>DECOR_FADE_IN+DECOR_FADE_HOLD)
							alphaSelect = (DECOR_FADE_OUT - (elapsed-(DECOR_FADE_IN+DECOR_FADE_HOLD)))/DECOR_FADE_OUT * 255;

						avsn_params.model = def->model;
						avsn_params.mat = mat;
						avsn_params.mid = mid;
						avsn_params.alpha = alphaSelect;
						avsn_params.materials = def->model->tex_binds;
						avsn_params.brightScale = 1.f;
						avsn_params.from_tray = 1;	// don't use BMB_SHADOWMAP on base defs
						addViewSortNodeWorldModel();
					}
				}
			}
		}
	}

	return def;
}

void baseStartDraw(void)
{
	curr_room_def = 0;
	curr_room = 0;
}

void baseDrawFadeCheck(void)
{
	int i;

	if (!fade_timer[0])
	{
		fade_timer[0] = timerAlloc();
		fade_timer[1] = timerAlloc();
	}
	for(i=0;i<2;i++)
	{
		if (!clip_height[i])
			timerStart(fade_timer[i]);
		clip_height[i] = 0;
	}
	for(i=0;i<ARRAY_SIZE(light_cache);i++)
	{
		if (light_cache[i].def_mod_time)
			light_cache[i].def_mod_time--;
	}
}

void drawDefSimple(GroupDef *def,Mat4 mat,Color color[2])
{
	Mat4 matx,viewmat;
	DefTracker *tracker = getCachedDefTracker(def, mat);

	gfxSetViewMat(cam_info.cammat,viewmat,0);
	mulMat4(viewmat,mat,matx);

	if(color)
	{
		def->has_tint_color = 1;
		memcpy(def->color, color, 4*2);
	}

	if (tracker) {
		copyMat4(mat, tracker->mat);
		trackerUpdate(tracker, def, 0);
		tracker->def_mod_time = def->mod_time;
	}
	{
		EntLight * light = NULL;
		FxMiniTracker * fxminitracker = NULL;
		F32 viewScale = 1;
		DefTracker *lighttracker = NULL;
		DrawParams	draw = {0};
		GroupEnt	ent = {0};
		GroupDraw	*gd = &group_draw;

		draw.view_scale = viewScale;
		draw.light_trkr = lighttracker;
		if (color)
		{
			draw.alpha=(double)((color[0].integer & 0xff000000) >> 24)/(double)(0xff);
			if (draw.alpha==0)
				draw.alpha=1e-5;
		} else
			draw.alpha=1;
		gd->globFxLight = light; //only give the light if light->used, cause I never check it in modelDrawX
		gd->globFxMiniTracker = fxminitracker;

		groupDrawPushMiniTrackers();
		groupDrawBuildMiniTrackersForDef(def, 0);

		group_draw.do_welding = 0;

		makeAGroupEnt( &ent, def, unitmat, draw.view_scale, 0 );
		drawDefInternal(&ent, matx, tracker, VIS_DRAWALL, &draw, 0);

		groupDrawPopMiniTrackers();

		gd->globFxLight = 0;
		gd->globFxMiniTracker = 0;
	}
	if(color)
		def->has_tint_color = 0;
}

void drawScaledBox(Mat4 mat,Vec3 size,GroupDef *box,Color tint[2])
{
	Vec3 scalevec;
	Mat4 boxmat;
	int i;

	if (!box)
	{
		char buff[256];
		sprintf( buff, "_Bases_ground_mount_%i", g_base.roomBlockSize );
		box = groupDefFindWithLoad(buff);
	}
	if (!box)
		return;
	for(i=0;i<3;i++)
		scalevec[i] = (size[i]) / (box->max[i] - box->min[i]);

	scaleMat3Vec3Xfer(mat,scalevec,boxmat);
	copyVec3(mat[3],boxmat[3]);
	boxmat[3][1] -= size[1] / 2;
	drawDefSimple(box,boxmat,tint);
}

void drawBoxExtents(Vec3 pos,Vec3 min,Vec3 max, TintedGroupDef *box)
{
	Vec3 size,half;
	Mat4 mat;
	unsigned char color[4] = { 255, 0, 0, 255 };

	subVec3(max,min,size);
	scaleVec3(size,0.5,half);
	addVec3(min,half,mat[3]);
	//addVec3(pos,mat[3],mat[3]);
	copyMat3(unitmat,mat);

	drawScaledBox(mat,size,box->pDef,box->tint);
}

void drawDetailBox(RoomDetail *detail, Mat4 mat, Mat4 surface, TintedGroupDef *box, TintedGroupDef *box_volume, bool editor)
{
	Vec3 min,max,vol_min,vol_max;
	int i;

	if (!detail)
		return;

	if (editor)
	{
		DetailEditorBounds e;
		if (!baseDetailEditorBounds(&e, detail, mat, surface, baseEdit_state.gridsnap, true))
			return;
		copyVec3(e.min, min);
		copyVec3(e.max, max);
		copyVec3(e.vol_min, vol_min);
		copyVec3(e.vol_max, vol_max);
	} else {
		DetailBounds e;
		if (!baseDetailBounds(&e, detail, mat, true))
			return;
		copyVec3(e.min, min);
		copyVec3(e.max, max);
		copyVec3(e.vol_min, vol_min);
		copyVec3(e.vol_max, vol_max);
	}

	for( i = 0; i < 3; i++ ) // prevent annoying z-flicker on coincident surfaces
	{                        // stupid, but works
		min[i] -= .01f;
		max[i] += .01f;
	}
	drawBoxExtents(mat[3],min,max,box);

	if (detail->info->bHasVolumeTrigger && box_volume)
	{
		for( i = 0; i < 3; i++ ) // prevent annoying z-flicker on coincident surfaces
		{                        // stupid, but works
			vol_min[i] -= .01f;
			vol_max[i] += .01f;
		}
		drawBoxExtents(mat[3],vol_min,vol_max,box_volume);
	}
}

void drawScaledObject(TintedGroupDef *box, Mat4 mat,Vec3 size)
{
	Vec3 scalevec;
	Mat4 boxmat;
	int i;

	for(i=0;i<3;i++)
		scalevec[i] = (size[i]) / (box->pDef->max[i] - box->pDef->min[i]);

	scaleMat3Vec3Xfer(mat,scalevec,boxmat);
	copyVec3(mat[3],boxmat[3]);

	drawDefSimple(box->pDef,boxmat,box->tint);
}

/* End of File */
