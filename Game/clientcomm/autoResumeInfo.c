#include "autoResumeInfo.h"
#include "file.h"
#include "uiLogin.h"
#include "uiEditText.h"
#include "cmdgame.h"
#include "utils.h"
#include "win_init.h"
#include "RegistryReader.h"
#include "AppRegCache.h"
#include "sound_sys.h"
#include "timing.h"
#include "textparser.h"
#include "uiOptions.h"
#include "crypt.h"

//ResumeInfo is now in the registry
#define REG_STR_BUF_SIZE 256

static GfxSettings gfxSettings_temp;

// Load/Save these regardless of safe-mode
TokenizerParseInfo autoResumeRegInfoSafe[] = {
	{ "accountName",			TOK_STRING_X, (int)&gfxSettings_temp.accountName, ARRAY_SIZE_CHECKED(gfxSettings_temp.accountName)},
	{ "gamma",					TOK_F32_X,	(int)&gfxSettings_temp.gamma},
	{ "autoFOV",				TOK_INT_X,	(int)&gfxSettings_temp.autoFOV},
	{ "fieldOfView",			TOK_INT_X,	(int)&gfxSettings_temp.fieldOfView},
	{ "fxSoundVolume",			TOK_F32_X,	(int)&gfxSettings_temp.fxSoundVolume},
	{ "musicSoundVolume",		TOK_F32_X,	(int)&gfxSettings_temp.musicSoundVolume},
	{ "voiceoverSoundVolume",	TOK_F32_X,	(int)&gfxSettings_temp.voSoundVolume},
	{ "dontSaveName",			TOK_INT_X,	(int)&gfxSettings_temp.dontSaveName},
	{ "", 0, 0 }
};

