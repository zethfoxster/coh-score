
#include "uiNet.h"
#include "uiGame.h"
#include "uiUtil.h"
#include "uiTray.h"
#include "uiInput.h"
#include "uiDialog.h"
#include "uiPowers.h"
#include "uiEditText.h"
#include "uiUtilMenu.h"
#include "uiScrollBar.h"
#include "uiLevelPower.h"
#include "uiCombineSpec.h"

#include "sprite_base.h"
#include "sprite_text.h"
#include "sprite_font.h"
#include "textureatlas.h"

#include "earray.h"
#include "player.h"
#include "powers.h"
#include "origins.h"
#include "classes.h"
#include "cmdgame.h"
#include "win_init.h"
#include "entVarUpdate.h"
#include "character_base.h"
#include "character_level.h"
#include "clientcomm.h"
#include "contactCommon.h"
#include "sound.h"

#include "sm_parser.h"
#include "smf_parse.h"
#include "smf_format.h"
#include "smf_render.h"
#include "entity.h"
#include "MessageStoreUtil.h"


static TextAttribs gTextAttr =
{
	/* piBold        */  (int *)0,
	/* piItalic      */  (int *)0,
	/* piColor       */  (int *)0x48ff48ff,
	/* piColor2      */  (int *)0,
	/* piScale       */  (int *)(int)(1.f*SMF_FONT_SCALE),
	/* piFace        */  (int *)&game_12,
	/* piFont        */  (int *)0,
	/* piAnchor      */  (int *)0,
	/* piLink        */  (int *)0,
	/* piLinkBG      */  (int *)0,
	/* piLinkHover   */  (int *)0,
	/* piLinkHoverBG */  (int *)0,
	/* piOutline     */  (int *)0,
	/* piShadow      */  (int *)0,
};


static NewPower gCurNewPower = {0};

#define FRAME_HT    520
#define POW_SPACER   50


static char *gCurrHelpTxt = 0;
static AtlasTex * gCurrHelpIcon = 0;
static int gReparseHelpText = 0;

//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
//
//
static void powerSetHelp( AtlasTex * icon, char * txt )
{
 	gCurrHelpTxt = txt;
    gCurrHelpIcon = icon;
	gReparseHelpText = 1;
}

//
//
static void displayPowerLevelHelp(void)
{
	// check here for box collisons
	CBox box;

   	BuildCBox( &box, SKILL_FRAME_SPACE, 70, SKILL_FRAME_WIDTH, FRAME_HT );
	if( !mouseCollision( &box ) )
	{
		BuildCBox( &box, DEFAULT_SCRN_WD/2 - SKILL_FRAME_WIDTH/2, 70, SKILL_FRAME_WIDTH, FRAME_HT );
		if( !mouseCollision( &box ) )
		{
			BuildCBox( &box, DEFAULT_SCRN_WD - SKILL_FRAME_WIDTH - SKILL_FRAME_SPACE, 70, SKILL_FRAME_WIDTH, FRAME_HT );
			if( !mouseCollision( &box ) )
			{
				gCurrHelpTxt = 0;
				gCurrHelpIcon = 0;
			}
		}
	}

	if( gCurrHelpTxt )
	{
		int w, h;
		static SMFBlock sm;

		windowClientSizeThisFrame( &w, &h );

		if( gCurrHelpIcon && stricmp( gCurrHelpIcon->name, "white" ) != 0 )
			display_sprite( gCurrHelpIcon, 120 - gCurrHelpIcon->width/2, 645 - gCurrHelpIcon->height/2, 16, 1.f, 1.f, CLR_WHITE );

		smf_ParseAndDisplay(&sm, textStd(gCurrHelpTxt), 160, 615, 20, 780, 0, gReparseHelpText, false, &gTextAttr, NULL);
		gReparseHelpText = 0;
	}
}


