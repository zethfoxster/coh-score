#include "gfxSettings.h"
#include "cmdgame.h"
#include "font.h"
#include "autoResumeInfo.h"
#include "renderUtil.h"
#include "particle.h"
#include "sound.h"
#include "win_init.h"
#include "gfx.h"
#include "file.h"
#include "tex.h"
#include "NwWrapper.h"
#include "gfxLoadScreens.h"
#include "authUserData.h"
#include "dbclient.h"
#include "osdependent.h"
#include "fxdebris.h"
#include "renderWater.h"

void printGfxSettings()
{
	int y=31;
	xyprintf( 30, y++, "game_state.screen_x             %d", game_state.screen_x );
	xyprintf( 30, y++, "game_state.screen_y             %d", game_state.screen_y );
	xyprintf( 30, y++, "game_state.refresh_rate         %d", game_state.refresh_rate );
	xyprintf( 30, y++, "game_state.screen_x_restored    %d", game_state.screen_x_restored );
	xyprintf( 30, y++, "game_state.screen_y_restored    %d", game_state.screen_y_restored );
	xyprintf( 30, y++, "game_state.fullscreen           %d", game_state.fullscreen );
	xyprintf( 30, y++, "game_state.maximized            %d", game_state.maximized );
	xyprintf( 30, y++, "game_state.mipLevel             %d", game_state.mipLevel );
	xyprintf( 30, y++, "game_state.entityMipLevel       %d", game_state.entityMipLevel );
	xyprintf( 30, y++, "game_state.texLodBias           %d", game_state.texLodBias );
	xyprintf( 30, y++, "game_state.texAniso             %d", game_state.texAnisotropic );
	xyprintf( 30, y++, "game_state.antialiasing         %d", game_state.antialiasing );
	xyprintf( 30, y++, "game_state.LODBias              %f", game_state.LODBias );
	xyprintf( 30, y++, "game_state.maxParticles         %d", game_state.maxParticles );
	xyprintf( 30, y++, "game_state.maxParticleFill      %d", game_state.maxParticleFill );
	xyprintf( 30, y++, "game_state.musicSoundVolume     %f", game_state.musicSoundVolume );
	xyprintf( 30, y++, "game_state.fxSoundVolume        %f", game_state.fxSoundVolume );
	xyprintf( 30, y++, "game_state.voSoundVolume        %f", game_state.voSoundVolume );
}


GfxSettings globalGfxSettingsForNextTime;


OptionWater gfxGetMaxWaterMode()
{
	if (rdr_caps.features & GFXF_WATER)
	{
		if (rdr_caps.features & GFXF_WATER_DEPTH)
		{
			// NOTE: reflection are not working without FBOs right now (10/26/09)
			if (rdr_caps.features & GFXF_WATER_REFLECTIONS && rdr_caps.supports_fbos)
				return WATER_ULTRA;
			else
				return WATER_MED;
		}

		return WATER_LOW;
	}
	return WATER_OFF;
}

// Called from getAutoResumeInfoFromRegistry()
void gfxSettingsFixup( GfxSettings * gfxSettings )
{
	if (gfxSettings->version < GFXSETTINGS_VERSION)
	{
		// Upgrade to version 1
		if (gfxSettings->version < 1)
		{
			// Graphics Quality slider changed from 4 to 5 values.
			if (gfxSettings->slowUglyScale < 0.166f)
				gfxSettings->slowUglyScale = 0.0f;
			else if (gfxSettings->slowUglyScale < 0.5f)
				gfxSettings->slowUglyScale = 0.25f;
			else if (gfxSettings->slowUglyScale < 0.833f)
				gfxSettings->slowUglyScale = 0.5f;
			else
				gfxSettings->slowUglyScale = 0.75f;

			gfxSettings->version = 1;
		}

		// Upgrade to version 2
		if (gfxSettings->version < 2)
		{
			// Antialiasing moved from advanced to normal graphcis options.
			// Set default value to off
			gfxSettings->antialiasing = 0;
		}

		// Upgrade to version 3
		if (gfxSettings->version < 3)
		{
			// OptionCubemap enum changed from off/low/high/debug to off/low/medium/high/ultra/debug
			if (gfxSettings->advanced.cubemapMode == 2)
				gfxSettings->advanced.cubemapMode = CUBEMAP_HIGHQUALITY;
			else if (gfxSettings->advanced.cubemapMode == 3)
				gfxSettings->advanced.cubemapMode = CUBEMAP_DEBUG_SUPERHQ;
		}

		// Upgrade to version 4
		if (gfxSettings->version < 4)
		{
			// Added advanced shadow map settings
			gfxSettings->advanced.shadowMap.showAdvanced = 0;

			// OptionShadow enum changed from off/stencil/shadow maps to off/lowest (stencil)/low/medium/high/highest
			// Added shadowMap size and distance.
			// The shader was moved into the new shadowMap struct (no code change necessary here).
			if (gfxSettings->advanced.shadowMode == SHADOW_SHADOWMAP_LOW)
			{
				// change the shadowMode based on what the player had selected for the shader mode
				if (gfxSettings->advanced.shadowMap.shader == SHADOWSHADER_HIGHQ)
				{
					gfxSettings->advanced.shadowMode = SHADOW_SHADOWMAP_HIGH;
					gfxSettings->advanced.shadowMap.size = SHADOWMAPSIZE_MEDIUM;
					gfxSettings->advanced.shadowMap.distance = SHADOWDISTANCE_FAR;
				}
				else if (gfxSettings->advanced.shadowMap.shader == SHADOWSHADER_MEDIUMQ)
				{
					gfxSettings->advanced.shadowMode = SHADOW_SHADOWMAP_MEDIUM;
					gfxSettings->advanced.shadowMap.size = SHADOWMAPSIZE_MEDIUM;
					gfxSettings->advanced.shadowMap.distance = SHADOWDISTANCE_MIDDLE;
				}
				else
				{
					gfxSettings->advanced.shadowMode = SHADOW_SHADOWMAP_LOW;
					gfxSettings->advanced.shadowMap.shader = SHADOWSHADER_FAST;
					gfxSettings->advanced.shadowMap.size = SHADOWMAPSIZE_MEDIUM;
					gfxSettings->advanced.shadowMap.distance = SHADOWDISTANCE_MIDDLE;
				}
			}
			else
			{
				gfxSettings->advanced.shadowMode = SHADOW_SHADOWMAP_LOW;
				gfxSettings->advanced.shadowMap.shader = SHADOWSHADER_FAST;
				gfxSettings->advanced.shadowMap.size = SHADOWMAPSIZE_MEDIUM;
				gfxSettings->advanced.shadowMap.distance = SHADOWDISTANCE_MIDDLE;
			}

		}

		// Upgrade to version 5
		if (gfxSettings->version < 5)
		{
			if (gfxSettings->advanced.mipLevel >= 0)
				gfxSettings->advanced.mipLevel = MIN(gfxSettings->advanced.mipLevel + 1, 3);
			if (gfxSettings->advanced.entityMipLevel >= 1)
				gfxSettings->advanced.entityMipLevel = MIN(gfxSettings->advanced.entityMipLevel + 1, 4);
		}

		// Upgrade to version 6
		// if (gfxSettings->version < 5)
		// {
		// }

	}
}

