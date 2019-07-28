#include "entity.h"
#include "uiGame.h"
#include "uiKeybind.h"
#include "edit_cmd.h"
#include "cmdcommon.h"
#include "cmdgame.h"
#include "AppVersion.h"
#include "AppLocale.h"
#include "timing.h"
#include "boost.h"
#include "authUserData.h"
#include "uiChatUtil.h"
#include "entVarUpdate.h"
#include "motion.h"
#include "truetype/ttFontDraw.h"
#include "grouputil.h"
#include "groupdraw.h"
#include "groupscene.h"
#include "sun.h"
#include "grid.h"
#include "gridcoll.h"
#include "font.h"
#include "model_cache.h"
#include "utils.h"
#include "tricks.h"
#include "cmdcontrols.h"
#include "clientcomm.h"
#include "uiCursor.h"
#include "tex.h"
#include "costume_data.h"
#include "powers.h"
#include "textparser.h"
#include "player.h"
#include "costume_client.h"
#include "varutils.h"
#include "entclient.h"
#include "origins.h"
#include "classes.h"
#include "character_base.h"
#include "input.h"
#include "sprite_base.h"
#include "uiUtil.h"
#include "win_cursor.h"
#include "uiInput.h"
#include "arenastruct.h"
#include "win_init.h"
#include "costume_data.h"
#include "entPlayer.h"
#include "uiCostume.h"
#include "gfx.h"
#include "uiGender.h"
#include "fx.h"
#include "uiLoadCostume.h"
#include "uiAvatar.h"
#include "groupfileload.h"
#include "uiUtilGame.h"
#include "wdwbase.h"
#include "sprite_font.h"
#include "sprite_text.h"
#include "ttFontUtil.h"
#include "gridfind.h"
#include "textureatlas.h"
#include "rt_water.h"
#include "dbclient.h"
#include "camera.h"
#include "uiBaseInput.h"
#include "uiEdit.h"
#include "demo.h"
#include "uiNet.h"
#include "input.h"
#include "bases.h"
#include "edit_select.h"
#include "NwWrapper.h"

#define SPLASH_TIME 0.f
#define KOREAN_SPLASH_TIME 10.0f

static char *cc_spawn_url = 0;

extern int gE3CostumeCreator;
extern int gKoreanCostumeCreator;
extern int gLoadRandomPresetCostume;

void splashMenu()
{
	static int tim = -1;
	KeyInput *input;
	U8	buf[64];
	AtlasTex * splash_tex = atlasLoadTexture("creator_ending_kr.tga");
	int x, y;


	toggle_3d_game_modes(SHOW_NONE);

	if (green_blobby_fx_id)
	{
		fxDelete(green_blobby_fx_id, HARD_KILL);
		green_blobby_fx_id = 0;
	}

	if (tim < 0)
	{
		tim = timerAlloc();
		timerStart(tim);
	}

#define SPAWN_URL_AND_EXIT {if(cc_spawn_url)ShellExecute(0, "open", cc_spawn_url, 0, 0, 0);windowExit();}


	if (timerElapsed(tim) > (gKoreanCostumeCreator ? KOREAN_SPLASH_TIME : SPLASH_TIME) )
		SPAWN_URL_AND_EXIT;

	for (input = inpGetKeyBuf(buf); input; inpGetNextKey(&input))
	{
		if(input->key)
			SPAWN_URL_AND_EXIT;
	}

	if ( mouseButtonEvent(MS_LEFT, MS_DOWN, &x, &y) || mouseButtonEvent(MS_RIGHT, MS_DOWN, &x, &y) )
		SPAWN_URL_AND_EXIT;

	display_sprite( splash_tex, 0, 0, 2, 1.f, 1.f,  0xffffffff );

	//font_color( CLR_YELLOW, CLR_ORANGE );
	//prnt( 200, 150, 30, 6.f, 4.f, "PLACEHOLDER" );
	//prnt( 200, 350, 30, 6.f, 4.f, "Insert AD here." );
}

void resetGenderUI();

int drawCCMainButon(float cx, float cy, float z, float wd, float ht, char *txt)
{
	int okselect = 0, retval = 0;
	Cbox box;
	int backcolor = 0x00000088;
	int txtclr = 0xffffffff;

	font(&game_14);
	font_color(txtclr, txtclr);


	BuildCbox( &box, cx - wd/2, cy - ht/2, wd, ht );

	if (mouseCollision(&box))
	{
		if (isDown(MS_LEFT))
		{
			backcolor = CLR_CONSTRUCT_EX(123, 252, 164, 1.0);
			retval = 1;
		}
		else
			backcolor = CLR_CONSTRUCT_EX(61, 126, 82, 1.0);
	}

	drawFlatFrame( PIX3, R10, box.lx, box.ly, z, box.hx-box.lx, box.hy-box.ly, 1.f, CLR_BLUE, backcolor );

	if( txt )
	{
		float txtSc = 1;

		// scale text to fit within button
		if( str_wd( font_grp, txtSc, txtSc, txt ) > wd-5 )
			txtSc = str_sc( font_grp, wd-5, txt );

		cprntEx(cx, cy, z+10, txtSc, txtSc, (CENTER_X | CENTER_Y), txt );		
	}

	//return retval;
	if( mouseUpHit( &box, MS_LEFT ) )
		return TRUE;
	else
		return FALSE;
}

void ccMainMenu()
{
	static int init = 0;
	Entity *e2 = playerPtrForShell(1);
	Entity *e = playerPtr();
	AtlasTex * menu_tex;
	gLoadRandomPresetCostume = 1;

	if (!init)
	{
		init = 1;

		resetGenderUI();

		costume_init(e->costume);
		costume_destroy(e->costume);
		e->pl->costume[0] = costume_create(bodyPartCount);
		entSetMutableCostumeUnowned(e, e->pl->costume[0]);	// e->costume = e->pl->costume[0] = costume_create(bodyPartCount);
		e->pl->capesUnlocked = 1;
		e->pl->glowieUnlocked = 1;
		e2->pl->capesUnlocked = 1;
		e2->pl->glowieUnlocked = 1;
		setGender( e, "male" );
		setGender( e2, "male" );
		gCurrentGender = AVATAR_GS_MALE;
		e->costume->appearance.bodytype = kBodyType_Male;
		e2->costume->appearance.bodytype = kBodyType_Male;

		toggle_3d_game_modes(SHOW_NONE);
		resetCostume(1);
	}

	if (gE3CostumeCreator)
	{
		start_menu(MENU_COSTUME);
		return;
	}

	menu_tex = atlasLoadTexture("creator_intro_kr.tga");
	display_sprite( menu_tex, 0, 0, 2, 1.f, 1.f,  0xffffffff );

	toggle_3d_game_modes(SHOW_NONE);

	if (drawCCMainButon(670, 470, 5000, 200, 40, "CreateNewCostumeString"))
	{
		start_menu(MENU_GENDER);
		init = 0;
	}

	if (drawCCMainButon(670, 515, 5000, 200, 40, "LoadExistingCostumeString"))
	{
		loadCostume_start(MENU_CCMAIN);
		init = 0;
	}

	if (drawCCMainButon(670, 560, 5000, 200, 40, "BuyCityOfHeroesString"))
	{
		cc_spawn_url = "http://www.coh.co.kr/event/characterEvt/event.asp ";
		start_menu(MENU_SPLASH);
		init = 0;
	}

	if (drawCCMainButon(670, 610, 5000, 200, 40, "ExitCostumeCreatorString"))
	{
		start_menu(MENU_SPLASH);
		//menu_tex = atlasLoadTexture("background_skyline_alphacut_tintable.tga");
		//display_sprite( menu_tex, 0, 0, 2, 1.f, 1.f,  0xffffffff );
		init = 0;
	}
}


