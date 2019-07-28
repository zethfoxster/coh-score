/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "color.h"
#include "earray.h"

#include "entity.h"
#include "entPlayer.h"

#include "cmdoldparse.h"
#include "cmdgame.h"
#include "gridcache.h"
#include "gfx.h"

#include "player.h"
#include "uiKeybind.h"
#include "uiWindows.h"
#include "uiConsole.h"
#include "baseedit.h"
#include "wdwbase.h"
#include "bases.h"
#include "basedata.h"
#include "baseclientsend.h"
#include "mathutil.h"
#include "MessageStore.h"
#include "MessageStoreUtil.h"

#include "uiBaseInput.h"
#include "uiBaseRoom.h"
#include "uiBaseInventory.h"
#include "uiNet.h"
#include "uiCursor.h"
#include "uiTray.h"
#include "file.h"
#include "uiGame.h"
#include "uiScrollBar.h"
#include "uiCursor.h"
#include "uiFx.h"

static bool s_bWasLooking = false;

enum
{
	BCMD_QUIT,
	BCMD_ZOOM,
	BCMD_ZOOMIN,
	BCMD_ZOOMOUT,
	BCMD_FORWARD,
	BCMD_BACKWARD,
	BCMD_LEFT,
	BCMD_RIGHT,
	BCMD_TURNLEFT,
	BCMD_TURNRIGHT,
	BCMD_MOUSELOOK,
	BCMD_DRAG,
	BCMD_SELECT,
	BCMD_DRAGROOM,
	BCMD_CENTER_VIEW,
	BCMD_ROTATE,
	BCMD_CENTER_SEL,
	BCMD_SELL,
	BCMD_TOTAL,
	BCMD_SELECT_NEXT,
	BCMD_SELECT_LAST,
	BCMD_GRID_SNAP,
	BCMD_GRID_SNAP_CYCLE,
	BCMD_ANGLE_SNAP,
	BCMD_ANGLE_SNAP_CYCLE,
	BCMD_ROOM_CLIP,
	BCMD_ROOM_CLIP_CYCLE,
	BCMD_ATTACH_CYCLE,
	BCMD_UNDO,
	BCMD_REDO,
	BCMD_DEFAULT_SKY,
	BCMD_ITEM_SEARCH,
};

KeyBindProfile baseEdit_binds_profile;
BaseEditState baseEdit_state = {0};
static float s_fOriginalVisScale = 1.0f;


static char tmp_str[1024], tmp_str2[1024];
static int tmp_int, tmp_int2, tmp_int3;
static F32 tmp_f32;

Cmd baseEdit_cmds[] =
{
	{ 0, "quit",		BCMD_QUIT,			{{0}}, 0, "Stop base editing."},
	{ 0, "mousedrag",	BCMD_DRAG,			{{PARSETYPE_S32, &tmp_int}},			0, "Start a mouse drag." },
	{ 0, "baseselect",	BCMD_SELECT,		{0},											0, "Selects at mouse position." },
	{ 0, "dragroom",	BCMD_DRAGROOM,		{{PARSETYPE_S32, &tmp_int},{PARSETYPE_S32, &tmp_int2}},	0, "Drags a room around <x,y>" },
	{ 0, "center",		BCMD_CENTER_VIEW,	{0},											0, "Centers view on current x/y." },
	{ 0, "rotate",		BCMD_ROTATE,		{{PARSETYPE_S32, &tmp_int}},							0, "Rotate Selection." },
	{ 0, "centersel",	BCMD_CENTER_SEL,	{0},											0, "Center on current selection."},
	{ 0, "sell",		BCMD_SELL,			{{0}},											0, "Sell current selection." },
	{ 0, "select_next",	BCMD_SELECT_NEXT,	{{0}},											0, "Select next visible detail" },
	{ 0, "select_last",	BCMD_SELECT_LAST,	{{0}},											0, "Select previous visible detail" },
	{ 0, "grid_snap",   BCMD_GRID_SNAP,     {{PARSETYPE_FLOAT, &baseEdit_state.gridsnap}},	0, "Set the grid size, 0 to disable grid" },
	{ 0, "grid_snap_cycle", BCMD_GRID_SNAP_CYCLE, {{0}},									0, "Cycle through grid snap sizes." },
	{ 0, "angle_snap",  BCMD_GRID_SNAP,     {{PARSETYPE_FLOAT, &baseEdit_state.anglesnap}},	0, "Set the angle to snap to while rotating, 0 to disable snap" },
	{ 0, "angle_snap_cycle", BCMD_ANGLE_SNAP_CYCLE, {{0}},									0, "Cycle through angle snap values." },
	{ 0, "room_clip",   BCMD_ROOM_CLIP,     {{PARSETYPE_S32, &baseEdit_state.roomclip}},	0, "Allow items to clip through room walls" },
	{ 0, "room_clip_cycle", BCMD_ROOM_CLIP_CYCLE, {{0}},									0, "Cycle through room clip modes." },
	{ 0, "attach_cycle", BCMD_ATTACH_CYCLE, {{0}},											0, "Cycle through attach modes." },
	{ 0, "base_undo",	BCMD_UNDO,			{{0}},											0, "Undoes the last base editor action." },
	{ 0, "base_redo",	BCMD_REDO,			{{0}},											0, "Redoes the last base editor action that was undone." },
	{ 0, "base_default_sky",	BCMD_DEFAULT_SKY,	{{PARSETYPE_S32, &tmp_int}},			0, "Sets the sky number to be used in the base outside of sky volumes." },
	{ 0 },
};


