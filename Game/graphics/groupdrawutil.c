/***************************************************************************
 *     Copyright (c) 2001-2005, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
*/
#include "utils.h"
#include "timing.h"
#include "crypt.h"
#include "light.h"
#include "groupdraw.h"
#include "sun.h"
#include "grouputil.h"
#include "renderUtil.h"
#include "camera.h"
#include "model_cache.h"
#include "fx.h"
#include "tricks.h"
#include "groupfileload.h"
#include "gfxLoadScreens.h"
#include "fxgeo.h"
#include "groupdrawutil.h"
#include "rendermodel.h"
#include "tex.h"
#include "rt_model.h"
#include "demo.h"
#include "cmdgame.h"

void colorTracker(DefTracker *tracker,Mat4 mat,DefTracker *light_trkr,int force, F32 brightScale)
{
	static int tracker_verts_this_frame = 0;
	static int cur_draw_id = 0;
	Mat4	mat_orig;
	Model   *model;
	Vec3	testpos = {-1373,-440.36,-68.50};
	int		buffer_size;
	U8		*buf;

	PERFINFO_AUTO_START("colorTracker", 1);

	if(tracker->parent && groupDefIsDoorGenerator(tracker->parent->def) ) //this may be total craziness
		lightGiveLightTrackerToMyDoor(tracker->parent, light_trkr);

	if (!tracker || !tracker->def || !(tracker->def->model || tracker->def->welded_models) ||
		!light_trkr->light_grid) {
		PERFINFO_AUTO_STOP();
		return;
	}

	if (group_draw.draw_id != cur_draw_id)
	{
		cur_draw_id = group_draw.draw_id;
		tracker_verts_this_frame = 0;
	}

	if (!force && tracker_verts_this_frame > game_state.maxColorTrackerVerts)
    {
        PERFINFO_AUTO_STOP();
		return;
    }

	mulMat4(cam_info.inv_viewmat,mat,mat_orig);

	eaCreate(&tracker->src_rgbs);

	if (tracker->def->auto_lod_models)
	{
		int i;
		for (i = 0; i < eaSize(&tracker->def->auto_lod_models); i++)
		{
			model = tracker->def->auto_lod_models[i];
			if (!model)
			{
				eaPush(&tracker->src_rgbs, 0);
				continue;
			}

			buffer_size = model->vert_count * 4;
			if(rdr_caps.use_vbos)
				buf = _alloca(buffer_size);
			else
				buf = malloc(buffer_size);
			lightModel(model,mat_orig,buf,light_trkr, brightScale);
			eaPush(&tracker->src_rgbs, modelConvertRgbBuffer(buf,model->vert_count));

			tracker_verts_this_frame+=model->vert_count;
		}
	}
	else if (tracker->def->welded_models)
	{
		int i;
		for (i = 0; i < eaSize(&tracker->def->welded_models); i++)
		{
			model = tracker->def->welded_models[i];
			if (!model)
			{
				eaPush(&tracker->src_rgbs, 0);
				continue;
			}

			buffer_size = model->vert_count * 4;
			if(rdr_caps.use_vbos)
				buf = _alloca(buffer_size);
			else
				buf = malloc(buffer_size);
			lightModel(model,mat_orig,buf,light_trkr, brightScale);
			eaPush(&tracker->src_rgbs, modelConvertRgbBuffer(buf,model->vert_count));

			tracker_verts_this_frame+=model->vert_count;
		}
	}
	else
	{
		model = tracker->def->model;
		buffer_size = model->vert_count * 4;
		if(rdr_caps.use_vbos)
			buf = _alloca(buffer_size);
		else
			buf = malloc(buffer_size);
		lightModel(model,mat_orig,buf,light_trkr, brightScale);
		eaPush(&tracker->src_rgbs, modelConvertRgbBuffer(buf,model->vert_count));

		tracker_verts_this_frame+=model->vert_count;
	}

	PERFINFO_AUTO_STOP();
}

