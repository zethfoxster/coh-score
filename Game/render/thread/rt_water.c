#define RT_PRIVATE
#define RT_ALLOW_BINDTEXTURE
#include "rt_water.h"
#include "ogl.h"
#include "wcw_statemgmt.h"
#include "rt_state.h"
#include "rt_tex.h"
#include "mathutil.h"
#include "rt_font.h"
#include "rt_stats.h"
#include "rt_model.h"
#include "rt_pbuffer.h"
#include "assert.h"
#include "cmdgame.h"
#include "timing.h"
#include "renderWater.h"
#include "rt_cgfx.h"

#include "font.h"

// Change this for extra debugging
#if 0
#undef CHECKGL
#define CHECKGL rdrCheckThread(); checkgl(__FILE__,__LINE__);
#endif

static RdrSetReflectionPlaneParams s_reflection_plane_params;

void rdrFrameGrabDirect(RdrFrameGrabParams *params)
{
	int x0=0,y0=0;
	int w, h;

	int chunksize=1024;
	PBuffer *pbuf = pbufGetCurrentDirect();
	int blit_flags = 0;
	int handle = params->allocated_handles ? params->handle[rdr_frame_state.curFrame % params->allocated_handles] : 0;
	int depth_handle = params->allocated_depth_handles ? params->depth_handle[rdr_frame_state.curFrame % params->allocated_depth_handles] : 0;

	PERFINFO_AUTO_START("rdrFrameGrabDirect", 1);
	rdrBeginMarker(__FUNCTION__);
	
	w = params->screen_width;
	h = params->screen_height;

	if (rdr_caps.copy_subimage_from_rtt_invert_hack && pbufGetCurrentDirect() && !pbufGetCurrentDirect()->isFakedRTT) {
		y0 = params->texture_height - h;
		h += y0;
	}

	WCW_ActiveTexture(0);

	if (!params->copySubImage)
	{
		GLint color_internal_format = GL_RGB8;
		GLenum color_format = GL_RGB;
		GLenum color_type = GL_UNSIGNED_BYTE;
		GLint depth_internal_format = GL_DEPTH_COMPONENT24;
		GLenum depth_format = GL_DEPTH_COMPONENT;
		GLenum depth_type = GL_UNSIGNED_INT;

		if (pbuf)
		{
			color_internal_format = pbuf->color_internal_format;
			color_format = pbuf->color_format;
			color_type = pbuf->color_type;
			depth_internal_format = pbuf->depth_internal_format;
			depth_format = pbuf->depth_format;
			depth_type = pbuf->depth_type;
		}
		else if (params->floatBuffer)
		{
			color_internal_format = GL_RGB16F;
			color_format = GL_RGB;
			color_type = GL_FLOAT;
		}

		if (handle) {
			WCW_BindTexture(GL_TEXTURE_2D, TEXLAYER_BASE, handle);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL;
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL;
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params->floatBuffer?GL_NEAREST:GL_LINEAR); CHECKGL;
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params->floatBuffer?GL_NEAREST:GL_LINEAR); CHECKGL;
			glTexImage2D(GL_TEXTURE_2D, 0, color_internal_format, params->texture_width, params->texture_height, 0, color_format, color_type, NULL); CHECKGL;
		}
		if (depth_handle) {
			WCW_BindTexture(GL_TEXTURE_2D, TEXLAYER_BASE, depth_handle);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL;
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL;
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); CHECKGL;
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); CHECKGL;
			glTexImage2D(GL_TEXTURE_2D, 0, depth_internal_format, params->texture_width, params->texture_height, 0, depth_format, depth_type, NULL); CHECKGL;
		}
	}

	winCopyFrameBufferDirect(GL_TEXTURE_2D, handle, depth_handle, 0, 0, w, h, 0, false, false);

	rdrEndMarker();
	PERFINFO_AUTO_STOP();
}