// Load always, Save these only if not in safe-mode
TokenizerParseInfo autoResumeRegInfo[] = {
	{ "version",			TOK_INT_X,	(int)&gfxSettings_temp.version},
	{ "screenX",			TOK_INT_X,	(int)&gfxSettings_temp.screenX},
	{ "screenY",			TOK_INT_X,	(int)&gfxSettings_temp.screenY},
	{ "refreshRate",		TOK_INT_X,	(int)&gfxSettings_temp.refreshRate},
	{ "screenX_pos",		TOK_INT_X,	(int)&gfxSettings_temp.screenX_pos},
	{ "screenY_pos",		TOK_INT_X,	(int)&gfxSettings_temp.screenY_pos},
	{ "maximized",			TOK_INT_X,	(int)&gfxSettings_temp.maximized},
	{ "fullScreen",			TOK_INT_X,	(int)&gfxSettings_temp.fullScreen},
	{ "mipLevel",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.mipLevel},
	{ "characterMipLevel",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.entityMipLevel},
	{ "texLodBias",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.texLodBias},
	{ "texAniso",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.texAniso},
	{ "worldDetailLevel",	TOK_F32_X,	(int)&gfxSettings_temp.advanced.worldDetailLevel},
	{ "entityDetailLevel",	TOK_F32_X,	(int)&gfxSettings_temp.advanced.entityDetailLevel},
	{ "shadowMode",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.shadowMode},
	{ "shadowMapShowAdvanced",TOK_INT_X,	(int)&gfxSettings_temp.advanced.shadowMap.showAdvanced},
	{ "shadowMapShader",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.shadowMap.shader},
	{ "shadowMapSize",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.shadowMap.size},
	{ "shadowMapDistance",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.shadowMap.distance},
	{ "cubemapMode",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.cubemapMode},
	{ "buildingPlanarReflections",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.buildingPlanarReflections},
	{ "ageiaOn",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.ageiaOn},
	{ "physicsQuality",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.physicsQuality},
	{ "showAdvanced",		TOK_INT_X,	(int)&gfxSettings_temp.showAdvanced},
	{ "graphicsQuality",	TOK_F32_X,	(int)&gfxSettings_temp.slowUglyScale},
	{ "maxParticles",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.maxParticles},
	{ "maxParticleFill",	TOK_F32_X,	(int)&gfxSettings_temp.advanced.maxParticleFill},
	{ "suppressCloseFx",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.suppressCloseFx},
	{ "suppressCloseFxDist",TOK_F32_X,	(int)&game_state.suppressCloseFxDist},
	{ "forceSoftwareAudio",	TOK_INT_X,	(int)&gfxSettings_temp.forceSoftwareAudio},
	{ "enableVBOs",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.enableVBOs},
	{ "enable3DSound",		TOK_INT_X,	(int)&gfxSettings_temp.enable3DSound},
	{ "renderScaleX",		TOK_F32_X,	(int)&gfxSettings_temp.renderScaleX},
	{ "renderScaleY",		TOK_F32_X,	(int)&gfxSettings_temp.renderScaleY},
	{ "useRenderScale",		TOK_INT_X,	(int)&gfxSettings_temp.useRenderScale},
	{ "shaderDetail",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.shaderDetail},
	{ "useWater",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.useWater},
	{ "useBloom",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.useBloom},
	{ "useDesaturate",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.useDesaturate},
	{ "bloomMagnitude",		TOK_F32_X,	(int)&gfxSettings_temp.advanced.bloomMagnitude},
	{ "useDOF",				TOK_INT_X,	(int)&gfxSettings_temp.advanced.useDOF},
	{ "dofMagnitude",		TOK_F32_X,	(int)&gfxSettings_temp.advanced.dofMagnitude},
	{ "ambientShowAdvanced",TOK_INT_X,	(int)&gfxSettings_temp.advanced.ambient.showAdvanced},
	{ "ambientOptionScale",	TOK_F32_X,	(int)&gfxSettings_temp.advanced.ambient.optionScale},
	{ "ambientStrength",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.ambient.strength},
	{ "ambientResolution",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.ambient.resolution},
	{ "ambientBlur",		TOK_INT_X,	(int)&gfxSettings_temp.advanced.ambient.blur},
	{ "antiAliasing",		TOK_INT_X,	(int)&gfxSettings_temp.antialiasing},
	{ "useVSync",			TOK_INT_X,	(int)&gfxSettings_temp.advanced.useVSync},
	{ "fancyMouseCursor",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.colorMouseCursor},
	{ "ShowLoginDialog",	TOK_INT_X,	(int)&game_state.showLoginDialog},
	{ "enableHWLights",		TOK_INT_X,	(int)&gfxSettings_temp.enableHWLights},
	{ "razerMouseTray",		TOK_INT_X,	(int)&game_state.razerMouseTray},
	{ "showExperimental",	TOK_INT_X,	(int)&gfxSettings_temp.advanced.showExperimental},
	{ "useCelShader",		TOK_INT_X,	(int)&gfxSettings_temp.useCelShader},
	{ "celPosterization",	TOK_INT_X,	(int)&gfxSettings_temp.celshader.posterization},
	{ "celAlphaQuant",		TOK_INT_X,	(int)&gfxSettings_temp.celshader.alphaquant},
	{ "celSaturation",		TOK_F32_X,	(int)&gfxSettings_temp.celshader.saturation},
	{ "celOutlines",		TOK_INT_X,	(int)&gfxSettings_temp.celshader.outlines},
	{ "celLighting",		TOK_INT_X,	(int)&gfxSettings_temp.celshader.lighting},
	{ "celSuppressFilters",	TOK_INT_X,	(int)&gfxSettings_temp.celshader.suppressfilters},
	{ "stupidShaderTricks",	TOK_INT_X,	(int)&gfxSettings_temp.experimental.stupidShaderTricks},
	{ "", 0, 0 }
};