// All of the settings which are restricted due to rdr_caps (outside of features) should be applied here
// NOTE: All of these restrictions should also be in uiOptions.c
static void gfxSettingsApplyRestrictions(GfxSettings * gfxSettings)
{
	// Don't do anything if the rdr_caps have not been filled in yet
	if (!rdr_caps.filled_in)
		return;

	gfxSettings->version = GFXSETTINGS_VERSION;

	// DoF, Bloom, and Desaturation now use FBO's which require
	// ext_framebuffer_multisample to do antialiasing
	if (rdr_caps.supports_fbos)
	{
		// rdrInitExtensionsDirect() makes sure rdr_caps.max_fbo_samples is at least 1
		gfxSettings->antialiasing = MIN(gfxSettings->antialiasing, rdr_caps.max_fbo_samples);
	}
	else
	{
		// rdrInitExtensionsDirect() makes sure rdr_caps.max_samples is at least 1
		gfxSettings->antialiasing = MIN(gfxSettings->antialiasing, rdr_caps.max_samples);
	}

	if (rdr_caps.disable_stencil_shadows)
		gfxSettings->advanced.shadowMode = SHADOW_OFF;

	if( gfxSettings->advanced.shadowMode >= SHADOW_SHADOWMAP_LOW && !(rdr_caps.allowed_features & GFXF_SHADOWMAP) )
		gfxSettings->advanced.shadowMode = SHADOW_STENCIL;

	// If we turn off shadow maps, we want to remember which shader was chosen in case
	// we turn shadow maps back on
	//if (gfxSettings->advanced.shadowMode < SHADOW_SHADOWMAP_LOW )
	//	gfxSettings->advanced.shadowMap.shader = SHADOWSHADER_NONE;

	{
		OptionWater max_water_mode = gfxGetMaxWaterMode();
		gfxSettings->advanced.useWater = MIN(gfxSettings->advanced.useWater, max_water_mode);
	}


	if (rdr_caps.supports_anisotropy)
	{
		gfxSettings->advanced.texLodBias		= MIN(gfxSettings->advanced.texLodBias, 2);
		gfxSettings->advanced.texAniso			= MIN(gfxSettings->advanced.texAniso, rdr_caps.maxTexAnisotropic);
	}
	else
	{
		gfxSettings->advanced.texLodBias		= 0;
		gfxSettings->advanced.texAniso			= 1;
	}

	if ( !(rdr_caps.allowed_features & GFXF_AMBIENT) )
		gfxSettings->advanced.ambient.strength = AMBIENT_OFF;

	if ( !(rdr_caps.allowed_features & GFXF_CUBEMAP) )
		gfxSettings->advanced.cubemapMode = CUBEMAP_OFF;

	if (!(rdr_caps.allowed_features & GFXF_WATER) )
		gfxSettings->advanced.useWater = WATER_OFF;
	else if ( !(rdr_caps.allowed_features & GFXF_WATER_REFLECTIONS) && gfxSettings->advanced.useWater > WATER_MED )
		gfxSettings->advanced.useWater = WATER_MED;

	//	Hack to not allow shader detail to go below high when ultra features are enabled.
	if( gfxSettings->advanced.cubemapMode > CUBEMAP_OFF || gfxSettings->advanced.shadowMode >= SHADOW_SHADOWMAP_LOW)
		gfxSettings->advanced.shaderDetail = SHADERDETAIL_HIGHEST_AVAILABLE;

	// After expanding the cubemaps from off/low/high to off/low/medium/high/ultra,
	// running an older build which saved one of the newer enums could cause problems.
	if (gfxSettings->advanced.cubemapMode > CUBEMAP_DEBUG_SUPERHQ)
		gfxSettings->advanced.cubemapMode = CUBEMAP_LOWQUALITY;
}


// Just populates the current window settings (full screen/windowed, and window size)
void gfxGetWindowSettings( GfxSettings * gfxSettings )
{
	gfxSettings->screenX			= game_state.screen_x; 
	gfxSettings->screenY			= game_state.screen_y;
	gfxSettings->refreshRate		= game_state.refresh_rate;
	gfxSettings->screenX_pos		= game_state.screen_x_pos;
	gfxSettings->screenY_pos		= game_state.screen_y_pos;
	gfxSettings->maximized			= game_state.maximized;
	gfxSettings->fullScreen			= game_state.fullscreen;
}

void gfxGetSettings( GfxSettings * gfxSettings )
{
	gfxSettings->advanced.mipLevel			= game_state.mipLevel;
	gfxSettings->advanced.entityMipLevel	= game_state.entityMipLevel;
	gfxSettings->advanced.texLodBias		= game_state.texLodBias;
	gfxSettings->advanced.texAniso			= game_state.texAnisotropic;
	gfxSettings->advanced.worldDetailLevel	= game_state.vis_scale;
	gfxSettings->advanced.entityDetailLevel	= game_state.LODBias;
	gfxSettings->gamma						= game_state.gamma;
	gfxSettings->antialiasing				= game_state.antialiasing;
	gfxSettings->autoFOV					= game_state.fov_auto;
	gfxSettings->fieldOfView				= game_state.fov_1st;

	gfxSettings->advanced.shadowMode		= game_state.shadowMode;
	gfxSettings->advanced.shadowMap.showAdvanced	= game_state.shadowMapShowAdvanced;
	gfxSettings->advanced.shadowMap.shader			= game_state.shadowShaderSelection;
	gfxSettings->advanced.shadowMap.distance		= game_state.shadowmap_num_cascades - 2;		// map 2->close, 3->middle, 4->far
	if (game_state.shadowMapTexSize == 2048)
		gfxSettings->advanced.shadowMap.size		= SHADOWMAPSIZE_LARGE;
	else if (game_state.shadowMapTexSize == 1024)
		gfxSettings->advanced.shadowMap.size		= SHADOWMAPSIZE_MEDIUM;
	else
		gfxSettings->advanced.shadowMap.size		= SHADOWMAPSIZE_SMALL;

	gfxSettings->advanced.cubemapMode		= game_state.cubemapMode;
#if NOVODEX
	gfxSettings->advanced.ageiaOn			= nx_state.hardware;
	gfxSettings->advanced.physicsQuality	= nx_state.physicsQuality;
#else
	gfxSettings->advanced.ageiaOn			= 0;
	gfxSettings->advanced.physicsQuality	= 0;
#endif
	gfxSettings->showAdvanced				= game_state.showAdvancedOptions;
	gfxSettings->advanced.showExperimental	= game_state.showExperimentalOptions;
	gfxSettings->slowUglyScale				= game_state.slowUglyScale;

	gfxSettings->advanced.suppressCloseFx	= game_state.suppressCloseFx;

	gfxSettings->advanced.maxParticles		= game_state.maxParticles;
	gfxSettings->advanced.maxParticleFill	= (F32)game_state.maxParticleFill / (F32)1000000.0;

	gfxGetWindowSettings(gfxSettings);

	gfxSettings->fxSoundVolume		= game_state.fxSoundVolume;
	gfxSettings->musicSoundVolume	= game_state.musicSoundVolume;
	gfxSettings->voSoundVolume		= game_state.voSoundVolume;

	gfxSettings->advanced.enableVBOs			= game_state.enableVBOs;
	gfxSettings->forceSoftwareAudio	= globalGfxSettingsForNextTime.forceSoftwareAudio;
	gfxSettings->enable3DSound		= globalGfxSettingsForNextTime.enable3DSound;
	gfxSettings->enableHWLights		= globalGfxSettingsForNextTime.enableHWLights;


	gfxSettings->useRenderScale = game_state.useRenderScale;
	gfxSettings->renderScaleX = game_state.renderScaleX;
	gfxSettings->renderScaleY = game_state.renderScaleY;

	gfxSettings->advanced.shaderDetail = shaderDetailFromFeatures(rdr_caps.features);

	gfxSettings->advanced.useWater = game_state.waterMode;

	if (game_state.reflectionEnable & 2)
		gfxSettings->advanced.buildingPlanarReflections = 1;
	else
		gfxSettings->advanced.buildingPlanarReflections = 0;

	if (rdr_caps.features & GFXF_BLOOM) {
		if (game_state.bloomScale==0)
			gfxSettings->advanced.useBloom = BLOOM_OFF;
		else if (game_state.bloomScale<3)
			gfxSettings->advanced.useBloom = BLOOM_REGULAR;
		else
			gfxSettings->advanced.useBloom = BLOOM_HEAVY;
	} else
		gfxSettings->advanced.useBloom = BLOOM_OFF;

	gfxSettings->advanced.bloomMagnitude = game_state.bloomWeight;

	if (rdr_caps.features & GFXF_DOF)
		gfxSettings->advanced.useDOF = 1;
	else 
		gfxSettings->advanced.useDOF = 0;

	if (rdr_caps.features & GFXF_DESATURATE)
		gfxSettings->advanced.useDesaturate = 1;
	else 
		gfxSettings->advanced.useDesaturate = 0;

	gfxSettings->advanced.dofMagnitude = game_state.dofWeight;

	gfxSettings->advanced.ambient.strength = (rdr_caps.features & GFXF_AMBIENT) ? game_state.ambientStrength : AMBIENT_OFF;
	gfxSettings->advanced.ambient.resolution = game_state.ambientResolution;
	gfxSettings->advanced.ambient.blur = game_state.ambientBlur;
	gfxSettings->advanced.ambient.showAdvanced = game_state.ambientShowAdvanced;
	gfxSettings->advanced.ambient.optionScale = game_state.ambientOptionScale;

	gfxSettings->useCelShader = game_state.useCelShader;
	gfxSettings->celshader.alphaquant = game_state.celAlphaQuant;
	gfxSettings->celshader.lighting = game_state.celLighting;
	gfxSettings->celshader.outlines = game_state.celOutlines;
	gfxSettings->celshader.posterization = game_state.celPosterization;
	gfxSettings->celshader.saturation = game_state.celSaturation;
	gfxSettings->celshader.suppressfilters = game_state.celSuppressFilters;

	gfxSettings->experimental.stupidShaderTricks = game_state.stupidShaderTricks;

	gfxSettings->advanced.useVSync = game_state.useVSync;
	gfxSettings->advanced.colorMouseCursor = !game_state.compatibleCursors;
	gfxSettings->filledIn = true;
	gfxSettings->version = GFXSETTINGS_VERSION;

	gfxSettingsApplyRestrictions(gfxSettings);
}

