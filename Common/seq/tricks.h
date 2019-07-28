#ifndef _TRICKS_H
#define _TRICKS_H

#include "stdtypes.h"
#include "rt_state.h"
#include "texEnums.h"
#include "earray.h"

typedef struct StAnim StAnim;

//ObjectInfo->flags
typedef enum ModelFlags
{
	OBJ_ALPHASORT		= 1 << 0, // At least some of the subs on this model need to be alpha sorted
	OBJ_FANCYWATER		= 1 << 1,
	OBJ_FULLBRIGHT		= 1 << 2,
	OBJ_SUNFLARE		= 1 << 3,
	OBJ_NOLIGHTANGLE	= 1 << 4,


	OBJ_LOD				= 1 << 7,
	OBJ_TREEDRAW		= 1 << 8,
	OBJ_ALLOW_MULTISPLIT = 1 << 9,
	OBJ_FORCEOPAQUE		= 1 << 10,
	OBJ_BUMPMAP			= 1 << 11,
	OBJ_WORLDFX			= 1 << 12, //draw this object using model draw
	OBJ_CUBEMAP			= 1 << 13,
	OBJ_DRAWBONED		= 1 << 14, //draw this object using model draw boned node
	OBJ_STATICFX		= 1 << 15, //add static lighting to this object
	OBJ_HIDE			= 1 << 16, //don't draw me except in wireframe

	OBJ_ALPHASORTALL	= 1 << 18, // All subs on this model must be alpha sorted

	OBJ_DONTCASTSHADOWMAP	= 1 << 19,			// will not appear in the shadow map
	OBJ_DONTRECEIVESHADOWMAP	= 1 << 20,		// will not receive shadows from the shadow map

} ModelFlags;

enum
{
	GROUP_VIS_OUTSIDE		= (1 << 0),
	GROUP_VIS_BLOCKER		= (1 << 1),
	GROUP_VIS_ANGLEBLOCKER	= (1 << 2),
	GROUP_VIS_TRAY			= (1 << 3),
	GROUP_VIS_WINDOW		= (1 << 5),
	GROUP_VIS_DOORFRAME		= (1 << 6),
	GROUP_REGION_MARKER		= (1 << 7),
	GROUP_VOLUME_TRIGGER	= (1 << 8),
	GROUP_WATER_VOLUME		= (1 << 9),
	GROUP_LAVA_VOLUME		= (1 << 10),
	GROUP_DOOR_VOLUME		= (1 << 11),
	GROUP_PARENT_FADE		= (1 << 12),
	GROUP_KEY_LIGHT			= (1 << 13),
	GROUP_SEWERWATER_VOLUME	= (1 << 14),
	GROUP_REDWATER_VOLUME	= (1 << 15),
	GROUP_MATERIAL_VOLUME	= (1 << 16),
	GROUP_STACKABLE_FLOOR	= (1 << 17),
	GROUP_STACKABLE_WALL	= (1 << 18),
	GROUP_STACKABLE_CEILING	= (1 << 19),
	GROUP_VIS_OUTSIDE_ONLY	= (1 << 20),
	GROUP_SOUND_OCCLUDER	= (1 << 21),
};

typedef enum BlendIndex
{
	BLEND_OLD_TEX_BASE,
	BLEND_OLD_TEX_GENERIC_BLEND,
	BLEND_BASE1,
	BLEND_MULTIPLY1,
	BLEND_BUMPMAP1,
	BLEND_DUALCOLOR1,
	BLEND_MASK,
	BLEND_BASE2,
	BLEND_MULTIPLY2,
	BLEND_BUMPMAP2,
	BLEND_DUALCOLOR2,
	BLEND_ADDGLOW1,
	BLEND_CUBEMAP,
	BLEND_NUM_BLEND_NAMES,
} BlendIndex;

typedef enum TexOptScrollType
{
	TEXOPTSCROLL_NORMAL,
	TEXOPTSCROLL_PINGPONG,
	TEXOPTSCROLL_OVAL,
} TexOptScrollType;

TexLayerIndex blendIndexToTexLayerIndex(BlendIndex bi);
BlendIndex texLayerIndexToBlendIndex(TexLayerIndex tli);
char * blendIndexToName(BlendIndex bi);

typedef struct TexOpt TexOpt;

