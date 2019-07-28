
#include "uiQuit.h"
#include "uiUtil.h"
#include "uiChat.h"
#include "wdwbase.h"
#include "uiWindows.h"
#include "uiUtilGame.h"
#include "uiSuperRegistration.h"
#include "uiUtilMenu.h"
#include "uiDialog.h"
#include "uiCompass.h"
#include "uiEditText.h"
#include "uiLogin.h"
#include "uiEmail.h"
#include "uiContextMenu.h"
#include "uiGroupWindow.h"
#include "cmdgame.h"
#include "clientcomm.h"
#include "authclient.h"
#include "win_init.h"
#include "entrecv.h"
#include "gfx.h"
#include "gfxLoadScreens.h"
#include "player.h"
#include "initClient.h"
#include "sound.h"
#include "fx.h"
#include "uiNet.h"
#include "edit_cmd.h"
#include "entDebug.h"
#include "costume_client.h"
#include "groupscene.h"
#include "sun.h"
#include "uiTailor.h"
#include "uiSuperCostume.h"
#include "arena/ArenaGame.h"
#include "entity.h"
#include "baseedit.h"
#include "bases.h"
#include "group.h"
#include "entclient.h"
#include "raidstruct.h"
#include "gameData/sgraidClient.h"
#include "uiPlaque.h"
#include "uiFx.h"
#include "uiMissionSearch.h"
#include "uiAutomap.h"
#include "ZowieClient.h"
#include "uiAuction.h"
#include "uiLogin.h"

extern int gClickToMoveButton;

void quitToLogin(void * data)
{
	Entity *e = playerPtr();

	if(e)
	{
		e->logout_login = 0;
		e->logout_timer = 0;
	}

	game_state.pendingTSMapXfer = 0;
	window_closeAlways();
	clearCutScene();
	entDebugClearServerPerformanceInfo();
	resetArenaVars();
	editSetMode(0, 0);
	commDisconnect();
	resetStuffOnMapMove();
	plaqueClearQueue();
	authLogout();
	chatCleanup();
	restartLoginScreen();
	clearDestination( &activeTaskDest );
	clearDestination( &waypointDest );
	clearDestination( &serverDest );
	dialogClearQueue( 1 );
	contextMenu_closeAll();
	emailResetHeaders(1);
	srClearAll();
	costumereward_clear( 0 );
	costumereward_clear( 1 );
	fadingText_clearAll();
	electric_clearAll();
	attentionText_clearAll();
	priorityAlertText_clearAll();
	movingIcon_clearAll();
	fxReInit();
	zowieReset();
	sunSetSkyFadeClient(0, 1, 0.0);
	sceneLoad("scenes/default_scene.txt");
	sndStopAll();
	commNewInputPak();
	searchClearComment();
	basedit_Clear();
	baseReset(&g_base);
	BaseRaidClearAll();
	if (g_raidinfos)
		eaClearEx(&g_raidinfos, SupergroupRaidInfoDestroy);
	entReset();
	playerSetEnt(0);
	groupReset();
	server_visible_state.timestepscale = 1;
	clearAuctionFields();

	loadScreenResetBytesLoaded();
	showBgReset();
	gSentMoTD = 0;
	gSentRespecMsg = 0;
	if( gTailoredCostume )
		costume_destroy( gTailoredCostume );
	gTailoredCostume = 0;
	if( gSuperCostume )
		costume_destroy( gSuperCostume );
	gSuperCostume = 0;

	missionsearch_clearAllPages();

	gClickToMoveButton = 0;
	gPlayerNumber = 0;
	gChatLogonTimer = 0;

}

extern int g_keepPassword;

void quitToCharacterSelect(void *data)
{
	int i;
	char err_msg[1000];

	g_keepPassword = true;
	quitToLogin(data);
	g_keepPassword = false;
	
	s_loggedIn_serverSelected = loginToAuthServer(10); // retry 10 times
	if (s_loggedIn_serverSelected != LOGIN_STAGE_START) // successful login to auth server
	{
		for (i = 0; i < auth_info.server_count; i++)
		{
			if (auth_info.server_count == 1
				|| auth_info.servers[i].id == auth_info.last_login_server_id)
			{
				s_loggedIn_serverSelected = loginToDbServer(i, err_msg);
				respondToDbServerLogin(i, err_msg, auth_info.servers[i].name);
			}
		}
	}
}

void promptQuit(char *reason)
{
	dialog(DIALOG_TWO_RESPONSE,-1,-1,-1,-1,reason,"QuitToLogin",quitToLogin,"QuitToDesktop",windowExitDlg,0,0,0,0,0,0,0);
}

int quitWindow()
{
	float x, y, z, wd, ht, scale;
	int color, bcolor;
	Entity * e = playerPtr();

 	if( !window_getDims( WDW_QUIT, &x, &y, &z, &wd, &ht, &scale, &color, &bcolor ) )
		return 0;

	drawFrame( PIX3, R10, x, y, z, wd, ht, scale, color, 0x00000088 );

   	if( D_MOUSEHIT == drawStdButton( x + wd/2, y + 20*scale + PIX3*scale, z, 190*scale, 30*scale, CLR_ORANGE, "QuitToLogin", 1.3f*scale, 0 ) )
	{
		window_setMode( WDW_QUIT, WINDOW_DOCKED );
		e->logout_login = 1;
		commSendQuitGame(0);
		return 1;
	}

	if( D_MOUSEHIT == drawStdButton( x + wd/2, y + 55*scale + PIX3*scale, z, 190*scale, 30*scale, CLR_RED, "QuitToCharacterSelect", 1.1f*scale, 0 ) )
	{
		window_setMode( WDW_QUIT, WINDOW_DOCKED );
		e->logout_login = 2;
		commSendQuitGame(0);
		return 1;
	}

  	if( D_MOUSEHIT == drawStdButton( x + wd/2, y + 95*scale - PIX3*scale, z, 190*scale, 30*scale, CLR_DARK_RED,  "QuitToDesktop", 1.3f*scale, 0 ) )
	{
		window_setMode( WDW_QUIT, WINDOW_DOCKED );
 		e->logout_login = 0;
		commSendQuitGame(0);
	}
	return 0;
}
