/***************************************************************************
 *     Copyright (c) 2000-2005, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * $Workfile: uiAnimateCharacter.c $
 * 
 ***************************************************************************/

#include "utils.h"
#include "earray.h"
#include "player.h"
#include "cmdgame.h"
#include "costume.h"
#include "entPlayer.h"
#include "entclient.h"
#include "costume_client.h"
#include "character_base.h"
#include "seq.h"
#include "seqstate.h"
#include "entity.h"

#include "gameData/costume_data.h"
#include "language/langClientUtil.h"

#include "uiGame.h"
#include "uiUtil.h"
#include "uiInput.h"
#include "uiAvatar.h"
#include "uiTailor.h"
#include "uiCostume.h"
#include "uiUtilGame.h"
#include "uiUtilMenu.h"  
#include "uiWindows.h"
#include "uiSupercostume.h"
#include "uiEditText.h"
#include "uiNet.h"

#include "ttFontUtil.h"
#include "sprite_base.h" 
#include "sprite_text.h"
#include "sprite_font.h"
#include "tex.h"

#include "uiListView.h"
#include "uiClipper.h"
#include "uiTailor.h"

#include "uiComboBox.h"

#include "sound.h"
#include "gfxDebug.h"
#include "textureatlas.h"
#include "seqgraphics.h"

typedef struct CCDemoAnim
{
	char *	name;
	char	xlatedName[256];
	char *	animBits[4];
}CCDemoAnim;

CCDemoAnim gCCAnimList[] = 
{
	"AnimNone",			"",		{"PROFILE","EMOTE",0,0},		// default
		"AnimYes",			"",		{"EMOTEYES",0,0,0},
		"AnimNo",			"",		{"EMOTENO",0,0,0},
		"AnimLaugh",		"",		{"LAUGH","LOW","EMOTE",0},
		"AnimArrrgh",		"",		{"ANGRY","SAD","MID","EMOTE"},
		"AnimFancyBow",		"",		{"BOW","GREETING","HIGH","EMOTE"},
		"AnimVictory",		"",		{"CHEER","FIERCE","HIGH","EMOTE"},
		"AnimSalute",		"",		{"SALUTE","HIGH","EMOTE",0},
		"AnimAtEase",		"",		{"SALUTE","HIGH","LOW","EMOTE"},
		"AnimJumpingJacks",	"",		{"TRAIN","LOW","STRONG","EMOTE"},
		"AnimWarmUp",		"",		{"KUNGFU","TRAIN","STRONG","EMOTE"},
		"AnimOverHere",		"",		{"EMOTEWAVE",0,0,0},
		"AnimDance",		"",		{"DANCE","LOW","EMOTE",0},
		"AnimFlex1",		"",		{"FLEX","HIGH","EMOTE",0},
		"AnimFlex2",		"",		{"FLEX","MID","EMOTE",0},
		"AnimFlex3",		"",		{"FLEX","LOW","EMOTE",0},
		"Angry",			"",		{"ANGRY","POSE","LOW", "EMOTE"},
		"Drat",				"",		{"EMOTE", "ANGRY", "RUDE", "SHRUG"},
//		"Shucks",			"",		{"EMOTE","ANGRY","RUDE","SHRUG","LOW"},
		"Winner",			"",		{"EMOTE","CHEER","FLEX",0},
		"Yoga",				"",		{"EMOTE","SIT","MEDITATE","LOW"},
		"Tarzan",			"",		{"EMOTE","COMBAT","TAUNTA","ANGRY"},
		"Taunt",			"",		{"EMOTE","COMBAT","TAUNTC","ANGRY"},
		"TheWave",			"",		{"EMOTE","CHEER","WAVE","HIGH"},
		"Roar",				"",		{"EMOTE","ANGRY","HIGH","FIERCE"},
		"Kneel",			"",		{"EMOTE","KNEEL","KUNGFU","SIT"},
		"Praise",			"",		{"EMOTE","BOWING","LOW"},
//		"Point",			"",		{"EMOTE","POINT"},
		"WaveFist",			"",		{"EMOTE","CHEER","MID"},
		"Grief",			"",		{"EMOTE","SAD","POSE","SHRUG"},
		"Shrug",			"",		{"EMOTE","QUESTION","LOW"},
		"Clap",				"",		{"EMOTE","CHEER","LOW"},
		"Cheer",			"",		{"EMOTE","CHEER","HIGH"},
		"Bow",				"",		{"EMOTE","BOW","GREETING","LOW"},
	{0},
};

void ccSetAnim( int i )
{
	int j;

	seqClearState(playerPtrForShell(1)->seq->state);
	for (j = 0; j < 4; j++)
	{
		if (gCCAnimList[i].animBits[j] > 0)
			seqSetState(playerPtrForShell(1)->seq->state, TRUE, seqGetStateNumberFromName(gCCAnimList[i].animBits[j]));
	}
}

