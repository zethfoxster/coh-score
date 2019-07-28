#ifndef _VIEWPORT_H
#define _VIEWPORT_H

#include "stdtypes.h"
#include "rt_pbuffer.h"		// for PBuffer
#include "rt_queue.h"		// for ClearScreenFlags
//#include "gfxSettings.h"	// for OptionRenderScale

typedef struct AtlasTex AtlasTex;

// OpenGL guarantees at least 6 clip planes
#define MAX_CLIP_PLANES 6
#define ClipPlaneFlags_CostumesOnly 1

typedef struct ClippingInfo
{
	int		num_clip_planes;
	Vec4	clip_planes[MAX_CLIP_PLANES];
	int		flags[MAX_CLIP_PLANES];
} ClippingInfo;

typedef struct ViewportInfo ViewportInfo;

typedef bool (*ViewportRenderCallback)(ViewportInfo *pViewportInfo);

typedef struct ViewportInfo
{
	U32		update_interval;	// update interval in frames (1 or less means update every frame)
	U32		next_update;		// in frames.  Can be initialized to > 1 to stagger updates which have the same interval to avoid performance problems

	bool	isRenderToTexture; // initialized by viewport_InitRenderToTexture().  Set to false otherwise
	bool	needColorTexture;	// If not set, then just a renderbuffer is created
	bool	needDepthTexture;	// If not set, then just a renderbuffer is created
	bool	needAuxColorTexture; // set this flag if you also want a color texture created on attachment1 (useful for filter ping-ponging)
	bool	needAlphaChannel;	// by default an RGB fbo is created, set this flag if you want RGBA
	bool	isMipmapped;
	int		maxMipLevel;		// ignored if isMipmapped != true

	// both perspective and ortho
	F32		nearZ;
	F32		farZ;
	bool	bFlipYProjection;
	bool	bFlipXProjection;

	// if fov is non-zero use perspective
	F32		fov;				// ignored if useMainViewportFOV is true
	F32		aspect;				// ignored if useMainViewportFOV is true

	// ortho parameters
	int		left;
	int		right;
	int		top;
	int		bottom;

	// viewport
	int		width;
	int		height;
	bool	floatComponents;

	// for rendering into part of the texture
	int		viewport_width;		// default -1
	int		viewport_height;	// default -1
	int		viewport_x;			// ignored if viewport_width or viewport_height is -1
	int		viewport_y;			// ignored if viewport_width or viewport_height is -1

	// TODO: Fog settings?  Maybe automatically tied in with lod_scale?

	// lod_scale is used by the static terrain/buildings/etc
	F32		lod_scale;			// 1.0 for no scaling, smaller to force lower LODs sooner

	// lod_model_bias will force rendering of lower LOD models without affecting LOD distances
	// (NOT CURRENTLY SUPPORTED)
//	int		lod_model_bias;		// 0 for no bias, automatically clamped to lowest LOD 

	// lod_model_override is used to force rendering of a specific environment model (GroupDef) LOD 
	int		lod_model_override;		// 0 for no override, MAX_LODS to force to lowest LOD (any other values will cause incorrect results)

	// lod_entity_override is used by entities
	int		lod_entity_override;		// 0 for no override, MAX_LODS to force to lowest LOD

	// viewport entity distance from player visibility limit
	float	vis_limit_entity_dist_from_player;	// e.g. only want to reflect entities local to player

	F32		debris_fadein_size;	// default 4.0
	F32		debris_fadeout_size;	// default 8.0

	// cameras location/orientation. corresponds to cam_info.cammat.
	// N.B. This is NOT view matrix. That needs to be derived using gfxSetViewMat()
	Mat4	cameraMatrix;  

	int		renderPass;

	// these are options which control rendering into a viewport
	bool	renderSky;
	bool	renderEnvironment;					// traverse the map groups scene graph (world geometry, trays)
	bool	renderCharacters;					// traverse the entity tree (e.g. skinned models and attachments)
	bool	renderPlayer;						// if renderCharacters is true, can additionally specify to hide or show player
	bool	renderWater;						// for fancy water
	bool	renderUnderwaterAlphaObjectsPass;
	bool	renderStencilShadows;				// currently this is only for the old style stencil shadow volumes
	bool	renderLines;						// editor/debug
	bool	renderParticles;
	bool	render2D;							// sprites, GUI;

	bool	doHeadshot;

	// these are primarily debug options which controls rendering of
	// entire classes of models. However this does not skip some traversals
	// and models from groups, entities, etc are all affected by these
	// as they are co-mingled in the sorting buckets by the point they are applied
	bool	renderOpaquePass;
	bool	renderAlphaPass;

	bool	useMainViewportFOV;

	int		desired_multisample_level;
	int		required_multisample_level;
	int		stencil_bits;
	int		depth_bits;
	int		num_aux_buffers;

	ClippingInfo		clipping_info;
	ClearScreenFlags	clear_flags;				// what to have gl clear at start of render

	AtlasTex	*backgroundimage;	// used by head/body shot

	const char *name;		// mainly for debugging
	int		target;			// GL_TEXTURE_2D or [GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB ... GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB]

	// called before the viewport has been rendered
	ViewportRenderCallback	pPreCallback;	// return false if want to abort the render call

	// called to render the viewport if you need a custom render loop (leave NULL to use the default viewport render)
	ViewportRenderCallback	pCustomRenderCallback;

	// called after the viewport has been rendered
	ViewportRenderCallback	pPostCallback;
	// callback user data
	void * callbackData;

	// Initialized via viewport_InitRenderToTexture()
	PBuffer PBufferTexture;
	PBuffer *pSharedPBufferTexture; // uses a shared pbuffer instead of its own (e.g., for cubemap faces 1-5, shadowmap atlas)
	
} ViewportInfo;

void viewport_InitStructToDefaults(ViewportInfo *viewport_info);

bool viewport_InitRenderToTexture(ViewportInfo *viewport_info);
bool viewport_Delete(ViewportInfo *viewport_info);

bool viewport_Add(ViewportInfo *viewport_info);
bool viewport_Remove(ViewportInfo *viewport_info);

void viewport_RenderAll();
bool viewport_RenderOneUtility(ViewportInfo *viewport);

static INLINEDBG PBuffer* viewport_GetPBuffer(ViewportInfo *viewport_info)
	{ return (viewport_info->pSharedPBufferTexture ? viewport_info->pSharedPBufferTexture : &viewport_info->PBufferTexture); }

bool viewport_GenericPostCallback(ViewportInfo *viewport);

#endif // _VIEWPORT_H