////////////////////////////////////////////////////////////////////////////////////////

void commSendQuitGame(int abort_quit)
{	
	start_menu(MENU_SPLASH);
}


void costume_setCurrentBodyType( Entity *e )
{
	char * genderName[] = { "Male", "Female", "BasicMale", "BasicFemale", "Huge", "Villain" };
	int i, j;

	// set the bodyType index and origin index
	for( i = 0; i < EArrayGetSize( &gCostumeMaster.bodySet ); i++ )
	{
		if ( stricmp( gCostumeMaster.bodySet[i]->name, genderName[e->pl->costume[0]->appearance.bodytype] ) == 0 )
		{
			gCurrentBodyType = i;

			for( j = 0; j < EArrayGetSize( &gCostumeMaster.bodySet[i]->originSet ); j++ )
			{
				if ( stricmp( gCostumeMaster.bodySet[i]->originSet[j]->name, "All" ) == 0 )//e->pchar->porigin->pchDisplayName ) == 0 )
				{
					gCurrentOriginSet = j;
					break;
				}
			}
			break;
		}
	}
}

void sendTailoredCostume( Costume * costume ) {}

int okToInput()
{
	if(shell_menu())
		return 0;
	else
		return 1;
}

F32     auto_delay = 0.4;
U32 last_pkt_id;
int gSelectedDBID = 0;
char gSelectedName[64] = {0};
char gSelectedHandle[64] = {0};

int commDisconnect() {return 1;}
void authLogout() {}
void demoStop() {}
int demoIsPlaying() {return 0;}
DemoState demo_state;
void demoFreeEntityRecordInfo(Entity* e) {}

void printDebugString(char* outputString,
					  int x,
					  int y,
					  float scale,
					  int lineHeight,
					  int overridecolor,
					  int defaultcolor,
					  U8 alpha,
					  int* getWidthHeightBuffer)
{}
void printDebugStringAlign(char* outputString,
						   int x,
						   int y,
						   int dx,
						   int dy,
						   int align,
						   float scale,
						   int lineHeight,
						   int overridecolor,
						   int defaultcolor,
						   U8 alpha)
{}

int editMode() {return 0;}
void showGridExternal() {;}

EditState	edit_state;
GlobalState global_state;
GameState game_state;
ControlState control_state = {0};
ServerVisibleState server_visible_state;
SunLight sun;
Packet*	client_input_pak;

char g_achAccountName[32] = "";
char g_achPassword[32] = "";
int g_iDontSaveName = 0;

ServerControlState server_state = {0};

int client_frame_timestamp;

void gameStateInit()
{
	memset(&game_state,0,sizeof(game_state));
	game_state.screen_x = 800;// + 8;
	game_state.screen_y = 600;// + 25;
	game_state.fullscreen = 0;
	game_state.fov_1st = 55;
	game_state.fov_3rd = 55;
	game_state.fov_custom = 0;
	// MS: This seems to be approximately the biggest nearz can be without clipping in first-person mode.
	game_state.nearz = 0.73;
	game_state.farz = 20000.f;
	game_state.scene[0] = 0;
	game_state.maxfps = 0;
	game_state.port = //DEFAULT_PORT_UDP;
	game_state.showhelp = 2;
	game_state.vis_scale = 1;
	game_state.showfps = 0;
	game_state.camdist = 10;
	game_state.minimized = 0;
	setCurrentLocale(locGetIDInRegistry());

	game_state.mipLevel = 0;
	game_state.reduce_min = 16;
	game_state.ortho_zoom = 1000;

	game_state.shadowvol = 0; //disabled for now
	game_state.LODBias = 1.0;
	game_state.bShowPointers = false;

	game_state.stopInactiveDisplay = 0;

	strcpy(game_state.sound_bank_fname,"audioinfo");
	strcpy(game_state.address,"127.0.0.1");
	strcpy(game_state.scene,"scene.txt");

	client_frame_timestamp = game_state.client_frame_timestamp = timerCpuTicks();

	game_state.enableJoystick = true;
	game_state.reverseMouseButtons = false;

	game_state.fixArmReg = 1;
	game_state.fixLegReg = 1;

	game_state.ctm_autorun_timer = timerAlloc();
	TIMESTEP = 1;

	control_state.server_state = &server_state;
}

int cmdParse(char *str)
{
	return 0;
}

void cmdOldInit(Cmd *cmds) {}
Cmd *cmdOldRead(CmdList *cmd_list,char *cmdstr,int access_level,CmdOutput* output) { return NULL; }
int cmdAccessLevel() { return 0; }

void pmotionRecordStateBeforePhysics(Entity* e, const char* trackName, ControlStateChange* csc) {;}
void pmotionRecordStateAfterPhysics(Entity* e, const char* trackName) {;}
void pmotionReceivePhysicsRecording(Packet* pak, Entity* e) {;}

void clearQueuedDeletes() {}

void entDebugAddText(Vec3 pos, const char* textString, int flags) {}
void entDebugAddLine(Vec3 p1, int argb1, Vec3 p2, int argb2) {}

void boost_Destroy(Boost *pboost) {}
Boost *boost_Create(BasePower *ppowBase, int idx, Power *ppowParent, int iLevel, int iNumCombines, float const *afVars, int numVars, PowerupSlot const *aPowerupSlots, int numPowerupSlots ) { return 0; }

bool authUserHasDVDSpecialEdition(U32 *data) { return 0; }
PreorderCode authUserPreorderCode(U32 *data) { return PREORDER_NONE; }
bool authUserHasVillainAccess(U32 *data) {return 0;}

void addChatMsg(char *s, int type, int id) {}
void addToChatMsgs( char *txt, INFO_BOX_MSGS msg, ChatFilter * filter) {}

void entMotionUpdateCollGrid(Entity *e) {}
void entMotionFreeColl(Entity *e) {}

MotionState mstate = {0};
MotionState *createMotionState() { return &mstate; }
void destroyMotionState(MotionState* motion) {}

void getCapsule( SeqInst * seq, EntityCapsule * cap )
{
	Vec3 bounds;
	F32 l, r;

	seqGetCollisionSize( seq, bounds, cap->offset );

	if ( bounds[0] > bounds[1] )
	{
		if ( bounds[0] > bounds[2] )
		{
			l = bounds[0];
			cap->dir = 0;

			if ( bounds[1] > bounds[2] )
				r = bounds[1];
			else
				r = bounds[2];
		}
		else
		{
			l = bounds[2];
			r = bounds[0];
			cap->dir = 2; // this may get screwy and need to be centered
		}
	}
	else
	{
		if ( bounds[1] > bounds[2] )
		{
			l = bounds[1];
			cap->dir = 1;

			if ( bounds[0] > bounds[2] )
				r = bounds[0];
			else
				r = bounds[2];
		}
		else
		{
			l = bounds[2];
			r = bounds[1];
			cap->dir = 2;
		}
	}

	cap->length = l - r;
	cap->radius = r/2;
	cap->has_offset = !vecIsZero( cap->offset );

	cap->max_coll_dist = lengthVec3(cap->offset) +
		cap->radius +
		cap->length;

	if(	cap->dir )
		cap->max_coll_dist += cap->radius;

}

void powerInfo_UpdateTimers(PowerInfo* info, float elapsedTime) {}

float boost_CombineChance(Boost *pboostA, Boost *pboostB) {return 0;}

Entity *current_target = 0;

void pmotionCheckVolumeTrigger(Entity *e) {}
int pmotionGetNeighborhoodPropertiesInternal(DefTracker *neighborhood,char **namep,char **musicp) {return 0;}