CmdList baseEdit_cmdlist =
{
	{{ baseEdit_cmds },
	{ 0 }}
};

int ignoreInput() {
	if( control_state.canlook || collisions_off_for_rest_of_frame || cursor.dragging || gWindowDragging || gScrollBarDragging)
		return true;
	if (window_collision())
		return true;
	return false;
}

/**********************************************************************func*
 * baseEditCmdParse
 *
 */
int baseEditCmdParse(char *str, int x, int y)
{
	Cmd			*cmd;
	CmdContext	output = {0};
	bool		didEditCommand=false;
	bool		needRefresh=false;

	output.found_cmd = false;
	output.access_level = cmdAccessLevel();
	cmd = cmdOldRead(&baseEdit_cmdlist,str,&output);

	if (output.msg[0])
	{
		const char *unknowncmd = msGetUnformattedMessageConst(menuMessages,"UnknownCommandString");
		const char *head = strchr(unknowncmd, '{');

		// Compare everything up to the first parameter.
		if (unknowncmd && head && strncmp(output.msg,unknowncmd,head-unknowncmd)==0)
			return 0;

		conPrintf("%s",output.msg);
		return 1;
	}

	if (!cmd || !g_base.rooms)
		return 0;

	switch(cmd->num)
	{
		case BCMD_QUIT:
			baseedit_CancelDrags();
			baseedit_CancelSelects();
			break;

		case BCMD_ZOOM:
			baseedit_SetCamDist();
			break;

		case BCMD_SELECT:
			if(!ignoreInput())
				baseedit_Select(x, y);
			break;

		case BCMD_DRAGROOM:
			if(!ignoreInput())
				baseedit_StartRoomDrag(tmp_int, tmp_int2, NULL);
			break;

		case BCMD_CENTER_VIEW:
			if(!ignoreInput())
				baseCenterXY(x, y);
			break;
		case BCMD_CENTER_SEL:
			baseedit_LookAtCur();
			break;

		case BCMD_DRAG:
  			if(tmp_int && !ignoreInput() && baseedit_Mode() != kBaseEdit_AddPersonal)
			{
				baseEdit_state.dragging = true;
				g_iXDragStart = x;
				g_iYDragStart = y;
			}
			else
			{
				baseEdit_state.dragging = false;
			}
			break;

		case BCMD_ROTATE:
			baseedit_Rotate(tmp_int);
			break;

		case BCMD_SELL:
			baseedit_DeleteCurDetail();
			baseedit_SetCurDetail(NULL,NULL);
			break;

		case BCMD_SELECT_NEXT:
			baseedit_SelectNext(false);
			break;
		case BCMD_SELECT_LAST:
			baseedit_SelectNext(true);
			break;

		case BCMD_GRID_SNAP_CYCLE:
		{
			const char *snapnum = "";
			char snapmsg[256];
			priorityAlertText_clearAll();

			if (baseEdit_state.gridsnap >= 4)
				baseEdit_state.gridsnap = 0;
			else if (baseEdit_state.gridsnap >= 2) {
				baseEdit_state.gridsnap = 4;
				snapnum = "4";
			} else if (baseEdit_state.gridsnap >= 1) {
				baseEdit_state.gridsnap = 2;
				snapnum = "2";
			} else if (baseEdit_state.gridsnap >= 0.5) {
				baseEdit_state.gridsnap = 1;
				snapnum = "1";
			} else if (baseEdit_state.gridsnap >= 0.25) {
				baseEdit_state.gridsnap = 0.5;
				snapnum = "1/2";
			} else {
				baseEdit_state.gridsnap = 0.25;
				snapnum = "1/4";
			}

			if (baseEdit_state.gridsnap == 0)
				priorityAlertText_add("Grid Disabled", 0xFFFFFFFF, NULL);
			else {
				sprintf_s(SAFESTR(snapmsg), "Grid Size: %s x %s", snapnum, snapnum);
				priorityAlertText_add(snapmsg, 0xFFFFFFFF, NULL);
			}
			break;
		}

		case BCMD_ANGLE_SNAP_CYCLE:
		{
			char snapmsg[256];
			priorityAlertText_clearAll();

			if (baseEdit_state.anglesnap >= 45)
				baseEdit_state.anglesnap = 0;
			else if (baseEdit_state.anglesnap >= 30)
				baseEdit_state.anglesnap = 45;
			else if (baseEdit_state.anglesnap >= 15)
				baseEdit_state.anglesnap = 30;
			else if (baseEdit_state.anglesnap >= 10)
				baseEdit_state.anglesnap = 15;
			else if (baseEdit_state.anglesnap >= 5)
				baseEdit_state.anglesnap = 10;
			else if (baseEdit_state.anglesnap >= 3)
				baseEdit_state.anglesnap = 5;
			else if (baseEdit_state.anglesnap >= 1)
				baseEdit_state.anglesnap = 3;
			else
				baseEdit_state.anglesnap = 1;

			if (baseEdit_state.anglesnap == 0)
				priorityAlertText_add("Angle Snap Disabled", 0xFFFFFFFF, NULL);
			else {
				sprintf_s(SAFESTR(snapmsg), "Angle Snap: %g\xC2\xB0", baseEdit_state.anglesnap);
				priorityAlertText_add(snapmsg, 0xFFFFFFFF, NULL);
			}
			break;
		}

		case BCMD_ROOM_CLIP_CYCLE:
			priorityAlertText_clearAll();
			baseEdit_state.roomclip = !baseEdit_state.roomclip;
			priorityAlertText_add(baseEdit_state.roomclip ?
				"Room Clipping Enabled" : "Room Clipping Disabled", 0xFFFFFFFF, NULL);
			break;

		case BCMD_ATTACH_CYCLE:
			priorityAlertText_clearAll();
			if (!g_refCurDetail.p) {
				priorityAlertText_add("No Item Selected!", 0xFFFFFFFF, NULL);
				break;
			}

			if (DetailHasFlag(g_refCurDetail.p->info, ForceAttach)) {
				priorityAlertText_add("Cannot change attachment type for this item!", 0xFFFFFFFF, NULL);
				break;
			}

			if (baseEdit_state.nextattach == -1)
				baseEdit_state.nextattach = detailAttachType(g_refCurDetail.p);

			if (g_refCurDetail.p->info->eAttach == kAttach_Wall) {
				baseEdit_state.nextattach--;
				if (baseEdit_state.nextattach < 0)
					baseEdit_state.nextattach = kAttach_Count - 1;
			} else {
				baseEdit_state.nextattach = (baseEdit_state.nextattach + 1) % kAttach_Count;
			}

			switch(baseEdit_state.nextattach) {
				case kAttach_Floor:
					priorityAlertText_add("Attachment: Floor", 0xFFFFFFFF, NULL);
					break;
				case kAttach_Wall:
					priorityAlertText_add("Attachment: Wall", 0xFFFFFFFF, NULL);
					break;
				case kAttach_Ceiling:
					priorityAlertText_add("Attachment: Ceiling", 0xFFFFFFFF, NULL);
					break;
				case kAttach_Surface:
					priorityAlertText_add("Attachment: Surface", 0xFFFFFFFF, NULL);
					break;
			}
			break;

		case BCMD_UNDO:
			baseClientSendUndo();
			break;

		case BCMD_REDO:
			baseClientSendRedo();
			break;

		case BCMD_DEFAULT_SKY:
			g_base.defaultSky = tmp_int < 0 ? 0 : tmp_int;
			baseClientSendDefaultSky(g_base.defaultSky);
			break;

		default:
			break;
	}

	return 1;
}