// Load only
TokenizerParseInfo autoResumeRegInfoReadOnly[] = {
	{ "Auth",			TOK_STRING_X,	(int)&game_state.auth_address, ARRAY_SIZE_CHECKED(game_state.auth_address)},
	{ "DbServer",		TOK_STRING_X,	(int)&game_state.cs_address, ARRAY_SIZE_CHECKED(game_state.cs_address)},
	{ "", 0, 0 }
};

void saveAutoResumeInfoToRegistry(void)
{
	gfxGetSettingsForNextTime( &gfxSettings_temp );

	if (!gfxSettings_temp.filledIn)
		return;

	//TO DO Should be params to this function really
	gfxSettings_temp.dontSaveName = g_iDontSaveName;

	if( gfxSettings_temp.dontSaveName )
		Strncpyt(gfxSettings_temp.accountName, "");
	else
		Strncpyt(gfxSettings_temp.accountName, g_achAccountName);
	
	//Kind of a hack: if your vis_scale is silly high, you almost certainly
	//don't want that saved to the registry.  You were probably editing maps
	if( gfxSettings_temp.advanced.worldDetailLevel > WORLD_DETAIL_LIMIT )
		gfxSettings_temp.advanced.worldDetailLevel = 1.0;
	//End hack

	ParserSaveToRegistry(regGetAppKey(), autoResumeRegInfoSafe, NULL, 0, 0);
	if (!game_state.safemode || game_state.options_have_been_saved)
	{
		ParserSaveToRegistry(regGetAppKey(), autoResumeRegInfo, NULL, 0, 0);
	}
}

int getAutoResumeInfoFromRegistry( GfxSettings * gfxSettings, char * accountName, int * dontSaveName )
{
	// Determine what settings may need to be overwritten
	bool isFirstRun = true;
	bool isFirstRunSinceSlowUglySlider = true;
	ZeroStruct(&gfxSettings_temp);
	gfxSettings_temp.slowUglyScale = -1;
	ParserLoadFromRegistry(regGetAppKey(), autoResumeRegInfo, NULL);
	if (gfxSettings_temp.screenX != 0)
		isFirstRun = false;
	if (gfxSettings_temp.slowUglyScale != -1)
		isFirstRunSinceSlowUglySlider = false;

	gfxSettings_temp = *gfxSettings; // Fill in defaults 
	gfxSettings_temp.version = 0;

	ParserLoadFromRegistry(regGetAppKey(), autoResumeRegInfoSafe, NULL);
	ParserLoadFromRegistry(regGetAppKey(), autoResumeRegInfo, NULL);

	gfxSettings_temp.filledIn = true;

	if (isFirstRunSinceSlowUglySlider && !isFirstRun) {
		// User had previous settings, don't overwrite them!
		gfxSettings_temp.slowUglyScale = 0.6;
		gfxSettings_temp.showAdvanced = 1;
	}

	gfxSettingsFixup(&gfxSettings_temp);

	*gfxSettings = gfxSettings_temp;

	// Sanity checks and copying appropriate data
	if (accountName)
		strcpy(accountName, gfxSettings->accountName);

	if (gfxSettings->advanced.worldDetailLevel < 0.5f)
		gfxSettings->advanced.worldDetailLevel = 0.5f;
	else if (gfxSettings->advanced.worldDetailLevel > WORLD_DETAIL_LIMIT)
		gfxSettings->advanced.worldDetailLevel = WORLD_DETAIL_LIMIT;

	if (gfxSettings->advanced.entityDetailLevel < 0.3f)
		gfxSettings->advanced.entityDetailLevel= 0.6f;
	else if (gfxSettings->advanced.entityDetailLevel> 2.0f)
		gfxSettings->advanced.entityDetailLevel = 2.0f;

	g_audio_state.software = gfxSettings->forceSoftwareAudio;

	if (dontSaveName)
		*dontSaveName = gfxSettings->dontSaveName;

	g_audio_state.uisurround = gfxSettings->enable3DSound;

	game_state.enableHardwareLights = gfxSettings->enableHWLights;

	if (gfxSettings->useRenderScale) {
		int effResX, effResY;
		if (gfxSettings->useRenderScale==RENDERSCALE_FIXED) {
			effResX = gfxSettings->renderScaleX;
			effResY = gfxSettings->renderScaleY;
		} else if (gfxSettings->useRenderScale==RENDERSCALE_SCALE) {
			effResX = gfxSettings->renderScaleX*gfxSettings->screenX;
			effResY = gfxSettings->renderScaleY*gfxSettings->screenY;
		}
		if (effResX < 16 || effResX > gfxSettings->screenX ||
			effResY < 16 || effResY > gfxSettings->screenY)
		{
			gfxSettings->useRenderScale=RENDERSCALE_OFF;
		}
	}

	if (gfxSettings->autoFOV || gfxSettings->fieldOfView < FIELDOFVIEW_MIN || gfxSettings->fieldOfView > FIELDOFVIEW_MAX)
		gfxSettings->fieldOfView = FIELDOFVIEW_STD;
	if (gfxSettings->advanced.bloomMagnitude < 0.1 || gfxSettings->advanced.bloomMagnitude > 4.0)
		gfxSettings->advanced.bloomMagnitude = 1.0;
	if (gfxSettings->advanced.dofMagnitude < 0.1 || gfxSettings->advanced.dofMagnitude > 4.0)
		gfxSettings->advanced.dofMagnitude = 1.0;

	if (game_state.safemode)
	{
		g_audio_state.software = gfxSettings->forceSoftwareAudio = 1;
		optionSet(kUO_EnableJoystick, 0, 0);
		g_audio_state.uisurround = gfxSettings->enable3DSound = 0;
		game_state.enableHardwareLights = gfxSettings->enableHWLights = 0;

		gfxGetSafeModeSettings(gfxSettings);
	}

	return 1;
}