void buildPowerTrayObj(TrayObj *pto, int iset, int ipow, int autoPower ) {}
void buildMacroTrayObj( TrayObj *pto, char * command, char *shortName, char *iconName, int hideName ) {}

Packet *input_pak = 0;
int		glob_have_camera_pos;

int dbChoosePlayer(int slot_number,U32 local_map_ip, int start_map_id, F32 timeout) {return 0;}

void sendCharacterToServer(	Packet *pak, Entity	*e ) {}

int groupDynId(DefTracker *tracker)
{
	return (int)tracker->dyn_group;
}

int isEntitySelectable(Entity *e) {return 0;}

int randInt2(int max) {
	return (rand()*max-1)/RAND_MAX;
}

void sendThirdPerson() {}

void commGetData() {}
void drawHealthBar( float x, float y, float z, float width, float curr, float total, float scale, int color, int type ) {}

void entMotionDrawDebugCollisionTrackers() {}
void drawStuffOnEntities() {}
void DoorAnimCheckFade() {}
void displayEntDebugInfo() {}

static int force_fade = 0;
int startFadeInScreen(int set)
{
	int force = force_fade;
	force_fade = 0;
	if (set) {
		force = force_fade = 1;
	}
	return force;
}

void selDraw() {}
void BugReport(const char * desc, int mode) {}

void texWordsEdit_reloadCallback(void) {}

void drawSphere(Mat4 mat,F32 rad,int nsegs,U32 color) {}

DefTracker	*mouseover_tracker = 0;

void clearSetParentAndActiveLayer() {}
void entMotionFreeCollAll() {}

GroupInfo group_info = {0};
GroupDraw group_draw = {0};

SceneInfo scene_info;
void groupDrawRefs(Mat4 cam_mat, Mat4 inv_cam_mat) {}


int groupDynDraw(int world_id,Mat4 view_xform) {return 0;}
void groupDrawDefTracker( GroupDef *def,DefTracker *tracker,Mat4 matx, EntLight * light, FxMiniTracker * fxminitracker, F32 viewScale, DefTracker *lighttracker ) {}

Line	lightlines[1000];
int		lightline_count;

void getDefColorVec3(GroupDef *def,int which,Vec3 rgb_out)
{
	Vec3	rgb;

	scaleVec3(def->color[which],1.f/63,rgb);
	sceneAdjustColor(rgb,rgb_out);
	scaleVec3(rgb_out,0.25,rgb_out);
}

void getDefColorU8(GroupDef *def,int which,U8 color[3])
{
	Vec3	v;

	getDefColorVec3(def,which,v);
	scaleVec3(v,255,color);
}

void lightModel(Model *model,Mat4 mat,U8 *rgb,DefTracker *light_trkr)
{
	int			is_light=0,i,j,count=0,c;
	Vec3		v,nv,color,dv,base_pos = {-8e16,-8e16,-8e16,},ambient;
	DefTracker	*tracker,*lights[1000];
	GroupDef	*def;
	F32			falloff,dist=0,angle,intensity,*scene_amb;
	GridCell	*cell;
	Grid		*light_grid;
	U8			maxbright[4],*base=0;
	static Vec3	*verts,*norms;
	static int	vert_max,norm_max;

	assert(rgb && light_trkr);

	light_grid = light_trkr->light_grid;
	if (!light_grid || !model->pack.verts.data || !model->pack.norms.data)
		return;

	dynArrayFit((void**)&verts,sizeof(Vec3),&vert_max,model->vert_count);
	dynArrayFit((void**)&norms,sizeof(Vec3),&norm_max,model->vert_count);

	if (model->unpack.verts)
		memcpy(verts, model->unpack.verts, model->vert_count * 3 * sizeof(F32));
	else
		geoUnpackDeltas(&model->pack.verts,verts,3,model->vert_count,PACK_F32,model->name,model->filename);

	if (model->unpack.norms)
		memcpy(norms, model->unpack.norms, model->vert_count * 3 * sizeof(F32));
	else
		geoUnpackDeltas(&model->pack.norms,norms,3,model->vert_count,PACK_F32,model->name,model->filename);

	getDefColorVec3(light_trkr->def,0,ambient);
	scene_amb = sceneGetAmbient();
	if (scene_amb)
		scaleVec3(scene_amb,.25,ambient);

	copyVec3(light_trkr->def->color[1],maxbright);
	if ((maxbright[0]|maxbright[1]|maxbright[2]) == 0)
		maxbright[0] = maxbright[1] = maxbright[2] = 255;
	else
		getDefColorU8(light_trkr->def,1,maxbright);

	if (strstri(model->name,"omni"))
		is_light = 1;
	for(i=0;i<model->vert_count;i++)
	{
		mulVecMat4(verts[i],mat,v);
		copyVec3(ambient,color);
		// if vert is within same light grid as prev vert, dont call gridFindCell
		for(j=0;j<3;j++)
		{
			falloff = v[j] - base_pos[j];
			if (falloff >= dist || falloff < 0)
				break;
		}
		if (j < 3)
		{
			++light_grid->tag;
			cell = gridFindCell(light_grid,v,(void *)lights,&count,&dist);
			if (dist > 128) // wtf?
				dist = 128;
			for(j=0;j<3;j++)
				base_pos[j] = qtrunc(v[j]) & ~(qtrunc(dist)-1);
		}
		for(j=0;j<count;j++)
		{
			F32 dist;

			tracker	= lights[j];
			def		= tracker->def;
			if (!def || def->key_light)
				continue;
			subVec3(v,tracker->mat[3],dv);
			dist = normalVec3(dv);
			if (is_light && dist > 3.f)
				continue;
			if (dist > def->light_radius)
				continue;

			mulVecMat3(norms[i],mat,nv);
			angle = -dotVec3(dv,nv);
			if (model->flags & OBJ_NOLIGHTANGLE)
				angle = 1.f;
			if (angle < 0)
				continue;

			falloff = def->light_radius * 0.333f;
			if (dist < falloff)
				intensity = 1;
			else
			{
				intensity = (def->light_radius - dist) / (def->light_radius - falloff);
				intensity = intensity * intensity;
			}
			intensity *= angle;
			if (intensity < 0.05)
				continue;

			getDefColorVec3(def,0,dv);
			scaleVec3(dv,intensity,dv);
			if (def->color[0][3] & 1)  // negative light
				subVec3(color,dv,color);
			else
				addVec3(color,dv,color);
		}
		for(j=0;j<3;j++)
		{
			c = color[j] * 255;
			if (c > maxbright[j])
				c = maxbright[j];
			if (c < 0)
				c = 0;
			*rgb++ = c;
		}
		*rgb++ = 255;
	}
}

#define DISTANCE_TO_MOVE_BEFORE_RECALCULATING_LIGHT 2.0
static int no_light_interp = 0; //debug