/**********************************************************************func*
 * baseEditKeybindInit
 *
 */
void baseEditKeybindInit(void)
{
	static int baseEditInitialized = 0;

	if(!baseEditInitialized)
	{
		baseEditInitialized = 1;

		cmdOldInit(baseEdit_cmds);

		bindKeyProfile(&baseEdit_binds_profile);
		memset(&baseEdit_binds_profile, 0, sizeof(baseEdit_binds_profile));
		baseEdit_binds_profile.name = "baseEdit";
		baseEdit_binds_profile.parser = baseEditCmdParse;
		baseEdit_binds_profile.trickleKeys = 1;

		bindKey("esc",				"quit",				0);
		bindKey("leftdrag",			"+mousedrag",		0);
		bindKey("leftclick",		"baseselect",		0);
		bindKey("r",				"rotate 0",			0);
		bindKey("leftdoubleclick",	"center",			0);
		bindKey("RightClick",		"rotate 0",			0);
		bindKey("delete",			"sell",				0);
		bindKey("tab",				"select_next",		0);
		bindKey("shift+tab",		"select_last",		0);
		bindKey("f1",				"grid_snap_cycle",	0);
		bindKey("f2",				"angle_snap_cycle",	0);
		bindKey("f3",				"room_clip_cycle",	0);
		bindKey("f5",				"attach_cycle",		0);
		bindKey("ctrl+z",			"base_undo",		0);
		bindKey("ctrl+y",			"base_redo",		0);
		unbindKeyProfile(&baseEdit_binds_profile);
	}
}




