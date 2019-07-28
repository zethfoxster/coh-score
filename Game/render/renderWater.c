#include "renderWater.h"
#include "rt_queue.h"
#include "rt_water.h"
#include "win_init.h"
#include "pbuffer.h"
#include "cmdgame.h"
#include "renderUtil.h"
#include "gfx.h"
#include "gfxSettings.h"
#include "tex.h"
#include "viewport.h"
#include "camera.h"
#include "sprite_base.h"
#include "seq.h" //MAX_LODS
#include "font.h"


static RdrFrameGrabParams framegrab_params;
static RdrFrameGrabParams water_framegrab_params;
static RdrWaterParams water_params;
static RdrSetReflectionPlaneParams reflection_plane_params;

static bool water_reflection_requested = false;
static bool generic_reflection_requested = false;

static ViewportInfo generic_reflection_viewport;
static bool generic_reflection_viewport_inited = false;
static ViewportInfo water_reflection_viewport;
static bool water_reflection_viewport_inited = false;

void rdrFrameGrab(void)
{
	if (!framegrab_params.allocated_handles) {
		framegrab_params.handle[0] = rdrAllocTexHandle();
		framegrab_params.allocated_handles = 1;
	}
	framegrab_params.depth_handle[0] = 0;
	framegrab_params.copySubImage = 0;
	framegrab_params.floatBuffer = gfx_state.postprocessing && (rdr_caps.features & GFXF_FPRENDER);

	windowSize(&framegrab_params.screen_width, &framegrab_params.screen_height);

	framegrab_params.texture_width = waterTextureSize(framegrab_params.screen_width);
	framegrab_params.texture_height = waterTextureSize(framegrab_params.screen_height);

	rdrQueue(DRAWCMD_FRAMEGRAB,&framegrab_params,sizeof(framegrab_params));
}

void rdrFrameGrabShow(F32 alpha)
{
#if 1
	if (!framegrab_params.handle)
		return;
	framegrab_params.alpha = alpha;
	framegrab_params.noAlphaBlend = alpha==2;
	rdrQueue(DRAWCMD_FRAMEGRAB_SHOW,&framegrab_params,sizeof(framegrab_params));
#else
	// Show water frame grab
	if (!water_framegrab_params.handle)
		return;
	water_framegrab_params.alpha = alpha;
	water_framegrab_params.noAlphaBlend = alpha==2;
	rdrQueue(DRAWCMD_FRAMEGRAB_SHOW,&water_framegrab_params,sizeof(water_framegrab_params));
#endif
}

void rdrShowPBuffer(F32 alpha, PBuffer *pbuf, F32 scale)
{
	RdrFrameGrabParams params;
	if (!pbuf->color_handle[0])
		return;
	memcpy(params.handle, pbuf->color_handle, sizeof(params.handle));
	params.alpha = alpha;
	params.screen_width = pbuf->width*scale;
	params.screen_height = pbuf->height*scale;
	params.texture_width = waterTextureSize(params.screen_width);
	params.texture_height = waterTextureSize(params.screen_height);
	params.noAlphaBlend = 1;
	rdrQueue(DRAWCMD_FRAMEGRAB_SHOW,&params,sizeof(params));
}

static bool last_was_float_buffer=false;


int waterTextureSize(int x)
{
	if (!rdr_caps.supports_arb_np2)
		x = pow2(x);
	/*
	if (rdr_caps.chip & NV2X && rdr_caps.supports_arb_np2)
	{
		// Can't read the depth buffer from/to a 2048 wide texture or higher
		if (x>=2048)
			x = 2047;
	}
	*/
	return x;
}