void gfxGetSettingsForNextTime( GfxSettings * gfxSettings )
{
	*gfxSettings = globalGfxSettingsForNextTime;

	//If you are in windowed mode and you aren't supposed to go into 
	//fullscreen next time, then save your screenx and y
	gfxSettings->maximized = game_state.maximized;

	//If you are coming back in full screen, you want the numbers you put in 
	//the config screen not your resize numbers...
	if( !gfxSettings->fullScreen && !game_state.fullscreen )
	{
		if( game_state.screen_x_restored && game_state.screen_y_restored )
		{
			gfxSettings->screenX = game_state.screen_x_restored;
			gfxSettings->screenY = game_state.screen_y_restored;
		}
	}

	//Attempt to figure out why you sometimes don't get the screen up.
	if( gfxSettings->screenX == 0 )
		gfxSettings->screenX = 100;
	if( gfxSettings->screenY == 0 )
		gfxSettings->screenY = 100;

	//Gfx Screen doesn't set the screen position, so always get it from the game_state
	gfxSettings->screenX_pos = game_state.screen_x_pos;
	gfxSettings->screenY_pos = game_state.screen_y_pos;
}

//If something has gone wrong, restore to nothing 
void gfxResetGfxSettings()
{
	GfxSettings gfxSettings;

	gfxGetInitialSettings( &gfxSettings );

	//If the current mipLevel is legal and is lower than the default,
	//then used the current so you don't screw people with low memory settings.
	if( game_state.mipLevel >= -1 && game_state.mipLevel <= 3 ) 
		if( game_state.mipLevel > gfxSettings.advanced.mipLevel )
			gfxSettings.advanced.mipLevel = game_state.mipLevel;
	if (game_state.entityMipLevel >= 0 && game_state.entityMipLevel <= 3)
		if (game_state.entityMipLevel > gfxSettings.advanced.entityMipLevel)
			gfxSettings.advanced.entityMipLevel = game_state.entityMipLevel;

	globalGfxSettingsForNextTime = gfxSettings;
	saveAutoResumeInfoToRegistry();
}

// Make adjustments to some of the recommended advanced settings
// if the user is running older low end hardware, e.g. low end video
// card or low system memory.
// Moderate hardware and any future hardware will just retain
// the nominal defaults which the user can then adjust in options menus
// as desired.
static void gfxAdjustRecommendedSettingsForHardware(GfxSettings * gfxSettings)
{
	// @hack -AB: performance boost hack for I7 launch. change when get good numbers :05/31/06
	#define GFX_PERFORMANCE_TWEAK_SCALE (0.85f)

	// Look at the chipset information from the GL context and see if any
	// adjustments are necessary to nominal recommended defaults for decent hardware
	if ( (rdr_caps.chip&DX10_CLASS) || ((rdr_caps.chip&GLSL) && (rdr_caps.chip&ARBFP)) )
	{
		// no adjustment
	}
	else if ( (rdr_caps.chip&NV4X) || (rdr_caps.chip&R300) )
	{
		// no adjustment (wouldn't expect to get here as should be caught by first)
	}
	else if ( (rdr_caps.chip&NV3X) || (rdr_caps.chip&R200) )
	{
		// not same generation but lumped together because of poor GeForce 5 performance?
		gfxSettings->advanced.texLodBias		= 0;
		gfxSettings->advanced.texAniso			= 1;
	}
	else if ( (rdr_caps.chip&NV2X) )
	{
		// GeForce 3 class
		gfxSettings->advanced.texLodBias		= 0;
		gfxSettings->advanced.texAniso			= 1;
		gfxSettings->advanced.worldDetailLevel	= 0.7*GFX_PERFORMANCE_TWEAK_SCALE;
		gfxSettings->advanced.entityDetailLevel	= 0.7*GFX_PERFORMANCE_TWEAK_SCALE;
		gfxSettings->advanced.maxParticles		= 10000; //TO DO : good value for this is what?
		gfxSettings->advanced.enableVBOs		= 0;
	}
	else
	{
		// lowest supported hardware, e.g. TEX_ENV_COMBINE, Intel Extreme, Radeon 7xxx
		gfxSettings->advanced.mipLevel			= 2;
		gfxSettings->advanced.entityMipLevel	= 2;
		gfxSettings->advanced.texLodBias		= 0;
		gfxSettings->advanced.texAniso			= 1;
		gfxSettings->advanced.worldDetailLevel	= 0.6*GFX_PERFORMANCE_TWEAK_SCALE;
		gfxSettings->advanced.entityDetailLevel	= 0.6*GFX_PERFORMANCE_TWEAK_SCALE;
		gfxSettings->advanced.maxParticles		= 5000; //TO DO : good value for this is what?
		gfxSettings->advanced.enableVBOs		= 0;
	}

	rdrGetSystemSpecs( &systemSpecs );

	//Less than 512 megs of RAM == lower mip level
	if( systemSpecs.maxPhysicalMemory <= 512*1024*1024 )
		gfxSettings->advanced.mipLevel = 2;
	else if( systemSpecs.maxPhysicalMemory <= 768*1024*1024 )
		gfxSettings->advanced.mipLevel = 1;
	else if (gfxSettings->advanced.mipLevel == 1)
	{
		// @todo NOTE!
		// This has a side effect of bumping the nominal setting
		// up to mip level -1 or 'very high' whenever it was
		// at 1 and the system has more than 768M main ram
		gfxSettings->advanced.mipLevel = -1;
	}

	// Video cards with 128MB or less want reduced texture memory
	// @todo rdrGetSystemSpecs()  is going to grab specs for the primary display adapter
	// which may not be the adapter the GL context is running on (e.g. Optimus systems)
	// So this may not be using the correct memory for comparison.
	// @todo Also, I don't believe the current method of obtaining the video memory
	// even gives correct numbers on modern systems.
	if ( systemSpecs.videoMemory) {
		if (systemSpecs.videoMemory <= 64 ) {
			// Medium
			gfxSettings->advanced.mipLevel = MAX(gfxSettings->advanced.mipLevel, 2);
		} else if (systemSpecs.videoMemory <= 128) {
			// High
			gfxSettings->advanced.mipLevel = MAX(gfxSettings->advanced.mipLevel, 1);
		} else {
			// 256+
			// Very High
		}
	}
}