typedef struct ScrollsScales
{
	F32		texopt_scale[BLEND_NUM_BLEND_NAMES][2];
	F32		texopt_scroll[BLEND_NUM_BLEND_NAMES][2];
	TexOptScrollType texopt_scrollType[BLEND_NUM_BLEND_NAMES];
	U8		alphaMask; // Use just the alpha channel of the Mask layer
	U8		hasRgb34;
	U8		multiply1Reflect;
	U8		multiply2Reflect;
	U8		baseAddGlow;
	U8		minAddGlow;
	U8		maxAddGlow;
	U8		addGlowMat2;			// flag to add the glow texture to MULTI sub-material 2 instead of sub-material 1 (the default)
	U8		tintGlow;				// flag to tint the glow by the current tint 'const color 1'
	U8		tintReflection;			// flag to tint environment reflections by current tint 'const color 1'
	U8		desaturateReflection;	// flag to desaturate the environment reflection to grayscale before applying
	U8		alphaWater;				// flag to honor base1 texture alpha value when applying water (e.g. for puddles)
	U8		rgba3[4];
	U8		rgba4[4];
	U8		specularRgba1[4];
	U8		specularRgba2[4];
	F32		specularExponent1;
	F32		specularExponent2;
	F32		reflectivity; // for environment mapping (deprecated)
	F32		reflectivityBase;
	F32		reflectivityScale;
	F32		reflectivityPower;
	F32		maskWeight;
	Vec3	diffuseScale;
	Vec3	ambientScale;
	Vec3	diffuseScaleTrick;
	Vec3	ambientScaleTrick;
	Vec3	ambientMin;
	TexOpt *debug_backpointer;
} ScrollsScales;

typedef struct TexOptFallback
{
	char *base_name;
	char *blend_name;
	char *bumpmap;
	BlendModeShader	blend_mode;
	Vec2 scaleST[2];
	Vec3	diffuseScaleTrick;
	Vec3	ambientScaleTrick;
	Vec3	ambientMin;
	U8	useFallback;
} TexOptFallback;

typedef struct CgFxParamVal 
{
	char	*paramName;				// Memory buffer allocated when trick is parsed.
	int		paramKey;				// opaque value set by the cgfx module
	float	vals[4];
} CgFxParamVal;

typedef struct TexOpt
{
	char	*file_name;	// used for artist debug
	char	*name; // modified after text parser finishes (skips beginning slashes and truncstes at period)
	const char *blend_names[BLEND_NUM_BLEND_NAMES];
	U8		swappable[BLEND_NUM_BLEND_NAMES];	// Whether or not each layer can be swapped
	F32		texopt_fade[2];
	ScrollsScales scrollsScales;
	TexOptFlags	flags;
	BlendModeShader	blend_mode;
	int		surface; //seq state flag
	char	*surface_name;
	char	*df_name;
	F32		gloss;
	F32		specularExponent1_parser;
	F32		specularExponent2_parser;
	U32		fileAge;
	ModelFlags model_flags; // Trickle-down flags to apply to a model this texture is on
	TexOptFallback fallback;
} TexOpt;

enum
{
	STANIM_FRAMESNAP	= ( 1 << 0 ),
	STANIM_PINGPONG		= ( 1 << 1 ),
	STANIM_LOCAL_TIMER	= ( 1 << 2 ),	// meaning: ! STANIM_GLOBAL_TIMER
	STANIM_GLOBAL_TIMER	= ( 1 << 3 ),	// meaning, ! STANIM_LOCAL_TIMER
};

typedef struct TrickInfo TrickInfo;

typedef struct TrickNode
{
	F32		tex_scale_y;		// Only used by FX
	F32		st_anim_age;
	U8		trick_rgba[4];
	U8		trick_rgba2[4];		// Keep this right after trick_rgba, as it is referenced as a [2][4]
	union {
		struct {
			int		flags1;
			int		flags2;
		};
		U64 flags64;
	};
	TrickInfo	*info;
} TrickNode;

typedef struct AutoLOD AutoLOD;

typedef struct TrickInfo
{
	char		*file_name; // file this trick was loaded from, for artist debug
	char		*name;
	TrickNode	tnode;
	ModelFlags	model_flags;
	U32			group_flags;
//	BlendModeType	blend_mode;
	F32			lod_near,lod_far,lod_nearfade,lod_farfade;
	StAnim		**stAnim;  //only ever one 
	F32			shadow_dist;
	F32			alpha_ref;	// 0-1
	F32			alpha_ref_parser; // The value read from the parser (0-255)
	F32			tex_bias;
	F32			nightglow_times[2];
	Vec2		sway; // 0 ~ speed, 1 ~ amount
	Vec2		sway_random;  // 0 ~ speed, 1 ~ amount
	Vec2		sway_pitch;
	Vec2		sway_roll;
	F32			lod_scale;
	F32			tighten_up;
	U32			fileAge;
	Vec3		texScrollAmt[2];
	F32			alpha_sort_mod;
	F32			water_reflection_skew;
	F32			water_reflection_strength;
	AutoLOD		**auto_lod; // if NULL, use the model geometry.  if present, each of these generates an LOD.
} TrickInfo;

