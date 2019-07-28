#define RT_PRIVATE
#include "rendermodel.h"
#include "model.h"
#include "model_cache.h"
#include "ogl.h"
#include <stdio.h>
#include "tricks.h"
#include "error.h"
#include "memcheck.h"
#include "assert.h" 
#include "render.h"
#include "mathutil.h"
#include "cmdgame.h"
#include "bump.h" 
#include "light.h"
#include "camera.h"
#include "utils.h"
#include "rt_queue.h"
#include "rt_tex.h"
#include "rt_cgfx.h"
#include "sun.h"
#include "rt_tricks.h"
#include "tex.h"
#include "group.h"
#include "uiOptions.h"
#include "gfx.h"
#include "renderprim.h"
#include "edit_cmd.h"
#include "cubemap.h"
#include "rendershadowmap.h"
#include "rt_shadowmap.h"

void modelDrawGetCharacterLighting(GfxNode *node, Vec3 ambient, Vec3 diffuse, Vec3 lightdir)
{
	SeqGfxData		*seqGfxData;
	seqGfxData	= node->seqGfxData;

	//TO DO move these checks to light.use
	//Mirrors code in ModelDraw() maybe I should make it a function
	if(seqGfxData && ((game_state.game_mode == SHOW_GAME) || (game_state.game_mode == SHOW_LOAD_SCREEN)) && seqGfxData->light.use & ENTLIGHT_INDOOR_LIGHT )
	{
		copyVec4( seqGfxData->light.ambient, ambient );
		copyVec4( seqGfxData->light.diffuse, diffuse );
		mulVecMat3( seqGfxData->light.direction, cam_info.viewmat, lightdir ); //Could pre calc, but I think it's actually cheaper this way
	}
	else
	{
		F32* scene_amb;

		if( node->flags & GFXNODE_LIGHTASDOOROUTSIDE )
		{
			copyVec3( g_sun.ambient,	ambient );
			copyVec3( g_sun.diffuse,	diffuse );
		}
		else //( seqGfxData->flags & SEQ_USE_DOOR_LIGHT )
		{
			copyVec3( g_sun.ambient_for_players,	ambient );
			copyVec3( g_sun.diffuse_for_players,	diffuse );
		}

		copyVec3( g_sun.direction_in_viewspace, lightdir); //assumes all players are brighter.  Is that always true?

		scene_amb = sceneGetAmbient(); //Done in entLight for characters
		if (scene_amb)
			scaleVec3(scene_amb,.25,ambient);
	}
	//normalVec3(lightdir);
	if (sameVec3(lightdir, zerovec3)) { // Shouldn't happen, but put this in here to be safe
		setVec3(lightdir, 0, 1, 0);
		setVec3(diffuse, 0, 0, 0);
	}

	{	// Ambient light from scene file
		F32* scene_amb;
		scene_amb = sceneGetCharacterAmbientScale();
		if (scene_amb)
		{
			ambient[0] *= scene_amb[0];
			ambient[1] *= scene_amb[1];
			ambient[2] *= scene_amb[2];
		}

		scene_amb = sceneGetMinCharacterAmbient();
		if (scene_amb)
			MAXVEC3(scene_amb, ambient, ambient);
	}

	//Used by pulsing only right now
	if( seqGfxData && seqGfxData->light.use & ( ENTLIGHT_CUSTOM_AMBIENT | ENTLIGHT_CUSTOM_DIFFUSE ) )  
	{
		scaleVec3( ambient, seqGfxData->light.ambientScale, ambient );
		scaleVec3( diffuse, seqGfxData->light.diffuseScale, diffuse );
	}
}

