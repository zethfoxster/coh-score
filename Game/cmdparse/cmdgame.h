#ifndef _CMDGAME_H
#define _CMDGAME_H

#include "stdtypes.h"
#include "cmdoldparse.h"
#include "ArrayOld.h"
#include "cmdcommon.h"
#include "groupscene.h" // For DOFValues struct
#include "gfxSettings.h" // For OptionCubemap

typedef __int64 EntityRef;

enum
{
	SHOW_GAME     = 1,
	SHOW_TEMPLATE = 2,
	SHOW_LOAD_SCREEN = 3,
	SHOW_NONE     = 4,
};

enum
{
	PICKPET_NOWINDOW			 = 0,
	PICKPET_WINDOWOPEN			 = 1,
	PICKPET_WAITINGFORACKTOCLOSE = 2,
};

void toggle_3d_game_modes(int mode);

//Fx enums for debug 
enum
{
	FX_DEBUG_BASIC			= 1,
	FX_SHOW_DETAILS			= 2,
	FX_PRINT_VERBOSE		= 4,
	FX_ENABLE_DEBUG_UI		= 8,
	FX_SHOW_SOUND_INFO		= 16,
	FX_DISABLE_PART_CALCULATION = 64,
	FX_DISABLE_PART_ARRAY_WRITE = 128,
	FX_DISABLE_PART_RENDERING  = 256,
	FX_HIDE_FX_GEO			= 512,
	FX_CHECK_LIGHTS			= 1024, //not really an fx problem

};

typedef enum UrlEnv 
{
    URLENV_NONE,	
    URLENV_DEV,
    URLENV_LIVE,
} UrlEnv;

// The numbering is important because it maintains compatibility with the
// "cov 0" and "cov 1" commands.
typedef enum UISkin
{
	UISKIN_HEROES		= 0,
	UISKIN_VILLAINS		= 1,
	UISKIN_PRAETORIANS	= 2
} UISkin;

// What actual city map this zone is a part of. CITY_NONE in case it appears on no city map.
typedef enum CityMap
{
	CITY_PARAGON		= UISKIN_HEROES,
	CITY_ROGUE_ISLES	= UISKIN_VILLAINS,
	CITY_PRAETORIA		= UISKIN_PRAETORIANS,
	CITY_NONE,
	CITY_COUNT,
} CityMap;

// The second check is the way it is so the default case for bad ents or bad
// state within the ent will be the hero skin. Praetorians travelling to Primal
// Earth still use the Praetorian skin because they haven't arrived yet.
#define UISKIN_FROM_ENT(e)	((ENT_IS_IN_PRAETORIA(e) || ENT_IS_LEAVING_PRAETORIA(e)) ? UISKIN_PRAETORIANS : (ENT_IS_VILLAIN(e) ? UISKIN_VILLAINS : UISKIN_HEROES))

// This equates to (a) if the skin is heroes or unknown, (b) if the skin is
// villains and (c) if the skin is praetorians. it'll work with strings or
// numbers so you can pick sounds, colors, texture names or whatever.
#define SELECT_FROM_UISKIN(h, v, p)		(game_state.skin == UISKIN_PRAETORIANS ? (p) : (game_state.skin == UISKIN_VILLAINS ? (v) : (h)))

