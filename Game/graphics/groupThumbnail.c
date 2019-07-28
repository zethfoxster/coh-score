#include "groupThumbnail.h"
#include "textureatlas.h"
#include "tex.h"
#include "group.h"
#include "StashTable.h"
#include "camera.h"
#include "win_init.h"
#include "cmdgame.h"
#include "sprite_base.h"
#include "sun.h"
#include "gfx.h"
#include "anim.h"
#include "renderprim.h"
#include "gfxwindow.h"
#include "rendertree.h"
#include "renderUtil.h"
#include "tex_gen.h"
#include "pbuffer.h"
#include "seqgraphics.h"
#include "utils.h"
#include "tricks.h"
#include "fx.h"
#include "particle.h"
#include "grouptrack.h"
#include "groupdraw.h"
#include "Color.h"
#include "HashFunctions.h"
#include "baseedit.h"
#include "fxdebris.h"
#include "groupMiniTrackers.h"

#define GROUP_UID_BOOST 1000

extern int client_frame_timestamp;
extern FxEngine fx_engine;

bool defHasFx(GroupDef *def)
{
	int i;
	if (def->has_fx)
		return true;
	for (i=0; i<def->count; i++) {
		if (defHasFx(def->entries[i].def))
			return true;
	}
	return false;
}

bool defsHaveFx(GroupDef **defs)
{
	int i;
	for (i=0; i<eaSize(&defs); i++) {
		if (defHasFx(defs[i]))
			return true;
	}
	return false;
}

static int cached_version;
static int last_frame;
struct {
	Vec3 pos;
	DefTracker *tracker;
	int frame;
} **cached_trackers;

void hardKillFx(DefTracker *tracker)
{
	int i;
	if (tracker->fx_id)
		tracker->fx_id = fxDelete( tracker->fx_id, HARD_KILL );
	for(i=0;i<tracker->count;i++)
	{
		hardKillFx(&tracker->entries[i]);
	}
}

void clearTrackerCache(int last_frame)
{
	int i;
	if (game_state.groupDefVersion != cached_version) {
		cached_version = game_state.groupDefVersion;
		// Clear it out!
		eaDestroyEx(&cached_trackers, NULL);
	}
	for (i=eaSize(&cached_trackers)-1; i>=0; i--) {
		// Free things older than last_frame
		if (cached_trackers[i]->frame < last_frame || last_frame==0) {
			// Close stuff
			hardKillFx(cached_trackers[i]->tracker);
			trackerClose(cached_trackers[i]->tracker);
			free(cached_trackers[i]->tracker);
			// Remove and free it
			free(eaRemoveFast(&cached_trackers, i));
		}
	}
}

DefTracker *getCachedDefTracker(GroupDef *def, Mat4 mat)
{
	int i;
	if (!defHasFx(def))
		return NULL;
	if (game_state.groupDefVersion != cached_version) {
		cached_version = game_state.groupDefVersion;
		// Clear it out!
		eaDestroyEx(&cached_trackers, NULL);
	}
	if (global_state.global_frame_count != last_frame) {
		clearTrackerCache(last_frame);
		last_frame = global_state.global_frame_count;
	}
	for (i=eaSize(&cached_trackers)-1; i>=0; i--)
	{
		if (nearSameVec3Tol(cached_trackers[i]->pos, mat[3], 1))
		{
			if (cached_trackers[i]->tracker->def == def)
			{
				cached_trackers[i]->frame = global_state.global_frame_count;
				return cached_trackers[i]->tracker;
			}
			// Otherwise, near the same place, but not the same one, kill its FX quickly!
			// Close stuff
			hardKillFx(cached_trackers[i]->tracker);
			trackerClose(cached_trackers[i]->tracker);
			free(cached_trackers[i]->tracker);
			// Remove and free it
			free(eaRemoveFast(&cached_trackers, i));
		}
	}
	// Create new entry
	i = eaPush(&cached_trackers, calloc(sizeof(**cached_trackers), 1));
	copyVec3(mat[3], cached_trackers[i]->pos);
	cached_trackers[i]->tracker = calloc(sizeof(DefTracker), 1);
	cached_trackers[i]->tracker->def = def;
	copyMat4(mat, cached_trackers[i]->tracker->mat);
	trackerOpen(cached_trackers[i]->tracker);
	cached_trackers[i]->frame = global_state.global_frame_count;
	return cached_trackers[i]->tracker;
}

