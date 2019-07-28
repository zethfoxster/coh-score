//
//	rt_shadowmap.h
//
//	Shadow map functions executed on the render thread.
//
#ifndef __rt_shadowmap_h__
#define __rt_shadowmap_h__

#include "rendershadowmap.h"
#include "viewport.h"

#define SHADOWMAP_MAX_SHADOWS 4

typedef enum shadowCull {
	cullNone=0,
	cullBack,
	cullFront
} shadowCull;

typedef enum EShadowDir {
	dirSun=0,
	dirY,
	dirZ,
	dirView,
	dir30,		// 30 degree altitude off horizon
	dir45,		// 45 degree altitude off horizon
	dir60,		// 60 degree altitude off horizon
	dirTweak	// degree elevation given by tweak value
} EShadowDir;

typedef enum shadowDebugShowFlags {
	showMaps2D=1,
	showVolumes3D = 2,
	showAxis = 4,
	showFrustum = 8,
	showSplits = 16,
	showOverview2D = 32,
	showExtrudeCasters = 0x00200000
} shadowDebugFlags;

typedef enum shadowMapFlags {
	renderUsingMaps					=0x00000001,
	quantizeLocal					=0x00000002,
	quantizeScaleOffset				=0x00000004,
	orientOnView					=0x00000008,
	kSortModels						=0x00000010,
	kSortShadowModelsCustom			=0x00000020,
	kSortModelsCustom				=0x00000040,
	kUseCustomRenderLoop			=0x00000080,
	kRenderSkinnedIntoMap			=0x00000100,
	kSingleDrawWhite				=0x00000200,
	kShadowDrawDirect				=0x00000400,
	kShadowSortDrawDirect			=0x00000800,
	kNoQueueThread					=0x00001000,
	kNewThreadModelCmd				=0x00002000,
	kBlockRenderShadowMap			=0x00004000,
	kMainViewRelativeVis			=0x00008000,
	kShadowCapsuleCull				=0x00010000,
	kRenderSkinnedInteriorIntoMap	=0x00020000,
	kUpgradeShadowedToPerPixel		=0x00040000,
	kUpgradeAlwaysToPerPixel		=0x00080000,
	kForceFallbackMaterial			=0x00100000,
	kShadowTestToggle				=0x80000000,
} shadowMapFlags;

typedef enum shadowSampleMode {
	sampleOne=1,
	sampleDither = 2,
	sample16 = 3,
} shadowSampleMode;

typedef enum shadowDeflickerMode {
	deflickerNone=1,
	deflickerStatic = 2,
	deflickerAlways = 3,
} shadowDeflickerMode;

typedef enum shadowViewBound {
	viewBoundFixed=0,
	viewBoundTight,
	viewBoundSphere,
	viewBoundApprox,
} shadowViewBound;

typedef enum shadowStippleFadeMode {
	stippleFadeNever = 0,
	stippleFadeAlways = 1,
	stippleFadeHQOnly = 2,
} shadowStippleFadeMode;

// Different styles of calculating the cascades splits are useful in different circumstances.
typedef enum shadowSplitCalcMode {
	shadowSplitCalc_DynamicMaxSplits = 0,	// recalculate splits dynamically using lambda but always for max number of splits (4) - *current setting for end users*
	shadowSplitCalc_Dynamic = 1,			// recalculate splits dynamically using lambda and number of active cascades
	shadowSplitCalc_StaticAssignment = 2,	// Use the statically assigned split values from the debug menu (used for tweaking/debug at the moment)
} shadowSplitCalcMode;

typedef enum RTShadowMapFlags
{
	kDrawInstanced = 1,
	kShowViewFrustInMap = 2,

} RTShadowMapFlags;

typedef struct shadowMap {
	bool active;
	char index;

	float biasScale;

	Vec3	shadowDir;						// direction of shadowing light (view vector of map)
	Vec3	splitAnchorRaw;				// world space anchor of split (center of shadow map, pre quantization)
	Vec3	splitAnchorQuant;			// world space anchor of split (center of shadow map, post quantization)
	float mapWorldDimension;		// pixel density of map wrt world space

	float csm_eye_z_split_near;	// view frustum z split for cascade (if relevant)
	float csm_eye_z_split_far;	// view frustum z split for cascade (if relevant)

	Mat44 shadowMat;
	Mat44 shadowTexMat;
	Mat4 shadowViewMat;
	Mat4 shadowViewMatInv;

	int texture[MAX_SLI];
	ViewportInfo * viewport;

	float	minModelRadius;		// models with radius below this will be culled out.  Used on main thread.

	Vec4 debugColor;
	char name[32];
} shadowMap;

