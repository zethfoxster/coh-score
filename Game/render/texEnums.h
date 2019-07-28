#ifndef TEXENUMS_H
#define TEXENUMS_H

typedef enum {
	TEX_NOT_LOADED		= 1 << 0,
	TEX_LOADING			= 1 << 1,
	TEX_LOADED			= 1 << 2,
} TexLoadState;

typedef enum {
	TEX_LOAD_IN_BACKGROUND					= 1,
	TEX_LOAD_NOW_CALLED_FROM_MAIN_THREAD,
	TEX_LOAD_NOW_CALLED_FROM_LOAD_THREAD,
	TEX_LOAD_DONT_ACTUALLY_LOAD,
	TEX_LOAD_IN_BACKGROUND_FROM_BACKGROUND,
} TexLoadHow;

typedef enum
{
	TEX_FOR_UI		= 1 << 0,
	TEX_FOR_FX		= 1 << 1,
	TEX_FOR_ENTITY	= 1 << 2,
	TEX_FOR_WORLD	= 1 << 3,
	TEX_FOR_UTIL	= 1 << 4,
	TEX_FOR_NOTSURE	= 1 << 5,
} TexUsage;

typedef enum TexOptFlags
{
	TEXOPT_FADE					= 1 << 0,
	TEXOPT_TRUECOLOR			= 1 << 1,
	TEXOPT_TREAT_AS_MULTITEX	= 1 << 2, // New texture, possibly rendered with old shader, but treat it as a new one
	TEXOPT_MULTITEX				= 1 << 3, // New texture TEXLAYERing scheme
	TEXOPT_NORANDOMADDGLOW		= 1 << 4,
	TEXOPT_FULLBRIGHT			= 1 << 5,
	TEXOPT_CLAMPS				= 1 << 6,
	TEXOPT_CLAMPT				= 1 << 7,
	TEXOPT_ALWAYSADDGLOW		= 1 << 8,
	TEXOPT_MIRRORS				= 1 << 9,
	TEXOPT_MIRRORT				= 1 << 10,
	TEXOPT_REPLACEABLE			= 1 << 11,
	TEXOPT_BUMPMAP				= 1 << 12,
	TEXOPT_REPEATS				= 1 << 13,
	TEXOPT_REPEATT				= 1 << 14,
	TEXOPT_CUBEMAP				= 1 << 15,
	TEXOPT_NOMIP				= 1 << 16,
	TEXOPT_JPEG					= 1 << 17,
	TEXOPT_NODITHER				= 1 << 18,
	TEXOPT_NOCOLL				= 1 << 19,
	TEXOPT_SURFACESLICK			= 1 << 20,
	TEXOPT_SURFACEICY			= 1 << 21,
	TEXOPT_SURFACEBOUNCY		= 1 << 22,
	TEXOPT_BORDER				= 1 << 23,
	TEXOPT_OLDTINT				= 1 << 24,
	TEXOPT_DOUBLEFUSION			= 1 << 25,  // deprecated
	TEXOPT_POINTSAMPLE			= 1 << 26,
	TEXOPT_NORMALMAP			= 1 << 27,
	TEXOPT_SPECINALPHA			= 1 << 28,
	TEXOPT_FALLBACKFORCEOPAQUE	= 1 << 29,
} TexOptFlags;

typedef enum TexLayerIndex
{
	TEXLAYER_BASE,							// texture0			
	TEXLAYER_BASE1=TEXLAYER_BASE,			// texture0
	TEXLAYER_GENERIC,						// texture1	// Old-style second texture (used differently based on type of blend mode)
	TEXLAYER_MULTIPLY1=TEXLAYER_GENERIC,	// texture1 // Share this index for new/old style blending
	TEXLAYER_BUMPMAP1,						// texture2	// Also used for AddGlow building light utility texture
	TEXLAYER_DUALCOLOR1,					// texture3	
	TEXLAYER_MASK,							// texture4	
	TEXLAYER_BASE2,							// texture5	
	TEXLAYER_REFRACTION=TEXLAYER_BASE2,		// texture5	
	TEXLAYER_MULTIPLY2,						// texture6	
	TEXLAYER_PLANAR_REFLECTION=TEXLAYER_MULTIPLY2,// texture6	
	TEXLAYER_BUMPMAP2,						// texture7	
	TEXLAYER_DUALCOLOR2,					// texture8	
	TEXLAYER_ADDGLOW1,						// texture9	
	TEXLAYER_MAX_SCROLLABLE,
	TEXLAYER_CUBEMAP = TEXLAYER_MAX_SCROLLABLE,// texture10	
	
	TEXLAYER_MAX_LAYERS,

	// Non-standard samplers
	TEXLAYER_SHADOWMAP = TEXLAYER_MAX_LAYERS,

	TEXLAYER_ALL_LAYERS = 16,
} TexLayerIndex;

typedef struct BasicTexture BasicTexture;
typedef struct TexBind TexBind;

typedef struct TextureOverride {
	TexBind *base; // Overrides the base texture (and brings with it it's bumpmap)
	BasicTexture *generic;  // Overrides the 2nd texture (detail, etc)
} TextureOverride;

#endif