void lightEnt(EntLight *light,Vec3 pos, F32 min_base_ambient, F32 max_ambient )
{
	DefTracker	*tracker;
	GridCell	*cell;
	DefTracker	*lights[1000];
	F32			dist;
	int			j,count;
	GroupDef	*def;
	Vec3		dv;
	Grid		*light_grid;
	Vec3		color;

	Vec3	diffuse_dir;
	Vec3	one_light_dir;
	F32		total_intensity;
	F32		len_diffuse_dir;
	F32		part_diffuse;
	Vec3	diffuse_added;
	Vec3	ambient_added;
	Vec3	gross_diffuse_dir;
	//	Line * line;
#define LIGHTINFO 0
	//extern int camera_is_inside;

	if(	light->hasBeenLitAtLeastOnce )
		no_light_interp = 0; //debug
	else
		no_light_interp = 1;

	light->hasBeenLitAtLeastOnce = 1;	

	//IF anyone wnats to custom set the ambinet or diffuse sclae, they have to do it every frame
	/*
	light->use &= ~ENTLIGHT_CUSTOM_AMBIENT;
	light->use &= ~ENTLIGHT_CUSTOM_DIFFUSE;
	light->ambientScale = 1.0;
	light->diffuseScale = 1.0;
	light->target_ambientScale;
	light->target_diffuseScale;
	light->ambientScaleUp;
	light->ambientScale
	*/
	light->ambientScale = 1.0;	  
	light->diffuseScale = 1.0; 
	if( light->use & ( ENTLIGHT_CUSTOM_AMBIENT | ENTLIGHT_CUSTOM_DIFFUSE ) )
	{
		F32 scale;
		F32 pulseEndTime = light->pulseEndTime;
		F32 pulsePeakTime = light->pulsePeakTime;
		F32	pulseBrightness = light->pulseBrightness;

		light->pulseTime += TIMESTEP;

		//AS long as you are repeatedly told by the fx to hold your pulse value, hold it.
		if( light->pulseClamp && light->pulseTime > pulsePeakTime ) 
		{
			light->pulseTime = pulsePeakTime;
			light->pulseClamp -= TIMESTEP;
			light->pulseClamp = MAX( 0, light->pulseClamp );
		}

		if( light->pulseTime > pulseEndTime )     
		{
			light->use &= ~ENTLIGHT_CUSTOM_AMBIENT;
			light->use &= ~ENTLIGHT_CUSTOM_DIFFUSE;
			light->ambientScale = 1.0;
			light->diffuseScale = 1.0;
			light->pulseTime = 0;
			scale = 0.0;
		}
		else if( light->pulseTime > pulsePeakTime ) 
		{
			scale = ( 1.0 - ( (light->pulseTime - pulsePeakTime) / (pulseEndTime - pulsePeakTime ) ) );
		}
		else 
		{
			scale = ( light->pulseTime / pulsePeakTime );
		}

		scale = 1 + (scale * (pulseBrightness - 1.0) );

		//xyprintf( 20, 20, "Light %f at age %f ", scale, light->pulseTime);

		//if( light->pulseTime > pulsePeakTime && scale2 > light->pulseClamp )
		//	light->pulseTime = pulsePeakTime * scale;

		light->ambientScale = scale;
		light->diffuseScale = scale; 

		//light->ambientScale = 1.0;
		//light->diffuseScale = 1.0;
	}

	//xyprintf( 22, 21, "ambientScale %f", light->ambientScale); 
	//xyprintf( 22, 22, "diffuseScale %f", light->diffuseScale); 

	//Only update special lighting when you've moved more than 2 feet from the last pos at which you updated light
	subVec3(pos,light->calc_pos,dv);   

	if ( lengthVec3Squared(dv) > DISTANCE_TO_MOVE_BEFORE_RECALCULATING_LIGHT*DISTANCE_TO_MOVE_BEFORE_RECALCULATING_LIGHT ) //4.0    
	{
		copyVec3(pos,light->calc_pos);

		//What is the player inside of?  Only use special lighting if it has special lighting
		tracker = groupFindInside( pos, FINDINSIDE_TRAYONLY,0,0 ); //find the group you are inside right now
		if (!tracker || !tracker->light_grid)
		{
			//this tracker hasn't been processed yet, so dont give up on it for lighting
			//slightly hacky way to say this light still needs to be processed even though it hasn't moved lately
			if( tracker && tracker->def && tracker->def->has_ambient )
				light->calc_pos[1] = -1000;  

			light->use &= ~ENTLIGHT_INDOOR_LIGHT;
			return;
		}
		light->use |= ENTLIGHT_INDOOR_LIGHT; 

		//Get Base Ambient Light (from the group tracker) (can be augmented by dispersed diffuse lights)
		getDefColorVec3(tracker->def,0,light->tgt_ambient);
		scaleVec3(light->tgt_ambient,155.f/255.f,light->tgt_ambient);
		//scaleVec3(tracker->def->color[0],(1.f/155.f),light->tgt_ambient); 
		for(j=0;j<3;j++) //mm 
			light->tgt_ambient[j] = MAX( light->tgt_ambient[j], min_base_ambient );
		//Get Diffuse Light(get all the lights around you and add their influences based on distance to a single color)
		//(Where is the direction of the diffuse light figured out?)
		light_grid = tracker->light_grid;
		light_grid->tag++;
		count = 0;
		cell = gridFindCell(light_grid,pos,(void *)lights,&count,&dist);

		zeroVec3(color);
		zeroVec3(diffuse_dir);
		diffuse_dir[1] = 0.001; //so if there are no ligths, the direction will be up and left
		diffuse_dir[0] = 0.001; // 

		total_intensity = 0;

		for(j=0;j<count;j++)
		{
			F32	falloff,intensity;

			def	= lights[j]->def;

			subVec3(lights[j]->mat[3],pos,one_light_dir);
			dist = normalVec3(one_light_dir);
			if (dist >= def->light_radius)
				continue;

			falloff = def->light_radius * 0.333f;
			if (dist < falloff)
			{
				intensity = 1;
			}
			else 
			{
				intensity = (def->light_radius - dist) / (def->light_radius - falloff);
				intensity = intensity * intensity;
			}
			intensity *= 1.f / 255.f;

			if (def->key_light)
			{
				//Find diffuse_dir(*= -1 ?)
				scaleVec3(one_light_dir, intensity * 1000 , one_light_dir); //*1000 to make it work
				addVec3( diffuse_dir, one_light_dir, diffuse_dir  );
			}
			else
			{
				getDefColorVec3(def,0,dv);
				scaleVec3(dv,intensity * 255,dv);
				//scaleVec3(def->color[0],intensity,dv);
				if ( def->color[0][3] )  // negative light
					subVec3(color,dv,color);
				else
					addVec3(color,dv,color);
				total_intensity += intensity; 
			}
		}

		////////////////////////////////////////////////////////
		copyVec3(diffuse_dir, gross_diffuse_dir); //rem gross diffuse dir for debug
		len_diffuse_dir = normalVec3(diffuse_dir); 

		part_diffuse = MIN( 1.0, (len_diffuse_dir / (total_intensity * 0.5)) ); 

		scaleVec3(color, part_diffuse,     diffuse_added );    
		scaleVec3(color, ((1.0 - part_diffuse)/2.0), ambient_added ); 

		copyVec3(diffuse_added,light->tgt_diffuse);

		addVec3(light->tgt_ambient, ambient_added, light->tgt_ambient );  

		for(j=0;j<3;j++) //mm 
			light->tgt_ambient[j] = MIN( light->tgt_ambient[j],max_ambient );

		copyVec3( diffuse_dir, light->tgt_direction ); 
		////////////////////////////////////////////////////////

		light->diffuse[3] = 1.0; 
		light->ambient[3] = 1.0;
		light->direction[3] = 0.0;

		light->interp_rate = 7.0; //magic number, how many frames to interpolate the new light over
	}

	if( light->interp_rate || no_light_interp)
	{
		F32 percent_new; 

		light->interp_rate -= TIMESTEP;

		if( light->interp_rate <= 0.0 || no_light_interp)
		{
			light->interp_rate = 0.0;
			copyVec3(light->tgt_ambient, light->ambient);
			copyVec3(light->tgt_diffuse, light->diffuse);
			copyVec3(light->tgt_direction, light->direction);
		}
		else
		{
			//interpolate between the tgt_and current.
			percent_new = 1 / (1 + light->interp_rate);//true linear

			posInterp(percent_new, light->ambient,  light->tgt_ambient,   light->ambient);
			posInterp(percent_new, light->diffuse,  light->tgt_diffuse,   light->diffuse);
			posInterp(percent_new, light->direction,light->tgt_direction, light->direction);
			normalVec3(light->direction);
		}
		copyVec3(pos,light->calc_pos);
	}


	//#if LIGHTINFO
	if( game_state.fxdebug & FX_CHECK_LIGHTS ){
		Vec3 dbig; 
		F32 len;

		Line * line;

		total_intensity = 1.0;

		//Draw line in direction of diffuse light, it's length is the intensity of the diffuse light
		line = &(lightlines[lightline_count++]); 
		len = normalVec3(light->direction); 
		scaleVec3( light->direction, -5 * total_intensity, dbig);  
		subVec3(pos, dbig, line->a); 
		copyVec3(pos, line->b);
		line->color[0] = line->color[1] = 100;
		line->color[2] = line->color[3] = 255;

		//Draw line of all lights added up
		//line = &(lightlines[lightline_count++]);

		//scaleVec3( gross_diffuse_dir, -5 , dbig);  
		//subVec3(pos, dbig, line->a); 
		//copyVec3(pos, line->b);
		//line->color[0] = line->color[1] = 255;
		//line->color[2] = line->color[3] = 100;

		//line->a[2] += .2;
		//line->b[2] += .2;

		xyprintf(5,16,"blue: total diffuse intensity");  
		xyprintf(5,15,"red: combined lights.%%<.5 blue=ambient");

		xyprintf(5,19,"light->tgt_ambient  : %d %d %d",(int)(light->tgt_ambient[0] * 255),(int)(light->tgt_ambient[1] * 255),(int)(light->tgt_ambient[2] * 255));
		xyprintf(5,20,"light->ambient      : %d %d %d",(int)(light->ambient[0] * 255),(int)(light->ambient[1] * 255),(int)(light->ambient[2] * 255));
		xyprintf(5,21,"light->tgt_diffuse  : %d %d %d",(int)(light->tgt_diffuse[0] * 255),(int)(light->tgt_diffuse[1] * 255),(int)(light->tgt_diffuse[2] * 255));
		xyprintf(5,22,"light->diffuse      : %d %d %d",(int)(light->diffuse[0] * 255),(int)(light->diffuse[1] * 255),(int)(light->diffuse[2] * 255));
		xyprintf(5,23,"light->tgt_direction: %d %d %d",(int)(light->tgt_direction[0] * 255),(int)(light->tgt_direction[1] * 255),(int)(light->tgt_direction[2] * 255));
		xyprintf(5,24,"light->direction    : %d %d %d",(int)(light->direction[0] * 255),(int)(light->direction[1] * 255),(int)(light->direction[2] * 255));
		xyprintf(5,25,"light->interp_rate  : %2.2f", light->interp_rate); 
		//xyprintf(5,26,"part_diffuse        : %2.2f", part_diffuse); 
		xyprintf(5,18,"%f %f %f",pos[0],pos[1],pos[2]);
	}
	//#endif 

}