static INLINEDBG void modelSetupLighting(EntLight *light,RdrModel *draw)
{
	if (!light)
	{
		copyVec4(g_sun.ambient, draw->ambient );
		copyVec4(g_sun.diffuse, draw->diffuse );
		draw->has_lightdir = 0;
		//copyVec4(g_sun.direction_in_viewspace, draw->lightdir);
		return;
	}

	if ( light->use & ENTLIGHT_USE_NODIR) {
		copyVec4(light->ambient, draw->ambient);
		copyVec4(light->diffuse, draw->diffuse);
		draw->has_lightdir = 0;
	} else if ( light->use & ENTLIGHT_INDOOR_LIGHT) {
		//Mirrors code in render boned model, maybe make this a function
		//if there's a custom light direction, use it (only worldfx flagged stuff should really use this)

		copyVec4(light->ambient, draw->ambient);
		copyVec4(light->diffuse, draw->diffuse);
		//Another of the many wacky light hacks because character light and world light are different animals
		//and have become not even the same scales, and this needs to be sorted out at some point
		//scaleVec3( light->ambient, 0.5, draw->ambient );
		//scaleVec3( light->diffuse, 0.5, draw->diffuse );
		draw->has_lightdir = 1;
		copyVec4(light->direction, draw->lightdir);

	}
	else
	{
		copyVec4(g_sun.ambient, draw->ambient );
		copyVec4(g_sun.diffuse, draw->diffuse );
		draw->has_lightdir = 0;
		//copyVec4(g_sun.direction_in_viewspace, lightdir);
	}

	if( light->use & ( ENTLIGHT_CUSTOM_AMBIENT | ENTLIGHT_CUSTOM_DIFFUSE ) )  
	{
		scaleVec3( draw->ambient, light->ambientScale, draw->ambient );
		scaleVec3( draw->diffuse, light->diffuseScale, draw->diffuse );
	}
}

static INLINEDBG TexBind *getSwap0(TextureOverride *swaps, TexBind *tex_bind, int require_replaceable)
{
	if (swaps && swaps->base && (!require_replaceable || (tex_bind->tex_layers[TEXLAYER_BASE]->flags & TEX_REPLACEABLE)))
		return (tex_bind==tex_bind->orig_bind)?swaps->base:texFindLOD(swaps->base);
	else
		return tex_bind;
}

static INLINEDBG BasicTexture *getSwapX(int idx,TextureOverride *swaps,BasicTexture *tex_bind,int require_replaceable,TexBind *basebind)
{
	if (swaps && swaps->generic) {
		if (idx==1 && require_replaceable && (tex_bind->flags & TEX_REPLACEABLE))
			return swaps->generic; // World swap
		if (!require_replaceable && basebind->tex_swappable[idx])
			return swaps->generic; // Character swap
	}
	return tex_bind;
}

ScrollsScales *getFallbackScrollsScales(void)
{
	static ScrollsScales *fallbackScrollsScales=NULL;
	static bool inited=false;
	if (!inited) {
		TexOpt *opt = trickFromTextureName("MultiSubFallback", NULL);
		if (opt) {
			fallbackScrollsScales = &opt->scrollsScales;
			copyVec3(opt->scrollsScales.ambientScaleTrick, opt->scrollsScales.ambientScale);
			copyVec3(opt->scrollsScales.diffuseScaleTrick, opt->scrollsScales.diffuseScale);
		}
		inited = true;
	}
	return fallbackScrollsScales;
}

static INLINEDBG BlendModeType swappedBlendModeInline(BlendModeType model_blend_mode, BlendModeType tex_blend_mode, bool is_world_model)
{
	if ((rdr_caps.features & GFXF_BUMPMAPS_WORLD) && 
		((model_blend_mode.shader == BLENDMODE_ADDGLOW && tex_blend_mode.shader <= BLENDMODE_MULTIPLY) ||
		  model_blend_mode.shader == BLENDMODE_WATER ||
		  model_blend_mode.shader == BLENDMODE_SUNFLARE))
	{
		// These blend modes get set specially on the model
		return model_blend_mode;
	} else {
		return blendModeFilter(tex_blend_mode, is_world_model);
	}
}

BlendModeType swappedBlendMode(BlendModeType model_blend_mode, BlendModeType tex_blend_mode, bool is_world_model)
{
	return swappedBlendModeInline(model_blend_mode, tex_blend_mode, is_world_model);
}

extern DefTexSwapNode * texSwaps;