void rdrWaterGrabBuffer(bool doReflection)
{
	int i;	
	bool is_float_buffer = gfx_state.postprocessing && (rdr_caps.features & GFXF_FPRENDER);
	int wantHandles = 0;
	int wantDepthHandles = 0;
	static int recreate = 0;
	int screen_width, screen_height, texture_width, texture_height;

	wantHandles = (game_state.waterMode > WATER_OFF) ? game_state.sliFBOs : 0;
	wantDepthHandles = (game_state.waterMode >= WATER_MED) ? game_state.sliFBOs : 0;

	if (wantHandles != water_framegrab_params.allocated_handles) {
		for(i=0; i<water_framegrab_params.allocated_handles; i++) {
			if(water_framegrab_params.handle[i]) {
				rdrTexFree(water_framegrab_params.handle[i]);
				water_framegrab_params.handle[i] = 0;
			}
		}
		for(i=0; i<wantHandles; i++) {
			water_framegrab_params.handle[i] = rdrAllocTexHandle();
		}
		water_framegrab_params.allocated_handles = wantHandles;
		recreate = MAX(recreate, wantHandles);

		water_params.allocated_handles = water_framegrab_params.allocated_handles;
		memcpy(water_params.handle, water_framegrab_params.handle, sizeof(water_params.handle));
	}

	if (wantDepthHandles != water_framegrab_params.allocated_depth_handles) {
		for(i=0; i<water_framegrab_params.allocated_depth_handles; i++) {
			if(water_framegrab_params.depth_handle[i]) {
				rdrTexFree(water_framegrab_params.depth_handle[i]);
				water_framegrab_params.depth_handle[i] = 0;
			}
		}
		for(i=0; i<wantDepthHandles; i++) {
			water_framegrab_params.depth_handle[i] = rdrAllocTexHandle();
		}
		water_framegrab_params.allocated_depth_handles = wantDepthHandles;
		recreate = MAX(recreate, wantDepthHandles);

		water_params.allocated_depth_handles = water_framegrab_params.allocated_depth_handles;
		memcpy(water_params.depth_handle, water_framegrab_params.depth_handle, sizeof(water_params.handle));

		if(!water_params.allocated_depth_handles) {		
			water_params.allocated_depth_handles = 1;
			water_params.depth_handle[0] = texDemandLoad(white_tex);
		}
	}

	windowSize(&screen_width, &screen_height);
	texture_width = waterTextureSize(screen_width);
	texture_height = waterTextureSize(screen_height);

	// Determine whether or not we need to resize our texture
	if (recreate || screen_width != water_framegrab_params.screen_width || screen_height != water_framegrab_params.screen_height || 
		texture_width != water_framegrab_params.texture_width || texture_height != water_framegrab_params.texture_height || 
		is_float_buffer != last_was_float_buffer)
	{
		water_framegrab_params.copySubImage = 0;
		water_framegrab_params.screen_width = screen_width;
		water_framegrab_params.screen_height = screen_height;
		water_framegrab_params.texture_width = texture_width;
		water_framegrab_params.texture_height = texture_height;

		if(recreate) recreate--;
	} else {
		water_framegrab_params.copySubImage = 1;
	}
	water_framegrab_params.floatBuffer = is_float_buffer;
	last_was_float_buffer = is_float_buffer;
	rdrQueue(DRAWCMD_FRAMEGRAB,&water_framegrab_params,sizeof(water_framegrab_params));

	water_params.refraction_texture_width = texture_width;
	water_params.refraction_texture_height = texture_height;
	water_params.screen_width = screen_width;
	water_params.screen_height = screen_height;
	water_params.water_reflection_pbuffer = (water_reflection_requested && doReflection) ? viewport_GetPBuffer(&water_reflection_viewport) : NULL;
	water_params.generic_reflection_pbuffer = (generic_reflection_requested && doReflection) ? viewport_GetPBuffer(&generic_reflection_viewport) : NULL;
	rdrQueue(DRAWCMD_WATERSETPARAMS,&water_params,sizeof(water_params));
}