void rdrFrameGrabShowDirect(RdrFrameGrabParams *params)
{
	int handle = params->allocated_handles ? params->handle[rdr_frame_state.curFrame % params->allocated_handles] : 0;
	int depth_handle = params->allocated_depth_handles ? params->depth_handle[rdr_frame_state.curFrame % params->allocated_depth_handles] : 0;
	F32 s = params->screen_width / (F32)params->texture_width;
	F32 t = params->screen_height / (F32)params->texture_height;

	rdrBeginMarker(__FUNCTION__);
	rdrSetup2DRenderingDirect();
	if (params->noAlphaBlend)
		WCW_BlendFuncPush(GL_ONE, GL_ZERO);

	texSetWhite(TEXLAYER_GENERIC);
	texBind(TEXLAYER_BASE, params->showDepth ? depth_handle : handle);

	WCW_ActiveTexture(TEXLAYER_BASE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE); CHECKGL;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); CHECKGL;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); CHECKGL;

	WCW_Color4(255, 255, 255, params->alpha * 255); CHECKGL;
	glBegin( GL_QUADS );
	glTexCoord2f( 0.5/params->texture_width, t-0.5/params->texture_height );
	glVertex2f( 0, params->screen_height );
	glTexCoord2f( s+0.5/params->texture_width, t-0.5/params->texture_height );
	glVertex2f( params->screen_width, params->screen_height );
	glTexCoord2f( s+0.5/params->texture_width, 0.5/params->texture_height );
	glVertex2f( params->screen_width, 0 );
	glTexCoord2f( 0.5/params->texture_width, 0.5/params->texture_height );
	glVertex2f( 0, 0 );
	glEnd(); CHECKGL;
	RT_STAT_DRAW_TRIS(2)

	WCW_Color4(255, 255, 255, 255);

	texSetWhite(TEXLAYER_BASE);

	if (params->noAlphaBlend)
		WCW_BlendFuncPop();
	rdrFinish2DRenderingDirect();
	rdrEndMarker();
}

static RdrWaterParams water_params;

void rdrWaterSetParamsDirect(RdrWaterParams *params)
{
	water_params = *params;
}

int rdrGetBuildingReflectionTextureHandle()
{
	return water_params.generic_reflection_pbuffer ? water_params.generic_reflection_pbuffer->color_handle[water_params.generic_reflection_pbuffer->curFBO] : rdr_view_state.black_tex_id;
}

// these functions copied from rt_model.c, maybe put them in a header?
static INLINEDBG void drawElements(int tri_count, int ele_base, int ele_count, int *tris)
{
	glDrawElements(GL_TRIANGLES,ele_count,GL_UNSIGNED_INT,&tris[ele_base]); CHECKGL;
	RT_STAT_DRAW_TRIS(tri_count)
}