typedef struct GameState
{
	U32		*packet_ids;
	char	scene[128];
	char	world[128];
	char	connect[128];
	int		fullscreen;
	int		fullscreen_toggle;
	int		maximized;
	int		minimized;	// Set to 1 to stop the game display from being updated.
	int		stopInactiveDisplay;	// Set to 1 to stop the game display when the game window is inactive.
	int		inactiveDisplay;

	int		allow_frames_buffered;
	int		screen_x;			//Real moment to moment screen x and y
	int		screen_y;
	int		refresh_rate;
	int		screen_x_restored;  //Screen x and ywhen not minimized 
	int		screen_y_restored;
	int		screen_x_pos;
	int		screen_y_pos; // Window position for windowed mode
	F32		fov,fov_1st,fov_3rd;
	F32     fov_custom; 
	F32		fov_thisframe;
	bool	fov_auto;
	int		ortho;
	int		ortho_zoom;

	int		safemode;
	int		options_have_been_saved;
	int		gfx_features_disable; // Communication from options/registry
	int		gfx_features_enable;  // Communication from options/registry

	int		info;
	int		seq_info,ambient_info,sound_info,surround_sound;
	F32		camera_shake;
	F32		camera_shake_falloff;
	F32		camera_blur;
	F32		camera_blur_falloff;
	int		floor_info;
	int		wireframe;
	char	pathname[128];
	int		showhelp;
	
	int		freemouse;
	int		netgraph;
	int		bitgraph;
	int		netfloaters;
	F32		fps;
	int		maxfps;
	int		maxInactiveFps;
	int		maxMenuFps;
	F32		showfps;
	int		showActiveVolume;
	int		graphfps;
	int		sliClear;
	U32		sliFBOs;
	U32		sliLimit;
	int		usenvfence;
	int     gpu_markers;
	int		frame_delay;
	int		verbose;
	int		tcp;
	int		port;
	char	address[256];
	int		edit;
	char	cs_address[256];
	char	auth_address[256];

	// Maximum number of world geometry verts to relight per frame
	int		maxColorTrackerVerts;
	int		fullRelight;
	int		useNewColorPicker;

	//Real Gamestate stuff
	int     game_mode;			//SHOW_GAME  SHOW_TEMPLATE (player only in shell)  SHOW_NONE
	int		groupDefVersion;	//are pointers to a groupDef good? Kindof a hack for fx using existing groufDefs

	int		powerLoopingSoundsGoForever; //hack so UI can have a toggle to make power looping sounds loop forever, if you are so inclined

	//#########Performance Parameters
	F32		partVisScale;       //magic number that says how prone the engine is to not draw insignificant particle systems
	int		maxParticles;   //most particles allowed at a time
	int		maxParticleFill;
	F32		splatShadowBias;	//scales distance to draw entitys' splat shadow
	int		disableSimpleShadows;//Turns off all splat shadows
	F32		LODBias;		    //scale the LOD switch distances for entities
	int		disableSky;

	F32		fxSoundVolume;
	F32		musicSoundVolume;
	F32		voSoundVolume;
	int		soundMuteFlags;
	int		maxSoundChannels;
	int		maxSoundSpheres;
	F32		maxPcmCacheMB;
	F32		maxSoundCacheMB;
	int		maxOggToPcm;
	int		ignoreSoundRamp;
	char	soundDebugName[64];

	int		mipLevel;			//needs unloadgfx to take effect
	int		actualMipLevel;
	int		entityMipLevel;

	int		reduce_min;
	int		texLodBias;			// Caps the texLodBias for newer cards
	int		texAnisotropic;
	int		antialiasing;

	int		showAdvancedOptions;
	int		showExperimentalOptions;
	F32		slowUglyScale;

	F32		vis_scale;
	F32		lod_scale;			//Scales lod range for world geometry
	F32		lod_fade_range;		//reduce the lod_nearfade and lod_farfade ranges (0 = no fade, 1= as authored)
	F32		draw_scale;			//Scales draw distance for world geometry
	F32		gamma;				//0.0 = no change down to -0.7 up to 3.4
	int		shadowvol;			//Needs clean up.  ShadowVol 2 turns off stencil shadows
	int		noStencilShadows;	//Startup flag to disable stencil shadows

	int		fancytrees;			//turn on masking trick for tree's alpha edge.  I don't think its used right now
	int		nofancywater;		//Disable pretty water
	int		writeProcessedShaders;
	int		nvPerfShaders;
	int		renderThreadDebug;

	// Manually setable render parameters
	int		nopixshaders;
	int		noVBOs;
	int		noparticles;
	int		novertshaders;
	int		useARBassembly;
	int		noARBfp;
	int		useTexEnvCombine;
	int		noNV;
	int		noATI;
	int		forceNv1x;
	int		forceNv2x;
	int		forceR200;
	int		forceNoChip;
	int		useChipDebug;
	int		maxTexUnits;
	int		oldVideoTest;
	int		shaderTest;
	int		nohdr;
	int		no_sun_flare;
	int		noNP2;
	int		noPBuffers;
	int		usePBuffers;
	int		useFBOs;
	int		useCg;
	int		shaderCache;
	int		shaderInitLogging;
	int		shaderOptimizationOverride;
	int		saveCgShaderListing;
	int		useHQ;
	int		useMRTs;
	int		noBump;
	int		shaderTestMode1;
	int		shaderTestMode2;
	int		use_hard_shader_constants;
	int		use_prev_shaders;
	int		noRTT;
	F32		renderScaleX;
	F32		renderScaleY;
	int		useRenderScale;
	int		renderScaleFilter;
	F32		bloomWeight;
	int		bloomScale;
	F32		dofWeight;
	int		compatibleCursors;
	int		ambientStrength;
	int		ambientResolution;
	int		ambientBlur;
	int		ambientShowAdvanced;
	F32		ambientOptionScale;

	F32		desaturateWeight;
	F32		desaturateWeightTarget;
	F32		desaturateDelay;
	F32		desaturateFadeTime;

	int		specLightMode;
	int		waterMode;

	int		useViewCache;

	//force the state of VBOs
	int		enableVBOs;
	int		enableVBOsforparticles;  
	int		disableVBOs; 
	int		disableVBOsforparticles;

	// End Manually setable render parameters

	//######### End Performance Parameters

	// Override sun/scene settings
	Vec3	fogcolor;  
	Vec2	fogdist;
	DOFValues dof_values;
	int		skyFade1, skyFade2;
	F32		skyFadeWeight;
	int		showtime;

	F32		nearz, farz;
	bool	bFlipYProjection;

	int		fogDeferring;
	int		noFog;
	//Debug and Testing stuff

	F32		test1;  //a handful of floats to have around for off the cuff debugging
	F32		test2;
	F32		test3;
	F32		test4;
	F32		test5;
	F32		test6;
	F32		test7;
	F32		test8;
	F32		test9;
	F32		test10;
	int		testInt1;
	Vec3	testVec1;
	Vec3	testVec2;
	Vec3	testVec3;
	Vec3	testVec4;
	int		testDisplayValues;
	int		test1flicker;
	int		test2flicker;
	int		test3flicker;
	int		test4flicker;
	int		test5flicker;
	int		test6flicker;
	int		test7flicker;
	int		test8flicker;
	int		test9flicker;
	int		test10flicker;

	int		whiteParticles;
	int		tintFxByType;
	int		suppressCloseFx;
	F32		suppressCloseFxDist;
	int		showDepthComplexity;
	int		showPerf; //Show the screen size
	int		headShot;
	char	groupShot[128];
	int		disableGeometryBackgroundLoader;
	int		simpleShadowDebug;
	F32		client_age;
	U32		client_frame;
	F32		client_loop_timer;  //client age looping from 0 to 65k
	U32		client_frame_timestamp; //used for timestamping texture binds, updated once per frame
	int		resetfx;
	int		reloadfx;
	int		fxdebug;
	int		fxontarget; // debug: Places tfx file FX on target instead of self
	int		renderdump; // debug: dump render buffers
	int		framegrab; // debug: capture frame
	F32		framegrab_show; // debug: show captured frame
	int		hdrThumbnail; // debug: show hdr thumbnails
	int		tintAlphaObjects; // debug: show all objects which are alphasorted
	int		tintOccluders; // debug: show all occluders and potential occluders
	F32		normal_scale; // debug: scale for drawing normals
	int		normalmap_test_mode;
	int		meshIndex;	// if >=0, draw only this mesh index when rendering sorted list in drawList_ex
	int		gfxdebug;	// debug: Bitfield of various graphical debug stuff (normals, etc)
	char	gfxdebugname[32];	// debug: Filters what objects get gfxdebug info
	int		lightdebug;	// debug: Displays current lighting values
	int		renderinfo;	// debug: Displays counts of different render modes, objects, etc
	int		animdebug;
	int		reloadSequencers; //development mode
	int		showSpecs;
	int		quickLoadAnims;
	int		nofx;
	int		quickLoadFonts;
	int		pretendToBeOnArenaMap;
	int		cod;			// indicates that D_ data should be loaded.
	UISkin	skin;			// heroes, villains or praetorians for colors, sounds etc
	int		checkVolume;
	int     cameraDebug;
	Vec3	cameraDebugPos;
	int		selectionDebug;
	int		show_entity_state;   //debug
	char	entity_to_show_state[128];//debug
	int		store_entity_id;  //The entity also set on the server as the current debug entity (propagated up, not down: must be set on all clients)
	//***
	int		wackyxlate;
	int		netfxdebug;
	int		perf;
	int		pbuftest;
	int		fxprint;
	int     resetparthigh; 
	F32		bonescale;
	char	tfx[100]; 
	F32		fxpower;
	int		fxinfo; 
	int		checkcostume;
	int		checklod;
	int		nopredictplayerseq;
	int		nointerpolation;
	int		fxkill;
	int		fxkillDueToInternalError;
	int		martinboxes;
	int		fixArmReg;
	int		fixLegReg;
	int		fixRegDbg;
	int		waterDebug;
	int		waterReflectionDebug;
	int		reflectionEnable;
	int		separate_reflection_viewport;
	int		allow_simple_water_reflection;	// Sky only when separate_reflection_viewport is off
	F32		planar_reflection_mip_bias;
	F32		cubemap_reflection_mip_bias;
	int		gen_static_cubemaps;
	int		reflectionDebug;
	char	reflectiondebugname[128];
	int		reflection_planar_mode;
	F32		reflection_planar_lod_scale;			// for controlling how much of the nearby environment we render into the reflection
	int		reflection_planar_lod_model_override;	// control much world detail we render into the reflection, at this time only valid values are 0 for 'no override' and MAX_LODS to use the lowest detail lod.
	float	reflection_planar_fade_begin;
	float	reflection_planar_fade_end;
	F32		reflection_water_lod_scale;				// for controlling how much of the nearby environment we render into the reflection
	int		reflection_water_lod_model_override;	// control much world detail we render into the reflection, at this time only valid values are 0 for 'no override' and MAX_LODS to use the lowest detail lod.
	int		reflection_generic_fbo_size;
	int		reflection_cubemap_lod_model_override;	// control much world detail we render into the reflection, at this time only valid values are 0 for 'no override' and MAX_LODS to use the lowest detail lod.
	float	reflection_cubemap_entity_vis_limit_dist_from_player;
	float	reflection_planar_entity_vis_limit_dist_from_player;
	float	reflection_water_entity_vis_limit_dist_from_player;
	float	reflection_distortion_strength;
	float	reflection_base_strength;
	float	reflection_distort_mix;
	float	reflection_base_normal_fade_near;
	float	reflection_base_normal_fade_range;
	int		reflection_proj_clip_mode;
	float	water_reflection_min_screen_size;
	int		newfeaturetoggle;
	int		no_nv_clip;							// disable using vertex program user clipping profile even if available

	int		spriteSortDebug;

	// This group of variables are controlled via the options menu
	OptionShadow		shadowMode;			// 0 = off, 1 = stencil, 2 = shadowMap_low, 3 = shadowMap_Medium, 4 = SM_High, 5 = SM_Highest
	int					shadowMapShowAdvanced;
	OptionShadowShader	shadowShaderSelection;
	OptionShadowShader	shadowShaderInUse;
	int					shadowmap_num_cascades;
	int					shadowMapTexSize;

	bool				shadowMapsAllowed;	// 1 if outdoors, 0 if indoors/disabled for some other reason
	int		shadowMapDebug;
	int		shadowDirType;
	float shadowDirAltitude;
	float shadowMaxSunAngleDeg;
	int		shadowSplitCalcType;
	int   shadowMapBind;
	float shadowFarMax;
	float shadowWidthMax;
	int		shadowViewBound;
	int		shadowMapFreeze;
	float shadowLightSizeZ;
	float	shadowSplitLambda;
	float	shadowSplit1;
	float	shadowSplit2;
	float	shadowSplit3;
	float biasScale[6];
	int		shadowStippleOpaqueFadeMode;
	int		shadowStippleAlphaFadeMode;
	int		shadowDebugFlags;
	int		shadowDeflickerMode;
	float	shadowScaleQuantLevels;
	float shadowDebugPercent1;
	float shadowDebugPercent2;
	int		shadowmap_lod_alpha_snap_entities;	// what alpha value to snap on drawing into shadowmap (i.e. no stipple fading) - entities
	int		shadowmap_lod_alpha_snap;			// what alpha value to snap on drawing into shadowmap (i.e. no stipple fading) - world geo
	int		shadowmap_lod_alpha_begin;
	int		shadowmap_lod_alpha_end;
	float	shadowmap_extra_fov;
	Vec4	shadowmap_split_overlap;

	int		debugPowerCustAnim;

	//end mm gamestate

	//Image Server
	int		imageServer;
	char	imageServerSource[2048];
	char	imageServerTarget[2048];
	int		imageServerDebug;
	int		noSaveTga;
	int		noSaveJpg;
	//End image server

	char	startupExec[256];

	int		nodebug;
	int		spin360;
	F32		camdist;
	int		tri_hist;
	int		tri_cutoff;
	
	int		packet_mtu;				// Override for setting size of packets the client generates

	int		can_edit;
	int		local_map_server;
	int		no_version_check;
	int		ask_local_map_server;
	int		create_bins;			// create all .bin files and exit
	int		create_map_images;		// create the image files for all of the maps
	int		costume_diff;			// check costume changes then exit

	int		ask_no_audio;
	int		editnpc;				// to let users edit npc's
	int		maptest;				// so you can force mission maps to show up for testing
	int		cryptic;
	int		base_map_id;
	int		intro_zone;
	int		team_area;				// Which part of the game world are we in?

	int		vtuneCollectInfo;		// Should vtune be collecting performance info right now?
	char	song_name[1000];
	int		see_everything;

	int		hasEntDebugInfo;
	int		ent_debug_client;
	int		debug_collision;
	int		script_debug_client;

	int		quick_login;
	int		ask_quick_login;
	int		disable2Dgraphics;
	int     disableHeadsUp;  //just disable screen overlay, not all the other stuff
	int		disableGameGUI;

	int		viewCutScene;			// viewing a cut scene
	int		forceCutSceneLetterbox;	// for cutscene creation, force the letterbox effect
	Mat4	cutSceneCameraMat;
	F32		cutSceneDepthOfField;
	Mat4	cutSceneCameraMat1;
	F32		cutSceneDepthOfField1;
	Mat4	cutSceneCameraMat2;
	F32		cutSceneDepthOfField2;
	U32		cutSceneLERPStart;
	U32		cutSceneLERPEnd;
	char    *cutSceneMusic;

	int		pendingTSMapXfer;		//	pending turnstile map transfer soon. block some actions (menu's)
									//	map xfers interrupt menus (some of them need to be exited properly)
									//	rather than exiting properly, just don't allow players to enter the menu

	int		place_entity;
	int		super_vis;
	int		select_any_entity;
	
	EntityRef	camera_follow_entity;
	int		camera_shared_slave;
	int		camera_shared_master;
	
	int		can_set_cursor;

	int		reloadCostume;
	int		showMouseCoords;
	int		ttDebug;
	int 	bMapShowPlayers; // show all players on map
	int 	bShowPointers;   // show pointers on player's feet

	int		tutorial;
	int		alwaysMission;
	int		noMapFog;
	int		screenshot_ui;	// 1=make screenshots with ui, 0=just save 3d graphics
	
	char	world_name[MAX_PATH];
	int		mission_map;
	int		map_instance_id;
	int		making_map_images;

	char	shard_name[128];

	int		clothDebug;
	F32		clothWindMax;
	F32		clothRipplePeriod;
	F32		clothRippleRepeat;
	F32		clothRippleScale;
	F32		clothWindScale;
	int		clothLOD;

	int		e3screenshot;

	int		oldDriver;
	char	oldDriverMessage[5][1024];

	int		ignoreBadDrivers;	// This is for false positives in the bad driver logic
	bool	fullDump;

	char	chatHandle[128];
	char	chatShard[128];

	bool	gFriendHide;
	bool	gTellHide;

	int		doNotAutoGroup; //editor setting gotten from the server when a map is loaded
	int		iAmAnArtist;	//After connecting to a map, the mapserver said it was launched with -ImAnArtist
	int		remoteEdit;		//this is a remote-editable static map

	// The texture to invoke the texWordEditor on
	// ARM NOTE: We check if this is not NULL to determine if we're in texWordEdit mode.
	//   As far as I can tell, once you set this, it was never being unset.
	char	*texWordEdit;

	int		texWordVerbose;
	int		texWordPPS;
	int		showCustomErrors;

	char	exit_launch[1024]; // program to launch when game exits
	
	int		cursor_cache;

	int		ctm_autorun_timer;

	int		showcomplexity;
    int     showrdrstats;
	int		costbars;
	int		costbars_forceoff;
	int		costgraph;
	int		stats_cards; // Bit field indicating which cards to display on the cost bars.  The first one will be used for the cost graph.

	int		atlas_stats, atlas_display; // texture atlas debug

	int		conorhack;

	int		mapsetpos;

	int		dont_atlas;

	int		noTexFreeOnResChange;

	int		hideCollisionVolumes;

	int		zocclusion, zocclusion_debug, zocclusion_off;
	int		auto_perf;

	// lod_entity_override is used by entities
	int		lod_entity_override;		// 0 for no override, MAX_LODS for force to lowest LOD

	int		scaleEntityAmbient; // Scale, rather than clamp, ambient light components
								// outside min and max ambient.

	F32		maxEntAmbient;		// Maximum entity ambient in the range [0-255]

	int		forceFullRelight;

	// base test commands
	int		edit_base;
	int		room_look[2];
	int		random_base;
	int		base_block_size;
	int		base_raid;

	int		pickPets;
	int		inPetsFromArenaList;

	F32		console_scale;

	unsigned int	beacons_loaded: 1;

	int activity_detected;
	int ignore_activity;

	int minimal_loading;

	int		enableBoostDiminishing;		// Enable diminishing returns on boosts (aka Enhancement Diversification)
	int		enablePowerDiminishing;		// Enable diminishing returns on powers (for PvP) - default off
	int		beta_base;
	int		no_base_edit;

	int		texture_special_flags;
	int		texoff;

	int fileDataDirLastChangeTime;

	int useVSync;

	int no_welding;
	int show_portals;
	F32 debug_tex;

	int search_visible;

	int missioncontrol;

	F32 dbTimeZoneDelta; //in hrs: time zone of the shard I'm connected to
	int trainingshard;

	int scrolled;

	int showLoginDialog;
		 
    UrlEnv url_env; // use live URLs in debug mode
  
    int allegorithmic_disabled;
    int allegorithmic_enabled;
    int allegorithmic_debug;

	int disable_two_pass_alpha_water_objects;

	float waterRefractionSkewNormalScale;
	float waterRefractionSkewDepthScale;
	float waterRefractionSkewMinimum;
	int	  waterReflectionOverride;
	float waterReflectionSkewScale;
	float waterReflectionStrength;
	float waterReflectionNightReduction; /* percentage from 0.0 to 1.0f */
	float waterFresnelBias;
	float waterFresnelScale;
	float waterFresnelPower;
	int   reflectivityOverride;
	float buildingFresnelBias;
	float buildingFresnelScale;
	float buildingFresnelPower;
	float buildingReflectionNightReduction; /* percentage from 0.0 to 1.0f */
	float skinnedFresnelBias;
	float skinnedFresnelScale;
	float skinnedFresnelPower;

	int		cubemap_flags;
	OptionCubemap 	cubemapMode;
	int		cubemap_size;
	int		cubemap_size_capture;
	int		cubemap_size_gen;
	float 	cubemapLodScale;
	float 	cubemapLodScale_gen;
	int		display_cubeface;
	int		static_cubemap_terrain;
	int		static_cubemap_chars;
	int		cubemapUpdate4Faces;
	int		cubemapUpdate3Faces;

	int reflectNormals;			// multi9 shader.  true if want to reflect using normalmap normals
	float cubemapAttenuationStart;	// distance from camera to start attenuating cubemap reflectivity (in model radius units)
	float cubemapAttenuationEnd;	// distance from camera to end attenuating cubemap reflectivity (in model radius units)
	float cubemapAttenuationSkinStart;	// distance from camera to start attenuating cubemap reflectivity (in model radius units) for skinned models
	float cubemapAttenuationSkinEnd;	// distance from camera to end attenuating cubemap reflectivity (in model radius units) for skinned models
	int forceCubeFace;
	int charsInCubemap;
	int debugCubemapTexture;	//turn on an assert if the cubemap texture header isn't good.


	int profiling_memory;
	int profile;
	int profile_spikes;

	int architect_dumpid;

	int isBetaShard;
	int isVIPShard;
	int GRNagAndPurchase;			// show the nag dialogs for Going Rogue and the buy button


	// TODO: Remove this once ATI memory leak regarding glEnable(GL_POLYGON_STIPPLE) in rt_shadow.c is fixed (11/06/09)
	int ati_stipple_leak;
	int ati_stencil_leak;

	// TODO: remove this once the zowie crash bug is verified fixed
	int test_zowie_crash;
	
	int	use_dxt5nm_normal_maps;		// for testing the new mode

	int force_fallback_material;

	int dbg_log_batches;			// debug: if set and a dev build then we write log files with viewport render batches

	// TODO: remove once transform toolbar is fully approved
	int transformToolbar;

	int useCelShader;
	int celPosterization;
	int celAlphaQuant;
	F32 celSaturation;
	int celOutlines;
	int celLighting;
	int celSuppressFilters;

	int stupidShaderTricks;

	int supportHardwareLights;
	int enableHardwareLights;
	int emulateHardwareLights;
	int priorityBoost;	// Boost process priority
	int razerMouseTray;

	int can_skip_praetorian_tutorial;
	int skipEULANDA;	// Skip EULA/NDA in non-production modes
	int forceNDA;		// Force NDA
	int usedLauncher;	// Game was started with NCsoft Launcher instead of CohUpdater
	int doFullVerify;	// Force a full checksum verify at startup
	int noVerify;		// skip verify
	
	// Leave these at/near the end since they are large and rarely used
	char newAccountURL[256];
	char linkAccountURL[256];
} GameState;