// "Ugly"/"Minimum" Graphics Quality
// Also used for basis of safe-mode settings
static void gfxGetMinAdvancedSettings( GfxSettings * gfxSettings )
{
	gfxSettings->advanced.suppressCloseFx = 1;
//	game_state.suppressCloseFxDist = 3.0f;

	gfxSettings->advanced.physicsQuality = 0;
	gfxSettings->advanced.ageiaOn = 0;
	gfxSettings->advanced.mipLevel = 3;
	gfxSettings->advanced.entityMipLevel = 3;
	gfxSettings->advanced.worldDetailLevel = 0.5f;
	gfxSettings->advanced.entityDetailLevel = 0.3f;
	gfxSettings->advanced.maxParticles = 100;
	gfxSettings->advanced.maxParticleFill = 10;
	gfxSettings->advanced.useVSync=0;
	if (IsUsingWin9x()) // Otherwise we don't allow this anyway!
		gfxSettings->advanced.colorMouseCursor=0;
	else
		gfxSettings->advanced.colorMouseCursor=1;
	gfxSettings->advanced.shadowMode = SHADOW_OFF;
	gfxSettings->advanced.cubemapMode = CUBEMAP_OFF;
	gfxSettings->advanced.enableVBOs = 1;
	gfxSettings->advanced.texLodBias		= 0;
	gfxSettings->advanced.texAniso			= 1;
	gfxSettings->advanced.shaderDetail=SHADERDETAIL_NOBUMPS;
	gfxSettings->advanced.useWater=WATER_OFF;
	gfxSettings->advanced.buildingPlanarReflections = 0;
	gfxSettings->advanced.useBloom=0;
	gfxSettings->advanced.bloomMagnitude=1.0;
	gfxSettings->advanced.useDOF=0;
	gfxSettings->advanced.dofMagnitude=1.0;
	gfxSettings->advanced.useDesaturate = 0;
	gfxSettings->advanced.ambient.option=AMBIENT_DISABLE;

	gfxSettingsApplyRestrictions(gfxSettings);

	gfxUpdateAmbientAdvanced( gfxSettings );
	gfxUpdateShadowMapAdvanced( gfxSettings );
}

static void gfxGetRecommendedAdvancedSettings( GfxSettings * gfxSettings );
// "Performance" Graphics Quality
static void gfxGetPerformanceAdvancedSettings( GfxSettings * gfxSettings )
{
	//eCardClass cardClass = getCardClass();
	// Recommended low: CoH-level graphics, reduce textures a bit

	// Start with the "Recommended" Graphics Quality and modify it
	gfxGetRecommendedAdvancedSettings(gfxSettings);

	gfxSettings->showAdvanced  = 0;
	gfxSettings->slowUglyScale = 0.25f;
	gfxSettings->advanced.shaderDetail=SHADERDETAIL_NOWORLDBUMPS;
	gfxSettings->advanced.mipLevel			= 2;
	gfxSettings->advanced.worldDetailLevel	*= 0.8;
	gfxSettings->advanced.useDesaturate = 1;

	// Disable special features
	gfxSettings->advanced.useBloom=BLOOM_OFF;
	gfxSettings->advanced.useDOF=0;
	gfxSettings->advanced.useWater=WATER_OFF;
    
	// disable all physics stuff
	gfxSettings->advanced.physicsQuality = 0;
	gfxSettings->advanced.ageiaOn = 0;

	gfxSettingsApplyRestrictions(gfxSettings);

	gfxUpdateAmbientAdvanced( gfxSettings );
	gfxUpdateShadowMapAdvanced( gfxSettings );
}

// "Recommended" Graphics Quality
static void gfxGetRecommendedAdvancedSettings( GfxSettings * gfxSettings )
{
	// nominal settings for capable hardware
	gfxSettings->advanced.mipLevel			= 1;
	gfxSettings->advanced.entityMipLevel	= 1;
	gfxSettings->advanced.texLodBias		= 2;
	gfxSettings->advanced.texAniso			= 4;
	gfxSettings->advanced.worldDetailLevel	= 1.0;
	gfxSettings->advanced.entityDetailLevel	= 1.0;
	gfxSettings->advanced.maxParticles		= MAX_PARTICLES;
	gfxSettings->advanced.maxParticleFill	= 10;
	gfxSettings->advanced.enableVBOs		= 1;

	// If necessary adjust settings for less capable hardware
	// e.g. low end video device or low system memory
	gfxAdjustRecommendedSettingsForHardware( gfxSettings );

	#if NOVODEX
	if ( nwHardwarePresent() )
	{
		gfxSettings->advanced.ageiaOn = 1;
		gfxSettings->advanced.physicsQuality = ePhysicsQuality_VeryHigh;
	}
	else
	#endif
	{
		gfxSettings->advanced.ageiaOn = 0;
		#if NOVODEX
			gfxSettings->advanced.physicsQuality = ePhysicsQuality_Medium;
		#endif
	}

	gfxSettings->advanced.shaderDetail=SHADERDETAIL_HIGHEST_AVAILABLE;

	game_state.suppressCloseFx = 0;
	game_state.suppressCloseFxDist = 3.0f;

	gfxSettings->showAdvanced  = 0;
	gfxSettings->slowUglyScale = 0.5f;
	gfxSettings->advanced.bloomMagnitude=1.0;
	gfxSettings->advanced.dofMagnitude=1.0;
	gfxSettings->advanced.useWater=WATER_MED;
	gfxSettings->advanced.buildingPlanarReflections = 0;
	gfxSettings->advanced.useBloom=BLOOM_REGULAR;
	gfxSettings->advanced.useDOF=1;
	gfxSettings->advanced.useVSync=1;
	gfxSettings->advanced.cubemapMode = CUBEMAP_OFF;
	gfxSettings->advanced.ambient.option=AMBIENT_DISABLE;

	if (IsUsingWin9x())
		gfxSettings->advanced.colorMouseCursor=0;
	else
		gfxSettings->advanced.colorMouseCursor=1;

	if (game_state.noVBOs)
		gfxSettings->advanced.enableVBOs=0;

	// Default shadows to stencil shadows
	gfxSettings->advanced.shadowMode		= SHADOW_STENCIL;

	// inherit this from other settings
	gfxSettings->advanced.useDesaturate = (gfxSettings->advanced.useBloom || gfxSettings->advanced.useDOF);

	gfxSettingsApplyRestrictions(gfxSettings);

	gfxUpdateAmbientAdvanced( gfxSettings );
	gfxUpdateShadowMapAdvanced( gfxSettings );
}

