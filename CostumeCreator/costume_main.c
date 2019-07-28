#include "game.h"
#include "timing.h"
#include "SuperAssert.h"
#include "win_init.h"
#include "fpmacros.h"
#include "clienterror.h"
#include "FolderCache.h"
#include "autoResumeInfo.h"
#include "cmdgame.h"
#include "textparser.h"
#include "uiAutomap.h"
#include "uiLogin.h"
#include "initClient.h"
#include "uiGame.h"
#include "utils.h"
#include "seqstate.h"
#include "AppLocale.h"

int gPlayerNumber;
extern int gKoreanCostumeCreator;
extern int gCostumeCreator;
extern int gE3CostumeCreator;
extern int g_force_production_mode;

extern HWND	hwnd;

extern char coh_registry_name[];

// this will change for our different compiles
// used to mark the costume creator origin in case of a leak
/*
static char *e3_booth_origin = "E3_BOOTH_ORIGIN_Microsoft";
static char *e3_booth_origin = "E3_BOOTH_ORIGIN_Nvidia";
static char *e3_booth_origin = "E3_BOOTH_ORIGIN_Intel";
static char *e3_booth_origin = "E3_BOOTH_ORIGIN_InsideE3";
*/
static char *e3_booth_origin = "E3_BOOTH_ORIGIN_NCsoft";
static char *e3_booth_origin2 = "CCVER_1"; // in case the other one is hexedited
// CCVER_1 = NCsoft
// CCVER_2 = Microsoft For Gaming
// CCVER_3 = Nvidia
// CCVER_4 = Intel
// CCVER_5 = Inside E3 Media Party

static int runWindowed = 0;

void game_beforeRegisterWinClass(int argc, char **argv)
{
	int i;

	gCostumeCreator = 1;
	gE3CostumeCreator = 0;
	gKoreanCostumeCreator = 1;
	//g_force_production_mode = 1;
	

	if (gE3CostumeCreator) {
		game_state.cov = 1;
	}
	if (gKoreanCostumeCreator)
	{
		strcpy( coh_registry_name, "KRCOH" );
		game_state.cov = 0;
		locSetIDInRegistry(3);
	}
	for (i = 0; i < argc; i++)
	{
		if (stricmp(argv[i], "-windowed")==0)
			runWindowed = 1;
	}
}


void doNothingErrorCallback(char*err){}


int main(int argc, char **argv)
{
	char buffer[1024];
	int timer = timerAlloc();


	setAssertMode(ASSERTMODE_ERRORREPORT);	// we'll get reset better later

	SET_FP_CONTROL_WORD_DEFAULT

	FatalErrorfSetCallback(clientFatalErrorfCallback);
	ErrorfSetCallback(doNothingErrorCallback);

	if (gE3CostumeCreator)
		game_state.cov = 1;

	game_beforeFolderCacheIgnore(timer, argc, argv);

	// dev file ignore
	FolderCacheAddIgnorePrefix("d_");

	parseArgs0(argc, argv);

	game_beforeParseArgs(0);

	if (gE3CostumeCreator) {
		int i;
		game_state.cov = 1;
		for (i=0; i<argc; i++)
			if (strstri(argv[i], "noglow"))
				game_state.nohdr = 1;
	}

	seqLoadStateBits();

	game_loadSoundsTricksFonts(argc, argv);

	game_loadData(1);

	// ensure the e3 booth origin gets included by using it
	strcpy(buffer, e3_booth_origin);
	strcpy(buffer, e3_booth_origin2);

	if (gE3CostumeCreator)
		game_state.cov = 1;

#if 1
	// splash screen
	game_state.screen_x = 1280;
	game_state.screen_y = 1024;
	game_state.screen_x_pos = 0;
	game_state.screen_y_pos = 0;
	game_state.fullscreen = !runWindowed;
#endif

	game_initWindow(0);
	SetWindowText(hwnd, "Character Creator");

	game_beforeLoop(1, timer);

	gPlayerNumber = 0;

	doCreatePlayer(0);
	start_menu( MENU_CCMAIN );

	return game_mainLoop(timer);
}
