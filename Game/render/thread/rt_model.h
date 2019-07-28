#ifndef _RT_MODEL_H
#define _RT_MODEL_H

#include "rt_model_cache.h"
#include "tricks.h"

typedef struct Model Model;
typedef struct EntLight EntLight;

typedef struct RdrTexList
{
	int		texid[TEXLAYER_MAX_LAYERS];
	F32		gloss1, gloss2;
	BlendModeType blend_mode;
	ScrollsScales *scrollsScales;
} RdrTexList;

typedef struct VBO VBO;
typedef struct PerformanceInfo PerformanceInfo;

typedef struct RdrModelStruct
{
	Mat4		mat;
	VBO			*vbo;
	U32			alpha : 8;
	U32			stipple_alpha : 8;
	U32			has_tint : 1;
	U32			has_fx_tint : 1;
	U32			has_trick : 1;
	U32			has_lightdir : 1;
	U32			draw_white : 1;
	U32			is_gfxnode : 1;
	U32			is_tree : 1;
	U32			can_do_water_reflection : 1;
	U32			is_fallback_material : 1;
	U32			texlist_size;
	U8			*rgbs;
	F32			brightScale;
	Vec4		ambient;
	Vec4		diffuse;
	Vec4		diffuse_reverse;
	Vec4		lightdir;
	U32			uid;
	BlendModeType blend_mode;
	S16			tex_index; // only draw this texture
	U8			tint_colors[2][4];
	F32			cubemap_attenuation;
	TrickNode	trick;
#ifdef MODEL_PERF_TIMERS
	const char *model_name;
	PerformanceInfo **model_perfInfo;
#endif
	Model*		debug_model_backpointer; // for debugging threaded crashes
} RdrModel;

#ifdef RT_PRIVATE
	#ifndef FINAL
		void modelDrawVertexBasisTBN( VBO* vbo, Vec3* override_positions, Vec3* override_normals, Vec3* override_tangents );
	#endif
void modelDrawDirect(RdrModel *draw);
void modelDrawSolidColor(VBO *vbo, U32 color);
void modelDrawAlphaSortHackDirect(RdrModel *draw);
void drawNormalDir(float* v,float* n);
void setupBumpPixelShader(Vec4 ambient, Vec4 diffuse, Vec4 lightdir, RdrTexList *texlist, BlendModeType blend_mode, bool bSkinnedModel, bool scaleLighting, F32 reflection_attenuation);
void setupBumpMultiPixelShader(Vec4 ambient, Vec4 diffuse, const Vec4 lightdir, RdrTexList *texlist, BlendModeType blend_mode,
							    bool static_lighting, bool scaleLighting, bool bSkinnedModel, F32 reflection_attenuation);
void setupFixedFunctionVertShader(TrickNode *trick);
void setupBumpMultiVertShader(RdrTexList *texlist, Vec4 lightdir);
void modelResetVBOsDirect(void);
#endif


typedef enum ExcludedLayerType {
	EXCLUDE_NONE = 0,
	EXCLUDE_SCROLLSSCALES = 1<<0,
	EXCLUDE_BIND = 1<<1,
	EXCLUDE_ALL = EXCLUDE_SCROLLSSCALES|EXCLUDE_BIND,
} ExcludedLayerType;

static INLINEDBG ExcludedLayerType excludedLayer(BlendModeType blend_mode, TexLayerIndex tli)
{
	if (blend_mode.blend_bits & BMB_SINGLE_MATERIAL) {
		if (tli != TEXLAYER_CUBEMAP && tli >= TEXLAYER_MASK && tli <= TEXLAYER_DUALCOLOR2)
			return EXCLUDE_ALL;
	}
	if (blend_mode.blend_bits & BMB_BUILDING) {
		if (tli != TEXLAYER_CUBEMAP && tli > TEXLAYER_MASK && tli != TEXLAYER_MULTIPLY2 && tli != TEXLAYER_ADDGLOW1)
			return EXCLUDE_ALL;
		if (tli == TEXLAYER_MULTIPLY1 || tli == TEXLAYER_MULTIPLY2 || tli==TEXLAYER_ADDGLOW1)
			return EXCLUDE_NONE;
		return EXCLUDE_SCROLLSSCALES;
	}
	return EXCLUDE_NONE;
}

#endif