TexBind *modelSubGetBaseTexture(Model *model, TextureOverride *swaps,int require_replaceable,int swapIndex, int tex_index, bool *material_swap)
{
	int		j;
	int tempSwapIndex=swapIndex;
	TexBind *bind = model->tex_binds[tex_index==-1?0:tex_index];
	TexBind *basebind;
	DefTexSwap ** dtswaps;
	basebind=getSwap0(swaps,bind,require_replaceable);
	*material_swap = false;

	// NOTE: Any changes to the texture swapping logic should also be made in checkForTextureSwaps2() in Game\entity\entityclient.c
	while (tempSwapIndex) {
		dtswaps=texSwaps[tempSwapIndex-1].swaps;
		for (j=0;j<eaSize(&dtswaps);j++) {
			if (dtswaps[j]->composite && dtswaps[j]->comp_tex_bind && dtswaps[j]->comp_replace_bind) {
				if (bind->orig_bind==dtswaps[j]->comp_tex_bind->orig_bind) {
					basebind=(bind->orig_bind==bind)?dtswaps[j]->comp_replace_bind:texFindLOD(dtswaps[j]->comp_replace_bind);
					*material_swap = true;
				}
			}
		}
		tempSwapIndex=texSwaps[tempSwapIndex-1].index;
	}
	return basebind;
}


static TexBind *modelGetFinalTexturesInline(BasicTexture **actual_textures,
	Model *model,TextureOverride *swaps,int require_replaceable,int swapIndex,
	int texidx,RdrTexList *rtexlist, BlendModeType *first_blend_mode, 
	bool is_world_model, DefTracker *pParent)
{
	int j, k;
	int tempSwapIndex=swapIndex;
	DefTexSwap ** dtswaps;
	bool bIsMultiTex;
	TexBind *bind,*basebind;
	bool bMaterialSwap;
	char* dfName = 0;
	bind = model->tex_binds[texidx];
	basebind = modelSubGetBaseTexture(model, swaps, require_replaceable, swapIndex, texidx, &bMaterialSwap);


	if (basebind->tex_layers[TEXLAYER_BASE]) // Old style swaps
	{
		TexOpt* trick;
		actual_textures[0] = basebind->tex_layers[TEXLAYER_BASE]->actualTexture;
		trick = trickFromTextureName(actual_textures[0]->name, NULL);

		if (trick && trick->df_name)
			dfName = trick->df_name;
	}

	bIsMultiTex = basebind->texopt &&
		( basebind->texopt->flags & TEXOPT_TREAT_AS_MULTITEX || (basebind->bind_blend_mode.shader == BLENDMODE_ADDGLOW) );

	if (bIsMultiTex || bMaterialSwap)
	{
		BasicTexture *tex = getSwapX(1,swaps,basebind->tex_layers[1],require_replaceable,basebind);
		if (tex)
			actual_textures[1] = tex->actualTexture;
	}
	else
	{
		BasicTexture *tex = getSwapX(1,swaps,bind->tex_layers[1],require_replaceable,basebind);
		if (tex)
			actual_textures[1] = tex->actualTexture;
	}
	for (k=2; k<TEXLAYER_MAX_LAYERS; k++) // Needs to do all layers to fill in 0s even if not used
	{
		BasicTexture *tex = getSwapX(k,swaps,basebind->tex_layers[k],require_replaceable,basebind);
		if (tex)
			actual_textures[k] = tex->actualTexture;
	}

	// New swaps
	// NOTE: Any changes to the texture swapping logic should also be made in checkForTextureSwaps2() in Game\entity\entityclient.c
	while (tempSwapIndex) {
		dtswaps=texSwaps[tempSwapIndex-1].swaps;
		for (j=0;j<eaSize(&dtswaps);j++) {
			if (!dtswaps[j]->composite && dtswaps[j]->tex_bind && dtswaps[j]->replace_bind) {
				for (k=0; k<TEXLAYER_MAX_LAYERS; k++) 
				{
					BasicTexture *swappedTex = dtswaps[j]->tex_bind->actualTexture;
					if (actual_textures[k] && actual_textures[k]==swappedTex)
					{
						actual_textures[k] = dtswaps[j]->replace_bind->actualTexture;
						if (k == 0)
						{
							TexOpt* trick = trickFromTextureName(actual_textures[0]->name, NULL);

							if (trick && trick->df_name)
								dfName = trick->df_name;
						}
					}
				}
			}
		}
		tempSwapIndex=texSwaps[tempSwapIndex-1].index;
	}

	if (actual_textures[TEXLAYER_BUMPMAP1])
		rtexlist->gloss1 = actual_textures[TEXLAYER_BUMPMAP1]->gloss;
	else
		rtexlist->gloss1 = 0;
	if (actual_textures[TEXLAYER_BUMPMAP2])
		rtexlist->gloss2 = actual_textures[TEXLAYER_BUMPMAP2]->gloss;
	else
		rtexlist->gloss2 = 0;

	assert(!basebind->texopt == !basebind->scrollsScales);
	rtexlist->scrollsScales = basebind->scrollsScales;

	rtexlist->blend_mode = swappedBlendModeInline(model->blend_modes[texidx], basebind->bind_blend_mode, is_world_model);

	if (texidx == 0) {
		*first_blend_mode = rtexlist->blend_mode;
	}

	if (model->common_blend_mode.shader == BLENDMODE_ADDGLOW &&
		(model->trick && (model->trick->flags2 & TRICK2_HASADDGLOW)) &&
		rtexlist->blend_mode.shader <= BLENDMODE_MULTIPLY)
	{
		// This sub must also get AddGlow!
		rtexlist->blend_mode = model->common_blend_mode;
	}

	if ((model->flags & (OBJ_FORCEOPAQUE|OBJ_ALLOW_MULTISPLIT))==OBJ_FORCEOPAQUE) {
		// Inherit the first blend mode
		if (texidx != 0 && (first_blend_mode->shader != 0)) {
			rtexlist->blend_mode = *first_blend_mode;
		}
	}

	// Blend-mode based texture choices
	if ( !blendModeHasBump(rtexlist->blend_mode) ) {
		actual_textures[TEXLAYER_BUMPMAP1] = NULL;
		actual_textures[TEXLAYER_BUMPMAP2] = NULL;
	}

	if ((rtexlist->blend_mode.shader == BLENDMODE_ADDGLOW)
		&& 
		(actual_textures[TEXLAYER_GENERIC] == grey_tex ||
		basebind->bind_blend_mode.shader != BLENDMODE_ADDGLOW))
	{
		actual_textures[TEXLAYER_GENERIC] = black_tex;
	}

	return basebind;
}