// "Slow"/"Quality" Graphics Quality
void gfxGetQualityAdvancedSettings( GfxSettings * gfxSettings )
{
	gfxSettings->advanced.suppressCloseFx = 0;
//	game_state.suppressCloseFxDist = 3.0f;

	#if NOVODEX
	if ( nwHardwarePresent() )
	{
		gfxSettings->advanced.ageiaOn = 1;
		gfxSettings->advanced.physicsQuality = ePhysicsQuality_VeryHigh;
	}
	else
	#endif
	{
		gfxSettings->advanced.ageiaOn = 0;
		#if NOVODEX
			gfxSettings->advanced.physicsQuality = ePhysicsQuality_High;
		#endif
	}
	gfxSettings->showAdvanced  = 0;
	gfxSettings->slowUglyScale = 0.75f;
	gfxSettings->advanced.mipLevel = -1;
	gfxSettings->advanced.entityMipLevel = 0;
	gfxSettings->advanced.worldDetailLevel = 1.0f;
	gfxSettings->advanced.entityDetailLevel = 1.0f;
	gfxSettings->advanced.maxParticles = 50000;
	gfxSettings->advanced.maxParticleFill = 10;
	gfxSettings->advanced.useVSync=1;
	gfxSettings->advanced.colorMouseCursor=1;
	gfxSettings->advanced.enableVBOs = 1;
	gfxSettings->advanced.texLodBias		= 2;
	gfxSettings->advanced.texAniso			= 4;
	gfxSettings->advanced.shaderDetail=SHADERDETAIL_HIGHEST_AVAILABLE;
	gfxSettings->advanced.buildingPlanarReflections = 1;
	gfxSettings->advanced.useBloom=BLOOM_REGULAR;
	gfxSettings->advanced.bloomMagnitude=1.0;
	gfxSettings->advanced.useDOF=1;
	gfxSettings->advanced.dofMagnitude=1.0;
	gfxSettings->advanced.useDesaturate = 1;

	// New features for I18/GR
	// Enable new features at fast settings
	gfxSettings->advanced.shadowMode = SHADOW_SHADOWMAP_LOW;
	gfxSettings->advanced.ambient.option=AMBIENT_HIGH_PERFORMANCE;
	gfxSettings->advanced.useWater=WATER_HIGH;
	gfxSettings->advanced.cubemapMode = CUBEMAP_LOWQUALITY;

	gfxSettingsApplyRestrictions(gfxSettings);

	gfxUpdateAmbientAdvanced( gfxSettings );
	gfxUpdateShadowMapAdvanced( gfxSettings );
}

// "Ultra" Graphics Quality
void gfxGetUltraAdvancedSettings( GfxSettings * gfxSettings )
{
	gfxSettings->advanced.suppressCloseFx = 0;
//	game_state.suppressCloseFxDist = 3.0f;

	#if NOVODEX
	if ( nwHardwarePresent() )
	{
		gfxSettings->advanced.ageiaOn = 1;
		gfxSettings->advanced.physicsQuality = ePhysicsQuality_VeryHigh;
	}
	else
	#endif
	{
		gfxSettings->advanced.ageiaOn = 0;
		#if NOVODEX
			gfxSettings->advanced.physicsQuality = ePhysicsQuality_High;
		#endif
	}
	gfxSettings->showAdvanced  = 0;
	gfxSettings->slowUglyScale = 1.0f;
	gfxSettings->advanced.mipLevel = -1;
	gfxSettings->advanced.entityMipLevel = 0;
	gfxSettings->advanced.worldDetailLevel = 1.0f;
	gfxSettings->advanced.entityDetailLevel = 1.0f;
	gfxSettings->advanced.maxParticles = 50000;
	gfxSettings->advanced.maxParticleFill = 10;
	gfxSettings->advanced.useVSync=1;
	gfxSettings->advanced.colorMouseCursor=1;
	gfxSettings->advanced.shadowMode = SHADOW_SHADOWMAP_HIGH;
	gfxSettings->advanced.cubemapMode = CUBEMAP_HIGHQUALITY;
	gfxSettings->advanced.enableVBOs = 1;
	gfxSettings->advanced.texLodBias		= 2;
	gfxSettings->advanced.texAniso			= 4;
	gfxSettings->advanced.shaderDetail=SHADERDETAIL_HIGHEST_AVAILABLE;
	gfxSettings->advanced.useWater=WATER_ULTRA;
	gfxSettings->advanced.buildingPlanarReflections = 1;
	gfxSettings->advanced.useBloom=BLOOM_REGULAR;
	gfxSettings->advanced.bloomMagnitude=1.0;
	gfxSettings->advanced.useDOF=1;
	gfxSettings->advanced.dofMagnitude=1.0;
	gfxSettings->advanced.useDesaturate = 1;

	if (rdr_caps.chip & DX10_CLASS)
		gfxSettings->advanced.ambient.option=AMBIENT_HIGH_QUALITY;
	else
		gfxSettings->advanced.ambient.option=AMBIENT_HIGH_PERFORMANCE;

	gfxSettingsApplyRestrictions(gfxSettings);

	gfxUpdateAmbientAdvanced( gfxSettings );
	gfxUpdateShadowMapAdvanced( gfxSettings );
}

// Used by gfxResetGfxSettings() and safe mode fall-back settings
void gfxGetInitialSettings( GfxSettings * gfxSettings )
{
	gfxGetRecommendedAdvancedSettings(gfxSettings);

	gfxSettings->gamma				= 1.0;
	gfxSettings->autoFOV			= 1;
	gfxSettings->fieldOfView		= FIELDOFVIEW_STD;
	gfxSettings->screenX			= 1024;
	gfxSettings->screenY			= 768;
	gfxSettings->refreshRate		= 60;
	gfxSettings->screenX_pos		= 0;
	gfxSettings->screenY_pos		= 0;
	gfxSettings->maximized			= 0; 
	gfxSettings->fullScreen			= 1; 

	gfxSettings->showAdvanced		= 0;
	gfxSettings->slowUglyScale		= 0.5;	// force recommended mode
	gfxSettings->fxSoundVolume		= DEFAULT_VOLUME_FX;
	gfxSettings->musicSoundVolume	= DEFAULT_VOLUME_MUSIC;
	gfxSettings->voSoundVolume		= DEFAULT_VOLUME_VO;
	gfxSettings->enable3DSound		= 0;
	gfxSettings->enableHWLights		= 1;
	gfxSettings->forceSoftwareAudio = 0;
	gfxSettings->renderScaleX		= 1.0;
	gfxSettings->renderScaleY		= 1.0;
	gfxSettings->useRenderScale		= RENDERSCALE_OFF;

	gfxSettings->advanced.showExperimental = 0;
	gfxSettings->celshader.suppressfilters = 1;
	gfxSettings->celshader.lighting = 1;
	gfxSettings->celshader.outlines = 1;
	gfxSettings->celshader.posterization = 2;
	gfxSettings->celshader.alphaquant = 2;
	gfxSettings->celshader.saturation = 0.3;
	gfxSettings->experimental.stupidShaderTricks = 0;

	gfxSettings->filledIn			= true;
	gfxSettings->version			= GFXSETTINGS_VERSION;

	gfxSettingsApplyRestrictions(gfxSettings);
}

void gfxGetSafeModeSettings( GfxSettings * gfxSettings )
{
	// Start with Minimum settings
	gfxGetMinAdvancedSettings(gfxSettings);

	gfxSettings->showAdvanced  = 0;
	gfxSettings->slowUglyScale = 0.0f;
	gfxSettings->screenX = 800;
	gfxSettings->screenY = 600;
	gfxSettings->refreshRate = 60;
	gfxSettings->screenX_pos = 0;
	gfxSettings->screenY_pos = 0;
	gfxSettings->maximized = 0;
	gfxSettings->fullScreen = 1;
	gfxSettings->showAdvanced = 0;
	gfxSettings->slowUglyScale = 0;
	gfxSettings->renderScaleX=1.0;
	gfxSettings->renderScaleY=1.0;
	gfxSettings->useRenderScale=RENDERSCALE_OFF;

	gfxSettings->advanced.enableVBOs = 0; // This is enabled in gfxGetMinAdvancedSettings
}

