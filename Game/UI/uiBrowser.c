/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "wdwbase.h"
#include "utils.h"

#include "uiWindows.h"
#include "uiUtil.h"
#include "uiUtilGame.h"
#include "uiScrollbar.h"
#include "uiSMFView.h"
#include "clientcomm.h"
#include "entVarUpdate.h"
#include "textureatlas.h"
#include "sprite_base.h"
#include "player.h"
#include "entity.h"
#include "character_base.h"
#include "entPlayer.h"
#include "cmdgame.h"

static SMFView *s_pview;
static StuffBuff s_sb;

/**********************************************************************func*
 * browserInit
 *
 */
static void browserInit(void)
{
	if(s_pview==NULL)
	{
		s_pview = smfview_Create(WDW_BROWSER);

		// Sets the location within the window of the text region
		smfview_SetLocation(s_pview, PIX3, PIX3+R10, 0);
		smfview_SetText(s_pview, "");

		initStuffBuff(&s_sb, 1024);
	}
}

/**********************************************************************func*
 * browserOpen
 *
 */
void browserOpen(void)
{
	window_setMode(WDW_BROWSER, WINDOW_GROWING);
}

/**********************************************************************func*
 * browserClose
 *
 */
void browserClose(void)
{
	window_setMode(WDW_BROWSER, WINDOW_SHRINKING);
}

/**********************************************************************func*
 * browserSetText
 *
 */
void browserSetText(char *pch)
{
	browserInit();

	clearStuffBuff(&s_sb);
	addSingleStringToStuffBuff(&s_sb, pch);

	smfview_Reparse(s_pview);
	smfview_SetText(s_pview, s_sb.buff);
}

/**********************************************************************func*
 * browserWindow
 *
 */
int browserWindow(void)
{
	float x, y, z, wd, ht, sc;
	int color, bcolor;
	AtlasTex *logo;
	Entity *e = playerPtr();
	UISkin skin;

	if(!window_getDims(WDW_BROWSER, &x, &y, &z, &wd, &ht, &sc, &color, &bcolor))
		return 0;

	// Matt H told me to make it opaque
	drawFrame(PIX3, R10, x, y, z, wd, ht, sc, color, bcolor|0xff);

	browserInit();

	set_scissor(1);
	scissor_dims( x+PIX3*sc, y+PIX3*sc, wd - PIX3*2*sc, ht - PIX3*2*sc );

	skin = skinFromMapAndEnt(e);

	if (skin == UISKIN_PRAETORIANS)
	{
		logo = atlasLoadTexture("Kiosk_PraetoriaLogo.tga");
	}
	else if (skin == UISKIN_VILLAINS)
	{
		logo = atlasLoadTexture("Kiosk_RogueIslesLogo.tga");
	}
	else
	{
		logo = atlasLoadTexture("Kiosk_ParagonLogo.tga");
	}
	display_sprite(logo, x + wd/2 - logo->width*sc/2, y+ht/2-logo->height*sc/2, z, sc, sc, 0xffffff44);
	set_scissor(0);

	// Sets the size of the region within the window of the text region
	// In this case, it resizes with the window
 	smfview_SetSize(s_pview, wd-(PIX3*2+5)*sc, ht-(PIX3+R10)*2*sc);
	smfview_Draw(s_pview);

	
	return 0;
}

/* End of File */