// Only used by skinnable models
RdrTexList *modelDemandLoadTextures(Model *model,TextureOverride *swaps,int require_replaceable, int tex_index, int is_shadowcaster, bool is_world_model, int use_fallback_material)
{
	int		i, k;
	static RdrTexList	*texlist;
	static int			texlist_max;
	int		count;
	BasicTexture *actual_textures[TEXLAYER_MAX_LAYERS];
	BlendModeType first_blend_mode={0};

	if (tex_index==-1)
		count = model->tex_count;
	else
		count = 1;

	if (count > texlist_max)
	{
		texlist_max = model->tex_count;
		texlist = realloc(texlist,sizeof(texlist[0]) * texlist_max);
	}

	for(i=0;i<count;i++)
	{
		TexBind *basebind;

		ZeroArray(actual_textures);
		basebind = modelGetFinalTexturesInline(actual_textures, model, swaps,
			require_replaceable, 0, tex_index==-1?i:tex_index, &texlist[i], 
			&first_blend_mode, is_world_model,0);
		if (use_fallback_material)
			basebind = texFindLOD(basebind);

		// White texture override
		if (game_state.texoff)
		{
			for (k=0; k<TEXLAYER_MAX_LAYERS; k++) 
				if (actual_textures[k])
					actual_textures[k] = (k == TEXLAYER_BUMPMAP1 || k == TEXLAYER_BUMPMAP2)?dummy_bump_tex:white_tex;
		}
		else if (is_shadowcaster)
		{
			int alphas = 0;
			for (k=0; k<TEXLAYER_MAX_LAYERS; k++)
			{
				if (actual_textures[k] && !(actual_textures[k]->flags & TEX_ALPHA))
				{
					alphas++;
					actual_textures[k] = (k == TEXLAYER_BUMPMAP1 || k == TEXLAYER_BUMPMAP2)?dummy_bump_tex:white_tex;
				}
			}
			if (alphas == TEXLAYER_MAX_LAYERS)
			{
				texlist[i].blend_mode.shader = BLENDMODE_MULTIPLY;
				texlist[i].blend_mode.blend_bits = 0;
			}
		}

		if (((model->trick && (model->trick->flags2 & TRICK2_HASADDGLOW)
			&& texlist[0].blend_mode.shader == BLENDMODE_ADDGLOW) ||
			(texlist[i].blend_mode.shader == BLENDMODE_ADDGLOW))
			&& 
			(actual_textures[TEXLAYER_GENERIC] == grey_tex ||
			basebind->bind_blend_mode.shader != BLENDMODE_ADDGLOW))
		{
			actual_textures[TEXLAYER_GENERIC] = black_tex;
		}

		// actual loading
		for (k=0; k<TEXLAYER_MAX_LAYERS; k++)
			texlist[i].texid[k] = texDemandLoad(actual_textures[k]);
	}
	return texlist;
}