static void InitViewport(const char *name, ViewportInfo *viewport, bool *inited, Vec3 reflectionPlane, bool water_reflection, bool sky_only )
{
	F32 distAbovePlane;
	int desired_width;
	int desired_height;
	int desire_filtering;
	F32	reflect_lod_scale = 1.0f;	// these get set on type of reflection and current mode settings
	int reflect_lod_model_override = 0;

	if (water_reflection)
	{
		desired_width = 512;
		desired_height = 512;
		desire_filtering = false;
	}
	else
	{
		desired_width = game_state.reflection_generic_fbo_size;
		desired_height = game_state.reflection_generic_fbo_size;
		desire_filtering = (game_state.cubemapMode >= CUBEMAP_MEDIUMQUALITY);	// based on quality setting we may want a filtering attachment
	}

	if ( !*inited || ((viewport->width != desired_width) || (viewport->height != desired_height) || (viewport->needAuxColorTexture != desire_filtering)) )
	{
		if (*inited)	// if dynamically changing the size then need to remove the old one
		{
			viewport_Remove(viewport);
		}

		viewport_InitStructToDefaults(viewport);

		viewport->width = desired_width;
		viewport->height = desired_height;

		if (!water_reflection)
		{
			// generic reflection
			viewport->needAlphaChannel = true;	// used as a mask for overlay with cubemap
			viewport->needAuxColorTexture = desire_filtering;	// based on quality setting we may want a filtering attachment
		}
		viewport->renderWater = false;
		viewport->useMainViewportFOV = true;
		viewport->name = name;

		viewport->renderPass = (water_reflection) ? RENDERPASS_REFLECTION : RENDERPASS_REFLECTION2;

		viewport->needColorTexture = true;
		viewport->needDepthTexture = true;


		// The water shader only supports tex2Dlod for nvidia right now
		// mipmaps needed for building reflection blurring
		//if (rdr_caps.supports_cg_fp40 && water_reflection)
		{
			//viewport->isMipmapped = true;
			//viewport->maxMipLevel = 4;
		}

		viewport_InitRenderToTexture(viewport);

		viewport_Add(viewport);

		*inited = true;
	}

	viewport->pPostCallback = viewport_GenericPostCallback;

	if (water_reflection)
	{
		viewport->vis_limit_entity_dist_from_player = game_state.reflection_water_entity_vis_limit_dist_from_player;

		// Use normal LODs for WATER_ULTRA. Use lower LODs for non-ultra water
		if ( game_state.waterMode < WATER_ULTRA)
		{
			reflect_lod_scale			= game_state.reflection_water_lod_scale;
			reflect_lod_model_override	= game_state.reflection_water_lod_model_override;
		}
		else
		{
			reflect_lod_scale			= 1.0f;
			reflect_lod_model_override	= game_state.reflection_water_lod_model_override;
		}

		if (sky_only && viewport->renderOpaquePass)
		{
			// Modify the viewport for sky only rendering
			viewport->renderEnvironment = false;
			viewport->renderCharacters = false;
			viewport->renderOpaquePass = false;
			viewport->renderWater = false;
			viewport->renderAlphaPass = false;
		}
		else if (!sky_only && !viewport->renderOpaquePass)
		{
			// Modify the viewport for normal rendering
			viewport->renderEnvironment = true;
			viewport->renderCharacters = true;
			viewport->renderOpaquePass = true;
			viewport->renderWater = true;
			viewport->renderAlphaPass = true;
		}
	}
	else
	{
		// building planar reflections has different modes available
		// We can do a 'full' reflection render or just a 'green screen' of characters (and local geo)
		// to be combined with another environment map texture (e.g. local static cubemap)
		viewport->vis_limit_entity_dist_from_player = game_state.reflection_planar_entity_vis_limit_dist_from_player;

		// use tuning variables to control how much detail we render into reflection
		reflect_lod_scale			= game_state.reflection_planar_lod_scale;
		reflect_lod_model_override	= game_state.reflection_planar_lod_model_override;

		switch (game_state.reflection_planar_mode)
		{
		case kReflectNearby:
			{
				viewport->renderSky = false;
				viewport->renderEnvironment = true;
				viewport->renderCharacters = true;
				viewport->renderWater = false;

				viewport->renderOpaquePass = true;	// make sure overall control flags set
				viewport->renderAlphaPass = true;

				// @todo CLEAR_ALPHA is currently the secret sauce which tells render thread
				// to clear the target to black, including the alpha channel
				viewport->clear_flags = CLEAR_DEPTH | CLEAR_COLOR | CLEAR_ALPHA;
			}
			break;
		case kReflectEntitiesOnly:
			{
				viewport->renderSky = false;
				viewport->renderEnvironment = false;
				viewport->renderCharacters = true;
				viewport->renderWater = false;

				viewport->renderOpaquePass = true;	// make sure overall control flags set
				viewport->renderAlphaPass = true;	// do we want/need alpha pass here?  Yes, some costume pieces are drawn in alpha pass

				// @todo CLEAR_ALPHA is currently the secret sauce which tells render thread
				// to clear the target to black, including the alpha channel
				viewport->clear_flags = CLEAR_DEPTH | CLEAR_COLOR | CLEAR_ALPHA;
			}
			break;
		case kReflectAll:
			{
				viewport->renderSky = true;
				viewport->renderEnvironment = true;
				viewport->renderCharacters = true;
				viewport->renderWater = false;

				viewport->renderOpaquePass = true;	// make sure overall control flags set
				viewport->renderAlphaPass = true;

			viewport->clear_flags = CLEAR_DEFAULT;
			}
			break;
		}
	}

	copyMat4(cam_info.cammat, viewport->cameraMatrix);

	// modify viewport rendering detail values as appropriate for type of reflector and mode
	// @todo this value won't update as game world vis scale is changed unless we change
	// the update function to monitor for it or hold off on doing the scaling until it is used?
	viewport->lod_scale				= game_state.lod_scale * reflect_lod_scale;
	viewport->lod_model_override	= reflect_lod_model_override;

	// Flip the camera pitch
	if (water_reflection)
	{
		viewport->bFlipYProjection = true;
		viewport->bFlipXProjection = true;
	}
	else
	{
		viewport->bFlipYProjection = false;
		viewport->bFlipXProjection = false;
	}

	if (0)//water_reflection)
	{
		// This doesn't handle camear shake properly
		viewport->cameraMatrix[1][0] *= -1;
		viewport->cameraMatrix[1][2] *= -1;
		viewport->cameraMatrix[2][1] *= -1;
	}
	else
	{
		// There's probably a better way to do this
		F32 zdot;
		F32 ydot;

		ydot = dotVec3(viewport->cameraMatrix[1], reflectionPlane);
		scaleAddVec3(reflectionPlane, -2 * ydot, viewport->cameraMatrix[1], viewport->cameraMatrix[1]);

		zdot = dotVec3(viewport->cameraMatrix[2], reflectionPlane);
		scaleAddVec3(reflectionPlane, -2 * zdot, viewport->cameraMatrix[2], viewport->cameraMatrix[2]);

		crossVec3(viewport->cameraMatrix[1], viewport->cameraMatrix[2], viewport->cameraMatrix[0]);
	}

	// offset camera below reflection plane
	distAbovePlane = dotVec3(viewport->cameraMatrix[3], reflectionPlane) + reflectionPlane[3];
	viewport->cameraMatrix[3][0] -= 2 * distAbovePlane * reflectionPlane[0];
	viewport->cameraMatrix[3][1] -= 2 * distAbovePlane * reflectionPlane[1];
	viewport->cameraMatrix[3][2] -= 2 * distAbovePlane * reflectionPlane[2];

	// setup clip plane
	viewport->clipping_info.num_clip_planes = 1;
	viewport->clipping_info.clip_planes[0][0] = reflectionPlane[0];
	viewport->clipping_info.clip_planes[0][1] = reflectionPlane[1];
	viewport->clipping_info.clip_planes[0][2] = reflectionPlane[2];
	viewport->clipping_info.clip_planes[0][3] = reflectionPlane[3];
	viewport->clipping_info.flags[0] = 0;

	// TWEAK: move the actual clip plane away from building to avoid the window rendering into the reflection image
	if (!water_reflection)
	{
		viewport->clipping_info.clip_planes[0][3] -= sqrt(SQR(reflectionPlane[0]) + SQR(reflectionPlane[1]) + SQR(reflectionPlane[2])) * 0.05f;
	}
}