void drawDefSimpleWithTrackers(GroupDef *def, Mat4 mat, Color color[2], int uid)
{
	Mat4 matx,viewmat;
	DefTracker *tracker = getCachedDefTracker(def, mat);

	gfxSetViewMat(cam_info.cammat,viewmat,0);
	mulMat4(viewmat,mat,matx);

	if(color)
	{
		def->has_tint_color = 1;
		memcpy(def->color, color, 4*2);
	}

	if (tracker) {
		copyMat4(mat, tracker->mat);
		trackerUpdate(tracker, def, 0);
		tracker->def_mod_time = def->mod_time;
	}

	groupDrawDefTracker(def,tracker,matx,0,0,1,0,uid);

	if(color)
		def->has_tint_color = 0;
}

static void renderGroupDef(Vec3 headshotCameraPos, GroupDef **defs, const Mat4 groupMat, bool rotate, AtlasTex * bind, int sizeOfPictureX, int sizeOfPictureY, bool lookFromBelow, Vec3 objPosition)
{
	CameraInfo oldCamInfo;
	int		disable2d;
	int		game_mode;
	int		fov_custom;
	int heldBackgroundLoaderState;
	int ULx, ULy;
	int screenX, screenY;
	Mat4 objMat;
	Vec3 cameraPos;
	Vec3 bg_color = {0.0, 0.0, 0.0};
	Vec3 avg_mid={0,0,0};
	int desired_aa=4;
	int required_aa=1;
	int num_defs = eaSize(&defs);
	int i;
	int old_zocclusion;
	bool use_pbuffers = rdr_caps.use_pbuffers; // No PBuffers for no Alpha channel 

	if (use_pbuffers) {
		initHeadShotPbuffer(sizeOfPictureX, sizeOfPictureY, desired_aa, required_aa);
	}

	windowSize( &screenX, &screenY );
	ULx = ((F32)screenX)/2.0 - (F32)sizeOfPictureX / 2.0; //TO DO UL really LL?
	ULy = ((F32)screenY)/2.0 - (F32)sizeOfPictureY / 2.0;

	copyMat3(groupMat, objMat);
	zeroVec3(objMat[3]);
	// Offset actual object position based on pivot vs. center point
	for (i=0; i<num_defs; i++)
	{
		int j;
		Vec3		min = {8e8,8e8,8e8},max = {-8e8,-8e8,-8e8},mid;
		groupGetBoundsMinMaxNoVolumes(defs[i],objMat,min,max);
		for (j=0; j <= 2; j++)
			mid[j] = (min[j]+max[j])/2.0;
		addVec3(avg_mid,mid,avg_mid);
	}
	copyVec3(objPosition, objMat[3]);
	scaleVec3(avg_mid, 1.f/num_defs, avg_mid);
	subVec3(objMat[3], avg_mid, objMat[3]);

	//Preserve the old params
	heldBackgroundLoaderState = game_state.disableGeometryBackgroundLoader;
	disable2d = game_state.disable2Dgraphics;
	game_state.disable2Dgraphics = 1;
	//set_scissor(0);
	//init_display_list();
	fov_custom = game_state.fov_custom; // I do fovMagic below, so don't need to set this
	game_mode = game_state.game_mode;
	oldCamInfo = cam_info; //copy off the old camera
	gfx_state.postprocessing = 0;
	gfx_state.renderscale = 0;
	gfx_state.renderToPBuffer = 0;
	fx_engine.no_expire_worldfx = 1;
	old_zocclusion = game_state.zocclusion;
	game_state.zocclusion = 0;
	sunPush();

	// Show the world
	toggle_3d_game_modes(SHOW_GAME);

	//Set up the camera
	copyMat3( unitmat, cam_info.mat );
	copyVec3( objPosition, cam_info.mat[3] );

	copyMat3( unitmat, cam_info.cammat );
	copyVec3( objPosition, cam_info.cammat[3] );

	{
		Vec3 rpy = {0.4, 0.4, 0};
		if (rotate) {
			static F32 value =0;
			static int last_update;
			if (last_update != client_frame_timestamp) {
				value+=TIMESTEP;
				last_update = client_frame_timestamp;
			}
			rpy[1]=value/80;;
		}
		if (lookFromBelow)
			rpy[0]*=-1;
		rotateMat3(rpy, cam_info.mat);
		rotateMat3(rpy, cam_info.cammat);
	}

	mulVecMat3(headshotCameraPos, cam_info.cammat, cameraPos); // Rotate position

	addVec3( cam_info.cammat[3], cameraPos, cam_info.cammat[3] );
	gfxSetViewMat(cam_info.cammat, cam_info.viewmat, NULL);
	camLockCamera( true );

	sunGlareDisable();
	rdrSetAdditiveAlphaWriteMask(1);

	// Queue up thingy to draw, start files loading
	setLightForCharacterEditor();
	scaleVec3(g_sun.ambient, 0.5, g_sun.ambient);
	scaleVec3(g_sun.diffuse, 1.0, g_sun.diffuse);
	mulVecMat3(g_sun.direction, unitmat, g_sun.direction_in_viewspace); 
	gfxWindowReshapeForHeadShot(35);
	gfxDrawSetupDrawing(NULL); // Send sun/lighting values

	gfxTreeFrameInit();
	for (i=0; i<num_defs; i++) {
		static U8 tint[2][4] = {{128, 128, 128, 255}, {128, 128, 128, 255}};
		bool had_alpha = defs[i]->has_alpha;
		defs[i]->has_alpha = 0;
		if (!defs[i]->has_tint_color)
			drawDefSimpleWithTrackers(defs[i], objMat, (Color*)tint, i * GROUP_UID_BOOST);
		else
			drawDefSimpleWithTrackers(defs[i], objMat, NULL, i * GROUP_UID_BOOST);
		defs[i]->has_alpha = had_alpha;
	}
	fxRunEngine();

	#if NOVODEX
		fxUpdateDebrisManager();
	#endif
	
	gfxTreeFrameFinishQueueing();

	sortModels(true, true);
	// Draw stuff to load, unless we know this isn't going to be cached
	if (!rotate) {
		rdrSetBgColor(bg_color);
		rdrClearScreenEx(CLEAR_DEFAULT|CLEAR_ALPHA);
		drawSortedModels(DSM_OPAQUE);
		drawSortedModels(DSM_ALPHA);
		partKickStartAll();
		partRunEngine(true);

		forceGeoLoaderToComplete();
		texForceTexLoaderToComplete(1);
		Sleep(5);
	}

	// Draw stuff a final time
	rdrSetBgColor(bg_color);
	rdrClearScreenEx(CLEAR_DEFAULT|CLEAR_ALPHA);
	drawSortedModels(DSM_OPAQUE);
	drawSortedModels(DSM_ALPHA);
	partRunEngine(true);

	// Restore stuff
	toggle_3d_game_modes(game_mode);
	game_state.game_mode = game_mode;
	game_state.disable2Dgraphics = disable2d;
	skyInvalidateFogVals(); // reset luminance, etc

	//set_scissor(1); // This sets the *wrong* scissor values
	cam_info = oldCamInfo; //also restored camera Func
	game_state.fov_custom = fov_custom;
	game_state.disableGeometryBackgroundLoader = heldBackgroundLoaderState;
	fx_engine.no_expire_worldfx = 0;
	game_state.zocclusion = old_zocclusion;
	rdrSetAdditiveAlphaWriteMask(0);
	sunPop();

	assert(bind);
	atlasUpdateTextureFromScreen( bind, ULx, ULy, sizeOfPictureX, sizeOfPictureY);
	//texGenUpdateFromScreen( bind, ULx, ULy, sizeOfPictureX, sizeOfPictureY);
	if (use_pbuffers)
		winMakeCurrent();

	gfxWindowReshape(); //Restore
}

