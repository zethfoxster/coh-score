
#include "view_cache.h"

#include "anim.h"
#include "camera.h"
#include "cmdcommon.h"
#include "edit_cmd.h"
#include "frustum_c.h"
#include "gfx.h"
#include "gfxwindow.h"
#include "groupdraw.h"
#include "mathutil.h"
#include "renderstats.h"
#include "SuperAssert.h"
#include "utils.h"
#include "viewport.h"
#include "zOcclusion.h"

#if !VIEWCACHEDISABLED
typedef struct AutoLOD AutoLOD;

void drawModel_ShadowmapNonInline(Model *model, Model *lowLodModel, AutoLOD *lod, GroupDef *def,Mat4 mat,Vec3 mid,DefTracker *tracker,DrawParams *draw,F32 d,VisType vis, int uid, int do_occluder_check, int lod_idx, int lod_override, bool is_welded, int hide);
int drawModelNonInline(Model *model, Model *lowLodModel, AutoLOD *lod, GroupDef *def,Mat4 mat,Vec3 mid,DefTracker *tracker,DrawParams *draw,F32 d,VisType vis, int uid, int do_occluder_check, int lod_idx, int lod_override, bool is_welded, int hide);
void drawDefModel(GroupDef *def, Mat4 mat, Vec3 mid, DefTracker *tracker, DrawParams *draw, F32 d, VisType vis, int uid);

//=============================================================================
static int					s_last_update = -1;
static int					s_update_renderPass = -1;
static GfxWindow			s_saved_gfx_window;
static GfxFrustum			s_saved_shadow_cull_frustum;

static ViewCacheEntry	   *s_entry_list = NULL;
static int					s_entry_count = 0;
static int					s_entry_max = 0;

//=============================================================================
bool ViewCache_Update(int renderPass)
{
	bool runUpdateLoop = true;

	if (game_state.useViewCache && game_state.shadowMode == SHADOW_SHADOWMAP && !editMode() && !game_state.edit_base &&
		(renderPass == RENDERPASS_COLOR || 
		(renderPass >= RENDERPASS_SHADOW_CASCADE1 && renderPass <= RENDERPASS_SHADOW_CASCADE_LAST)) && !editMode())
	{
		gfx_state.viewCacheInUse = true;

		if (s_last_update != global_state.global_frame_count)
		{
			float aspect = gfx_state.main_viewport_info->aspect;
			float fov_y  = gfx_state.main_viewport_info->fov;
			float fov_x = 2.0f * (fatan(aspect * ftan(RAD(fov_y)*0.5f)));
			float znear = fabs(game_state.nearz);
			float zfar = fabs(game_state.farz);

			s_last_update = global_state.global_frame_count;

			// save old info
			memcpy(&s_saved_gfx_window, &gfx_window, sizeof(GfxWindow));
			memcpy(&s_saved_shadow_cull_frustum, &group_draw.shadowmap_cull_frustum, sizeof(GfxFrustum));

			// build non-cascade specific frustum
			frustum_build(&group_draw.shadowmap_cull_frustum, fov_x, aspect, znear, zfar);

			{
				extern void gfxWindowSetAng(F32 fovy, F32 fovx, F32 znear, F32 zfar, bool bFlipY, bool bFlipX, int setMatrix);
				gfxWindowSetAng(fov_y, fov_y * aspect, znear, zfar,false, false, false );
			}

			// Do update
			s_entry_count = 0;

			gfx_state.viewCacheCaching = 1;
			s_update_renderPass = renderPass;
		}
		else
		{
			// Use the cached list
			runUpdateLoop = false;
			gfx_state.viewCacheCaching = 0;
		}

	}
	else
	{
		gfx_state.viewCacheInUse = false;
	}


	return runUpdateLoop;
}