void modelDemandLoadSetFlags(void){
	game_state.texture_special_flags = game_state.texoff;
}

static INLINEDBG RdrTexList *demandLoadTexturesInline2(TexBind *bind, int is_shadowcaster, int use_fallback_material)
{
	// Possible performance speedup scheme:
	//  Cache tex_ids on the TexBind, and only update them once per frame (skips all the dereferencing of textures)
	//  Only works once everything is using the miniTracker styled TexBinds that are authoratative.
	int		k;
	static RdrTexList	texlist;
	BasicTexture *actual_textures[TEXLAYER_MAX_LAYERS];

	if (use_fallback_material)
		bind = texFindLOD(bind);
		
	// Make duplicate since they may get overridden
	memcpy(actual_textures, bind->tex_layers, sizeof(actual_textures));
	texlist.blend_mode = bind->bind_blend_mode;
	
	texlist.scrollsScales = bind->scrollsScales;
	if (bind->tex_layers[TEXLAYER_BUMPMAP1])
		texlist.gloss1 = bind->tex_layers[TEXLAYER_BUMPMAP1]->gloss;
	else
		texlist.gloss1 = 0;
	if (bind->tex_layers[TEXLAYER_BUMPMAP2])
		texlist.gloss2 = bind->tex_layers[TEXLAYER_BUMPMAP2]->gloss;
	else
		texlist.gloss2 = 0;

	if (game_state.texture_special_flags) {
		// White texture override
		if (game_state.texoff)
			for (k=0; k<TEXLAYER_MAX_LAYERS; k++) 
				if (actual_textures[k])
					actual_textures[k] = (k == TEXLAYER_BUMPMAP1 || k == TEXLAYER_BUMPMAP2)?dummy_bump_tex:white_tex;
	}

	if (is_shadowcaster)
	{
		int notalphas = 0;
		for (k=0; k<TEXLAYER_MAX_LAYERS; k++)
		{
			if (actual_textures[k] && !(actual_textures[k]->flags & TEX_ALPHA))
			{
				notalphas++;
				actual_textures[k] = (k == TEXLAYER_BUMPMAP1 || k == TEXLAYER_BUMPMAP2)?dummy_bump_tex:white_tex;
			}
		}
		if (notalphas == TEXLAYER_MAX_LAYERS)
		{
			texlist.blend_mode.shader = BLENDMODE_MULTIPLY;
			texlist.blend_mode.blend_bits = 0;
		}
	}

	// actual loading
	// This does *not* need to dereference the actual_texture pointer, as that was already done on the minitracker
	for (k=0; k<TEXLAYER_MAX_LAYERS; k++)
	{
		// Don't load cubemap texture if not enabled
		if (k == TEXLAYER_CUBEMAP && !gfx_state.doCubemaps)
		{
			texlist.texid[k] = 0;
			continue;
		}

		texlist.texid[k] = texDemandLoad(actual_textures[k]);
	}

	return &texlist;
}

TexBind *modelGetFinalTextures(BasicTexture **actual_textures,Model *model,
	TextureOverride *swaps,int require_replaceable, int swapIndex,int texidx,
	RdrTexList *rtexlist, BlendModeType *first_blend_mode, bool is_world_model,
	DefTracker *pParent)
{
 	return modelGetFinalTexturesInline(actual_textures, model, swaps, 
		require_replaceable, swapIndex, texidx, rtexlist, first_blend_mode,
		is_world_model, pParent);
}