static float defRadiusNoVolumes(GroupDef *def, const Mat4 mat, Vec3 min, Vec3 max )
{
	int i;
	Model *model;
	TrickNode *trick;
	float maxrad = 0.0;
	if (def->volume_trigger) 
		return 0.0;
	
 	model = def->model;
	if (model && !strEndsWith(model->name,"__PARTICLE"))
	{
		trick = model->trick;
		if(!(trick && (trick->flags1 & TRICK_NOCOLL)))
		{
			groupGetBoundsMinMaxNoVolumes(def,mat,min,max);
			maxrad = MAX(maxrad, def->radius);
		}
	}
	
	for (i = def->count-1; i >= 0; i--)
	{
		F32 newrad;
		Mat4	child_mat;
		mulMat4(mat,def->entries[i].mat,child_mat);
		newrad = defRadiusNoVolumes(def->entries[i].def, child_mat, min, max);
  		maxrad = MAX(maxrad,newrad);
	}

	return maxrad;
}

static getObjPosition(U32 hash, Vec3 pos)
{
	pos[0] += (hash&0xf) * 1000;
	pos[1] += ((hash>>4)&0xf) * 1000;
	pos[2] += ((hash>>8)&0xf) * 1000;
}

static AtlasTex *getImage(const char *groupDefNames, const Mat4 groupMat, AtlasTex *oldbind, bool rotate, bool lookFromBelow)
{
	int sizeOfPictureX = 256; //Size of shot on screen.
	int sizeOfPictureY = 256; //Powers of two!!! ARgh.
	Vec3 headshotCameraPos;
	AtlasTex * tex;
	GroupDef ** defs=NULL;
	char *temp = _alloca(strlen(groupDefNames)+1);
	char *str;
	F32 maxradius=0;
	Vec3 min = {8e8,8e8,8e8}, max = {-8e8,-8e8,-8e8};
	static U32 last_names_hash;
	U32 names_hash;
	Vec3 objPosition = {20000, 20000, 20000};

	names_hash = hashString(groupDefNames, false);
	if (names_hash != last_names_hash) {
		clearTrackerCache(0);
		last_names_hash = names_hash;
	}
	getObjPosition(names_hash, objPosition);

	strcpy(temp, groupDefNames);
	str = strsep(&temp,";,\t");

	// Setup MiniTrackers
	groupDrawPushMiniTrackers();

	while (str)
	{
		GroupDef *def = groupDefFindWithLoad(str);
		if (def) 
		{
			// Setup MiniTrackers
			groupDrawBuildMiniTrackersForDef(def, eaSize(&defs)*GROUP_UID_BOOST);

			eaPush(&defs, def);
			maxradius = MAX(maxradius, defRadiusNoVolumes(def, unitmat, min, max));
		}
		str = strsep(&temp,";,\t");
	}

   	maxradius = MAX( maxradius, distance3(min,max)/2 );

	if (!eaSize(&defs))
		return atlasLoadTexture("ember"); // invisible_tex_atlas;

	if (!oldbind) {
		tex = atlasGenTexture(NULL, sizeOfPictureX, sizeOfPictureY, PIX_RGBA|PIX_DONTATLAS|PIX_NO_VFLIP, "GroupThumbnail");
	} else {
		tex = oldbind;
	}

	headshotCameraPos[0] = 0.0;
	headshotCameraPos[1] = 0; // def->radius/8.0;
	headshotCameraPos[2] = 15.0*maxradius;

	// Render into a texture
	renderGroupDef(headshotCameraPos, defs, groupMat, rotate, tex, sizeOfPictureX, sizeOfPictureY, lookFromBelow, objPosition);

	groupDrawPopMiniTrackers();

	return tex;
}