//=============================================================================
int ViewCache_Add(DrawParams *draw,F32 d,Vec3 mid,DefTracker *tracker,VisType vis,Mat4 parent_mat,int uid,GroupEnt *groupEnt)
{
	int i = 0;
	int index;
	ViewCacheEntry *new_entry = NULL;

	new_entry = dynArrayAdd(&s_entry_list, sizeof(ViewCacheEntry), &s_entry_count, &s_entry_max, 1);
	index = new_entry - s_entry_list;

	new_entry->view_cache_parent_index = draw->view_cache_parent_index;
	copyVec3(mid, new_entry->mid);
	//new_entry->radius = groupEnt->radius;
	new_entry->radius = groupEnt->def->radius;

	new_entry->has_model = (groupEnt->def->model != NULL);
	new_entry->has_tracker = (tracker != NULL);
	new_entry->child_count = 0;

	memcpy(&new_entry->draw, draw, sizeof(DrawParams));
	new_entry->d = d;

	if (tracker)
		memcpy(&new_entry->tracker, tracker, sizeof(DefTracker));
	else
		new_entry->tracker.def = NULL;

	new_entry->vis = vis;
	copyMat4(parent_mat, new_entry->parent_mat);
	new_entry->uid = uid;
	memcpy(&new_entry->groupEnt, groupEnt, sizeof(GroupEnt));

//	new_entry->last_render_pass_checked = RENDERPASS_COUNT;
//	new_entry->last_frame_checked = -1;

	return index;
}

//=============================================================================
void ViewCache_UpdateChildCount(int parent_index)
{
	assert(parent_index >= 0);
	assert(parent_index < s_entry_count);

	s_entry_list[parent_index].child_count = s_entry_count - parent_index - 1;
}

//=============================================================================
void ViewCache_Use(ViewCacheVisibilityCallback callback, int renderPass)
{
	int i;
	ViewCacheEntry *entry = s_entry_list;

	if (!game_state.useViewCache || !gfx_state.viewCacheInUse)
		return;

	if (renderPass == RENDERPASS_COLOR)
	{
		group_draw.znear = game_state.nearz;
		group_draw.zfar = game_state.farz;
	}

	// restore the frustum
	if (s_update_renderPass == renderPass)
	{
		// save old info
		memcpy(&gfx_window, &s_saved_gfx_window, sizeof(GfxWindow));
		memcpy(&group_draw.shadowmap_cull_frustum, &s_saved_shadow_cull_frustum, sizeof(GfxFrustum));

		s_update_renderPass = -1;
	}

	gfx_state.viewCacheInUse = false;

	if (renderPass == RENDERPASS_COLOR)
	{
		group_draw.zocclusion = !!game_state.zocclusion;
		if (group_draw.zocclusion && !(group_draw.see_everything && !edit_state.showvis))
		{
			zoInitFrame(gfx_state.projection_matrix, cam_info.viewmat, cam_info.inv_viewmat, game_state.nearz);
		}
	}
	else
		group_draw.zocclusion = false;

	onlydevassert(callback != NULL);

	for (i = 0; i < s_entry_count; i++, entry++)
	{
		if (entry->view_cache_parent_index == -1)
		{
			entry->visible = true;
		}
		else if (!s_entry_list[entry->view_cache_parent_index].visible)
		{
			entry->visible = false;
		}
		else
		{
			entry->visible = callback(entry);
		}

		if (!entry->visible)
		{
			i += entry->child_count;
			entry += entry->child_count;
		}
		else
		{
			if (entry->has_model)
			{
				GroupDef *def = entry->groupEnt.def;
				Model *model = def->model;
				DefTracker *tracker = NULL;

				if (entry->has_tracker)
					tracker = &entry->tracker;

				if (renderPass >= RENDERPASS_SHADOW_CASCADE1 && renderPass <= RENDERPASS_SHADOW_CASCADE_LAST)
				{
					drawModel_ShadowmapNonInline(model, 0, 0, def, entry->parent_mat, entry->mid, tracker, &entry->draw, entry->d, entry->vis, entry->uid, 1, 0, 0, 0, def->hideForHeadshot);
				}
				else
				{
					onlydevassert(renderPass == RENDERPASS_COLOR);

					//drawModelNonInline(model, 0, 0, def, entry->parent_mat, entry->mid, tracker, &entry->draw, entry->d, entry->vis, entry->uid, 1, 0, (gfx_state.lod_model_bias > 0), 0, def->hideForHeadshot);

					drawDefModel(def, entry->parent_mat, entry->mid, tracker, &entry->draw, entry->d, entry->vis, entry->uid);
				}
			}
		}
	}

	gfx_state.viewCacheInUse = false;
	gfx_state.viewCacheCaching = false;
}

#endif // VIEWCACHEDISABLED