/**********************************************************************func*
 * baseEditToggle
 *
 */
void baseEditToggle(int iOn)
{
	Entity *e = playerPtr();

	if(iOn == game_state.edit_base)
		return;

	// Disable see_everything if leaving the base editor.
	if (iOn == kBaseEdit_None) {
		game_state.see_everything = 0;
	}

	if( !isDevelopmentMode() )
	{
		if( (iOn == kBaseEdit_Architect || iOn == kBaseEdit_Plot) && !playerCanModifyBase(e, &g_base) )
			return;

		if( iOn == kBaseEdit_AddPersonal && !playerCanPlacePersonal(e, &g_base) )
			return;
	}

	// clear world cursor
	cursor.target_world = kWorldTarget_None;
	s_ptsActivated = NULL;

	if( iOn )
	{
		if(e->access_level && game_state.local_map_server)
		{
			char mapname[1024];

			if(!e->supergroup)
				return;

			// TODO: Fix this to work right.
			// BASE_TODO: Fix this to work right.
			// This should load my supergroup's base from the DB.
			sprintf(mapname, "loadmap \"supergroupid=%i,userid=0\"", e->supergroup_id);
			cmdParse(mapname);
		}
	}
	baseClientSendMode(iOn);
	if (!iOn) // Let players leave base edit mode after map disconnect
		baseSetLocalMode(0);
}

void baseSetLocalMode(int mode)
{
	Entity *e = playerPtr();
	static int *openwindows;
	static float baseInvX, baseInvY;
	int tempX, tempY;
	int i;
	
	if(mode == game_state.edit_base)
		return;
	
	game_state.edit_base = mode;

	if( mode == kBaseEdit_Architect )
		baseedit_SetCamDistAngle(400.f, RAD(80));
	if( mode == kBaseEdit_Plot )
		baseedit_SetCamDistAngle(800.f, RAD(90));

	setBaseInventorySelectedItem(NULL);
	if( mode )
	{
		gridCacheInvalidate();
		e->xlucency = 0.25f;

		window_getUpperLeftPos(WDW_BASE_INVENTORY, &tempX, &tempY);
		baseInvX = tempX;
		baseInvY = tempY;

		// close all other windows
		for( i = 0; i < MAX_WINDOW_COUNT+custom_window_count; i++ )
		{
			if( window_getMode(i) == WINDOW_DISPLAYING )
			{
				window_setMode(i, WINDOW_SHRINKING );
				eaiPush(&openwindows, i);
			}
		}

		bindKeyProfile(&baseEdit_binds_profile);

		window_setMode(WDW_BASE_PROPS, WINDOW_GROWING);
		window_setMode(WDW_BASE_INVENTORY, WINDOW_GROWING);
		
		if( mode == kBaseEdit_Architect || mode == kBaseEdit_AddPersonal)
		{
			window_setMode(WDW_BASE_ROOM, WINDOW_GROWING);
		}

		s_fOriginalVisScale = game_state.vis_scale;
		if (s_fOriginalVisScale < 2.0f)
			setVisScale(2.0f, 1);

		baseEdit_state.anglesnap = 5;
		baseEdit_state.gridsnap = 1;
		baseEdit_state.roomclip = 0;
		baseEdit_state.nextattach = -1;
	}
	else
	{
		baseedit_CancelDrags();
		baseedit_CancelSelects();
		gridCacheInvalidate();
		if (e)
			e->xlucency = 1.0f;

		unbindKeyProfile(&baseEdit_binds_profile);

		window_setMode(WDW_BASE_PROPS, WINDOW_SHRINKING);
		window_setMode(WDW_BASE_ROOM, WINDOW_SHRINKING);

		while( eaiSize(&openwindows) )
			window_setMode( eaiPop(&openwindows), WINDOW_GROWING );

		window_setDims( WDW_BASE_INVENTORY, baseInvX, baseInvY, 150, -1 );

		setVisScale(s_fOriginalVisScale, 1);
	}
}

/* End of File */