static INLINEDBG bool findReflectors(Model *model, U32 num_reflection_planes, Vec4 *reflection_planes)
{
	U32 i;

	if (!gfx_state.prevFrameReflectionValid)
		return false;

	if (!num_reflection_planes || !reflection_planes)
		return false;

	// Only do reflections on main viewport
	// Should be checked before processReflectors() was called
	assert(gfx_state.mainViewport);

	// see if this object should receive planar reflections this frame
	for (i = 0; i < num_reflection_planes; i++)
	{
		F32 dotproduct = dotVec3(reflection_planes[i], gfx_state.prevFrameReflectionPlane);

		// If the normals are close and the planes are within 1 foot of each other
		if ((dotproduct > 0.95f) && (dotproduct < 1.05) && (fabs(reflection_planes[i][3] - gfx_state.prevFrameReflectionPlane[3]) <= 1.0f))
		{
			return true;
		}
	}

	return false;
}

#define isOverideReflective(isSkinned)	\
	(((isSkinned) && (game_state.skinnedFresnelBias>0 || game_state.skinnedFresnelScale>0)) ||	\
	(!(isSkinned) && (game_state.buildingFresnelBias>0 || game_state.buildingFresnelScale>0)))

bool isReflective(const ScrollsScales *scrollsScales, bool isSkinned)
{
	if (!gfx_state.doCubemaps)
		return false;

#ifndef FINAL
	if (game_state.reflectivityOverride)
		return isOverideReflective(isSkinned);
#endif

	// All models must explicitly opt-in for reflections
	if( scrollsScales )
	{
		return (scrollsScales->reflectivityBase > 0 || scrollsScales->reflectivityScale > 0);
	}

	return false;
}

#include "font.h"

void drawReflector(void)
{
	Vec3 p0_ws, p1_ws, p2_ws, p3_ws;
	Vec3 temp;

	if (!game_state.reflectionDebug)
		return;

	if (gfx_state.reflectionCandidateScore <= 0)
		return;

	// pull the verts a bit closer to the camera to avoid some z-fighting (fake z-bias)
	subVec3(cam_info.cammat[3], gfx_state.reflectionQuadVerts[0], temp);
	normalVec3(temp);
	scaleVec3(temp, 0.1f, temp);
	addVec3(gfx_state.reflectionQuadVerts[0], temp, p0_ws);

	subVec3(cam_info.cammat[3], gfx_state.reflectionQuadVerts[1], temp);
	normalVec3(temp);
	scaleVec3(temp, 0.1f, temp);
	addVec3(gfx_state.reflectionQuadVerts[1], temp, p1_ws);

	subVec3(cam_info.cammat[3], gfx_state.reflectionQuadVerts[2], temp);
	normalVec3(temp);
	scaleVec3(temp, 0.1f, temp);
	addVec3(gfx_state.reflectionQuadVerts[2], temp, p2_ws);

	subVec3(cam_info.cammat[3], gfx_state.reflectionQuadVerts[3], temp);
	normalVec3(temp);
	scaleVec3(temp, 0.1f, temp);
	addVec3(gfx_state.reflectionQuadVerts[3], temp, p3_ws);

	drawQuad3D(p0_ws, p2_ws, p3_ws, p1_ws, 0x80800000, 0);

	xyprintf(1, 61, "building reflectionCandidateScore: %e", gfx_state.reflectionCandidateScore);
}

