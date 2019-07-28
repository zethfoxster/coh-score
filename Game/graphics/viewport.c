
#include <assert.h>
#include "viewport.h"

#include "gfx.h"
#include "mathutil.h"
#include "cmdgame.h"
#include "rt_init.h"
#include "rt_state.h"
#include "pbuffer.h"
#include "gfxwindow.h"
#include "camera.h"
#include "ogl.h"
#include "player.h"
#include "entity.h"
#include "seq.h"
#include "win_init.h"
#include "rt_queue.h"


#define MAX_VIEWPORTS 32

static ViewportInfo *s_viewports[MAX_VIEWPORTS];
static int			 s_numViewports = 0;

void viewport_InitStructToDefaults(ViewportInfo *viewport_info)
{
	viewport_info->update_interval = 1;
	viewport_info->next_update = 0;
	viewport_info->renderPass = RENDERPASS_UNKNOWN;
	// Assumes we just need the color texture
	viewport_info->isRenderToTexture = true;
	viewport_info->needColorTexture = true;
	viewport_info->needAuxColorTexture = false;
	viewport_info->needDepthTexture = false;
	viewport_info->needAlphaChannel = false;
	viewport_info->isMipmapped = false;
	viewport_info->maxMipLevel = 0;

	viewport_info->fov = 90;
	viewport_info->target = GL_TEXTURE_2D;
	viewport_info->aspect = 1;
	viewport_info->width = 128;
	viewport_info->height = 128;
	viewport_info->nearZ = game_state.nearz;
	viewport_info->farZ = game_state.farz;
	viewport_info->bFlipYProjection = false;
	viewport_info->bFlipXProjection = false;
	viewport_info->floatComponents = false;

	viewport_info->viewport_width = -1;
	viewport_info->viewport_height = -1;
	viewport_info->viewport_x = 0;
	viewport_info->viewport_y = 0;

	viewport_info->lod_scale = 1.0f;
//	viewport_info->lod_model_bias = 0;
	viewport_info->lod_model_override = 0;	// disabled
	viewport_info->lod_entity_override = 0;	// disabled
	viewport_info->vis_limit_entity_dist_from_player = FLT_MAX;
	viewport_info->debris_fadein_size = 4.0f;
	viewport_info->debris_fadeout_size = 8.0f;

	viewport_info->renderSky = true;
	viewport_info->renderEnvironment = true;
	viewport_info->renderCharacters = true;
	viewport_info->renderPlayer = true;
	viewport_info->renderOpaquePass = true;
	viewport_info->renderLines = false;
	viewport_info->renderUnderwaterAlphaObjectsPass = false;
	viewport_info->renderWater = true;
	viewport_info->renderAlphaPass = true;
	viewport_info->renderStencilShadows = false;
	viewport_info->renderParticles = false;
	viewport_info->render2D = false;

	viewport_info->doHeadshot = false;
	viewport_info->useMainViewportFOV = false;

	viewport_info->desired_multisample_level = 1;
	viewport_info->required_multisample_level = 1;
	viewport_info->stencil_bits = 0;
	viewport_info->depth_bits = 24;
	viewport_info->num_aux_buffers = 0;

	viewport_info->clipping_info.num_clip_planes = 0;
	viewport_info->clear_flags = CLEAR_DEFAULT;

	viewport_info->backgroundimage = NULL;

	viewport_info->name = NULL;
	viewport_info->pPreCallback = NULL;
	viewport_info->pCustomRenderCallback = NULL;
	viewport_info->pPostCallback = NULL;
	
	viewport_info->pSharedPBufferTexture = NULL;
}


bool viewport_Add(ViewportInfo *viewport_info)
{
	assert(s_numViewports < MAX_VIEWPORTS);
	assert(viewport_info);

	if (s_numViewports < MAX_VIEWPORTS)
	{
#if 1
		// Order the viewports so the higher number renderpasses render first
		// Note: the viewports are processed in reverse order
		int index = 0;
		ViewportInfo *bumped;
		while (index < s_numViewports && s_viewports[index]->renderPass < viewport_info->renderPass )
		{
			index++;
		}

		bumped = s_viewports[index];
		s_viewports[index] = viewport_info;

		index++;
		s_numViewports++;

		while(index < s_numViewports)
		{
			ViewportInfo *temp = s_viewports[index];
			s_viewports[index] = bumped;
			bumped = temp;

			index++;
		}

#else
		s_viewports[s_numViewports] = viewport_info;
		s_numViewports++;
#endif

		return true;
	}

	return false;
}