static __inline void drawLoopWater(VBO *vbo, RdrModel *draw, Vec4 lightdir, RdrTexList *texlists)
{
	int i,ele_count,tex_count=1,ele_base=0,j;
	int tex_index = draw->tex_index;
	F32 waterReflectionSkewScale = rdr_view_state.waterReflectionSkewScale;
	F32 waterReflectionStrength = rdr_view_state.waterReflectionStrength;

	if (draw->has_trick && draw->trick.info && !rdr_view_state.waterReflectionOverride)
	{
		waterReflectionSkewScale = draw->trick.info->water_reflection_skew * 0.01f;
		waterReflectionStrength = draw->trick.info->water_reflection_strength * 0.01f;
	}

	if (draw->can_do_water_reflection)
	{
		// Apply Time Of Day effect
		waterReflectionStrength = waterReflectionStrength * rdr_view_state.waterTODscale;
	}
	else
	{
		// Disable reflection
		waterReflectionStrength = 0.0f;
	}

	tex_count = vbo->tex_count;

	for(i = 0; i < tex_count; i++) 
	{
		F32 waterFresnelBias = rdr_view_state.waterFresnelBias;
		F32 waterFresnelScale = rdr_view_state.waterFresnelScale;
		F32 waterFresnelPower = rdr_view_state.waterFresnelPower;
		
		ele_count = vbo->tex_ids[i].count*3;

		if (tex_index==-1 || tex_index==i)
		{
			RdrTexList *texlist = &texlists[(tex_index==-1)?i:0];

			rdrBeginMarker("TEX:%d", i);

			if (rt_cgGetCgShaderMode())
			{
				cgGLSetManageTextureParameters(rt_cgfxGetGlobalContext(), CG_TRUE);
			}
					
			for (j=0; j<rdr_caps.max_layer_texunits; j++) {
				if (j == TEXLAYER_REFRACTION) {
					texBind(j, water_params.allocated_handles ? water_params.handle[rdr_frame_state.curFrame % water_params.allocated_handles] : 0);
				} else if (j==TEXLAYER_PLANAR_REFLECTION)
				{
					texBind(j, water_params.allocated_depth_handles ? water_params.depth_handle[rdr_frame_state.curFrame % water_params.allocated_depth_handles] : 0);
				} else if (j==TEXLAYER_DUALCOLOR1)
				{
					texBind(j, water_params.water_reflection_pbuffer ? water_params.water_reflection_pbuffer->color_handle[water_params.water_reflection_pbuffer->curFBO] : rdr_view_state.black_tex_id);
				} else {
					if (texlist->texid[j]==0) {
						texBind(j, (j==TEXLAYER_BUMPMAP1||j==TEXLAYER_BUMPMAP2)?rdr_view_state.dummy_bump_tex_id:(j==TEXLAYER_ADDGLOW1)?rdr_view_state.black_tex_id:rdr_view_state.white_tex_id);
					} else {
						texBind(j, texlist->texid[j]);
					}
				}
			}

			// Vertex shader inputs
			setupBumpMultiVertShader(texlist, lightdir);

			// Pixel shader inputs
			// NOTE: Should normally pass draw->blend_mode, but we use the BMB_PLANAR_REFLECTION for water reflections
			setupBumpMultiPixelShader(draw->ambient, draw->diffuse, lightdir, texlist, BlendMode(BLENDMODE_WATER,0), false, false, false, 1.0f);

			WCW_SetCgShaderParam4fv(kShaderPgmType_VERTEX, kShaderParam_PlanarReflectionPlaneVP, *rdrGetWaterReflectionPlane());

			{
				//Vec4 refractionTransform = {1.f/waterTextureSize(water_params.width), 1.f/waterTextureSize(water_params.height), 1, 0};
				Vec4 refractionTransform = {1.f/water_params.refraction_texture_width, 1.f/water_params.refraction_texture_height, 1, 0};
				WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_WaterRefractionTransformFP, refractionTransform);
			}

			if (draw->blend_mode.blend_bits & BMB_PLANAR_REFLECTION)
			{
				Vec4 reflectionTransform = {1.f/water_params.screen_width, 1.f/water_params.screen_height, 1, 0};
				WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_WaterReflectionTransformFP, reflectionTransform);
			}

			// Get the fresnel parameters from the scroll scales if available
			if (texlist->scrollsScales && !game_state.reflectivityOverride)
			{
				ScrollsScales *scrollsScales = texlist->scrollsScales;

				if (scrollsScales->reflectivityBase != -1.0f || scrollsScales->reflectivityScale != -1.0f || scrollsScales->reflectivityPower != -1.0f)
				{
					waterFresnelBias  = scrollsScales->reflectivityBase;
					waterFresnelScale = scrollsScales->reflectivityScale;
					waterFresnelPower = scrollsScales->reflectivityPower;
				}
			}

			{
				//F32 scaledWaterFresnelScale = rdr_view_state.waterFresnelScale * (0.25f + (1.0f - (rdr_view_state.lamp_alpha / 255.0f))*0.75f);

				// skew Normal Scale, skew Depth scale, minimum skew, refleciton skew scale
				//Vec4 refractionParams = {0.13f, 10.0f, 0.07f, 0.0f};
				Vec4 refractionParams = {rdr_view_state.waterRefractionSkewNormalScale,
										 rdr_view_state.waterRefractionSkewDepthScale,
										 rdr_view_state.waterRefractionSkewMinimum,
										 0.0f};
				// reflection skew normal scale, reflectivity, unused, unused
				//Vec4 reflectionParams = {0.3, 0.6, 0.0, 0.0};
				Vec4 reflectionParams = {waterReflectionSkewScale,
										 waterReflectionStrength,
										 0.0f,
										 0.0f};

				// fresnel bias, scale, power for equation
				// saturate(bias + scale * pow( 1 - (I dot N), power )
				Vec4 fresnelParams =  {0,0,0,0};

				if (game_state.waterMode >= WATER_HIGH)
				{
					fresnelParams[0] = waterFresnelBias;
					fresnelParams[1] = waterFresnelScale;
					fresnelParams[2] = waterFresnelPower;
				}

				WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_WaterRefractionParamsFP, refractionParams);
				WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_WaterReflectionParamsFP, reflectionParams);
				WCW_SetCgShaderParam4fv(kShaderPgmType_FRAGMENT, kShaderParam_WaterFresnelParamsFP, fresnelParams);
			}

			WCW_PrepareToDraw();
			drawElements(vbo->tex_ids[i].count, ele_base, ele_count, vbo->tris);

			if (rt_cgGetCgShaderMode())
			{
				cgGLSetManageTextureParameters(rt_cgfxGetGlobalContext(), CG_FALSE);
			}

			rdrEndMarker();
		}
		ele_base += ele_count;
	}
}