StashTable groupThumbnailHT;
StashTable groupThumbnailRotateHT;
AtlasTex* groupThumbnailGet( const char *groupDefNames, const Mat4 groupMat, bool rotate, bool lookFromBelow, int *not_cached )
{
	static AtlasTex *last_rotated_bind=NULL;
	static int last_rotated_bind_timestamp=0;
	AtlasTex* bind;

	if (!rdr_caps.use_pbuffers)
		rotate = false;

	if( !groupThumbnailHT )
	{
		groupThumbnailHT = stashTableCreateWithStringKeys(16, StashDeepCopyKeys);
		groupThumbnailRotateHT = stashTableCreateWithStringKeys(16, StashDeepCopyKeys);
	}
	if (rotate) {
		stashFindPointer( groupThumbnailRotateHT, groupDefNames, &bind );
	} else {
		stashFindPointer( groupThumbnailHT, groupDefNames, &bind );
	}

	if (rotate && bind &&
		bind == last_rotated_bind &&
		last_rotated_bind_timestamp == client_frame_timestamp)
	{
		// Already updated this frame
		return bind;
	}

	if (!bind || rotate) {
		// Need to create it!
		bind = getImage(groupDefNames, groupMat, bind, rotate, lookFromBelow);
		if (rotate) {
			stashAddPointer(groupThumbnailRotateHT, groupDefNames, bind, true);
		} else {
			stashAddPointer(groupThumbnailHT, groupDefNames, bind, true);
		}

		if( not_cached )
			*not_cached = true;
	}

	if (rotate) {
		last_rotated_bind = bind;
		last_rotated_bind_timestamp = client_frame_timestamp;
	}

	return bind;
}


