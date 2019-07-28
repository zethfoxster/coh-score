#pragma once

#include "stdtypes.h"

#define WORLD_DETAIL_LIMIT 20.0f
#define GFXSETTINGS_VERSION 5

#define FIELDOFVIEW_MIN 40
#define FIELDOFVIEW_STD 55
#define FIELDOFVIEW_MAX 90

typedef enum OptionGraphicsPreset {
	GRAPHICSPRESET_MINIMUM,				// aka Ugly
	GRAPHICSPRESET_PERFORMANCE,			// aka Fast
	GRAPHICSPRESET_RECOMMENDED,
	GRAPHICSPRESET_ULTRA_LOW,
	GRAPHICSPRESET_ULTRA_HIGH,
	GRAPHICSPRESET_NUM_PRESETS
} OptionGraphicsPreset;

typedef enum OptionRenderScale {
	RENDERSCALE_OFF,
	RENDERSCALE_SCALE,
	RENDERSCALE_FIXED,
} OptionRenderScale;

typedef enum OptionShaderDetail {
	SHADERDETAIL_GF4MODE,
	SHADERDETAIL_SINGLE_MATERIAL,
	SHADERDETAIL_DUAL_MATERIAL,
	SHADERDETAIL_HIGHEST_AVAILABLE,
	SHADERDETAIL_NOBUMPS, // No world or character
	SHADERDETAIL_NOWORLDBUMPS, // No world, but still character bumpmaps
} OptionShaderDetail;

typedef enum OptionBloom {
	BLOOM_OFF,
	BLOOM_REGULAR,
	BLOOM_HEAVY,
} OptionBloom;

typedef enum OptionWater {
	WATER_OFF,							// No refractions, no reflections
	WATER_LOW, //WATER_ON_NODEPTH,		// Refraction only with no depth test
	WATER_MED, //WATER_ON,				// Refraction only with depth test
	WATER_HIGH,							// Refraction plus some reflection
	WATER_ULTRA,						// Refraction plus lots of refleciton
} OptionWater;

typedef enum OptionCubemap {
	CUBEMAP_OFF,				
	CUBEMAP_LOWQUALITY,					// no characters, lower vis scale
	CUBEMAP_MEDIUMQUALITY,				// no characters, med vis scale, uses 3-face cubemap (update once face per frame)
	CUBEMAP_HIGHQUALITY,				// with characters, higher vis scale
	CUBEMAP_ULTRAQUALITY,				// high quality + more vis scale + update all faces every frame
	CUBEMAP_DEBUG_SUPERHQ,				// full quality, for debugging only
} OptionCubemap;

typedef enum OptionShadow {
	SHADOW_OFF,				
	SHADOW_STENCIL,						// AKA Lowest
	SHADOW_SHADOWMAP_LOW,				// old version 3 value for shadow maps
	SHADOW_SHADOWMAP_MEDIUM,
	SHADOW_SHADOWMAP_HIGH,
	SHADOW_SHADOWMAP_HIGHEST,
} OptionShadow;

typedef enum OptionShadowShader {
	SHADOWSHADER_NONE,				
	SHADOWSHADER_FAST,
	SHADOWSHADER_MEDIUMQ,
	SHADOWSHADER_HIGHQ,
	SHADOWSHADER_DEBUG,
} OptionShadowShader;

typedef enum OptionShadowMapSize		// new in version 4
{
	SHADOWMAPSIZE_SMALL,
	SHADOWMAPSIZE_MEDIUM,
	SHADOWMAPSIZE_LARGE,
} OptionShadowMapSize;

typedef enum OptionShadowMapDistance	// new in version 4
{
	SHADOWDISTANCE_CLOSE,
	SHADOWDISTANCE_MIDDLE,
	SHADOWDISTANCE_FAR,
} OptionShadowMapDistance;

typedef enum OptionAmbient {
	AMBIENT_DISABLE,
	AMBIENT_HIGH_PERFORMANCE,
	AMBIENT_PERFORMANCE,
	AMBIENT_BALANCED,
	AMBIENT_QUALITY,
	AMBIENT_HIGH_QUALITY,
	AMBIENT_SUPER_HIGH_QUALITY,
	AMBIENT_ULTRA_QUALITY,
	AMBIENT_OPTION_MAX
} OptionAmbient;

typedef enum OptionAmbientStrength {
	AMBIENT_OFF,
	AMBIENT_LOW,
	AMBIENT_MEDIUM,
	AMBIENT_HIGH,
	AMBIENT_ULTRA,
	AMBIENT_STRENGTH_MAX
} OptionAmbientStrength;

typedef enum OptionAmbientResolution {
	AMBIENT_RES_HIGH_PERFORMANCE,
	AMBIENT_RES_PERFORMANCE,
	AMBIENT_RES_QUALITY,
	AMBIENT_RES_HIGH_QUALITY,
	AMBIENT_RES_SUPER_HIGH_QUALITY,
	AMBIENT_RESOLUTION_MAX
} OptionAmbientResolution;

typedef enum OptionAmbientBlur {
	AMBIENT_NO_BLUR,
	AMBIENT_FAST_BLUR,
	AMBIENT_GAUSSIAN,
	AMBIENT_BILATERAL,
	AMBIENT_GAUSSIAN_DEPTH,
	AMBIENT_BILATERAL_DEPTH,
	AMBIENT_TRILATERAL,
	AMBIENT_BLUR_MAX
} OptionAmbientBlur;

typedef enum OptionStupidShaderTricks {
	SHADERTRICK_NONE,
	SHADERTRICK_OUTLINEONLY,
	SHADERTRICK_PAPER,
	SHADERTRICK_DEPTH,
	SHADERTRICK_MAX
} OptionStupidShaderTricks;