extern GameState game_state;
extern CmdList game_cmdlist;

// Client debug state variables
// Moving debug state out of GameState so that it is only defined
// in the non final builds and clearly identifies debug only variables
// @todo These should probably be moved to their own file or
// system specific structures
// @todo there are lots of fields in GameState that should be aggregated by system
// and/or moved into debug state structures
#ifndef FINAL
typedef struct ClientDebugState
{
	int		particles_only_index;

	int		submeshIndex;		// if >=0, draw only this submesh index for world models (use in conjunction with hideothers as a way to solo a single mesh in a snigle model)
	char	modelFilter[128];	// Only render models containing this case-insensitive substring
	char	trace_tex[128];		// Convenience to debug/trace textures containing this case-insensitive substring
	char	trace_trick[128];	// Convenience to debug/trace textures containing this case-insensitive substring

	int		pigg_logging;		// if > 0, enable pigg logging

	// lod_model_override is used to force rendering of a specific environment model (GroupDef) LOD 
	int		lod_model_override;	// 0 for no override, MAX_LODS to force to lowest LOD

	int		this_sprite_only;	// only draw this index of the sprite batch list
}ClientDebugState;
extern ClientDebugState g_client_debug_state;
#endif

typedef struct KeyBindProfile KeyBindProfile;
typedef struct KeyBind KeyBind;