bool viewport_Remove(ViewportInfo *viewport_info)
{
	int i;
	bool found = false;
	
	for (i = s_numViewports - 1; i >= 0; i--)
	{
		if (s_viewports[i] == viewport_info)
		{
			found = true;
			break;
		}
	}

	if (found)
	{
		for (; i < s_numViewports - 1; i++)
		{
			s_viewports[i] = s_viewports[i + 1];
		}

		s_viewports[s_numViewports] = NULL;

		s_numViewports--;
	}

	return found;
}

bool viewport_InitRenderToTexture(ViewportInfo *viewport_info)
{
	PBufferFlags flags;
	bool bIsCubemap;

	viewport_info->isRenderToTexture = true;

	if(viewport_info->pSharedPBufferTexture)
		return true; // using a shared pbuffer (e.g., cubemap faces 1-5, shadowmap atlas)

	bIsCubemap = (viewport_info->target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB && 
					viewport_info->target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB);

	// If needDepthTexture is specified, then depth_bits should not be 0
	assert(!viewport_info->needDepthTexture || (viewport_info->depth_bits > 0));

	flags = PB_RENDER_TEXTURE
			| (rdr_caps.supports_fbos ? PB_FBO : 0)
			| (!rdr_caps.use_pbuffers ? PB_ALLOW_FAKE_PBUFFER : 0)
			| ((viewport_info->floatComponents && (rdr_caps.features & GFXF_FPRENDER)) ? PB_FLOAT : 0)
			| (bIsCubemap ? PB_CUBEMAP : 0)
			| (viewport_info->needColorTexture ? PB_COLOR_TEXTURE : 0)
			| (viewport_info->needDepthTexture ? PB_DEPTH_TEXTURE : 0)
			| (viewport_info->isMipmapped ? PB_MIPMAPPED : 0)
			| (viewport_info->needAlphaChannel ? PB_ALPHA : 0)
			| (viewport_info->needAuxColorTexture ? PB_COLOR_TEXTURE_AUX : 0)
			;

	pbufInit(&viewport_info->PBufferTexture,
			 viewport_info->width,
			 viewport_info->height,
			 flags,
			 viewport_info->desired_multisample_level,
			 viewport_info->required_multisample_level,
			 viewport_info->stencil_bits,
			 viewport_info->depth_bits,
			 viewport_info->num_aux_buffers,
			 viewport_info->maxMipLevel);

	return true;
}

bool viewport_Delete(ViewportInfo *viewport_info)
{
	if( !viewport_info->pSharedPBufferTexture )
		pbufDelete(&viewport_info->PBufferTexture);

	return true;
}

void viewport_RenderAll()
{
	int i;
	F32 savedgamestatefov = game_state.fov;
	F32 savedgamestatefov_1st = game_state.fov_1st;
	F32 savedgamestatefov_3rd = game_state.fov_3rd;
	F32 savedgamestatefovcustom = game_state.fov_custom;
	F32 savedgamestatefovthisframe = game_state.fov_thisframe;
	F32 saved_lod_entity_override = game_state.lod_entity_override;
	F32 saved_lod_scale = game_state.lod_scale;

	Mat4 savedCamMat;

	Entity *player = playerPtr();
	SeqInst *seq = player ? player->seq : NULL;
	int player_alphacam = 0;
	int player_hidden = 0;

	// Early out because we do not want to call winMakeCurrent() and gfxWindowReshape()
	// if no viewports are rendered
	if (s_numViewports == 0)
		return;

	rdrBeginMarker(__FUNCTION__);

	if (seq)
	{
		player_alphacam = seq->alphacam;
		seq->alphacam = 255;
	}

	if (player)
	{
		player_hidden = ENTHIDE(player);
	}

	// Save the main camera matrix before rendering the other viewports
	// for a 'just in case' restoration when we are done
	copyMat4(cam_info.cammat, savedCamMat);
	
	// iterate over the array of viewports in reverse order
	// in case one removes itself
	for (i = s_numViewports - 1; i >= 0; i--)
	{
		ViewportInfo *viewport = s_viewports[i];
		assert(viewport != NULL);

		// Rotate the viewports at the beginning of a new SLI frame
		if(!(game_state.client_frame % game_state.sliFBOs)) {
			viewport->next_update = (viewport->next_update+1) % viewport->update_interval;
		}

		if (viewport->next_update == 0)
		{
			if (viewport->pPreCallback)
			{
				bool bSkipViewport;
				rdrBeginMarker(__FUNCTION__ ": %s [PreCallback]", viewport->name);
				bSkipViewport = !viewport->pPreCallback(viewport);
				rdrEndMarker();
				if( bSkipViewport )
				{
					// SKIP VIEWPORT
					continue; // skip rendering this viewport
				}
			}
			
			if( viewport->renderCharacters && player)
			{
				SET_ENTHIDE(player) = viewport->renderPlayer ? 0 : 1;
			}

			rdrBeginMarker(__FUNCTION__ ": %s [Render]", viewport->name);
			// if viewport has its own custom rendering call use that
			// otherwise stick with the default
			if (viewport->pCustomRenderCallback)
			{
				viewport->pCustomRenderCallback(viewport);
			}
			else
			{
				gfxRenderViewport(viewport);
			}
			rdrEndMarker();

			if (viewport->pPostCallback)
			{
				rdrBeginMarker(__FUNCTION__ ": %s [PostCallback]", viewport->name);
				viewport->pPostCallback(viewport);
				rdrEndMarker();
			}
		}
	}

	if (seq)
	{
		seq->alphacam = player_alphacam;
	}

	if (player)
	{
		SET_ENTHIDE(player) = player_hidden;
	}


	// restore some important setting after coming out
	// of the viewport render loop just in case something
	// tests before main viewport render loop resets them
	copyMat4(savedCamMat, cam_info.cammat);
	gfxSetViewMat(cam_info.cammat, cam_info.viewmat, cam_info.inv_viewmat);

	game_state.lod_entity_override = saved_lod_entity_override;
	game_state.lod_scale = saved_lod_scale;
	game_state.fov = savedgamestatefov;
	game_state.fov_1st = savedgamestatefov_1st;
	game_state.fov_3rd = savedgamestatefov_3rd;
	game_state.fov_custom = savedgamestatefovcustom;

	winMakeCurrent();
	gfxWindowReshape();
	rdrEndMarker();
}