/*
Set password if you have the "cryptic" setting.  Getting the account name is redundant with
the registry account name setting so it's backwards compatible with old resume info file
*/
void saveAutoResumeInfoCryptic(void)
{
	FILE	*file;

	PERFINFO_AUTO_START("saveAutoResumeInfoCryptic", 1);
		file = fileOpen( fileMakePath("resume_info.txt", "", "", false), "wt");

		if( file )
		{
			char password[32];
			assert(sizeof(password) == sizeof(g_achPassword));
			cryptRetrieve(password, g_achPassword, sizeof(g_achPassword));
			fprintf(file,"\"%s\"\n", g_achAccountName);
			fprintf(file,"%s\n", password);
			fileClose( file );
			memset(password, 0, sizeof(password));			
		}
	PERFINFO_AUTO_STOP();
}

/*
Get password if you have the "cryptic" setting.  Getting the account name is redundant with
the registry account name getting so it's backwards compatible with old resume info file
*/
int getAutoResumeInfoCryptic( void )
{
	char	*s,*mem,*args[10];
	int		count;

	// Encrypt an empty string
	g_achPassword[0] = 0;
	cryptStore(g_achPassword, g_achPassword, sizeof(g_achPassword));
	mem = fileAlloc( fileMakePath("resume_info.txt", "", "", false), 0 );
	if (!mem)
		return 0;

	//Get Account Name
	count = tokenize_line(mem,args,&s);
	if(!s)
	{
		fileFree(mem);
		return 0;
	}

	if (count)
		Strncpyt(g_achAccountName, args[0]);

	//Get Password
	count = tokenize_line(s,args,&s);
	if(!s)
	{
		fileFree(mem);
		return 0;
	}

	if (count)
	{
		// cryptStore() assumes both destination and source are the same size
		strncpy_s(g_achPassword, sizeof(g_achPassword), args[0], _TRUNCATE);
		cryptStore(g_achPassword, g_achPassword, sizeof(g_achPassword));
	}

	fileFree(mem);

	return 1;
}