extern KeyBindProfile game_binds_profile;	

// Generated by mkproto
void cmdAccessOverride(int level);
int cmdAccessLevel(void);
int encryptedKeyedAccessLevel(void);

void updateControlState(ControlId id, MovementInputSet set, int state, int time_stamp);
int cmdGameParse(char *str, int x, int y);
void cmdSetTimeStamp(int timeStamp);
int cmdGetTimeStamp(void);
int cmdParse(char *str);
void cmdParsef(char const *fmt, ...);
int cmdParseEx(char *str, int x, int y);
void gameStateSave(void);
void gameStateInit(void);
void gameStateLoad(void);
void cmdReceiveShortcuts(Packet *pak);
void cmdReceiveServerCommands(Packet *pak);

// given the beginning of the command, find the first match
//
// returns a search ID to be passed in to later calls.  pass 0 initially
int cmdCompleteComand(char *str, char *out, int searchID, int searchBackwards);
// add a server command to the list of server commands for tab completion
void cmdAddServerCommand(char * scmd);
// returns true if the string is an available server command
int cmdIsServerCommand(char *str);

void updateClientFrameTimestamp(void);

int cfg_ArePraetoriansAllowed(void);
int cfg_IsBetaShard(void);
int cfg_IsVIPShard(void);
int cfg_NagGoingRoguePurchase(void);

void cfg_setIsBetaShard(int data);
void cfg_setNagGoingRoguePurchase(int data);
void cfg_setIsVIPShard(int data);

#define E3_SCREENSHOT 1

// End mkproto
#endif