static int reapply_color_swaps;

void lightReapplyColorSwaps()
{
	reapply_color_swaps = 1;
}

void lightCheckReapplyColorSwaps()
{
	int		i;

	if (!reapply_color_swaps)
		return;
	reapply_color_swaps = 0;
	for(i=0;i<entities_max;i++)
	{
		if (entity_state[i] & ENTITY_IN_USE )
		{
			setVec3(entities[i]->seq->seqGfxData.light.calc_pos,8e10,8e10,8e10);
		}
	}
}


void lightTracker(DefTracker *tracker) {}

void groupResetAllTrickFlags() {}
void InfoBoxDbg(INFO_BOX_MSGS msg_type, char *txt, ...){}

void GetTextColorForType(int type, int rgba[4])
{
	rgba[0] = 0xffffffff;
	rgba[1] = 0xffffffff;
	rgba[2] = 0xffffffff;
	rgba[3] = 0xffffffff;
}

void GetTextStyleForType(int type, TTFontRenderParams *pparams)
{
	pparams->italicize         = 0;
	pparams->bold              = 1;
	pparams->outline           = 0;
	pparams->dropShadow        = 0;
	pparams->softShadow        = 0;
	pparams->outlineWidth      = 0;
	pparams->dropShadowXOffset = 0;
	pparams->dropShadowYOffset = 0;
	pparams->softShadowSpread  = 0;
}

void DoorAnimCheckForcedMove() {}
void compass_UpdateWaypoints() {}
void playerStopFollowMode() {}
FuturePush* addFuturePush(ControlState* controls) { return 0; }

void respecMenu() {}
void superRegMenu() {}
void combineSpecMenu() {}
void specMenu() {}
void levelPowerMenu() {}
void optionsMenu() {}
void powersMenu() {}
void originClassMenu() {}
void loginMenu() {}

void receiveCharacterFromServer( Packet *pak, Entity *e ) {}
void destroyControlStateChangeList(ControlStateChangeList* list) {}
void destroyFuturePushList(ControlState* controls) {}
int DoorAnimIgnorePositionUpdates() { return 1; }
void addControlLogEntry(ControlState* controls) {}
void updateControlState(ControlId id, MovementInputSet set, int state, int time_stamp) {}
void pmotionSetState(Entity* e, ControlState* controls) {}
void playerDoFollowMode() {}
void pmotionUpdateControlsPrePhysics(Entity* player, ControlState* controls, int curTime) {}
void pmotionSetVel(Entity *e,ControlState *controls, F32 inp_vel_scale) {}
ControlStateChange* createControlStateChange() { return 0; }
void destroyControlStateChange(ControlStateChange* csc) {}
void addControlStateChange(ControlStateChangeList* list, ControlStateChange* csc, int adjust_time) {}
void pmotionUpdateControlsPostPhysics(ControlState* controls, int curTime) {}

ControlId opposite_control_id[CONTROLID_BINARY_MAX];

const char* pmotionGetBinaryControlName(ControlId id) { return ""; }
F32 pmotionGetMaxJumpHeight(Entity* e, ControlState* controls) { return 4.0f; }
void entMotion(Entity* e, Mat3 control_mat) {}
int dump_grid_coll_info = 0;

void pmotionApplyFuturePush(MotionState* motion, FuturePush* fp) {}
void destroyFuturePush(FuturePush* fp) {}
int waitingForDoor() { return 0; }
GlobalMotionState global_motion_state = {0};
void entCalcInterp(Entity *e, Mat4 mat, U32 client_abs, Vec3 out_pyr) {}

void sendTradeCancel( int reason ) {}

int checkForCharacterCreate() { return 0; }
ConnectionStatus commCheckConnection() { return CS_DISCONNECTED; }

void runState() {}
void mapfogSetVisited() {}

Cursor cursor = {0};