void initSwayTable(F32 timestep)
{
	F32		val=0,scale;
	int		j;
	GroupDraw	*gd = &group_draw;

	// JE: Does this first block of code ever need to be ran once?
	for(j=0;j<64;j++)
	{
		gd->sway_table[j] = val;
		val += 0.00005 * j;
	}
	scale = 1.f / gd->sway_table[63];
	for(j=0;j<64;j++)
		gd->sway_table[    j] = scale * (val - gd->sway_table[j]);
	for(j=0;j<64;j++)
	{
		gd->sway_table[1*64+j] = -gd->sway_table[63-j];
		gd->sway_table[2*64+j] = -gd->sway_table[j];
		gd->sway_table[3*64+j] = gd->sway_table[63-j];
	}
	// JE: end block of code I think only needs to be ran once

	for (j=0; j<64; j++) {
#define MAX_DELTA_DELTA 0.0005
#define MAX_DELTA 0.005
		gd->sway_random_offs_delta[j] += rand() * MAX_DELTA_DELTA*2 / (F32)(RAND_MAX+1) - MAX_DELTA_DELTA;
		gd->sway_random_offs_delta[j] = CLAMP(gd->sway_random_offs_delta[j], 0, MAX_DELTA);
		gd->sway_random_offs[j] += gd->sway_random_offs_delta[j]*timestep;
		if (ABS(gd->sway_random_offs[j]) > 1) {
			int ival = (int)ABS(gd->sway_random_offs[j]);
			F32 fval = ABS(gd->sway_random_offs[j]) - ival;
			if (ival&1) { // Odd
				gd->sway_random_offs_clamped[j] = (1 - fval);
			} else {
				gd->sway_random_offs_clamped[j] = fval;
			}
		} else {
			gd->sway_random_offs_clamped[j] = ABS(gd->sway_random_offs[j]);
		}
	}
}

static F32 getSwayInterp(F32 f_idx)
{
	int		t,idx,idx2;
	F32		ratio;

	t = f_idx;
	ratio = f_idx - (F32)t;
	idx = t & 255;
	idx2 = (idx+1)&255;
	return (1.f - ratio) * group_draw.sway_table[idx] + ratio * group_draw.sway_table[idx2];
}

void swayThisMat( Mat4 mat, Model * model, U32 uid )
{
	F32		idx,amount;
	TrickInfo *info = model->trick->info;

	if (info->sway[0]) {
		idx = group_draw.sway_count * info->sway[0] + info->sway_random[0] * ABS(group_draw.sway_random_offs[uid&63])*(1/MAX_DELTA);
		amount = info->sway[1] + info->sway_random[1] * group_draw.sway_random_offs_clamped[(uid+1)&63];
		if (amount)
		{
			pitchMat3(amount * getSwayInterp(idx),mat);
			rollMat3(amount * getSwayInterp(idx*1.5f),mat);
		}
		else {
			yawMat3(idx,mat);
		}
	}
	if (info->sway_pitch[0]) {
		idx = group_draw.sway_count * info->sway_pitch[0] + info->sway_random[0] * ABS(group_draw.sway_random_offs[uid&63])*(1/MAX_DELTA);
		amount = info->sway_pitch[1] + info->sway_random[1] * group_draw.sway_random_offs_clamped[(uid+1)&63];
		if (amount)
		{
			pitchMat3(amount * getSwayInterp(idx),mat);
		}
	}
	if (info->sway_roll[0]) {
		idx = group_draw.sway_count * info->sway_roll[0] + info->sway_random[0] * ABS(group_draw.sway_random_offs[uid&63])*(1/MAX_DELTA);
		amount = info->sway_roll[1] + info->sway_random[1] * group_draw.sway_random_offs_clamped[(uid+1)&63];
		if (amount)
		{
			rollMat3(amount * getSwayInterp(idx),mat);
		}
	}
}