static INLINEDBG RdrModel *initRdrModel(DrawType drawcmd, Model *model, ViewSortNode *vs, TrickNode *trick, RdrTexList *texlist, BlendModeType force_blend_mode, int tex_index)
{
	RdrModel *draw;
	BlendModeType blend_mode;
	int alpha = vs->alpha;
	U8 * rgbs = vs->rgbs;
	EntLight * light = vs->light;

	// generally the caller is going to supply the blend mode this model is
	// going to use...
	if (force_blend_mode.shader)
	{
		blend_mode = force_blend_mode;
	}
	else
	{
		// but if it comes in as a generic 'modulate' then it is
		// replaced from the model definition (I haven't explored exactly why)
		if (tex_index==-1 || !texlist)
			blend_mode = model->blend_modes[(tex_index==-1)?0:tex_index];
		else
			blend_mode = texlist[0].blend_mode;

		// We do need to be sure to merge in any 'feature' variant blend bits
		// that were supplied by the caller.
		blend_mode.blend_bits |= force_blend_mode.blend_bits;
	}

	if (!blend_mode.shader)
		blend_mode.shader = BLENDMODE_MULTIPLY; // Replace "modulate" with "multiply"

	modelSetupVertexObject(model, VBOS_USE, blend_mode);

	if (!texlist)
		draw = rdrQueueAlloc(drawcmd,sizeof(RdrModel));
	else
		draw = rdrQueueAlloc(drawcmd,sizeof(RdrModel) + sizeof(RdrTexList) * ((tex_index==-1)?model->tex_count:1));

	draw->blend_mode = blend_mode;
	draw->draw_white = 0;
	draw->debug_model_backpointer = model;
	draw->alpha		= alpha;
	draw->rgbs		= rgbs;
	draw->vbo		= model->vbo;
	draw->is_tree	= (model->flags & OBJ_TREEDRAW) ? 1:0;
	draw->texlist_size = (texlist != NULL) ? ((tex_index==-1)?model->tex_count:1) : 0;
	draw->brightScale = vs->brightScale;
	draw->tex_index = tex_index;
	modelSetupLighting(light,draw);
	copyMat4(vs->mat, draw->mat);
	if (texlist)
		memcpy(draw+1,texlist,sizeof(RdrTexList) * ((tex_index==-1)?model->tex_count:1));

	if (trick)
	{
		draw->trick = *trick;
		if (draw->blend_mode.shader == BLENDMODE_ADDGLOW)
			draw->trick.flags2 |= TRICK2_HASADDGLOW;
		draw->has_trick = 1;
	}
	else
	{
		if (draw->blend_mode.shader == BLENDMODE_ADDGLOW)
		{
			ZeroStruct(&draw->trick);
			draw->trick.flags2 = TRICK2_HASADDGLOW;
			draw->has_trick = 1;
		} else {
			draw->has_trick = 0;
		}
	}

	if (draw->blend_mode.shader == BLENDMODE_WATER && model && (model->max[1] - model->min[1]) < 1.0f)
	{
		draw->can_do_water_reflection = 1;
	}

#ifdef MODEL_PERF_TIMERS
	draw->model_name = model->name;
	draw->model_perfInfo = &model->perfInfo;
#endif

	return draw;
}

static TrickNode *mergeTricks(TrickNode *vs_trick, TrickNode *model_trick)
{
	static TrickNode node;
	if (!vs_trick)
		return model_trick;
	if (!model_trick)
		return vs_trick;
	node = *model_trick;

#if 1
	node.flags1 |= vs_trick->flags1;
	node.flags2 |= vs_trick->flags2;

	// prevent downstream crash -- remove stanimate flag if combined node doesnt have the stAnim data
	if(node.flags2 & TRICK2_STANIMATE && !(node.info && node.info->stAnim)) {	
		node.flags2 &= ~TRICK2_STANIMATE;
	}
#else
	// alternate fix:  dont include any vs flags which rely on trick data that is not pulled into the combined trick
	node.flags1 |= vs_trick->flags1;
	node.flags2 |= vs_trick->flags2 & ~(TRICK2_STANIMATE|TRICK2_STSCROLL0|TRICK2_STSSCALE);
#endif

	if (vs_trick->trick_rgba[0]) {
		memcpy(node.trick_rgba, vs_trick->trick_rgba, sizeof(node.trick_rgba));
		memcpy(node.trick_rgba2, vs_trick->trick_rgba2, sizeof(node.trick_rgba2));
	}
	return &node;
}