static void ShowDebugTextures()
{
	// Show refraction texture
	if (game_state.waterReflectionDebug == 2 && game_state.waterMode > WATER_OFF)
	{
		static BasicTexture water_refraction_texture;
		static BasicTexture water_refraction_depth_texture;
		static bool texture_inited = false;
		int screen_width, screen_height;

		windowPhysicalSize(&screen_width, &screen_height);

		if (!texture_inited)
		{
			// Based on texGenNew()
			water_refraction_texture.target = 0x0DE1; //GL_TEXTURE_2D;
			water_refraction_texture.actualTexture = &water_refraction_texture;
			water_refraction_texture.load_state[0] = TEX_LOADED;
			water_refraction_texture.gloss = 1;
			water_refraction_texture.name = "water refraction texture";
			water_refraction_texture.dirname = "";
			water_refraction_texture.memory_use[0] = 0;

			water_refraction_depth_texture.target = 0x0DE1; //GL_TEXTURE_2D;
			water_refraction_depth_texture.actualTexture = &water_refraction_depth_texture;
			water_refraction_depth_texture.load_state[0] = TEX_LOADED;
			water_refraction_depth_texture.gloss = 1;
			water_refraction_depth_texture.name = "water refraction depth texture";
			water_refraction_depth_texture.dirname = "";
			water_refraction_depth_texture.memory_use[0] = 0;

			texture_inited = true;
		}

		water_refraction_texture.id = water_framegrab_params.handle[0];
		water_refraction_texture.width = water_refraction_texture.realWidth = 256;//screen_width / 4;
		water_refraction_texture.height = water_refraction_texture.realHeight = 256;//screen_height / 4;

		//display_sprite_tex(&water_refraction_texture, 128, 128, 1, 256.0f / water_refraction_texture.width, 256.0f / water_refraction_texture.height, 0xffffffff);
		display_sprite_ex(0, &water_refraction_texture,
			  128, 128, 1, 256.0f / water_refraction_texture.width, 256.0f / water_refraction_texture.height,
			  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0, 1, 1, 0, 0, 0, clipperGetCurrent(), 1, H_ALIGN_LEFT, V_ALIGN_TOP);

		// Show water refraction depth buffer
		if (game_state.waterMode > WATER_LOW)
		{
			water_refraction_depth_texture.id = water_framegrab_params.depth_handle[0];
			water_refraction_depth_texture.width = water_refraction_depth_texture.realWidth = 256;//screen_width / 4;
			water_refraction_depth_texture.height = water_refraction_depth_texture.realHeight = 256;//screen_height / 4;

			display_sprite_ex(0, &water_refraction_depth_texture,
				  512, 128, 1, 256.0f / water_refraction_depth_texture.width, 256.0f / water_refraction_depth_texture.height,
				  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0, 1, 1, 0, 0, 0, clipperGetCurrent(), 1, H_ALIGN_LEFT, V_ALIGN_TOP);
		}
	}

	// show a sprite with the reflection texture
	if (generic_reflection_requested)
	{
		static BasicTexture generic_reflection_texture;
		static bool texture_inited = false;

		if (!texture_inited)
		{
			generic_reflection_texture.target = 0x0DE1; //GL_TEXTURE_2D;
			generic_reflection_texture.actualTexture = &generic_reflection_texture;
			generic_reflection_texture.load_state[0] = TEX_LOADED;
			generic_reflection_texture.gloss = 1;
			generic_reflection_texture.name = "reflection texture";
			generic_reflection_texture.dirname = "";
			generic_reflection_texture.memory_use[0] = 0;

			texture_inited = true;
		}

		generic_reflection_texture.id = generic_reflection_viewport.PBufferTexture.color_handle[generic_reflection_viewport.PBufferTexture.curFBO];
		generic_reflection_texture.width = generic_reflection_texture.realWidth = generic_reflection_viewport.width;
		generic_reflection_texture.height = generic_reflection_texture.realHeight = generic_reflection_viewport.height;

		display_sprite_tex(&generic_reflection_texture, 128, 384, 1, 256.0f / generic_reflection_texture.width, 256.0f / generic_reflection_texture.height, 0xffffffff);
	}

	// show separate water reflection texture
	if (water_reflection_requested)
	{
		static BasicTexture water_reflection_texture;
		static bool texture_inited = false;

		if (!texture_inited)
		{
			water_reflection_texture.target = 0x0DE1; //GL_TEXTURE_2D;
			water_reflection_texture.actualTexture = &water_reflection_texture;
			water_reflection_texture.load_state[0] = TEX_LOADED;
			water_reflection_texture.gloss = 1;
			water_reflection_texture.name = "water reflection texture";
			water_reflection_texture.dirname = "";
			water_reflection_texture.memory_use[0] = 0;

			texture_inited = true;
		}

		water_reflection_texture.id = water_reflection_viewport.PBufferTexture.color_handle[water_reflection_viewport.PBufferTexture.curFBO];
		water_reflection_texture.width = water_reflection_texture.realWidth = water_reflection_viewport.width;
		water_reflection_texture.height = water_reflection_texture.realHeight = water_reflection_viewport.height;

		display_sprite_tex(&water_reflection_texture, 128, 640, 1, 256.0f / water_reflection_texture.width, 256.0f / water_reflection_texture.height, 0xffffffff);
	}
}