int entDebugMenuVisible() { return 0; }
void keybind_send( char *profile, int key, int mod, int secondary, char *command ) {}
void keybind_sendReset() {}
void keybind_sendClear( char *profile, int key, int mod ) {}
void keyprofile_send( char *profile ) {}
void cmdSetTimeStamp(int timeStamp) {}
KeyBindProfile game_binds_profile = {0};
int cmdGameParse(char *str, int x, int y) {return 1;}
int cmdParseEx(char *str, int x, int y) {return 1;}
void chat_exit() {}
void sendDividers(void) {}
void sgListReset() {}
void friendListReset() {}
bool isEmail() { return 0; }
void chatTabClose(bool save) {}
void cd_exit() {}
void cd_clear() {}
bool gWindowFade;
int chat_mode() { return 0; }
int arenaJoinWindow(void) { return 0; }
int arenaResultWindow() { return 0; }
int arenaListWindow(void) { return 0; }
int arenaCreateWindow(void) { return 0; }
int rewardChoiceWindow(void) { return 0; }
int badgesWindow(void) { return 0; }
int enhancementWindow() { return 0; }
int costumeSelectWindow(void) { return 0; }
int deathWindow(void) { return 0; }
int titleselectWindow(void) { return 0; }
int storeWindow() { return 0; }
int lfgWindow() { return 0; }
int targetOptionsWindow( void ) { return 0; }
int missionSummaryWindow(void) { return 0; }
int helpWindow() { return 0; }
int infoWindow() { return 0; }
int quitWindow() { return 0; }
int tradeWindow() { return 0; }
int clueWindow() { return 0; }
int missionWindow() { return 0; }
int contactsWindow() { return 0; }
int emailComposeWindow() { return 0; }
int emailWindow() { return 0; }
int supergroupWindow() { return 0; }
int inspirationWindow() { return 0; }
int contactWindow() { return 0; }
int friendWindow() { return 0; }
int mapWindow() { return 0; }
int compassWindow() { return 0; }
int groupWindow() { return 0; }
int powersWindow() { return 0; }
int channelWindow() { return 0; }
int chatTabWindow() { return 0; }
int chatWindow() { return 0; }
int trayWindow() { return 0; }
int targetWindow() { return 0; }
int statusWindow() { return 0; }
int inventWindow() {return 0;}
int supergroupListWindow() {return 0;}

int levelSkillMenu() { return 0; }
int levelSkillSpecMenu() { return 0; }

int character_SystemExperienceNeeded(Character *p, PowerSystem eSys) { return 0; }

bool character_BoostInventoryFull(Character *p) { return 1; }
int dock_draw() { return 0; }
void resetPrimaryChatSize() {}
void sendDescription( char * desc, char * motto ) {}

TexBind *GetOriginIcon(Entity *e)
{
	return white_tex_bind;
}

TexBind *GetArchetypeIcon(Entity *e)
{
	return white_tex_bind;
}

void pickGalaxyCity() {}
void activateTutorial() {}
void destroyCursorHashTable() {}
BasePower *powerdict_GetBasePowerByString(PowerDictionary *pdict, char *pchDesc, int *piLevel) { return 0; }
PowerDictionary g_PowerDictionary = {0};

TexBind *drawEnhancementOriginLevel( BasePower *ppowBase, int level, int iNumCombines, bool bDrawOrigin, float centerX, float centerY, float z, float sc, float textSc, int clr )
{
	return white_tex_bind;
}

Costume* npcDefsGetCostume(int npcIndex, int costumeIndex) { return 0; }
SupergroupStats *createSupergroupStats(void) { return 0; }
PowerInfo* powerInfo_Create(void) { return 0; }

void character_Destroy(Character *pchar)
{
	EArrayDestroy(&pchar->ppPowerSets);

	// Buffs aren't allocated, they point to BasePowers.
	EArrayDestroy(&pchar->ppowBuffs);

	EArrayDestroy(&pchar->pppowLastDamage);
	EArrayIntDestroy(&pchar->piStickyAnimBits);
	EArrayIntDestroy(&pchar->pPowerModes);
}

void friendDestroy(Friend *f) {}
void destroySupergroupStats(SupergroupStats *stats) {}
void destroyTeamup(Teamup *teamup) {}
void destroyTaskForce(TaskForce *taskforce) {}
void destroySupergroup(Supergroup *supergroup) {}
void powerInfo_Destroy(PowerInfo* info) {}

TokenizerParseInfo SendBasicCharacter[] = {{ 0 }};
TokenizerParseInfo SendFullCharacter[] = {{ 0 }};




//
// Gets called on way into 3d game.  This way new account is created here & not at start of menus.
//
int	player_being_created;

void newCharacter()
{
	player_being_created = TRUE;
}

//
//
CharacterOrigin corigin = { "", "", "", "", ""};
CharacterClass cclass = { "", "", "", "", "", "", 0};

void doCreatePlayer(int connectionless)
{

	Entity *e = playerPtr();

	if( !e )
	{
		e = entCreate("");
		playerSetEnt(e);
		//entSetSeq(e, seqLoadInst( "male", 0, SEQ_LOAD_FULL, e->randSeed, e  ));
		SET_ENTTYPE(e) = ENTTYPE_PLAYER;
		playerVarAlloc( e, ENTTYPE_PLAYER );    //must be after playerSetEnt
	}

	setGender( e, "male" );
	if (!connectionless)
		newCharacter();

	character_Create(e->pchar, e, &corigin, &cclass);

}

void character_Create(Character *pchar, Entity *e, CharacterOrigin *porigin, CharacterClass *pclass)
{
	assert(pchar!=NULL);
	assert(porigin!=NULL);
	assert(pclass!=NULL);

	memset(pchar, 0, sizeof(Character));

	pchar->entParent = e;
	pchar->porigin = porigin;
	pchar->pclass = pclass;

	EArrayCreate(&pchar->ppPowerSets);
	EArrayCreate(&pchar->ppowBuffs);
	EArrayCreate(&pchar->pppowLastDamage);
	EArrayIntCreate(&pchar->piStickyAnimBits);
	EArrayIntCreate(&pchar->pPowerModes);
}

#define DRAG_ICON_SC		0.8f
static HashTable cursorHashTable;

// Does the actual setup
void handleCursorInternal()
{
	// sometimes we draw a software cursor to prevent ugly detaching from window frame
	if( cursor.software )
	{
		int x, y;
		inpMousePos( &x, &y );
		display_sprite( cursor.base, x, y, 5000, 1.f, 1.f, CLR_WHITE );

		if( cursor.overlay )
			display_sprite( cursor.overlay, x, y, 5001, 1.f, 1.f, CLR_WHITE );

		if( cursor.HWvisible )
		{
			ShowCursor( FALSE );
			cursor.HWvisible = FALSE;
		}
	}
	else // most of the time we just use the hardware cursor
	{
		if( cursor.dragging )
		{
			cursor.base = atlasLoadTexture( "cursor_hand_closed.tga" );

			if ( iconIsNew( cursor.base, cursor.dragged_icon ) )
			{
				hwcursorClear();
				hwcursorSetHotSpot(cursor.dragged_icon->width*DRAG_ICON_SC/2+2,cursor.dragged_icon->height*DRAG_ICON_SC/2+2);

				if( cursor.overlay )
				{
					hwcursorBlit( cursor.overlay, pow2( cursor.overlay->width*DRAG_ICON_SC ),
						pow2( cursor.overlay->height*DRAG_ICON_SC ), 0, 0, DRAG_ICON_SC, CLR_WHITE );
				}
				hwcursorBlit( cursor.dragged_icon, pow2( cursor.dragged_icon->width*DRAG_ICON_SC ),
					pow2( cursor.dragged_icon->height*DRAG_ICON_SC ), 0, 0, DRAG_ICON_SC, CLR_WHITE );
				hwcursorBlit( cursor.base, pow2( cursor.base->width ), pow2( cursor.base->width ),
					cursor.dragged_icon->width*DRAG_ICON_SC/2-12, cursor.dragged_icon->height*DRAG_ICON_SC/2-12, 1.0f, CLR_WHITE );
				hwcursorReload(NULL, cursor.dragged_icon->width*DRAG_ICON_SC/2,cursor.dragged_icon->height*DRAG_ICON_SC/2);
			}
		}
		else if( !cursor.dragging && iconIsNew( cursor.base, cursor.overlay ) )
		{
			HashElement element;
			char cursor_name[1000] = {0};
			char* pos = cursor_name;
			HCURSOR hCursor;

			if(!cursorHashTable)
			{
				cursorHashTable = createHashTable();

				initHashTable(cursorHashTable, 100);

				hashSetMode(cursorHashTable, FullyAutomatic);
			}

			pos += sprintf(pos, "%s(%d,%d)", cursor.base->name, cursor.hotX, cursor.hotY);

			if( cursor.overlay )
				pos += sprintf(pos, "%s", cursor.overlay->name);

			element = hashFindElement(cursorHashTable, cursor_name);

			if(element)
			{
				hCursor = hashGetValue(element);
			}
			else
			{
				hCursor = NULL;
			}

			if(!hCursor)
			{
				hCursor = NULL;
				hwcursorClear();
				hwcursorSetHotSpot(cursor.hotX,cursor.hotY);
				hwcursorBlit( cursor.base, pow2( cursor.base->width ), pow2( cursor.base->width ), 0, 0, 1.f, CLR_WHITE );
				if( cursor.overlay )
					hwcursorBlit( cursor.overlay, pow2( cursor.overlay->width ), pow2( cursor.overlay->width ), 0, 0, 1.f, CLR_WHITE );
			}

			hwcursorReload(game_state.cursor_cache ? &hCursor : NULL, cursor.hotX, cursor.hotY);

			if(game_state.cursor_cache && !element && hCursor)
			{
				element = hashAddElement(cursorHashTable, cursor_name, hCursor);

				assert(element);
			}
		}

		if( !cursor.HWvisible )
		{
			ShowCursor( TRUE );
			cursor.HWvisible = TRUE;
		}
	}

	setCursor( NULL, NULL, FALSE, 2, 2 ); // reset cursor for next pass
}