// Tricks specifiable in trick files should go in this enum
// If the trick is only done programmatically, it should go in the second enum below
//   in order to keep the parser happy and easy
enum
{
	TRICK_ADDITIVE			= 1 << 0,
	TRICK_SUBTRACTIVE		= 1 << 1,
	TRICK_FRONTYAW			= 1 << 2,
	TRICK_FRONTFACE			= 1 << 3,
	TRICK_ALPHACUTOUT		= 1 << 4,

	TRICK_DOUBLESIDED		= 1 << 6,
	TRICK_NOZTEST			= 1 << 7,
	TRICK_REFLECT_TEX1		= 1 << 8,
	TRICK_FANCYWATEROFFONLY = 1 << 9,
	TRICK_NIGHTLIGHT		= 1 << 10,
	TRICK_NOZWRITE			= 1 << 11,
	TRICK_LIGHTMAPSOFFONLY  = 1 << 12,
	TRICK_HIDE				= 1 << 13,


	TRICK_NOCOLL			= 1 << 16,


	TRICK_NOFOG				= 1 << 19,

	TRICK_EDITORVISIBLE		= 1 << 21,
	TRICK_BASEEDITVISIBLE	= 1 << 22,
	TRICK_LIGHTFACE			= 1 << 23,
	TRICK_REFLECT			= 1 << 24,

	TRICK_SIMPLEALPHASORT	= 1 << 26,

	TRICK_PLAYERSELECT		= 1 << 29,
	TRICK_NOCAMERACOLLIDE   = 1 << 30,
	TRICK_NOT_SELECTABLE	= 1 << 31,
};

// Programmaticly specified tricks go here
enum {
	TRICK2_HASADDGLOW		= 1 << 0,
	TRICK2_STSCROLL0		= 1 << 1,
	TRICK2_STIPPLE1			= 1 << 2,
	TRICK2_STIPPLE2			= 1 << 3,
	TRICK2_HAS_SWAY			= 1 << 4,
	TRICK2_STSCROLL1		= 1 << 9,
	TRICK2_WIREFRAME		= 1 << 12,
	TRICK2_STANIMATE		= 1 << 14, 
	TRICK2_PARTICLESYS   	= 1 << 15,
	TRICK2_SETCOLOR			= 1 << 17, 
	TRICK2_NORANDOMADDGLOW	= 1 << 20,
	TRICK2_ALWAYSADDGLOW	= 1 << 21,
	TRICK2_CASTSHADOW		= 1 << 22,
	TRICK2_ALPHAREF			= 1 << 25,
	TRICK2_TEXBIAS			= 1 << 27,
	TRICK2_NIGHTGLOW		= 1 << 28,
	TRICK2_STSSCALE			= 1 << 30,
	TRICK2_FALLBACKFORCEOPAQUE	= 1 << 31,
};

//These are not just tricks but all sorts of gfx node flags (in fact only SORTFIRST is a trick file trick)
enum
{
	GFXNODE_OCCLUDED	= 1 << 0, // in frustum but not drawn
	GFXNODE_HIDE		= 1 << 1,
	GFXNODE_CUSTOMTEX	= 1 << 2,
	GFXNODE_HIDESINGLE  = 1 << 3, //mm hides this gfx node, but not it's kids
	GFXNODE_ALPHASORT	= 1 << 4, //this node has textures that need alpha sorting
	GFXNODE_CUSTOMCOLOR = 1 << 5,
	GFXNODE_SEQUENCER	= 1 << 6, //this gfxnode is part of a seqencer
	GFXNODE_DONTANIMATE	= 1 << 7, //for default bones
	GFXNODE_LIGHTASDOOROUTSIDE   = 1 << 8,
	GFXNODE_TINTCOLOR	= 1 << 9,
	GFXNODE_NEEDSVISCHECK = 1 << 10,
	GFXNODE_SKY			= 1 << 11,
	GFXNODE_APPLYBONESCALE = 1 << 12, // apply bone scale when rendering this node (for make non-skinned fx geo inherit physique scale)

	GFXNODE_DEBUGME		= 1<< 15 // flag set per object when sending to the render thread saying it matches gfxdebugname
	//U16 used for flags! make bigger to add more flags
};


extern int g_disableTrickReload;

typedef void (*TrickReloadCallback)(const char *path);

// Generated by mkproto
void trickLoad(void);
void trickReload(void);
void checkTrickReload(void);
	void trickReloadPostProcess(void);
TexOpt *trickFromTextureName(const char *name, TexOptFlags *texopt_flags);
void trickSetExistenceErrorReporting(int report_errors);
TrickInfo *trickFromObjectName(const char *name,const char *fullpath);
TrickInfo *trickFromName(const char *name,char *fullpath);
void trickCreateTextures(void); // Creates "textures" that are stubbed in tricks but don't exist as .tgas
void trickSetReloadCallback(TrickReloadCallback callback);
int trickCompare(TrickInfo *trick1, TrickInfo *trick2);

bool trickIsDelayedLoadTexOpt(TexOpt *texopt);

// End mkproto
#endif
