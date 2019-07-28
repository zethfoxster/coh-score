#include "groupdraw.h"
#include "groupdrawutil.h"
#include "groupdrawinline.h"
#include "cmdgame.h"
#include "camera.h"
#include "edit_cmd.h"
#include "edit_select.h"
#include "sun.h"
#include "font.h"
#include "utils.h"
#include "grouptrack.h"
#include "groupdyn.h"
#include "groupgrid.h"
#include "gridfind.h"
#include "anim.h"
#include "vistray.h"
#include "groupfileload.h"
#include "zOcclusion.h"
#include "renderprim.h"
#include "baseedit.h"
#include "basedraw.h"
#include "gfx.h"
#include "motion.h"
#include "edit_cmd_adjust.h"

GroupDraw	group_draw;
static bool invalidate_group_ents;

//Build a GroupEnt for passing into drawDefInternal. All calls to drawDefInternal need a GroupEnt 
//That's been run through this
void makeAGroupEnt( GroupEnt * groupEnt, GroupDef * def, const Mat4 mat, F32 view_scale, int has_dynamic_parent )
{
	groupLoadIfPreload(def);
	mulVecMat4(def->mid, mat, groupEnt->mid_cache);
	//cacheGroupEntMid( mat, groupEnt->mid_cache, def->mid ); 

	groupEnt->mat = (Mat4Ptr)mat; // WARNING: Loses const qualifier here, and groupEnt->mat's *are* changed in bits of the code
	groupEnt->def = def;
 
	groupEnt->vis_dist_scaled_cache	= (view_scale * def->vis_dist * def->lod_scale);
	groupEnt->vis_def_radius_cache	= def->radius;
	groupEnt->radius				= def->radius + def->shadow_dist;

	groupEnt->recursive_count_cache	= def->recursive_count;
	groupEnt->has_dynamic_parent_cache = !!has_dynamic_parent;

	groupEnt->flags_cache = 0;
	groupEnt->flags_cache |= def->vis_doorframe     ? VIS_DOORFRAME     : 0;
	groupEnt->flags_cache |= def->child_parent_fade ? CHILD_PARENT_FADE : 0;
	groupEnt->flags_cache |= def->parent_fade       ? PARENT_FADE       : 0;
	groupEnt->flags_cache |= def->has_fog			? HAS_FOG			: 0;
	groupEnt->flags_cache |= (def->model	&& def->shadow_dist)			? HAS_SHADOW		: 0;
	groupEnt->flags_cache |= def->sound_radius		? HAS_SOUND_RADIUS	: 0;
	if (def->has_fog | def->has_sound | def->has_light | def->editor_visible_only | def->has_cubemap)
		groupEnt->flags_cache |= ONLY_EDITORVISIBLE;
	if (def->model)
	{
		Model *model = def->model;
		if ( model->trick)
		{
			if ((model->trick->flags1 & TRICK_EDITORVISIBLE) || (model->trick->flags1 & TRICK_HIDE))
				groupEnt->flags_cache |= ONLY_EDITORVISIBLE;
		}
	}

	groupEnt->flags_cache |= CACHEING_IS_DONE; //could get rid of if I inited GroupEnts always when created by def
}

void groupDrawCacheInvalidate()
{
	invalidate_group_ents = true;
}

// kill Groupent cache
void checkForGroupEntValidity()
{	
	GroupDef * def;
	int i, j, k;

	if( invalidate_group_ents ) 
	{
		for(k=0;k<group_info.file_count;k++)
		{
			GroupFile	*file = group_info.files[k];

			for( i = 0 ; i < file->count ; i++ )
			{
				def = file->defs[i];
				if (!def)
					continue;
				for( j = 0 ; j < def->count ; j++ )
				{
					def->entries[j].flags_cache &=~CACHEING_IS_DONE;
				}
			}
		}
		invalidate_group_ents = false;
	}
}

//Draws a GroupDef outside the normal world tree traversal.
//If there is a tracker, the def must be tracker->def
void groupDrawDefTracker( GroupDef *def,DefTracker *tracker,Mat4 matx, EntLight * light, FxMiniTracker * fxminitracker, F32 viewScale, DefTracker *lighttracker, int uid)
{
	DrawParams	draw = {0};
	GroupEnt	ent = {0};
	GroupDraw	*gd = &group_draw;
	
	draw.view_scale = viewScale;
	draw.light_trkr = lighttracker;

	gd->globFxLight = light; //only give the light if light->used, cause I never check it in modelDrawX
	gd->globFxMiniTracker = fxminitracker;

	group_draw.do_welding = 0;

	makeAGroupEnt( &ent, def, unitmat, draw.view_scale, 0 );
	drawDefInternal(&ent, matx, tracker, VIS_DRAWALL, &draw, uid);

	gd->globFxLight = 0;
	gd->globFxMiniTracker = 0;
}