void setCursor( char *base, char *overlay,  int software, int hotX, int hotY )
{
	if( base )
		cursor.base = atlasLoadTexture( base );
	else
		cursor.base = atlasLoadTexture( "cursor_select.tga" );

	if( overlay )
		cursor.overlay = atlasLoadTexture( overlay );
	else
		cursor.overlay = NULL;

	cursor.software = software;
	cursor.hotX = hotX;
	cursor.hotY = hotY;

}


void handleCursor()
{
	static int frameDelay = 0, cursor_initted = FALSE;

	if (mouseDown(MS_LEFT))
	{
		int i = 3;
		i = 2;
	}

	if( !cursor_initted )
	{
		cursor_initted = TRUE;
		memset(	&cursor, 0, sizeof( Cursor ) );
		cursor.HWvisible = TRUE;
		setCursor( "cursor_select.tga", NULL, FALSE, 2, 2 );
	}

	if( !isMenu(MENU_GAME) )
	{
		control_state.canlook = FALSE;
	}


	PERFINFO_AUTO_START("change cursor", 1);

	handleCursorInternal();

	PERFINFO_AUTO_STOP();
}

ArenaEvent gArenaDetail;
void arenaSendPlayerDrop( ArenaEvent * event, int kicked_dbid ) {}

bool gShowSalvage;

int texSwapsCount = 0;
void positionCapsule( const Mat4 entMat, EntityCapsule * cap, Mat4 collMat ) {;}
int gShowSkillUI = 0;
int gShowPets = 0;
void basepropsWindow(void) {;}
void rdrStatsWindow(void) {;}
void skillsWindow(void) {;}
DbInfo db_info;
DefTexSwapNode * texSwaps = 0;
int shader_shrink, shader_hblur, shader_vblur, shader_tonemap;
void cmdAccessOverride(int level) {;}
void updateClientFrameTimestamp() {;}
int entNetUpdate() { return 1; }
int commCheck(int lookfor) { return 0; }
void commSendInput(void) {;}
int commConnected(void) { return 1; }
void dbPingIfConnected() {;}
int doMapXfer(void) { return 0; }
int do_map_xfer = 0;
void editProcess(void) {;}
void baseedit_Process(void) {;}
void neighborhoodCheck(void) {;}
int DoorAnimFrozenCam(void) { return 0; }
int baseedit_Mode(void) { return 0; }
void demoCheckPlay(void) {;}
int simulateCharacterCreate(int askuser) { return 0; }
int commReqScene(void) { return 0; }
void commNewInputPak(void) {;}
void commInit(void) {;}
void commResetControlState(void) {;}
int dbConnect(char *server,int port,int user_id,int cookie,char *auth_name,int no_version_check,F32 timeout,int no_timeout) { return 1; }
void dbDisconnect() {;}
int tryConnect(void) { return 1; }
int choosePlayerWrapper(int player_slot, int tutorial) { return 1; }
int loginToDbServer(int id,char *err_msg) { return 1; }
int loginToAuthServer(void) { return 1; }
void checkModelReload(void) {;}
void checkDoneResumingMapServerEntity(void) {;}
void DoorAnimClientCheckMove(void) {;}
void quitToLogin() {;}
void CheckMissionKickProgress(int disconnecting) {;}
int MissionKickSeconds() { return 0; }
void authUserSetVillainAccess(U32 *data, int i) {;}
void gameStateLoad() {;}
void cmdOldCommonInit() {;}
void texWordsEditor(int fromEditor) {;}
void initChatUIOnce() {;}
void chatClientInit() {;}
void modelInitReload(void) {;}
void editStateInit() {;}
void loadHelp() {;}
void basedata_Load(void) {;}
void load_Stores(void) {;}
void load_AllDefs(void) {;}
void demoPlay(void) {;}
void demoRecordClientInfo(void) {;}


int getDebugCameraPosition(CameraInfo* caminfo) { return 0; }

int baseRoomWindow(void) { return 0; }
int baseInventoryWindow(void) { return 0; }

void deactivateTutorial(void) {;}

AtlasTex* groupThumbnailGet( char *groupDefName ) { return 0; }

NxState nx_state;

void  nwCreatePhysicsSDK(void) {;}
void  nwDeletePhysicsSDK(void);

bool  nwEnabled(void) { return 0; }
int  nwCreateScene(void) {return 0;}
void  nwStepScene(double stepSize) {;}

//void* nwLoadMeshShape(const char* name, const void* vertArray, int vertCount, int vertStride, const void* triArray, int triCount, int triStride) { return 0; }
void* nwLoadMeshShape(const char* pcStream, int iStreamSize){return 0;}
void nwFreeMeshShape(void* shapeParam) {;}



void* nwCreateBoxActor(Mat4 mWorldSpaceMat, Vec3 vSize, NxGroupType group, float density, int iSceneNum){ return 0; }
void* nwCreateSphereActor(Mat4 mWorldSpaceMat, float fRadius, NxGroupType group, float fDensity, int iSceneNum){ return 0; }
void* nwCreateMeshActor(Mat4 meshTransform, GroupDef* def, void* shape, NxGroupType group, int iSceneNum){ return 0; }
void* nwCreateConvexActor(const char* name, Mat4 convexTransform, const float* verts, int vertcount, NxGroupType group, FxGeo* geoOwner, float fDensity, int iSceneNum){ return 0; }


void  nwPushSphereActor(const float* position, const float* vel, const float* angVel, float density, float radius, NxGroupType group, FxGeo* geoOwner, int iSceneNum) {}

NxActorCreationState nwGetActorCreationState( void* actor ) {return (NxActorCreationState)0;}
void  nwActorApplyAcceleration(void* actor, const float* acceleration, bool bPermanent) {}
void  nwSetActorCreationState( void* actor, NxActorCreationState eCreationState ) {}