typedef struct shadowFrustum {	// simple frustum container for shadow operations
	float fov_x, fov_y;
	float z_near, z_far;
	Vec3	origin;
	Vec3	viewDir;
	Mat4	view_to_world;
	Vec3 ntl,ntr,nbl,nbr,ftl,ftr,fbl,fbr;	// frustum corners ntl = near top left, fbr = far bottom right, etc.
} shadowFrustum;

typedef struct shadowMapGlobals {
	bool	enablePCF;
	shadowCull cull;
	float	slopeBias;
	float	constantBias;
	float	modulationClamp_ambient;
	float	modulationClamp_diffuse;
	float	modulationClamp_specular;
	float	modulationClamp_vertex_lit;			// for simple materials when not upgraded to per pixel (e.g. RGBS baked lighting)
	float	modulationClamp_NdotL_rolloff;		// extra N*L rolloff to help hide projection errors.
	float	modulationClamp_ambient_only_obj;	// if the obj doesn't use diffuse lighting (e.g. OBJ_NOLIGHTANGLE) then we need to shadow ambient much more
	float	nightModulationReduction;			// 0.0 = no reduction, 1.0 = 100% reduction
	Vec4	currentShadowParamsFP;				// cache of the currently set shadow params so that we can override more easily (@todo kind of a hack)
	float	current_modulationClamp_ambient_only_obj;	// current value of this as modulated for night time reduction (if any)
	float	debugDrawAlpha;
	float	debugShowSplits;
	int		sampleMode;				// single sample, dither sample, 16 sample, etc
	shadowFrustum	viewFrustum;	// last view frustum that shadows were based on (remembered when shadows 'frozen' for debug display)
	float	debugScale;
	int		debugFlags;
	int		debugShowMapSize;
	int		debugShowMapOffsetX;
	int		debugShowMapOffsetY;
	float	lastCascadeFade;		// 1.0f = fade over 100% of last cascade. 0.5 = fade over last 50% of last cascade.
} shadowMapGlobals;

typedef struct shadowMapSet {
	int numShadows;
	shadowMap maps[SHADOWMAP_MAX_SHADOWS];
} shadowMapSet;

typedef struct VBO VBO;

typedef struct
{
	VBO*			vbo;
	int				tri_count;			// number triangles from vbo to render, generally vbo->tri_count for aggregate batch of opaque but may be less if we need to skip alpha sub-objects at end
	U16				instanceCount;		// number of consecutive draw instances using this vbo, successive items will have 0 here since we just want their matrix data
	unsigned char	lod_alpha;			// overall lod alpha (control optional stipple fade)
	unsigned char	pad;
	Mat4			xform;				// @todo may want to align matrices if use SIMD
} RdrShadowmapModelOpaque;

typedef struct
{
	VBO*			vbo;
	int				tex_id;				// ogl texture id of the texture to alpha test against
	unsigned char	tex_coord_idx;		// index of texture coords in VBO that should be used for alpha test texture lookups
	unsigned char	lod_alpha;			// overall lod alpha (control optional stipple fade)
	unsigned short	tri_start;			// starting index of triangles in this alpha material batch
	int				tri_count;			// number triangles from vbo to render for this alpha material batch
	Mat4			xform;				// @todo may want to align matrices if use SIMD
} RdrShadowmapModelAlphaTest;

typedef struct
{
	VBO*	vbo;
	int		num_bones;				// number of bone transforms following item
	// array of 3x4 transposed bone transforms immediately follows
} RdrShadowmapModelSkinned;

typedef struct
{
	U32									batch_count_opaque;
	U32									batch_count_alphatest;
	U32									batch_count_skinned;

} RdrShadowmapItemsHeader;

void rt_initShadowMapMenu(void);

const shadowMapGlobals* rt_shadowMapOpts(void);


// get the specification of the shadowmapping data
// from the application for this frame (e.g., matrices, fbos, etc)
void rt_shadowmap_init(shadowMapSet* pMapSet);

// update the shadowmap settings for the current scene
void rt_shadowmap_scene_customizations( void* SceneInfo_p );

// setup state for rendering a viewport with shadowmaps
void rt_shadowmap_begin_using(void);
void rt_shadowmap_end_using(void);

// fast render of objects to a shadowmap
void rt_shadowmap_render( void* data );

// set and restore an ambient clamp override
void rt_shadowmap_override_ambient_clamp( void );
void rt_shadowmap_restore_viewport_ambient_clamp( void );

// debug
void rt_shadowmap_dbg_test_settings(void);
void rt_rdrShadowMapDebug2D(void);
void rt_rdrShadowMapDebug3D(void);

// deprecated (for slow shadowmap render path)
void rt_shadowViewportPreCallback(int map_index);
void rt_shadowViewportPostCallback(int map_index);

#endif // __rt_shadowmap_h__