void modelDrawWater( RdrModel *draw, U8 * rgbs, RdrTexList *texlist)
{
	VBO *vbo = draw->vbo;
	Vec4 lightdir_src = { 0.70710677,0.70710677,0,0};
	Vec4	lightdir = { 1,0,0,0};
	Mat4	matx;
	static	F32 offset;

	rdrBeginMarker(__FUNCTION__);
	MODEL_PERFINFO_AUTO_START(__FUNCTION__, 1);

	if (draw->has_lightdir)
		mulVecMat3( draw->lightdir, rdr_view_state.viewmat, lightdir );
	else if (rgbs) // Indoor, we don't have accurate light direction for this object
		mulVecMat3( lightdir_src, rdr_view_state.viewmat, lightdir );
	else // Outdoor
		copyVec3( rdr_view_state.sun_direction_in_viewspace, lightdir);

	// It's drawn through a different codepath if we don't have bumpmaps
	assert((rdr_caps.features & GFXF_BUMPMAPS));

	modelBindBuffer(vbo);
	transposeMat4Copy(draw->mat,matx);

	assert(draw->blend_mode.shader == BLENDMODE_WATER);

	modelDrawState(DRAWMODE_BUMPMAP_MULTITEX, ONLY_IF_NOT_ALREADY);
	modelBlendState(draw->blend_mode, ONLY_IF_NOT_ALREADY);
	WCW_PrepareToDraw();

	/////Regular Array Pointers//////////////////////////////////////////////////////////
	texBindTexCoordPointer(TEXLAYER_BASE, vbo->sts);
	WCW_NormalPointer(GL_FLOAT, 0, vbo->norms);
	WCW_VertexPointer(3,GL_FLOAT,0, vbo->verts);

	/////Special Bumpmapping Array Pointers///////////////////////////////////////
	devassert(vbo->tangents);
	glVertexAttribPointerARB( 7, 4, GL_FLOAT, GL_FALSE, 0, vbo->tangents ); CHECKGL;

	if (draw->has_fx_tint) {
		int i;
		for (i=0; i<3; i++) {
			draw->diffuse[i] = draw->diffuse[i]*draw->tint_colors[0][i]/255;
			draw->ambient[i] = draw->ambient[i]*draw->tint_colors[0][i]/255;
		}
	}

	drawLoopWater(vbo, draw, lightdir, texlist);
	MODEL_PERFINFO_AUTO_STOP();
	rdrEndMarker();
}


void rdrDebugCommandsDirect(void *data)
{
}

void rdrSetReflectionPlaneDirect(RdrSetReflectionPlaneParams *params)
{
	s_reflection_plane_params = *params;
}

const Vec4 *rdrGetWaterReflectionPlane()
{
	return &s_reflection_plane_params.water_reflection_plane;
}

const Vec4 *rdrGetBuildingReflectionPlane()
{
	return &s_reflection_plane_params.generic_reflection_plane;
}