// Used by uiOptions.c to get the appropriate settings for the given "Graphics Quality" slider value
void gfxGetScaledAdvancedSettings( GfxSettings * gfxSettings, F32 scale )
{
	OptionGraphicsPreset preset = gfxGetPresetEnumFromScale(scale);

	switch(preset)
	{
		case GRAPHICSPRESET_MINIMUM :		gfxGetMinAdvancedSettings( gfxSettings );			break;
		case GRAPHICSPRESET_PERFORMANCE :	gfxGetPerformanceAdvancedSettings( gfxSettings );	break;
		case GRAPHICSPRESET_RECOMMENDED :	gfxGetRecommendedAdvancedSettings( gfxSettings );	break;
		case GRAPHICSPRESET_ULTRA_LOW :		gfxGetQualityAdvancedSettings( gfxSettings );		break;
		case GRAPHICSPRESET_ULTRA_HIGH :	gfxGetUltraAdvancedSettings( gfxSettings );			break;
		default: assert("Unhandled value" == 0);
	}
}

// Utility functions to show and hide the loading screen
static int s_oldGameMode;
static bool s_showingLoadingScreen = false;

void gfxDrawFinishFrame(int doFlip, int headShot, int renderPass);

void gfxShowLoadingScreen()
{
	if (!s_showingLoadingScreen && game_state.game_mode != SHOW_LOAD_SCREEN)
	{
		s_oldGameMode = game_state.game_mode;

		game_state.game_mode = SHOW_LOAD_SCREEN;
		loadBG();
		loadScreenResetBytesLoaded();
		showBGAlpha(-1);
		gfxDrawFinishFrame(1, 0, 0);

		s_showingLoadingScreen = true;
	}
}

void gfxFinishLoadingScreen()
{
	if (s_showingLoadingScreen)
	{
		finishLoadScreen();
		game_state.game_mode = s_oldGameMode;	
		s_showingLoadingScreen = false;
	}
}

void checkFullScreenToggle(void);