void  nwCreateThreadQueues(void) {}
float  nwGetDefaultGravity() {return 0.0f;}
void  nwDeleteActor(void* actorParam, int iSceneNum) {;}
void  	nwGetOrientation(void* actor, float* position, float (*rotation)[3]) {;}
void  	nwMoveActor(void* actor, float* position, float (*rotation)[3]) {;}
void  	nwSetActorLinearVelocity(void* actor, const float* velocity) {;}
void  	nwGetActorLinearVelocity(void* actor, float* velocity) {;}
void  	nwSetActorAngularVelocity(void* actor, const float* velocity) {;}
void  	nwGetActorAngularVelocity(void* actor, float* velocity) {;}
void  	nwGetActorGlobalPosition(void* actor, float* position) {;}
void  	nwGetActorMat4( void* actor, float* mat4 ) {;}
void  	nwGetActorComMat4( void* actor, float* com) {;}
void nwStepScenes(double stepsize){}

void  	nwCoreDump(void) {;}

void  	nwEnterDebug(NwDrawFunc drawFunc) {;}
void  	nwLeaveDebug(void) {;}

int   	nwRaycast(Vec3 p0, Vec3 p1, Vec3 out) { return 0; }
void  	nwPrintInfoToConsole(int toConsole) {;}

void	nwCreateTestBoxes(Vec3 pos, int count) {;}
void	nwDestroyTestBoxes(void) {;}
void	nwDrawTestBoxes(void) {;}

bool nwDeinitializeNovodex(void) { return 1; }
bool nwInitializeNovodex(void) { return 1; }

void  nwPushConvexActor(const char* name, Mat4 convexTransform, const Vec3 velocity, const Vec3 angVelocity, const float* verts, int vertcount, NxGroupType group, FxGeo* geoOwner, float fDensity, int iSceneNum) {;}
void nwActorSetDrag(void* actor, F32 fDrag ) {;}

void  nwSetNovodexDefaults( ) {;}


int recipeinvWindow(void) { return 0; }
int conceptinvWindow(void) { return 0; }
int salvageWindow(void) { return 0; }
int inventoryWindow(void) { return 0; }
BaseEditState baseEdit_state;


int petWindow(void) {return 0;}

void pet_Add( int dbid ) {return;}
void pet_Remove( int dbid ){return;}
void pets_Clear(void){return;}
bool entIsPet( Entity * e ){return 0;}

void pet_command( char * cmd, char * target, int type ){return;}
void pet_selectCommand( int stance, int action, char * target, int command_type, Vec3 vec ){return;}
ContextMenu * interactPetMenu( void ){return 0;}
bool playerIsMasterMind(){return 0;}


void displayEntDebugInfo2D(void) {}
void displayEntDebugInfo3D(void) {}

ArenaPetManagementInfo g_ArenaPetManagementInfo;
void arena_CommitPetChanges(void) {}

int optionsWindow() {return 0;}
int workshopWindow() {return 0;}
int arenaGladiatorPickerWindow() { return 0;}
void displayEntDebugInfoTextEnd() {}
void displayEntDebugInfoTextBegin() {}
int amOnPetArenaMap() {return 0;}

//void lightmapRenderDebug(void) {}
int cmdCompleteComand(char *str, char *out, int searchID, int searchBackwards) {return 0;}

void displayDebugInterface2D(){}
void displayDebugInterface3D(){}
int cmdIsServerCommand(char*str){return 0;}

void addNewbMastermindMacros( int count ){}

typedef void LightmapSaveModelData;
typedef void LightmapSaveData;
char gMapName[256] = {0};

void makeAGroupEnt( GroupEnt * groupEnt, GroupDef * def, Mat4 mat, F32 draw_scale, int has_dynamic_parent ) {;}
void groupDrawCacheInvalidate(GroupDef *def){}
void drawBar( float x, float y, float z, float width, float scale, float percentage, float glow_scale, int color, int negative ){}
void handleGlowState( int * mode, float * last_percentage, float percentage, float * glow_scale, float speed ){}
void reloadGeoLoadData(GeoLoadData *gld){}
void detailrecipes_Load(void) {;}
void updatePlayerName(void) {;}
void groupRadiosityRunClient(char *serverName) {;}
void groupRadiosityRunServer(char *lightmapFolder, int disconnectMode) {;}
void groupRadiosityShutdownClient(void) {;}
void groupRadiosityShutdownServer(void) {;}
void groupRadiosityClientActivity(void) {;}
void load_InventoryAttribFiles( bool bNewAttribs ) {;}
int sgRaidListWindow(void) {return 0;}
void nwFreeStreamBuffer() {;}
void trackerUnselect(DefTracker *tracker) {;}
int baseToFile(char *fnamep,Base *base) {return 0;}
typedef struct BaseRoom BaseRoom;
BaseRoom *baseRoomLocFromPos(Base *base, const Vec3 pos, int block[2]) {return 0;}
int drawDefInternal( GroupEnt *groupEnt, Mat4 parent_mat_p, DefTracker *tracker, VisType vis, DrawParams *draw, int uid ) {return 0;}
Base	g_base;
EditSelect	sel;
SelInfo	sel_list[12000];
int		sel_count;
void selAdd(DefTracker *tracker,int group_id,int toggle, int selectionSrc) {;}
void editRefSelect(DefTracker *tracker,U32 color) {;}
DefTexSwap ** dirtySwaps;
Menu *		libraryMenu;
MenuEntry * addEntryToMenu(Menu * m,const char * name,void (*func)(MenuEntry *,ClickInfo *),void * info) { return 0; }
void deleteEntryFromMenu(Menu * m,const char * name) {;}
void lightGiveLightTrackerToMyDoor(DefTracker * tracker, DefTracker * lighttracker) {;}
void lightsToGrid(DefTracker *tracker,Grid *light_grid) {;}
void libraryMenuClickFunc(MenuEntry * me,ClickInfo * ci) {;}
void finishLoadMap(int tray_link_all, int loadlightmaps) {;}
void clientLoadMapStart(char *mapname) {;}
int map_full_load;
void entMotionClearAllWorldGroupPtr() {;}
void sgrp_UnTrackDbId(int idSgrp) {;}
void setClientFlags(GroupDef *def) {;}
void baseedit_InvalidateGroupDefs(void) {;}
void baseMakeAllReferenceArray() {;}
void baseAddReferenceName( char * name ) {;}
char *objectLibraryPathFromObj(char *obj_name) {return 0;}
GroupFileEntry *objectLibraryEntryFromObj(char *obj_name) { return 0; }
void objectLibraryAddName(GroupDef *def) {;}
void objectLibraryLoadNames(int force_rebuild) {;}
void objectLibraryClearTags() {;}
int objectLibraryGetBounds(char *objname,GroupDef *def) { return 0; }
GroupFileEntry *objectLibraryEntryFromFname(char *fname) { return 0; }
void objectLibraryWriteBoundsFileIfDifferent(GroupFile *file) {;}
void makeBaseMakeReferenceArrays(void) {;}

void srEnterRegistrationScreen() {;}
void sendSuperCostume( unsigned int primary, unsigned int secondary, unsigned int primary2, unsigned int secondary2, unsigned int tertiary, unsigned int quaternary, bool hide_emblem  ) {;}
void sendHideEmblemChange(int hide) {;}

void baseHackeryFreeCachedRgbs(void){}
void trayobj_Destroy( TrayObj *item ){}
int sgRaidTimeWindow(void){ return 0; }
TrayObj* getTrayObjCreate(Tray *tray, int i, int j){ return 0; }



