// update just one viewport (used by utility code at the moment)
// return true if viewport rendered
bool viewport_RenderOneUtility(ViewportInfo *viewport)
{
	F32 savedgamestatefov = game_state.fov;
	F32 savedgamestatefov_1st = game_state.fov_1st;
	F32 savedgamestatefov_3rd = game_state.fov_3rd;
	F32 savedgamestatefovcustom = game_state.fov_custom;
	F32 savedgamestatefovthisframe = game_state.fov_thisframe;
	F32 saved_lod_entity_override = game_state.lod_entity_override;
	F32 saved_lod_scale = game_state.lod_scale;
	Mat4 savedCamMat;
	bool bRenderViewport = true;

	// Save the main camera matrix before rendering the other viewports
	// for a 'just in case' restoration when we are done
	copyMat4(cam_info.cammat, savedCamMat);


	if (viewport->pPreCallback)
	{
		if( !viewport->pPreCallback(viewport) )
		{
			// pre callback returning false indicates that
			// the viewport render should be skipped
			bRenderViewport = false;
		}
	}

	if (bRenderViewport)
	{
		rdrBeginMarker(__FUNCTION__ ":%s", viewport->name);

		// if viewport has its own custom rendering call use that
		// otherwise stick with the default
		if (viewport->pCustomRenderCallback)
		{
			viewport->pCustomRenderCallback(viewport);
		}
		else
		{
			gfxRenderViewport(viewport);
		}

		if (viewport->pPostCallback)
		{
			viewport->pPostCallback(viewport);
		}

		rdrEndMarker();
	}

	// restore some important setting after coming out
	// of the viewport render loop just in case something
	// tests before main viewport render loop resets them
	copyMat4(savedCamMat, cam_info.cammat);
	gfxSetViewMat(cam_info.cammat, cam_info.viewmat, cam_info.inv_viewmat);

	game_state.lod_entity_override = saved_lod_entity_override;
	game_state.lod_scale = saved_lod_scale;
	game_state.fov = savedgamestatefov;
	game_state.fov_1st = savedgamestatefov_1st;
	game_state.fov_3rd = savedgamestatefov_3rd;
	game_state.fov_custom = savedgamestatefovcustom;

	return bRenderViewport;
}

// a generic post viewport render callback that can be used to
// do work in the render thread (RT) post viewport draw (e.g blur)
// The command handler in the RT can query the RT state to
// see what viewport/pass it is and decide what to do.
// Note that the viewports render targt/state should still be
// current when this is handled by the RT
bool viewport_GenericPostCallback(ViewportInfo *viewport)
{
	// Note: Passing a POINTER, not a copy of the data struct.
	// @todo make sure this won't cause thread safety issues
	// should render thread already have viewport info from when we began drawing?
	rdrQueue( DRAWCMD_VIEWPORT_POST, &viewport, sizeof(ViewportInfo*) );
	return true;
}