// pastStartup is only needed when called after the initial set up.
// onlyDynamic specifies to only change things that can quickly be changed on the fly
void gfxApplySettings( GfxSettings * gfxSettings, int pastStartUp, bool onlyDynamic )
{
	int currMipLevel, currEntityMipLevel;
	int currTexAniso;
	U32 gfx_features_enable=0;
	U32 gfx_features_disable=0;
	static int lastShadowShaderSelection = -1;

	assert(gfxSettings->filledIn);

	gfxSettingsApplyRestrictions(gfxSettings);

	currMipLevel = game_state.actualMipLevel; //Don't bother resetting textures if you didn't change miplevel
	currEntityMipLevel = game_state.entityMipLevel;
	currTexAniso = game_state.texAnisotropic;

	//This is what I want to save to the registry
	if (!onlyDynamic)
		globalGfxSettingsForNextTime = *gfxSettings;
	saveAutoResumeInfoToRegistry();

	if (!onlyDynamic)
	{
		game_state.mipLevel				= gfxSettings->advanced.mipLevel;
		game_state.actualMipLevel		= game_state.mipLevel + 1;
		if (game_state.actualMipLevel < 0)
			game_state.actualMipLevel = 0;
		game_state.entityMipLevel		= gfxSettings->advanced.entityMipLevel;

		game_state.fullscreen_toggle = gfxSettings->fullScreen;
		game_state.screen_x = gfxSettings->screenX;
		game_state.screen_y = gfxSettings->screenY;
		game_state.refresh_rate = gfxSettings->refreshRate;

		checkFullScreenToggle();
	}

	game_state.texLodBias			= gfxSettings->advanced.texLodBias;
	game_state.texAnisotropic		= gfxSettings->advanced.texAniso;
	game_state.antialiasing			= gfxSettings->antialiasing;
	setVisScale(gfxSettings->advanced.worldDetailLevel, 1);
	game_state.LODBias				= gfxSettings->advanced.entityDetailLevel;
	game_state.gamma				= gfxSettings->gamma;
	game_state.useVSync				= gfxSettings->advanced.useVSync;
	game_state.fov_auto				= gfxSettings->autoFOV;
	game_state.fov_1st				= game_state.fov_auto ? FIELDOFVIEW_STD : gfxSettings->fieldOfView;
	game_state.fov_3rd				= game_state.fov_auto ? FIELDOFVIEW_STD : gfxSettings->fieldOfView;

	rdrSetVSync(game_state.useVSync);

#if NOVODEX
	if (!onlyDynamic)
	{
		nx_state.softwareOnly			= !gfxSettings->advanced.ageiaOn;
		if ( !nx_state.allowed && gfxSettings->advanced.physicsQuality )
		{
			if (pastStartUp)
			{
				gfxShowLoadingScreen();
			}

			nx_state.allowed				= 1;
			nwInitializeNovodex();
		}
		else if ( nx_state.allowed && !gfxSettings->advanced.physicsQuality )
		{
			if (pastStartUp)
			{
				gfxShowLoadingScreen();
			}

			nx_state.allowed				= 0;
			nwDeinitializeNovodex();
		}
		else
		{
			nx_state.allowed				= !!gfxSettings->advanced.physicsQuality;
			if ( nx_state.hardware == nx_state.softwareOnly )
			{
				if (pastStartUp)
				{
					gfxShowLoadingScreen();
				}

				nwDeinitializeNovodex();
				nwInitializeNovodex();
			}
		}
		nwSetPhysicsQuality(gfxSettings->advanced.physicsQuality);
		gfxSettings->advanced.ageiaOn = nx_state.hardware;
	}
#endif

	game_state.showAdvancedOptions = gfxSettings->showAdvanced;
	game_state.showExperimentalOptions = gfxSettings->advanced.showExperimental;
	game_state.slowUglyScale = gfxSettings->slowUglyScale;

	if (gfxSettings->advanced.shadowMode >= SHADOW_SHADOWMAP_LOW)
	{
		// Shadow maps
		game_state.shadowMapShowAdvanced = gfxSettings->advanced.shadowMap.showAdvanced;
		game_state.shadowMode = gfxSettings->advanced.shadowMode;
		game_state.shadowvol = 2;

		game_state.shadowmap_num_cascades = gfxSettings->advanced.shadowMap.distance + 2;		// map close->2, middle->3, far->4;
		if (gfxSettings->advanced.shadowMap.size == SHADOWMAPSIZE_LARGE)
			game_state.shadowMapTexSize = 2048;
		else if (gfxSettings->advanced.shadowMap.size == SHADOWMAPSIZE_MEDIUM)
			game_state.shadowMapTexSize = 1024;
		else
			game_state.shadowMapTexSize = 512;

		if (game_state.shadowMapTexSize * 2 > rdr_caps.max_texture_size)
			game_state.shadowMapTexSize = rdr_caps.max_texture_size / 2;
	}
	else if (gfxSettings->advanced.shadowMode == SHADOW_STENCIL)
	{
		// Stencil shadows
		game_state.shadowMode = SHADOW_STENCIL;
		game_state.shadowvol = 0;
	}
	else
	{
		// No shadows
		game_state.shadowMode = SHADOW_OFF;
		game_state.shadowvol = 2;
	}

	game_state.suppressCloseFx			= gfxSettings->advanced.suppressCloseFx;

	game_state.maxParticles			= gfxSettings->advanced.maxParticles;
	game_state.maxParticleFill		= (int)(gfxSettings->advanced.maxParticleFill * 1000000);

	if (game_state.noVBOs)
		gfxSettings->advanced.enableVBOs = 0;
	game_state.enableVBOs			= gfxSettings->advanced.enableVBOs;

	if( !game_state.fullscreen )
	{
		game_state.screen_x_restored = game_state.screen_x	= gfxSettings->screenX;
		game_state.screen_y_restored = game_state.screen_y	= gfxSettings->screenY;
		game_state.screen_x_pos								= gfxSettings->screenX_pos;
		game_state.screen_y_pos								= gfxSettings->screenY_pos;
	}

	game_state.useRenderScale = gfxSettings->useRenderScale;
	game_state.renderScaleX = gfxSettings->renderScaleX;
	game_state.renderScaleY = gfxSettings->renderScaleY;

	shaderDetailToFeatures(gfxSettings->advanced.shaderDetail, &gfx_features_enable, &gfx_features_disable);

	if (gfx_features_enable & GFXF_WATER)
		game_state.waterMode = gfxSettings->advanced.useWater;
	else
		game_state.waterMode = WATER_OFF;

	if (game_state.waterMode >= WATER_HIGH)
	{
		game_state.reflectionEnable = 1;

		if (game_state.waterMode == WATER_ULTRA)
			game_state.reflection_water_entity_vis_limit_dist_from_player = 500;
		else
			game_state.reflection_water_entity_vis_limit_dist_from_player = 150;
	}
	else
	{
		game_state.reflectionEnable = 0;
	}

	/*
	// This is enabled/disabled with cubemaps
	if (gfxSettings->advanced.buildingPlanarReflections)
		game_state.reflectionEnable |= 2;
	*/
	
	// Anything that appends to gfx_features_enable/disable needs to come after shaderDetailToFeatures call
	// TODO: Remove having to set game_state.shadowShaderSelection here
	if (!onlyDynamic)
	{
		if (game_state.shadowMode >= SHADOW_SHADOWMAP_LOW)
		{
			if (gfxSettings->advanced.shadowMap.shader > SHADOWSHADER_NONE && gfxSettings->advanced.shadowMap.shader <= SHADOWSHADER_HIGHQ) {
				game_state.shadowShaderSelection = gfxSettings->advanced.shadowMap.shader;
				lastShadowShaderSelection = game_state.shadowShaderSelection; // so we know which one to go back to if they leave/re-enter shadow mode
			} else {
				game_state.shadowShaderSelection = lastShadowShaderSelection != -1 ? lastShadowShaderSelection : SHADOWSHADER_FAST;
				gfxSettings->advanced.shadowMap.shader = game_state.shadowShaderSelection; // keep ui of sub-selection in sync
			}
		}
		else
		{
			game_state.shadowShaderSelection = SHADOWSHADER_NONE;
		}
		// keep render features in sync
		if( game_state.shadowShaderSelection == SHADOWSHADER_NONE )
			gfx_features_disable |= GFXF_SHADOWMAP;
		else
			gfx_features_enable |= GFXF_SHADOWMAP;
	}
	else if( game_state.shadowMode >= SHADOW_SHADOWMAP_LOW && gfxSettings->advanced.shadowMap.shader == SHADOWSHADER_NONE)
	{
		// Get here when first cycle back to shadowmap mode.  Need to restore advanced.shadowMap.shader from NONE to the value
		//	that we're going to start at, which is either the default value or the last one we cached above when they last applied it.
		gfxSettings->advanced.shadowMap.shader = lastShadowShaderSelection != -1 ? lastShadowShaderSelection : SHADOWSHADER_FAST;
	}

	game_state.cubemapMode = gfxSettings->advanced.cubemapMode;
	if( game_state.cubemapMode && game_state.cubemapMode <= CUBEMAP_ULTRAQUALITY )
	{
		gfx_features_enable |= GFXF_CUBEMAP;

		// Enable building planar reflections
		game_state.reflectionEnable |= 2;
		// Only render characters into building planar reflections (composited with cubemap)
		game_state.reflection_planar_mode = kReflectNearby;
		// Allow both building and water reflections viewports to be active at the same time
		game_state.separate_reflection_viewport = 1;

		switch (game_state.cubemapMode)
		{
			case CUBEMAP_LOWQUALITY:
				// Render once face every frame
				game_state.forceCubeFace = 0;
				game_state.cubemapUpdate3Faces = 0;
				break;
			case CUBEMAP_MEDIUMQUALITY:
				// Render once face every frame
				game_state.forceCubeFace = 0;
				game_state.cubemapUpdate3Faces = 1;
				break;
			case CUBEMAP_HIGHQUALITY:
				// Render once face every frame
				game_state.forceCubeFace = 0;
				game_state.cubemapUpdate3Faces = 0;
				break;
			case CUBEMAP_ULTRAQUALITY:
				// Render all faces every frame
				game_state.forceCubeFace = -1;
				game_state.cubemapUpdate3Faces = 0;
				break;
			default:
				assert(0);  // unhandled value
		}
	}
	else
	{
		game_state.cubemapMode = 0;
		gfx_features_disable |= GFXF_CUBEMAP;
		game_state.reflectionEnable &= ~2;
		game_state.forceCubeFace = 0;
		game_state.cubemapUpdate3Faces = 0;
	}

	if( gfxSettings->advanced.ambient.strength )
		gfx_features_enable |= GFXF_AMBIENT;
	else
		gfx_features_disable |= GFXF_AMBIENT;
	game_state.ambientStrength = gfxSettings->advanced.ambient.strength;
	game_state.ambientResolution = gfxSettings->advanced.ambient.resolution;
	game_state.ambientBlur = gfxSettings->advanced.ambient.blur;
	game_state.ambientShowAdvanced = gfxSettings->advanced.ambient.showAdvanced;
	game_state.ambientOptionScale = gfxSettings->advanced.ambient.optionScale;

	if (gfxSettings->advanced.useBloom) {
		gfx_features_enable |= GFXF_BLOOM|GFXF_TONEMAP;
	} else {
		gfx_features_disable |= GFXF_BLOOM|GFXF_TONEMAP;
	}
	if (gfxSettings->advanced.useBloom == BLOOM_REGULAR)
		game_state.bloomScale = 2;
	else if (gfxSettings->advanced.useBloom == BLOOM_HEAVY)
		game_state.bloomScale = 4;
	else // off, set it to default
		game_state.bloomScale = 2;

	game_state.bloomWeight = gfxSettings->advanced.bloomMagnitude;

	if (gfxSettings->advanced.useDOF) {
		gfx_features_enable |= GFXF_DOF;
	} else {
		gfx_features_disable |= GFXF_DOF;
	}
	game_state.dofWeight = gfxSettings->advanced.dofMagnitude;

	if( gfxSettings->advanced.useDesaturate )
		gfx_features_enable |= GFXF_DESATURATE;
	else
		gfx_features_disable |= GFXF_DESATURATE;
	
	if (!gfxSettings->advanced.colorMouseCursor)
	{
		game_state.compatibleCursors = 1;
	} else {
		game_state.compatibleCursors = 0;
	}

	if( !pastStartUp ) //if its being called at startUp
		game_state.fullscreen= gfxSettings->fullScreen; 

	game_state.fxSoundVolume		= gfxSettings->fxSoundVolume;
	game_state.musicSoundVolume		= gfxSettings->musicSoundVolume;
	game_state.voSoundVolume		= gfxSettings->voSoundVolume;

	sndVolumeControl( SOUND_VOLUME_FX,		game_state.fxSoundVolume );
	sndVolumeControl( SOUND_VOLUME_MUSIC,	game_state.musicSoundVolume );
	sndVolumeControl( SOUND_VOLUME_VOICEOVER,	game_state.voSoundVolume );

	//You can't switch full screen mode after initialization
	if( !pastStartUp )
	{
		game_state.fullscreen = gfxSettings->fullScreen;
		game_state.refresh_rate = gfxSettings->refreshRate;

		if( !game_state.fullscreen )
		{
			windowResize(	&game_state.screen_x, &game_state.screen_y, &game_state.refresh_rate,
 							game_state.screen_x_pos, game_state.screen_y_pos, 0 );
		}
	}		

	//Has to come after windowSetSize
	game_state.maximized			= gfxSettings->maximized;

	game_state.gfx_features_disable = gfx_features_disable;
	game_state.gfx_features_enable = gfx_features_enable;

	if( gfxSettings->useCelShader != game_state.useCelShader && !onlyDynamic)
	{
		game_state.useCelShader = gfxSettings->useCelShader;
		if (pastStartUp && rdr_caps.filled_in) {
			reloadShaderCallback(NULL, 1);
		}
	}

	game_state.celAlphaQuant = gfxSettings->celshader.alphaquant;
	game_state.celLighting = gfxSettings->celshader.lighting;
	game_state.celOutlines = gfxSettings->celshader.outlines;
	game_state.celPosterization = gfxSettings->celshader.posterization;
	game_state.celSaturation = gfxSettings->celshader.saturation;
	game_state.celSuppressFilters = gfxSettings->celshader.suppressfilters;

	game_state.stupidShaderTricks = gfxSettings->experimental.stupidShaderTricks;

	if( pastStartUp )
	{
		int diff;

		diff = rdrToggleFeatures(gfx_features_enable, gfx_features_disable, onlyDynamic);

		if (currTexAniso != game_state.texAnisotropic)
			texResetAnisotropic();

		if (!onlyDynamic && (currMipLevel != game_state.actualMipLevel || currEntityMipLevel != game_state.entityMipLevel))
		{
			if (pastStartUp)
			{
				gfxShowLoadingScreen();
			}

			gfxResetTextures();
			texForceTexLoaderToComplete(1);
		}
	}

	gfxFinishLoadingScreen();
}

