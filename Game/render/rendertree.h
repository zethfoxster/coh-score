#ifndef _RENDERTREE_H
#define _RENDERTREE_H

#include "gfxtree.h"
#include "model.h"
#include "rendermodel.h"
#include "texEnums.h"

typedef struct BasicTexture BasicTexture;
typedef struct TexBind TexBind;
typedef struct DefTexSwapNode DefTexSwapNode;
typedef struct ViewportInfo ViewportInfo;

typedef struct ViewSortNode
{
	Mat4		mat;
	Vec3		mid;
	Model		*model;
	EntLight	*light;
	F32			brightScale;
	union {
		struct { // Old style, dynamic swaps
			TextureOverride tex_binds;
			int			swapIndex;
		};
		struct { // New style, precomputed swaps
			TexBind **materials;
		};
	};
	U8			*rgbs;
	U8			alpha;
	U8			stipple_alpha;
	U32			has_tint				: 1;
	U32			has_fx_tint				: 1;
	U32			has_tricks				: 1;
	U32			draw_white				: 1;
	U32			shadowcaster			: 1;
	U32			use_fallback_material	: 1;
	U32			hide					: 1;
	U32			is_gfxnode				: 1;
	U32			from_tray				: 1; // node w/ Tray portal visibility (VIS_EYE)
	U8			tint_colors[2][4];
	U32			uid;
	TrickNode	*tricks; // Only used in drawing DefTrackers with tricks (wireframe in the editor)
	Vec4		*reflection_planes;
	U32			num_reflection_planes;
} ViewSortNode;


typedef struct ShadowNode
{
	Model *model;
	Mat4 mat;
	U8 alpha;
	U8 shadowMask;
} ShadowNode;

typedef struct AVSNParams {
	GfxNode *node;
	Model *model;
	Mat4Ptr mat;
	F32 *mid;
	U8 alpha;
	U8 stipple_alpha;
	U8 *rgbs;
	U8 *tint_colors;
	EntLight * light;
	F32 brightScale;
	TrickNode *tricks;
	U32 isOccluder				: 1;
	U32 noUseRadius				: 1;
	U32 beyond_fog				: 1;
	U32 is_gfxnode				: 1;
	U32 use_fallback_material	: 1;
	U32 has_tint				: 1; // Tinting via shader (for characters) (V2)
	U32 has_fx_tint				: 1; // Tinting via lighting (for FX) (V2)
	U32 hide					: 1;
	U32 from_tray				: 1; // node w/ Tray portal visibility (VIS_EYE)
	U32	view_cache_add_occluder	:1;
	Model		*view_cache_lowLodModel;
	int uid;
	// Only in Version 1:
	TextureOverride *custom_texbinds;
	int swapIndex;
	// Version 2:
	TexBind **materials; // List of materials if defaults are not being used
	U32 num_reflection_quads;
	Vec3 *reflection_quad_verts;
} AVSNParams;

typedef enum
{
	DSM_OPAQUE,
	DSM_ALPHA_UNDERWATER,
	DSM_ALPHA,
	DSM_SHADOWS,
} DrawSortMode;

typedef enum
{
	TINT_NONE,
	TINT_RED,
	TINT_GREEN,
	TINT_BLUE,
	TINT_PURPLE,
	TINT_YELLOW,
	TINT_CYAN,

	TINT_COUNT
} TintColor;

extern AVSNParams avsn_params;
void addViewSortNodeSkinned(void);
void addViewSortNodeWorldModel(void);

void gfxTreeSetCustomTextures( GfxNode * gfx_node, TextureOverride binds );
void gfxTreeSetCustomTexturesRecur( GfxNode * gfx_node, TextureOverride *binds );
void gfxTreeSetCustomColors( GfxNode * gfx_node, const char rgba[], const char rgba2[], int gfxNodeFlag );
void gfxTreeSetCustomColorsRecur( GfxNode * gfx_node, const char rgba[], const char rgba2[], int gfxNodeFlag );
void gfxNodeSetScale(GfxNode * node, F32 scale, int root );
void gfxTreeSetScale(GfxNode *node);
void drawShadows(void);
void sortModels(int opaque, int alpha);
void drawSortedModels_ex(DrawSortMode mode, int headshot, ViewportInfo *viewport, bool render_water);
#define drawSortedModels(mode) drawSortedModels_ex(mode, 0, NULL, true)
#define drawSortedModels_headshot(mode) drawSortedModels_ex(mode, 1, NULL, true)
void gfxTreeDrawNode(GfxNode *node,Mat4 parent_mat);
void gfxTreeDrawNodeSky(GfxNode *node,Mat4 parent_mat);
void setLightForCharacterEditor(void);
void modelAddShadow( Mat4 mat, U8 alpha, Model *shadowModel, U8 shadowMask );
void gfxTreeSetBlendMode( GfxNode * node );
void clearMatsForThisFrame(void);
void gfxTreeFrameInit(void);
void gfxTreeFrameFinishQueueing(void);
void modelDrawGfxNode( GfxNode * node, BlendModeType blend_mode, int tex_index, TintColor tint, Mat4 viewMatrix );
ViewSortNode *getViewSortNode(int index);

void addViewSortNodeSkinned_Shadowmap(void);
void addViewSortNodeWorldModel_Shadowmap(void);

void drawSortedModels_shadowmap_new( ViewportInfo* viewport );

#endif