static int skillLevelSelector( NewPower *np, Character *pchar, int x, int y, int category, char * name, int *pSet, int *pPow )
{
	Entity *e = playerPtr();
	int i, j, displayedText = FALSE;
	PowerSet *pset;
	int oldY = y;

	font( &title_12 );
	font_color( CLR_YELLOW, CLR_ORANGE );

	// find the power set corresponding to this category
	// The zeroth slot in power sets is for temporary powers, they don't show up here.
	for( i = 1; i < EArrayGetSize(&pchar->ppPowerSets); i++)
	{
		pset = pchar->ppPowerSets[i];

		if( pset->psetBase->pcatParent == pchar->pclass->pcat[category] && stricmp( pset->psetBase->pchName, name ) == 0 )
		{
			BasePowerSet *psetBase = pset->psetBase;

			font_color( CLR_YELLOW, CLR_ORANGE );
 			prnt( x, y, 50, 1.2f, 1.2f, psetBase->pchDisplayName );
			y += 35;

			// loop though powers
			for( j = 0; j < EArrayGetSize( &psetBase->ppPowers ); j++ )
			{
				int mouse = FALSE;

				if ( character_OwnsPower( pchar, psetBase->ppPowers[j] ) )
				{
   					mouse = drawCheckBar( x, y, 50, 1.f, SKILL_FRAME_WIDTH-45, psetBase->ppPowers[j]->pchDisplayName, 1, 0, 0, 0, NULL );
				}
				else if ( np->basePow == psetBase->ppPowers[j] )
				{
					mouse = drawCheckBar( x, y, 50, 1.f, SKILL_FRAME_WIDTH-45, psetBase->ppPowers[j]->pchDisplayName, 1, 1, 0, 0, NULL );
				}
				else
				{
					// check availability
					int available = character_SystemCanBuyPower( pchar, kPowerSystem_Skills );
					if(powerset_BasePowerAvailableByIdx( pset, j, pchar->iWisdomLevel ) > 0
						|| !character_IsAllowedToHavePower(pchar, psetBase->ppPowers[j]))
					{
						available = FALSE;
					}

					mouse = drawCheckBar( x, y, 50, 1.f, SKILL_FRAME_WIDTH-45, psetBase->ppPowers[j]->pchDisplayName, 0, available, 0, 0, NULL );
				}

				if(mouse)
				{

					powerSetHelp( atlasLoadTexture(psetBase->ppPowers[j]->pchIconName), psetBase->ppPowers[j]->pchDisplayHelp );

					if( mouse == D_MOUSEHIT && !character_OwnsPower( pchar, psetBase->ppPowers[j] ))
					{
						sndPlay( "powerselect2", SOUND_GAME );
						np->pSet = pset;
						np->basePow = psetBase->ppPowers[j];

						pPow[kCategory_Pool] = -1;
						pSet[kCategory_Pool] = -1;
						pPow[kCategory_Epic] = -1;
						pSet[kCategory_Epic] = -1;
						np->baseSet = 0;
					}
				}

	 			y += 40;
			} // end for every power
		} // end if power set is in category
	} // end for every power set

	return (y-oldY);
}