UIBox animationsListViewDisplayItem(UIListView* list, UIPointFloatXYZ rowOrigin, void* userSettings, char* item, int itemIndex)
{
	UIBox box;
	UIPointFloatXYZ pen = rowOrigin;
	UIColumnHeaderIterator columnIterator; 
	int color;

	box.origin.x = rowOrigin.x;
	box.origin.y = rowOrigin.y;

	uiCHIGetIterator(list->header, &columnIterator);

	rowOrigin.z+=1;

	if(item == list->selectedItem)
	{
		color = CLR_WHITE;
	}
	else
	{
		color = CLR_GREY;
	}

	font_color(color, color);

	// Iterate through each of the columns.
	for(;uiCHIGetNextColumn(&columnIterator, list->scale);)
	{
		UIColumnHeader* column = columnIterator.column;
		UIBox clipBox;
		int rgba[4] = {color, color, color, color };

		pen.x = rowOrigin.x + columnIterator.columnStartOffset*list->scale;

		clipBox.origin.x = pen.x;
		clipBox.origin.y = pen.y;
		clipBox.height = 20*list->scale;

		if( columnIterator.columnIndex >= EArrayGetSize(&columnIterator.header->columns )-1 )
			clipBox.width = 1000;
		else
			clipBox.width = columnIterator.currentWidth;

		clipperPushRestrict(&clipBox);

		if(str_wd_notranslate(&game_12, list->scale , list->scale, gCCAnimList[(int)item].xlatedName) <= clipBox.width)
			font(&game_12);
		else
			font(&game_9);

		printBasic(font_grp, pen.x, pen.y+18*list->scale, rowOrigin.z, list->scale, list->scale, 0, gCCAnimList[(int)item].xlatedName, strlen(gCCAnimList[(int)item].xlatedName), rgba );

		clipperPop();
	}

	box.height = 0;
	box.width = list->header->width;

	// Returning the bounding box of the item does not do anything right now.
	// See uiListView.c for more details.
	return box;
}

static UIListView *lvAnimations = 0;

extern int drawCCMainButon(float cx, float cy, float z, float wd, float ht, char *txt);
extern void engine_update();


void ccAnimationMenu()
{
	static int init = 0;
	Entity *e = playerPtr();
	UIBox listViewDrawArea;
	UIPointFloatXYZ pen;

	listViewDrawArea.width = 200;
	listViewDrawArea.height = 500;
	listViewDrawArea.x = 1024 - listViewDrawArea.width - 80;
	listViewDrawArea.y = (768 - listViewDrawArea.height) / 2 - 20;
	pen.x = listViewDrawArea.x;
	pen.y = listViewDrawArea.y;
	pen.z = 5000;
	
	drawBackground( NULL );
	drawMenuTitle( 10, 30, 20, 450, textStd("AnimateCharacter"), 1.f, !init );


	if (!init)
	{
		int i = 0;
		init = 1;

		// anim list
		if (!lvAnimations)
		{
			lvAnimations = uiLVCreate();
			uiLVHAddColumnEx(lvAnimations->header, "Animation", "AnimationsString", listViewDrawArea.width, listViewDrawArea.width, 0);
			uiLVEnableMouseOver(lvAnimations, 1);
			uiLVEnableSelection(lvAnimations, 1);
			lvAnimations->displayItem = animationsListViewDisplayItem;
		}
		uiLVClearEx(lvAnimations, 0);

		while(gCCAnimList[i].name)
		{
			msPrintf(menuMessages, gCCAnimList[i].xlatedName, gCCAnimList[i].name);
			uiLVAddItem(lvAnimations, (void *)i);
			++i;
		}
	}

	drawMenuFrame( 12, listViewDrawArea.x-12, listViewDrawArea.y-12, 8, listViewDrawArea.width+24, listViewDrawArea.height+24, CLR_WHITE, 0x00000088, FALSE );
	uiLVSetDrawColor(lvAnimations, CLR_GREY);
	uiLVHSetDrawColor(lvAnimations->header, CLR_BLUE);
	uiLVSetHeight(lvAnimations, listViewDrawArea.height);
	uiLVHSetWidth(lvAnimations->header, listViewDrawArea.width);
	uiLVSetScale( lvAnimations, 1.f );
	clipperPushRestrict(&listViewDrawArea);
	uiLVDisplay(lvAnimations, pen);
	clipperPop();

	// Handle list view input.
	uiLVHandleInput(lvAnimations, pen);


	{
		int fullDrawHeight = uiLVGetFullDrawHeight(lvAnimations);
		int currentHeight = uiLVGetHeight(lvAnimations);
		if(fullDrawHeight > currentHeight)
		{
			doScrollBar( &lvAnimations->verticalScrollBar, currentHeight-12, fullDrawHeight, listViewDrawArea.x+listViewDrawArea.width-6, listViewDrawArea.y+5, pen.z+20 );
			lvAnimations->scrollOffset = lvAnimations->verticalScrollBar.offset;
		}
		else
		{
			lvAnimations->scrollOffset = 0;
		}
	}
	ccSetAnim( (int)lvAnimations->selectedItem );


	moveAvatar(AVATAR_E3SCREEN);

	toggle_3d_game_modes(SHOW_TEMPLATE);

	if ( drawNextButton( 40,  740, 10, 1.f, 0, 0, 0, "BackString" ))
	{
		sndPlay( "back", SOUND_GAME );
		start_menu(MENU_COSTUME);
		init = 0;
	}

	if ( drawNextButton( 980, 740, 10, 1.f, 1, 0, 0, "NextString" ))
	{
		sndPlay( "next", SOUND_GAME );
		start_menu(MENU_SAVECOSTUME);
		init = 0;
	}

	if (drawCCMainButon(512, 740, 10, 200, 40, "ScreenShot") )
	{	
		//AtlasTex * top;
		//top = atlasLoadTexture("background_skyline_alphacut_tintable.tga");
		//display_sprite( top, 0, 0, 100, 1.f, 1.f, CLR_WHITE );

		//moveAvatar(AVATAR_KOREAN_COSTUMECREATOR_SCREEN);

		gfxScreenDumpEx( "screenshots", 0, NULL, 0, "jpg" );
		init = 0;
	}
		
}


/* End of File */