int groupDynDraw(int world_id,Mat4 view_xform)
{
	DynGroup	*dyn = groupDynFromWorldId(world_id);
	int			invisible;

	if (!dyn || !dyn->tracker)
		return 0;
	invisible = dyn->tracker->invisible;
	dyn->tracker->invisible = 0;
	groupDrawDefTracker(dyn->tracker->def,dyn->tracker,view_xform,0,0,1,0,-1);
	dyn->tracker->invisible = invisible;
	return 1;
}


void groupDrawRefs(Mat4 cam_mat, Mat4 inv_cam_mat)
{
	int			vis;
	DrawParams	draw;

	if (group_draw.calls[0])
	{
		xyprintf(0,25,"%d %d %d %d %d %d %d %d %d %d %d",
			group_draw.calls[0],group_draw.calls[1],group_draw.calls[2],group_draw.calls[3],group_draw.calls[4],group_draw.calls[5],
			group_draw.calls[6],group_draw.calls[7],group_draw.calls[8],group_draw.calls[9],group_draw.calls[10]);
	}
	memset(group_draw.calls,0,sizeof(group_draw.calls));

	//Init Draw structure ; only used internally by drawDefInternal (should be reset between protal and world?)
	memset(&draw,0,sizeof(draw));
	draw.view_scale = 1.0f;

	checkForGroupEntValidity();

	//Set Editor Visiblity overrides
	if (game_state.see_everything & 1)
		group_draw.see_everything = 1;
	else
		group_draw.see_everything = 0;

	vis = VIS_NORMAL;
	// Set vis to VIS_DRAWALL to get objects rendering properly in the reflection viewport in the sewers
	if (gfx_state.renderPass == RENDERPASS_REFLECTION || gfx_state.renderPass == RENDERPASS_REFLECTION2)
		vis = VIS_DRAWALL;
	if (group_draw.see_everything && !edit_state.showvis)
		vis = VIS_DRAWALL; 

	//Assorted inits
	group_draw.see_outside = 1;		//Just draw visible portals, or outside, too?
	++group_draw.draw_id;				//Don't draw any portal more than once.

	copyMat4(inv_cam_mat, group_draw.inv_cam_mat);

	group_draw.zocclusion = !!game_state.zocclusion;

	if (vis != VIS_DRAWALL && group_draw.zocclusion)
	{
		zoInitFrame(gfx_state.projection_matrix, cam_mat, inv_cam_mat, game_state.nearz);
	}

	group_draw.do_welding = !game_state.no_welding && !game_state.edit_base && vis != VIS_DRAWALL;

	baseStartDraw();

	//Draw portals
	if (vis != VIS_DRAWALL)
		vistrayDraw(&draw, cam_mat);

	if (group_draw.see_outside)
	{
		DefTracker	*ref;
		int	i;
		int current_group_uid = 0;

		PERFINFO_AUTO_START("Draw outside world", 1);

		if (vis != VIS_DRAWALL)
			vistrayClearOutsideTrayList();

		for(i=0;i<group_info.ref_count;i++)
		{
			ref = group_info.refs[i];
#ifdef DEBUG_UIDS_GOING_BAD
				ref->cached_uid = current_group_uid;
#endif
			if (ref->def)
			{
				if (!ref->dyn_parent_tray || vis == VIS_DRAWALL)
				{
					GroupEnt ent = {0};
					makeAGroupEnt(&ent, ref->def, ref->mat, draw.view_scale, !!ref->dyn_parent_tray);
					drawDefInternal(&ent, cam_mat, ref, vis, &draw, current_group_uid);
				}
				else
				{
					vistraySetDetailUid(ref, current_group_uid);
				}

				current_group_uid += 1 + ref->def->recursive_count;
			}
		}

		if (vis != VIS_DRAWALL)
			vistrayDrawOutsideTrays(&draw, cam_mat);

		PERFINFO_AUTO_STOP();
	}

	entMotionCheckDoorOccluders();

	//Draw anyhting picked up in the editor (or base editor)
	PERFINFO_AUTO_START("selDraw", 1);
	selDraw();
	scalableVolumesDraw();
	PERFINFO_AUTO_STOP_START("baseedit_SelDraw", 1);

	if (gfx_state.mainViewport)
		baseedit_SelDraw();

	PERFINFO_AUTO_STOP();
}