//------------------------------------------------------------------------------------------------------
// I'm in charge here! /////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------------
void levelSkillMenu()
{
	Entity *e = playerPtr();
	static int iShowedPoolMessage = FALSE;
	static int init = FALSE, ht;
	static ScrollBar sb[4] = { 0 };
 	int y;
	bool bFakedLevel = false;
	bool bCanBuy = character_SystemCanBuyPower(e->pchar, kPowerSystem_Skills);

	// HACK: Pretend that we're already a level higher to get the right options
	if(character_SystemCalcExperienceLevel(e->pchar, kPowerSystem_Skills) > e->pchar->iWisdomLevel)
	{
		e->pchar->iWisdomLevel++;
		bFakedLevel = true;
	}

	drawBackground( NULL );
	toggle_3d_game_modes( SHOW_NONE );
	drawMenuTitle( 10, 30, 20, 670, "ChooseANewSkill", 1.f, !init );

	if( !init )
	{
		init = TRUE;
		gCurNewPower.basePow = NULL;
		gCurNewPower.pSet = NULL;
	}

	set_scissor( TRUE );
  	scissor_dims( 0, 74, 1024, FRAME_HT - 8 );

	// do power selectors - 
	y = 100 - sb[0].offset;
   	ht = skillLevelSelector( &gCurNewPower, e->pchar, FRAME_X(0)+30, y, kCategory_Skill, "Science", gCurrentPowerSet, gCurrentPower );
 	doScrollBar( &sb[0], FRAME_HT - 34, ht, FRAME_X(0)+SKILL_FRAME_WIDTH, 99, 20 );

	y = 100 - sb[1].offset;
	ht = skillLevelSelector( &gCurNewPower, e->pchar,FRAME_X(1)+30, y, kCategory_Skill, "Academia", gCurrentPowerSet, gCurrentPower );
	doScrollBar( &sb[1], FRAME_HT - 34, ht, FRAME_X(1)+SKILL_FRAME_WIDTH, 99, 20 );

	y = 100 - sb[2].offset;
  	ht = skillLevelSelector( &gCurNewPower, e->pchar,FRAME_X(2)+30, y, kCategory_Skill, "Detective", gCurrentPowerSet, gCurrentPower );
 	doScrollBar( &sb[2], FRAME_HT - 34, ht, FRAME_X(2)+SKILL_FRAME_WIDTH, 99, 20 );

 	y = 100 - sb[3].offset;
 	ht = skillLevelSelector( &gCurNewPower, e->pchar,FRAME_X(3)+30, y, kCategory_Skill, "Inherent", gCurrentPowerSet, gCurrentPower);
	ht += skillLevelSelector( &gCurNewPower, e->pchar,FRAME_X(3)+30, y+ht, kCategory_Skill, "Fundamentals", gCurrentPowerSet, gCurrentPower);
	ht += skillLevelSelector( &gCurNewPower, e->pchar,FRAME_X(3)+30, y+ht, kCategory_Skill, "Invention", gCurrentPowerSet, gCurrentPower);
	doScrollBar( &sb[3], FRAME_HT - 34, ht, FRAME_X(3)+SKILL_FRAME_WIDTH, 99, 20 );
	set_scissor( FALSE );
	
	// the frames
 	drawMenuFrame( R12, FRAME_X(0), 70, 10, SKILL_FRAME_WIDTH, FRAME_HT, CLR_WHITE, 0x00000088, 0 ); // left
  	drawMenuFrame( R12, FRAME_X(1), 70, 10, SKILL_FRAME_WIDTH, FRAME_HT, CLR_WHITE, 0x00000088, 0 ); // middle
	drawMenuFrame( R12, FRAME_X(2), 70, 10, SKILL_FRAME_WIDTH, FRAME_HT, CLR_WHITE, 0x00000088, 0 ); // right
 	drawMenuFrame( R12, FRAME_X(3), 70, 10, SKILL_FRAME_WIDTH, FRAME_HT, CLR_WHITE, 0x00000088, 0 ); // left

	// help text
	displayPowerHelpBackground();
	displayPowerLevelHelp();

	if ( drawNextButton( 40,  740, 10, 1.f, 0, 0, 0, "CancelString" ))
	{
		gCurNewPower.pSet = NULL;
		gCurNewPower.basePow = NULL;

		// Continue the dialog with the NPC
		START_INPUT_PACKET(pak, CLIENTINP_CONTACT_DIALOG_RESPONSE);
		pktSendBitsPack(pak, 1, CONTACTLINK_BYE);
		END_INPUT_PACKET

		start_menu( MENU_GAME );
	}

	if ( drawNextButton( 980, 740, 10, 1.f, 1, gCurNewPower.basePow!=NULL?FALSE:TRUE, 0, "FinishString" ))
	{
		if(gCurNewPower.basePow!=NULL)
		{
			Power *ppow;
			int tray = 0, slot = 0, found = 0;

			if( !gCurNewPower.baseSet ) // picked a power from primary or secondary
			{
				// update player's powers now
				level_buyNewPower( gCurNewPower.basePow, kPowerSystem_Skills );
				ppow = character_BuyPower( e->pchar, gCurNewPower.pSet, gCurNewPower.basePow );
			}
			else // picked a power pool power
			{
				PowerSet *pset;
				Power *power;

				// send now power to the server or something
				pset  = character_BuyPowerSet( e->pchar, gCurNewPower.baseSet );
				power = character_BuyPower(  e->pchar, pset, gCurNewPower.basePow );

				level_buyNewPower( power->ppowBase, kPowerSystem_Skills );
				ppow = character_BuyPower( e->pchar, pset, power->ppowBase );
			}

			// add new power to tray
			if( ppow->ppowBase->eType != kPowerType_Auto )
			{
				TrayObj to;
				buildPowerTrayObj( &to, ppow->psetParent->idx, ppow->idx, 0 );
				tray_AddObj( &to );
			}

			// TODO: This doesn't seem like the right place for this.
			//    Shouldn't there be some initialization done each time we
			//    enter this menu?
			gCurNewPower.basePow = NULL;
			gCurNewPower.baseSet = NULL;
			gCurrentPower[kCategory_Pool]    = -1;
			gCurrentPowerSet[kCategory_Pool] = -1;
			gCurrentPower[kCategory_Epic]    = -1;
			gCurrentPowerSet[kCategory_Epic] = -1;

			// Continue the dialog with the NPC
			START_INPUT_PACKET(pak, CLIENTINP_CONTACT_DIALOG_RESPONSE);
			pktSendBitsPack(pak, 1, CONTACTLINK_HELLO);
			END_INPUT_PACKET

			// Do this before adding Rest to the list because it properly
			// grants Rest. Do everything else before it.
			level_increaseLevel( kPowerSystem_Skills ); // Make the level increase permanent.

			// HACK: Add rest power to tray
			if( e->pchar->iLevel == 2 )
			{
				TrayObj to;
				BasePowerSet *psetBase = powerdict_GetBasePowerSetByName(&g_PowerDictionary, "inherent", "inherent");
				Power *ppow = character_OwnsPowerByName(e->pchar, psetBase, "Rest");

				if(ppow)
				{
					int ipow, iset;
					power_GetIndices(ppow, &iset, &ipow);
					buildPowerTrayObj(&to, iset, ipow, 0);
					tray_AddObj(&to);
				}
			}

			// Get rid of dialog if it is up.
			dialogRemove("FirstPowerPoolHelp", NULL, NULL);

			//go back to game
			start_menu( MENU_GAME );
		}
	}

	// HACK: stop pretending
	if(bFakedLevel)
	{
		e->pchar->iWisdomLevel--;
	}
}