void handleGlobFxMiniTracker( FxMiniTracker * globFxMiniTracker, GroupDef * def, Mat4 parent_mat, F32 draw_scale )
{
	Mat4 worldMat;

	mulMat4( cam_info.inv_viewmat, parent_mat, worldMat );

	if( globFxMiniTracker->count < MAX_FXMINITRACKERS )
	{
		globFxMiniTracker->fxIds[ globFxMiniTracker->count ] = fxManageWorldFx(
			globFxMiniTracker->fxIds[ globFxMiniTracker->count ],
			def->type_name,
			worldMat,
			draw_scale * def->vis_dist,
			draw_scale * def->lod_farfade,
			0,
			NULL
			);

		globFxMiniTracker->count++;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define MAX_TRIS 50
#define MAX_WELDS 1
#define MAX_TOTAL_VERTS 8000
static int do_weld_limiting = 1;

static int modelCanWeld(Model *m1, Model *m2)
{
	if (m1->gld != m2->gld)
		return 0;

	if ((!m1->trick) != (!m2->trick))
		return 0;

	if (m1->trick && m2->trick && (!m1->trick->info) != (!m2->trick->info))
		return 0;

	if (m1->trick && m2->trick && m1->trick->flags64 != m2->trick->flags64)
		return 0;

	if (m1->trick && m2->trick && m1->trick->info && m2->trick->info && trickCompare(m1->trick->info, m2->trick->info)!=0)
		return 0;

	if ((!m1->pack.norms.unpacksize) != (!m2->pack.norms.unpacksize))
		return 0;

	if ((!m1->pack.sts.unpacksize) != (!m2->pack.sts.unpacksize))
		return 0;

	if (m1->flags != m2->flags)
		return 0;

	return 1;
}

static Model *weldModels(Model **models, Mat4Ptr *mats, int count)
{
	int i, j, k, l;
	Model *welded;
	Model *basemodel;
	Vec3 dv;
	int *tex_ids = 0;

	basemodel = models[0];

	eaiCreate(&tex_ids);
	for (i = 0; i < count; i++)
	{
		Model *m = models[i];
		modelUnpackAll(m);

		for (j = 0; j < m->tex_count; j++)
			eaiPushUnique(&tex_ids, m->tex_idx[j].id);
	}

	welded = modelCopyForModification(basemodel, eaiSize(&tex_ids));

	for (i = 1; i < count; i++)
	{
		welded->tri_count += models[i]->tri_count;
		welded->vert_count += models[i]->vert_count;
	}

	welded->unpack.tris = malloc(welded->tri_count * 3 * sizeof(U32));
	welded->unpack.verts = malloc(welded->vert_count * 3 * sizeof(F32));
	if (basemodel->unpack.norms)
		welded->unpack.norms = malloc(welded->vert_count * 3 * sizeof(F32));
	if (basemodel->unpack.sts)
		welded->unpack.sts = malloc(welded->vert_count * 2 * sizeof(F32));

	// triangle remapping
	{
		int idx = 0;
		int *tris = 0;
		eaiCreate(&tris);

		for (j = 0; j < welded->tex_count; j++)
		{
			int vertoffset = 0;
			eaiSetSize(&tris, 0);

			for (i = 0; i < count; i++)
			{
				int srcoffset = 0;
				Model *m = models[i];
				for (k = 0; k < m->tex_count; k++)
				{
					if (tex_ids[j] == m->tex_idx[k].id)
					{
						for (l = 0; l < m->tex_idx[k].count * 3; l++)
						{
							int t = m->unpack.tris[srcoffset + l] + vertoffset;
							eaiPush(&tris, t);
						}
						break;
					}
					srcoffset += m->tex_idx[k].count * 3;
				}
				vertoffset += m->vert_count;
			}

			for (i = 0; i < eaiSize(&tris); i++, idx++)
				welded->unpack.tris[idx] = tris[i];

			welded->tex_idx[j].id = tex_ids[j];
			welded->tex_idx[j].count = eaiSize(&tris) / 3;

		}

		eaiDestroy(&tris);
	}

	// verts
	{
		int idx = 0;
		for (i = 0; i < count; i++)
		{
			Model *m = models[i];
			Mat4Ptr mat = mats[i];
			for (j = 0; j < m->vert_count; j++, idx++)
			{
				mulVecMat4(m->unpack.verts[j], mat, welded->unpack.verts[idx]);
				if (m->unpack.norms)
					mulVecMat3(m->unpack.norms[j], mat, welded->unpack.norms[idx]);
				if (m->unpack.sts)
					copyVec2(m->unpack.sts[j], welded->unpack.sts[idx]);
			}
		}
	}

	// remove duplicate verts
	{
		int old_vert_count = welded->vert_count;
		for (i = 0; i < welded->vert_count; i++)
		{
			for (j = i+1; j < welded->vert_count; j++)
			{
				if (sameVec3(welded->unpack.verts[i], welded->unpack.verts[j]) &&
					(!welded->unpack.norms || sameVec3(welded->unpack.norms[i], welded->unpack.norms[j])) &&
					(!welded->unpack.sts || sameVec2(welded->unpack.sts[i], welded->unpack.sts[j])))
				{
					for (k = 0; k < welded->tri_count * 3; k++)
					{
						if (welded->unpack.tris[k] == j)
							welded->unpack.tris[k] = i;
						else if (welded->unpack.tris[k] > j)
							welded->unpack.tris[k]--;
					}

					memmove(welded->unpack.verts + j, welded->unpack.verts + j + 1, (welded->vert_count - j - 1) * sizeof(Vec3));
					if (welded->unpack.norms)
						memmove(welded->unpack.norms + j, welded->unpack.norms + j + 1, (welded->vert_count - j - 1) * sizeof(Vec3));
					if (welded->unpack.sts)
						memmove(welded->unpack.sts + j, welded->unpack.sts + j + 1, (welded->vert_count - j - 1) * sizeof(Vec2));

					welded->vert_count--;
					j--;
				}
			}
		}

		if (old_vert_count != welded->vert_count)
		{
			welded->unpack.verts = realloc(welded->unpack.verts, welded->vert_count * sizeof(Vec3));
			if (welded->unpack.norms)
				welded->unpack.norms = realloc(welded->unpack.norms, welded->vert_count * sizeof(Vec3));
			if (welded->unpack.sts)
				welded->unpack.sts = realloc(welded->unpack.sts, welded->vert_count * sizeof(Vec2));
		}
	}

	setVec3(welded->min, 8e8, 8e8, 8e8);
	setVec3(welded->max, -8e8, -8e8, -8e8);
	for (i = 0; i < welded->vert_count; i++)
	{
		MINVEC3(welded->unpack.verts[i], welded->min, welded->min);
		MAXVEC3(welded->unpack.verts[i], welded->max, welded->max);
	}

	subVec3(welded->max, welded->min, dv);
	welded->radius = 0.5f * lengthVec3(dv);

	welded->vbo = 0;

	geoAddWeldedModel(welded);

	eaiDestroy(&tex_ids);

	if (!do_weld_limiting)
		loadUpdate("Welding", welded->vert_count * 100);

	welded->loadstate = LOADED;

	return welded;
}

static int defCanWeld(GroupDef *def)
{
	if (!def->model || def->model->tri_count > MAX_TRIS)
		return 0;

	// ONLY_EDITORVISIBLE
	if (def->has_fog || def->has_sound || def->has_light || def->has_volume || def->has_cubemap)
		return 0;
	if (def->model->trick && ((def->model->trick->flags1 & TRICK_EDITORVISIBLE) || (def->model->trick->flags1 & TRICK_HIDE)))
		return 0;

	if (def->model->flags & OBJ_ALPHASORT)
		return 0;

	return 1;
}

static int welds_this_frame = 0;
void createWeldedModels(GroupDef *def, int uid)
{
	static int cur_draw_id = 0;
	int i, j;
	Model **models;
	Mat4Ptr *mats;
	Model *welded;
	int *available=0, *weldlist=0, *available_uids=0;

	PERFINFO_AUTO_START("createWeldedModels",1);
	if (group_draw.draw_id != cur_draw_id)
	{
		cur_draw_id = group_draw.draw_id;
		welds_this_frame = 0;
	}

	if (do_weld_limiting && welds_this_frame > MAX_WELDS)
	{
		PERFINFO_AUTO_STOP();
		return;
	}

	if (def->count < 2 || def->model || def->lod_near)
	{
		def->try_to_weld = 0;
		PERFINFO_AUTO_STOP();
		return;
	}
	for (i = 0; i < def->count; i++)
	{
		if (def->entries[i].def->lod_near)
		{
			def->try_to_weld = 0;
			PERFINFO_AUTO_STOP();
			return;
		}
		if (def->entries[i].def->model && def->entries[i].def->model->loadstate != LOADED)
		{
			PERFINFO_AUTO_STOP();
			return;
		}
	}

	{
		int uidwalk = uid+1;
		eaiCreate(&available);
		eaiCreate(&available_uids);
		for (i = 0; i < def->count; i++) {
			uidwalk += 1 + def->entries[i].def->recursive_count;
			eaiPush(&available, i);
			eaiPush(&available_uids, uidwalk);
		}
	}

	eaiCreate(&weldlist);

	models = _alloca(def->count * sizeof(*models));
	mats = _alloca(def->count * sizeof(*mats));

	for (i = 0; i < eaiSize(&available); i++)
	{
		int totalverts = 0;
		int av1 = available[i];
		int first_uid=-1;

		if (!defCanWeld(def->entries[av1].def))
			continue;

		eaiSetSize(&weldlist, 0);
		eaiPush(&weldlist, av1);

		for (j = i+1; j < eaiSize(&available); j++)
		{
			int this_uid;
			int av2 = available[j];

			if (totalverts > MAX_TOTAL_VERTS)
				break;

			if (!defCanWeld(def->entries[av2].def))
				continue;

			if (!modelCanWeld(def->entries[av1].def->model, def->entries[av2].def->model))
				continue;

			if (def->entries[av1].def->lod_near != def->entries[av2].def->lod_near)
				continue;

			totalverts += def->entries[av2].def->model->vert_count;
			eaiPush(&weldlist, av2);
			eaiRemove(&available, j);
			this_uid = eaiRemove(&available_uids, j);
			if (first_uid == -1)
				first_uid = this_uid;
			j--;
		}

		if (eaiSize(&weldlist) > 1)
		{
			eaiRemove(&available, i);
			eaiRemove(&available_uids, i);
			i--;

			for (j = 0; j < eaiSize(&weldlist); j++)
			{
				models[j] = def->entries[weldlist[j]].def->model;
				mats[j] = def->entries[weldlist[j]].mat;
			}

			welded = weldModels(models, mats, eaiSize(&weldlist));
			if (!def->welded_models)
				eaCreate(&def->welded_models);
			eaPush(&def->welded_models, welded);
			eaiPush(&def->welded_models_duids, first_uid - uid);
			welds_this_frame++;
		}
	}

	if (eaiSize(&available) != def->count)
	{
		if (eaiSize(&available))
		{
			eaiCreate(&def->unwelded_children);
			eaiCopy(&def->unwelded_children, &available);
		}

		for (i = 0; i < eaSize(&def->welded_models); i++)
			def->radius = MAX(def->radius, def->welded_models[i]->radius);

		for (i = 0; i < def->count; i++)
		{
			def->lod_far = MAX(def->lod_far, def->entries[i].def->lod_far);
			def->lod_farfade = MAX(def->lod_farfade, def->entries[i].def->lod_farfade);
		}
	}

	eaiDestroy(&available_uids);
	eaiDestroy(&available);
	eaiDestroy(&weldlist);

	if (!def->welded_models)
		def->try_to_weld = 0;
	PERFINFO_AUTO_STOP();
}

static void clearWelded(GroupDef *def)
{
	int i;

	if (def->welded_models)
		eaDestroy(&def->welded_models);

	def->welded_models = 0;

	for (i = 0; i < def->count; i++)
		clearWelded(def->entries[i].def);
}

void groupClearAllWeldedModels(void)
{
	int i;
	DefTracker *ref;
	PERFINFO_AUTO_START("groupClearAllWeldedModels",1);
	for(i=0;i<group_info.ref_count;i++)
	{
		ref = group_info.refs[i];
		if (ref->def)
			clearWelded(ref->def);
	}
	PERFINFO_AUTO_STOP();
}

int groupWeldAllCallback(GroupDef *def, DefTracker *tracker, Mat4 world_mat, TraverserDrawParams *draw)
{
	if (!(def->child_ambient || def->has_ambient || draw->userDataInt)) {
		return 0; // Neither parents nor child have has_ambient
	}
	if (def->has_ambient)
		draw->userDataInt = 1; // Tell all children we have ambient

	if (def->try_to_weld && !def->welded_models && draw->userDataInt)
	{
		groupLoadIfPreload(def);
		createWeldedModels(def, draw->uid);
	}
	return 1;
}

void groupWeldAll(void)
{
	PERFINFO_AUTO_START("groupWeldAll",1);
	welds_this_frame = 0;
	loadstart_printf("Welding interior models.. ");
	do_weld_limiting = 0;
	groupTreeTraverse(groupWeldAllCallback, 0);
	do_weld_limiting = 1;
	loadend_printf("%d welded", welds_this_frame);
	PERFINFO_AUTO_STOP();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DefTexSwapNode * texSwaps;
int texSwapsCount;
int texSwapsMax;

int glob_last_uid;

static void groupTreeTraverseHelper(GroupDef *def, Mat4 local_mat_p, Mat4 parent_mat_p, DefTracker *tracker, TraverserDrawParams *parent_draw)
{
	TraverserDrawParams draw = *parent_draw;
	Mat4		parent_mat;
	int			i, j;
	GroupEnt	*childGroupEnt;
	DefTracker	*child=0;
	int			editor_visible_only = def->has_fog || def->has_sound || def->has_light || def->has_volume || def->editor_visible_only || def->has_cubemap;

	if (def->pre_alloc)
		groupFileLoadFromName(def->name);

	if (def->model && def->model->trick && ((def->model->trick->flags1 & TRICK_EDITORVISIBLE) || (def->model->trick->flags1 & TRICK_HIDE))) {
		draw.editor_visible_only = 1;
	} else {
		// Inherit from parent
	}

	if (draw.need_matricies)
		mulMat4Inline(parent_mat_p,local_mat_p,parent_mat);

	if (draw.need_texAndTintCrc || draw.need_texswaps)
	{
		U8 dummy_tint[2][4] = {255,255,255,255,255,255,255,255};

		// tex swaps and tints
		if (eaSize(&def->def_tex_swaps)>0) {
			DefTexSwapNode * dtsn=dynArrayAdd(&texSwaps,sizeof(DefTexSwapNode),&texSwapsCount,&texSwapsMax,1);
			dtsn->index=parent_draw->swapIndex;
			dtsn->swaps=def->def_tex_swaps;
			assert((dtsn-texSwaps)+1 == texSwapsCount); // TODO: remove this if it doesn't go off on the first run.
			draw.swapIndex=texSwapsCount;
		}

		if (def->has_tint_color)
			draw.tint_color_ptr = (U8*)def->color;

		if (def->has_tex)
		{
			if (def->has_tex & 1)
				draw.tex_binds.base = def->tex_binds.base;
			if (def->has_tex & 2)
				draw.tex_binds.generic = def->tex_binds.generic;
		}

		// CRC the data if needed
		if (draw.need_texAndTintCrc && def->model)
		{
			BlendModeType first_blend_mode;
			cryptAdler32Init();
			cryptAdler32Update(draw.tint_color_ptr?draw.tint_color_ptr:&dummy_tint[0][0], 8);
			for (i = 0; i < def->model->tex_count; i++)
			{
				BasicTexture *actualTextures[TEXLAYER_MAX_LAYERS] = {0};
				RdrTexList dummy = {0};
				modelGetFinalTextures(actualTextures, def->model, 
					(draw.tex_binds.base||draw.tex_binds.generic)?
					&draw.tex_binds:0, 1, draw.swapIndex, i, &dummy,
					&first_blend_mode, true, tracker->parent);

				for (j = 0; j < TEXLAYER_MAX_LAYERS; j++)
				{
					if (actualTextures[j])
						cryptAdler32Update(actualTextures[j]->name, strlen(actualTextures[j]->name));
				}
			}

			draw.texAndTintCrc = cryptAdler32Final();
		}
	}

	if (!draw.callback(def, tracker, draw.need_matricies?parent_mat:NULL, &draw))
		return;

	// increment for the current node
	draw.uid++;
	glob_last_uid = draw.uid;

	if (tracker)
		child = tracker->entries;
	for(childGroupEnt = def->entries,i=0; i < def->count; i++,childGroupEnt++)  
	{	
		int old_glob_last_uid = glob_last_uid;
		groupTreeTraverseHelper(childGroupEnt->def, childGroupEnt->mat, parent_mat, child, &draw);

		if (!demoIsPlaying())
			assert(draw.uid + 1 + childGroupEnt->def->recursive_count >= glob_last_uid);

		draw.uid += 1 + childGroupEnt->def->recursive_count;

		if (!demoIsPlaying())
			assert(draw.uid >= glob_last_uid); // Same assert as above

		glob_last_uid = draw.uid;
		if (child)
			child++;
	}
}

void groupTreeTraverseDefEx(Mat4 mat, TraverserDrawParams *draw, GroupDef *def, DefTracker *tracker)
{
	if (!def && tracker)
		def = tracker->def;

	// Assumes draw->uid is already filled in
	draw->swapIndex = 0;

	texSwapsCount = 0;
	glob_last_uid = 0;

	if (def)
	{
		groupTreeTraverseHelper(def, tracker?tracker->mat:mat, mat, tracker, draw);
		draw->uid += 1 + def->recursive_count;
		assert(draw->uid >= glob_last_uid);
	}
	SAFE_FREE(texSwaps);
	texSwapsCount = texSwapsMax = 0;
}

void groupTreeTraverseEx(Mat4 mat, TraverserDrawParams *draw)
{
	int i;

	draw->uid = 0;
	draw->swapIndex = 0;

	texSwapsCount = 0;
	glob_last_uid = 0;

	for (i = 0; i < group_info.ref_count; i++)
	{
		DefTracker	*tracker;
		tracker = group_info.refs[i];
		if (tracker->def)
		{
			groupTreeTraverseHelper(tracker->def, tracker->mat, mat, tracker, draw);
			draw->uid += 1 + tracker->def->recursive_count;
			if (!demoIsPlaying())
				assert(draw->uid >= glob_last_uid);
			glob_last_uid = draw->uid;
		}
	}
	SAFE_FREE(texSwaps);
	texSwapsCount = texSwapsMax = 0;
}

void groupTreeTraverse(GroupTreeTraverserCallback callback, int need_texAndTintCrc)
{
	Mat4 identity = {0};
	TraverserDrawParams draw = {0};

	identity[0][0] = 1;
	identity[1][1] = 1;
	identity[2][2] = 1;

	draw.callback = callback;
	draw.need_texAndTintCrc = !!need_texAndTintCrc;
	draw.need_matricies = true;
	draw.need_texswaps = false;
	groupTreeTraverseEx(identity, &draw);
}