// Graphics settings and other settings saved to registry
// Bump GFXSETTINGS_VERSION if we need to do new processing when this is loaded
// in gfxSettingsFixup()
// Also update the autoResumeRegInfo structure in clientcomm/autoResumeInfo.c to load/save this to the registry
typedef struct GfxSettings
{
	bool filledIn;
	int screenX;
	int screenY;
	int refreshRate;
	int screenX_pos;
	int screenY_pos;
	int maximized;
	int fullScreen;

	int showAdvanced;
	F32 slowUglyScale;

	struct {
		int physicsQuality;
		int ageiaOn;
		int mipLevel;							// -1 = full, 1 = lower, I think 3 is lowest
		int entityMipLevel;
		F32 worldDetailLevel;					// vis_scale.  1.0 = normal  
		F32	entityDetailLevel;					// scale the LOD switch distances for entities
		int	maxParticles;						// most particles allowed at a time
		F32	maxParticleFill;					// max particlefill allowed at a time
		int useVSync;
		int colorMouseCursor;
		OptionShadow shadowMode; 				// 0 = off, 1 = stencil, 2 = shadow maps
		OptionCubemap cubemapMode; 				// 0 = off, 1 = low, 2 = high
		int enableVBOs;
		int texAniso;							// 1 = default, 16 = max on GF6
		int texLodBias;							// 0 = default, 4 = no bias anywhere
		OptionShaderDetail shaderDetail;		// 0=Low/GF4, 1=SingleMaterial, 2=GF6
		OptionWater useWater;
		int buildingPlanarReflections;			// 0 = off (cubemap or none), 1 = planar reflections
		OptionBloom useBloom;
		F32 bloomMagnitude;
		int useDOF;
		F32 dofMagnitude;
		int suppressCloseFx;

		struct {
			int showAdvanced;
			OptionShadowShader shader;			// 0 = off, 1 = fast, 2 = medium, 3 = high quality
			OptionShadowMapSize size;			// 0 = small (512), 1 = medium (1024), 2 = large (2048)
			OptionShadowMapDistance distance;	// 0 = Close, 1 = Middle, 2 = Far
		} shadowMap;

		struct {
			int showAdvanced;
			F32 optionScale;
			OptionAmbient option;
			OptionAmbientStrength strength;
			OptionAmbientResolution resolution;
			OptionAmbientBlur blur;
		} ambient;

		int useDesaturate;

		int showExperimental;
	} advanced;

	int useCelShader;
	struct {
		int posterization;					// 0 = off, 1 = low, 2 = normal (default), 3 = high, 4 = extreme
		int alphaquant;						// 0 = off, 1 = low, 2 = normal (default), 3 = high
		F32 saturation;						// 0 = 1.0 exponent, 0.95 = 0.05 exponent
		int outlines;						// 0 = off, 1 = on (default), 2 = heavy (slow)
		int lighting;						// 0 = normal, 1 = cel (default)
		int suppressfilters;				// 0 = no, 1 = yes (default)
	} celshader;

	struct {
		OptionStupidShaderTricks stupidShaderTricks;
	} experimental;

	F32 gamma;					//0.0 = normal -0.7 = lowest 3.4 = highest  
	int antialiasing;

	int autoFOV;
	int fieldOfView;

	F32 fxSoundVolume;
	F32 musicSoundVolume;
	F32 voSoundVolume;
	int	forceSoftwareAudio;
	int enable3DSound;

	int enableHWLights;

	//F32	partVisScale;	//magic number that says how prone the engine is to not draw insignificant particle systems

	F32 renderScaleX;
	F32 renderScaleY;
	OptionRenderScale useRenderScale;

	// Non-graphics settings
	char accountName[128];
	int dontSaveName;

	int version;			// GFXSETTINGS_VERSION
} GfxSettings;

void gfxSettingsFixup( GfxSettings * gfxSettings );

// Used by gfxResetGfxSettings() and safe fall-back settings
void gfxGetInitialSettings( GfxSettings * gfxSettings );

// Used for safe-mode settings
void gfxGetSafeModeSettings( GfxSettings * gfxSettings );

void gfxGetScaledAdvancedSettings( GfxSettings * gfxSettings, F32 scale );
void gfxApplySettings( GfxSettings * gfxSettings, int pastStartUp, bool onlyDynamic );
void gfxReapplySettings(void);
void gfxGetWindowSettings( GfxSettings * gfxSettings );
void gfxGetSettings( GfxSettings * gfxSettings );
void gfxGetSettingsForNextTime( GfxSettings * gfxSettings );
void gfxResetGfxSettings(void);
void printGfxSettings(void);
void gfxUpdateAmbientAdvanced( GfxSettings * gfxSettings );
void gfxUpdateShadowMapAdvanced( GfxSettings * gfxSettings );

void gfxGetQualityAdvancedSettings( GfxSettings * gfxSettings );
void gfxGetUltraAdvancedSettings( GfxSettings * gfxSettings );

void gfxShowLoadingScreen();
void gfxFinishLoadingScreen();

OptionGraphicsPreset gfxGetPresetEnumFromScale(F32 scale);

void shaderDetailToFeatures(OptionShaderDetail shader_detail, int *features_enable, int *features_disable);
OptionShaderDetail shaderDetailFromFeatures(int features_enabled);
OptionShaderDetail shaderDetailInc(OptionShaderDetail shader_detail, bool *bWrapped);
OptionShaderDetail shaderDetailDec(OptionShaderDetail shader_detail, bool *bWrapped);
OptionShaderDetail shaderDetailIncAllowed(OptionShaderDetail shader_detail, bool *bWrapped); // Increments only through allowed shader detail levels
OptionShaderDetail shaderDetailHighestAvailable();


OptionWater gfxGetMaxWaterMode();
