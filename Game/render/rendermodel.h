#ifndef _RENDERMODEL_H
#define _RENDERMODEL_H

#include "rt_state.h"
#include "stdtypes.h"

typedef struct ViewSortNode ViewSortNode;
typedef struct DefTexSwapNode DefTexSwapNode;
typedef struct GfxNode GfxNode;
typedef struct EntLight EntLight;
typedef struct TrickNode TrickNode;
typedef struct Model Model;
typedef struct RdrTexList RdrTexList;
typedef struct TextureOverride TextureOverride;
typedef struct TexBind TexBind;
typedef struct BasicTexture BasicTexture;
typedef struct DefTracker DefTracker;
typedef struct ScrollsScales ScrollsScales;

void modelDrawGetCharacterLighting(GfxNode *node, Vec3 ambient, Vec3 diffuse, Vec3 lightdir);
RdrTexList *modelDemandLoadTextures(Model *model, TextureOverride *swaps, int require_replaceable, int tex_index, int is_shadowcaster, bool is_world_model, int use_fallback_material);
TexBind *modelGetFinalTextures(BasicTexture **actual_textures,Model *model,
	TextureOverride *swaps,int require_replaceable,int swapIndex,int texidx,
	RdrTexList *rtexlist, BlendModeType *first_blend_mode, bool is_world_model,
	DefTracker *pParent);
void modelDrawWorldModel( ViewSortNode *vs, BlendModeType blend_mode, int tex_index );

void modelDemandLoadSetFlags(void);

BlendModeType swappedBlendMode(BlendModeType model_blend_mode, BlendModeType tex_blend_mode, bool is_world_model);
TexBind *modelSubGetBaseTexture(Model *model, TextureOverride *swaps,int require_replaceable,int swapIndex, int tex_index, bool *material_swap);

#define blendModeFilter(blend_mode, is_world_model) \
	(!(is_world_model)||(rdr_caps.features & GFXF_BUMPMAPS_WORLD))?blend_mode:	\
	(blend_mode.shader==BLENDMODE_BUMPMAP_MULTIPLY)?BlendMode(BLENDMODE_MULTIPLY, 0): \
	(blend_mode.shader==BLENDMODE_BUMPMAP_COLORBLEND_DUAL)?BlendMode(BLENDMODE_COLORBLEND_DUAL, 0): \
	blend_mode

#endif

bool isReflective(const ScrollsScales *scrollsScales, bool isSkinned);