void requestReflection(void)
{
	static Vec4 s_lastframe_water_reflection_plane;
	static Vec4 s_lastframe_reflection_plane;

	bool doGenericReflection = false;
	bool doWaterReflection = false;
	bool doSimpleWaterReflection = false;

	bool generic_reflection_possible = false;
	bool water_reflection_possible = false;

	water_reflection_requested = false;
	generic_reflection_requested = false;

	// Get the previous frame reflection plane in view space and send it to the render thread
	if (reflection_plane_params.water_reflection_active || reflection_plane_params.generic_reflection_active)
	{
		Vec3 pointOnPlaneWS;
		Vec3 pointOnPlaneVS;

		if (reflection_plane_params.water_reflection_active)
		{
			scaleVec3(s_lastframe_water_reflection_plane, -s_lastframe_water_reflection_plane[3], pointOnPlaneWS);
			mulVecMat3(s_lastframe_water_reflection_plane, cam_info.viewmat, reflection_plane_params.water_reflection_plane);
			mulVecMat4(pointOnPlaneWS, cam_info.viewmat, pointOnPlaneVS);
			reflection_plane_params.water_reflection_plane[3] = -dotVec3(pointOnPlaneVS, reflection_plane_params.water_reflection_plane);
		}

		if (reflection_plane_params.generic_reflection_active)
		{
			scaleVec3(s_lastframe_reflection_plane, -s_lastframe_reflection_plane[3], pointOnPlaneWS);
			mulVecMat3(s_lastframe_reflection_plane, cam_info.viewmat, reflection_plane_params.generic_reflection_plane);
			mulVecMat4(pointOnPlaneWS, cam_info.viewmat, pointOnPlaneVS);
			reflection_plane_params.generic_reflection_plane[3] = -dotVec3(pointOnPlaneVS, reflection_plane_params.generic_reflection_plane);
		}

		rdrQueue(DRAWCMD_SETREFLECTIONPLANE,&reflection_plane_params,sizeof(reflection_plane_params));
	}

	if (game_state.waterReflectionDebug > 0 || game_state.reflectionDebug > 0)
	{
		xyprintf(1, 62, "waterReflectionScore: %e", gfx_state.waterReflectionScore);
	}

	if (((game_state.waterMode > WATER_MED && game_state.reflectionEnable > 0) ||
		 (game_state.reflectionEnable == 2))
		&& game_state.waterReflectionDebug != 1)
	{
		generic_reflection_possible = (gfx_state.reflectionCandidateScore != -FLT_MAX) && (game_state.reflectionEnable > 1);
		water_reflection_possible = (gfx_state.waterReflectionScore != -FLT_MAX) && (game_state.reflectionEnable & 1);

		if (game_state.separate_reflection_viewport)
		{
			doGenericReflection = generic_reflection_possible;
			doWaterReflection = water_reflection_possible;
		}
		else
		{
			if (generic_reflection_possible && water_reflection_possible)
			{
				if (gfx_state.waterReflectionScore > gfx_state.reflectionCandidateScore)
				{
					doWaterReflection = true;
				}
				else
				{
					doGenericReflection = true;

					if (game_state.allow_simple_water_reflection)
					{
						doSimpleWaterReflection = true;
					}
				}
			}
			else if (generic_reflection_possible)
			{
				doGenericReflection = true;
			}
			else
			{
				doWaterReflection = water_reflection_possible;
			}
		}
	}
	/*
	else
	{
		water_reflection_possible = true;
		if (game_state.separate_reflection_viewport)
		{
			water_reflection_requested = true;
		}
		else if (gfx_state.waterReflectionScore > gfx_state.reflectionCandidateScore)
		{
			gfx_state.reflectionCandidateScore = gfx_state.waterReflectionScore;
			gfx_state.nextFrameReflectionPlane[0] = gfx_state.waterPlane[0];
			gfx_state.nextFrameReflectionPlane[1] = gfx_state.waterPlane[1];
			gfx_state.nextFrameReflectionPlane[2] = gfx_state.waterPlane[2];
			gfx_state.nextFrameReflectionPlane[3] = gfx_state.waterPlane[3];

			water_reflection_requested = true;
		}
		else
		{
			water_reflection_requested = false;
		}
	}
	*/

	// Setup generic viewport
	if (doGenericReflection)
	{
		InitViewport("generic reflection", &generic_reflection_viewport, &generic_reflection_viewport_inited, gfx_state.nextFrameReflectionPlane, false, false);

		copyVec4(gfx_state.nextFrameReflectionPlane, gfx_state.prevFrameReflectionPlane);
		copyVec4(gfx_state.nextFrameReflectionPlane, s_lastframe_reflection_plane);

		gfx_state.prevFrameReflectionValid = 1;
		generic_reflection_requested = true;
	}
	else if (generic_reflection_viewport_inited)
	{
		viewport_Remove(&generic_reflection_viewport);
		generic_reflection_viewport_inited = false;
		gfx_state.prevFrameReflectionValid = 0;
	}

	// Setup water-specific viewport
	if (doWaterReflection || doSimpleWaterReflection)
	{
		InitViewport("water reflection", &water_reflection_viewport, &water_reflection_viewport_inited, gfx_state.waterPlane, true, doSimpleWaterReflection);

		copyVec4(gfx_state.waterPlane, s_lastframe_water_reflection_plane);

		water_reflection_requested = true;
	}
	else if (water_reflection_viewport_inited)
	{
		viewport_Remove(&water_reflection_viewport);
		water_reflection_viewport_inited = false;
	}

	// show debug textures
	if (game_state.waterReflectionDebug == 2 ||
		(game_state.reflectionDebug == 2 && (water_reflection_requested || generic_reflection_requested)))
	{
		ShowDebugTextures();
	}

	reflection_plane_params.generic_reflection_active = doGenericReflection;
	reflection_plane_params.water_reflection_active = doWaterReflection || doSimpleWaterReflection;

	gfx_state.reflectionCandidateScore = -FLT_MAX;
	gfx_state.waterReflectionScore = -FLT_MAX;
}