void modelDrawWorldModel( ViewSortNode *vs, BlendModeType blend_mode, int tex_index )
{
	Model		*model = vs->model;

	if (!model 
#ifndef FINAL
		|| !strstri(model->name, g_client_debug_state.modelFilter) || (g_client_debug_state.submeshIndex >= 0 && tex_index != g_client_debug_state.submeshIndex)
#endif
	)
		return;

	MODEL_PERFINFO_AUTO_START("modelDrawWorldmodel", 1);
	if (tex_index == -1 && !vs->draw_white)
	{
		int i;
		// FORCEOPAQUE objects must get their subs drawn back-to-back, do that now
		assert(vs->model->flags & OBJ_FORCEOPAQUE);
		for (i=0; i<model->tex_count; i++) {
			if (vs->materials[i]->bind_blend_mode.intval != vs->materials[0]->bind_blend_mode.intval) {
				// Hack for the editor drawing a forceopaque object before it's been placed and miniTracker'd
				BlendModeType saved = vs->materials[i]->bind_blend_mode;
				vs->materials[i]->bind_blend_mode = vs->materials[0]->bind_blend_mode;
				modelDrawWorldModel(vs, blend_mode, i);
				vs->materials[i]->bind_blend_mode = saved;
			} else {
				modelDrawWorldModel(vs, blend_mode, i);
			}
		}
	} else {
		int			drawcmd = DRAWCMD_MODEL;
		RdrModel	*draw;
		TrickNode *trick;
		RdrTexList	*texlist = NULL;
		F32 cubemap_attenuation = 0.0f;
		bool using_fallback_material = vs->use_fallback_material;

		if (model->flags & OBJ_TREEDRAW)
			drawcmd = DRAWCMD_MODELALPHASORTHACK;

		// TODO: Move this outside
		trick = mergeTricks(vs->has_tricks?vs->tricks:NULL, model->trick);

		if ( vs->draw_white ) {
			assert(tex_index==-1 || tex_index==0);
			blend_mode = BlendMode(0,0);
		} else {
			TexBind* bind;
			bool bIsTerrain = (vs->is_gfxnode == 0);

			assert(tex_index!=-1);
			bind = vs->materials[tex_index];
			texlist = demandLoadTexturesInline2(bind, vs->shadowcaster, vs->use_fallback_material);
			using_fallback_material |= bind->is_fallback_material;

			// set reflection blendbits and look for possible reflectors
			if ((blend_mode.blend_bits & BMB_CUBEMAP) && ( blend_mode.shader == BLENDMODE_MULTIPLY || blend_mode.shader == BLENDMODE_MULTI ) &&
					( game_state.reflectionEnable >= 2 && gfx_state.prevFrameReflectionValid))
			{
				if (findReflectors(model, vs->num_reflection_planes, vs->reflection_planes))
					blend_mode.blend_bits |= BMB_PLANAR_REFLECTION;
			}	

			if ((blend_mode.shader == BLENDMODE_WATER) && (game_state.waterMode >= WATER_HIGH))
			{
				blend_mode.blend_bits |= BMB_PLANAR_REFLECTION;
			}

			if( blend_mode.blend_bits & BMB_CUBEMAP )
			{
				int cubemapTexID = 0;
				bool forceDynamicCubemap = (bind->tex_layers[TEXLAYER_CUBEMAP] == dummy_dynamic_cubemap_tex);

				if( bind->tex_layers[TEXLAYER_CUBEMAP] && !forceDynamicCubemap ) {
					assert(bind->tex_layers[TEXLAYER_CUBEMAP]->target == GL_TEXTURE_CUBE_MAP);
					// artist specified a particular cubemap to use with this material
					cubemapTexID = texDemandLoad(bind->tex_layers[TEXLAYER_CUBEMAP]);
					cubemap_UseStaticCubemap();
				} else {
					// let cubemap module decide (will return either dynamic cubemap id, or local static static cubemap)
					cubemapTexID = cubemap_GetTextureID(bIsTerrain, vs->mid, forceDynamicCubemap);
				}
				texlist->texid[TEXLAYER_CUBEMAP] = cubemapTexID;
				cubemap_attenuation = cubemap_CalculateAttenuation(bIsTerrain, vs->mid, model->radius, forceDynamicCubemap);
			}			
		}

		draw = initRdrModel(drawcmd, model, vs, trick, texlist, blend_mode, tex_index);

		if (draw)
		{
			draw->stipple_alpha = vs->stipple_alpha;
			draw->draw_white = vs->draw_white;
			draw->has_tint	= vs->has_tint;
			draw->has_fx_tint = vs->has_fx_tint;
			draw->uid = vs->uid;
			draw->is_gfxnode = vs->is_gfxnode;
			draw->cubemap_attenuation = cubemap_attenuation;
			draw->is_fallback_material = using_fallback_material;
			memcpy(draw->tint_colors,vs->tint_colors,sizeof(draw->tint_colors));
			rdrQueueSend();
		}
	}
	MODEL_PERFINFO_AUTO_STOP();
}