void gfxReapplySettings(void)
{
	GfxSettings settings;
	gfxGetSettings(&settings);
	gfxApplySettings(&settings, 1, 0);
}

OptionGraphicsPreset gfxGetPresetEnumFromScale(F32 scale)
{
	return (OptionGraphicsPreset) ((scale + 0.5f / (GRAPHICSPRESET_NUM_PRESETS - 1)) * (GRAPHICSPRESET_NUM_PRESETS - 1));
}

static struct {
	OptionShaderDetail shader_detail;
	int features_enable;
} shader_details[] = {
	{SHADERDETAIL_NOBUMPS,},																																				/* Lowest */
	{SHADERDETAIL_NOWORLDBUMPS,                                                             GFXF_HQBUMP|GFXF_BUMPMAPS},														/* Low (No world bumpmaps) */
	{SHADERDETAIL_GF4MODE,                                                                  GFXF_HQBUMP|GFXF_BUMPMAPS|GFXF_BUMPMAPS_WORLD|GFXF_WATER|GFXF_WATER_DEPTH},		/* Low (With world bumpmaps) */
	{SHADERDETAIL_SINGLE_MATERIAL,                       GFXF_MULTITEX|GFXF_MULTITEX_HQBUMP|GFXF_HQBUMP|GFXF_BUMPMAPS|GFXF_BUMPMAPS_WORLD|GFXF_WATER|GFXF_WATER_DEPTH},		/* Medium */
	{SHADERDETAIL_DUAL_MATERIAL,      GFXF_MULTITEX_DUAL|GFXF_MULTITEX|GFXF_MULTITEX_HQBUMP|GFXF_HQBUMP|GFXF_BUMPMAPS|GFXF_BUMPMAPS_WORLD|GFXF_WATER|GFXF_WATER_DEPTH},		/* High */

	{SHADERDETAIL_HIGHEST_AVAILABLE,  GFXF_MULTITEX_DUAL|GFXF_MULTITEX|GFXF_MULTITEX_HQBUMP|GFXF_HQBUMP|GFXF_BUMPMAPS|GFXF_BUMPMAPS_WORLD|GFXF_WATER|GFXF_WATER_DEPTH},
};
int all_features =                    GFXF_MULTITEX_DUAL|GFXF_MULTITEX|GFXF_MULTITEX_HQBUMP|GFXF_HQBUMP|GFXF_BUMPMAPS|GFXF_BUMPMAPS_WORLD|GFXF_WATER|GFXF_WATER_DEPTH;

void shaderDetailToFeatures(OptionShaderDetail shader_detail, int *features_enable, int *features_disable)
{
	int i;
	for (i=0; i<ARRAY_SIZE(shader_details); i++) {
		if (shader_detail == shader_details[i].shader_detail) {
			*features_enable = shader_details[i].features_enable;
			*features_disable = all_features & ~shader_details[i].features_enable;
			return;
		}
	}
	Errorf("Invalid shader detail specified");
	*features_enable = shader_details[0].features_enable;
	*features_disable = all_features & ~shader_details[0].features_enable;
}

OptionShaderDetail shaderDetailFromFeatures(int features_enabled)
{
	int i;
	bool bWrapped=false;
	OptionShaderDetail ret;
	for (i=0; i<ARRAY_SIZE(shader_details); i++) {
		if (shader_details[i].features_enable == (features_enabled & all_features)) {
			return shader_details[i].shader_detail;
		}
	}
	ret = SHADERDETAIL_NOBUMPS;
	do {
		int enabled, disabled;
		shaderDetailToFeatures(ret, &enabled, &disabled);
		if ((features_enabled & enabled)==(features_enabled & all_features)) {
			return ret;
		}
		ret = shaderDetailInc(ret, &bWrapped);
	} while (!bWrapped);
	return SHADERDETAIL_HIGHEST_AVAILABLE; // Bad feature set (had more features than we know about)
}

OptionShaderDetail shaderDetailInc(OptionShaderDetail shader_detail, bool *bWrapped)
{
	int i;
	for (i=0; i<ARRAY_SIZE(shader_details); i++) {
		if (shader_detail == shader_details[i].shader_detail) {
			break;
		}
	}
	if (bWrapped)
		*bWrapped = false;
	i++;
retry:
	if (i == ARRAY_SIZE(shader_details)) {
		i = 0;
		if (bWrapped)
			*bWrapped = true;
		goto retry;
	}
	if (shader_details[i].shader_detail == SHADERDETAIL_HIGHEST_AVAILABLE) {
		// Just a pseudonym, ignore it
		i++;
		goto retry;
	}
	return shader_details[i].shader_detail;
}

OptionShaderDetail shaderDetailDec(OptionShaderDetail shader_detail, bool *bWrapped)
{
	int i;
	for (i=0; i<ARRAY_SIZE(shader_details); i++) {
		if (shader_detail == shader_details[i].shader_detail) {
			break;
		}
	}
	if (bWrapped)
		*bWrapped = false;
	i--;
retry:
	if (i == -1) {
		i = ARRAY_SIZE(shader_details) - 1;
		if (bWrapped)
			*bWrapped = true;
		goto retry;
	}
	if (shader_details[i].shader_detail == SHADERDETAIL_HIGHEST_AVAILABLE) {
		// Just a pseudonym, ignore it
		i--;
		goto retry;
	}
	return shader_details[i].shader_detail;
}

OptionShaderDetail shaderDetailIncAllowed(OptionShaderDetail shader_detail, bool *bWrapped)
{
	OptionShaderDetail starting_level = shader_detail;
	OptionShaderDetail ret = shader_detail;
	int features_enable, features_disable;
	int starting_features;
	shaderDetailToFeatures(ret, &features_enable, &features_disable);
	starting_features = features_enable & rdr_caps.allowed_features; // Currently used features
	do {
		ret = shaderDetailInc(ret, bWrapped);
		shaderDetailToFeatures(ret, &features_enable, &features_disable);
		if ((features_enable & rdr_caps.allowed_features) == starting_features) {
			// Incrementing the detail level got us no new features, increment until we wrap and gain/lose a feature
		} else {
			// Good
			return ret;
		}
		if (ret == starting_level)
			return ret; // We looped all the way around, we must have no features!
	} while (true);
}

// Finds the highest avaible shader detail
OptionShaderDetail shaderDetailHighestAvailable()
{
	OptionShaderDetail ret = SHADERDETAIL_NOBUMPS; // lowest availble setting
	OptionShaderDetail next = SHADERDETAIL_NOBUMPS;

	while( (next = shaderDetailIncAllowed(ret, NULL)) != SHADERDETAIL_NOBUMPS )
	{
		ret = next;
	}

	return ret;

}

