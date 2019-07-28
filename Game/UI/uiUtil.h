#ifndef UIUTIL_H
#define UIUTIL_H


// This file is contains utility and helper functions for ui stuff
//------------------------------------------------------------------------------------

#include "utils.h"
#include "uiInclude.h"

#define PIX1	1
#define PIX2	2
#define PIX3	3
#define PIX4	4
#define R4		4
#define R6		6
#define R7		7
#define R10		10
#define R12		12
#define R16		16
#define R22		22
#define R25		25
#define R27		27

#define DEFAULT_RGBA    ( 0xffffffff )
#define LIT_RGBA        ( 0xffff00ff )
#define SELECTED_RGBA   ( 0xffffff40 )
#define CLR_GREYED      ( 0xa0a0a0ff )
#define CLR_GREY        ( 0xa0a0a0ff )
#define CLR_GREYED_RED  ( 0xffa0a0ff )
#define CLR_YELLOW      ( 0xffff00ff )
#define CLR_ORANGE      ( 0xffa500ff )
#define CLR_WHITE       ( 0xffffffff )
#define CLR_BLUE        ( 0x5050ffff )
#define CLR_GREEN       ( 0x00ff00ff )
#define CLR_RED         ( 0xff0000ff )
#define CLR_DARK_RED	( 0x990000ff )
#define CLR_BLACK       ( 0x000000ff )
#define CLR_NAVY		( 0x111155ff )
#define CLR_MAGENTA		( 0xff00ffff )
#define CLR_CYAN		( 0x00ffffff )
#define CLR_TURQUOISE	( 0x40e0d0ff )
#define DARK_GREY       ( 0x444444ff )
#define CLR_PARAGON     ( 0x00deffff )
#define CLR_VILLAGON	( 0xfca91aff )
#define CLR_ROGUEAGON	( 0x6e6e6eff )
#define CLR_BTN_HERO	( 0x2a77ffff )
#define CLR_BTN_VILLAIN	( 0xff2222ff )
#define CLR_BTN_ROGUE	( 0x4e4e4eff )

#define CLR_TXT_LROGUE	0xe6daa4ff
#define CLR_TXT_ROGUE	0xe8cc4cff

#define CLR_TXT_GREY	0x8fa1b8ff
#define CLR_TXT_BLUE	0x90e2fcff
#define CLR_TXT_RED		0xfce290ff
#define CLR_TXT_GREEN	0x88ffaaff
#define CLR_TXT_LRED	0xff88aaff
//#define CLR_TXT_NAVY	0x6666bbff
//#define CLR_TXT_LNAVY	0x8888ccff
//
#define CLR_FRAME_LRED	0xa00008ff
#define CLR_BASE_LRED	0xa00008ff
#define CLR_CFILL_LRED	0xa0000868
//
#define CLR_FRAME_RED	0xff944fff
#define CLR_BASE_RED	0x580008ff
#define CLR_CFILL_RED	0x58000868
//
#define CLR_FRAME_BLUE	0x4f94ffff
#define CLR_BASE_BLUE	0x0061d9ff
#define CLR_CFILL_BLUE	0x0061d968
//
#define CLR_FRAME_GREEN	0x00d919ff
#define CLR_BASE_GREEN	0x00d919ff
#define CLR_CFILL_GREEN	0x00d91968
//
#define CLR_FRAME_GREY	0x444444ff
#define CLR_BASE_GREY	0x1e1e1eff
#define CLR_CFILL_GREY	0x6e6e6e68
//
#define CLR_FRAME_ROGUE		0xaaaa00ff
#define CLR_FRAME_LROGUE	0xdddd00ff
#define CLR_BASE_ROGUE		0x888800ff
#define CLR_BASE_LROGUE		0xbbbb00ff
#define CLR_HIGH_ROGUE		0x3b3b3b88
#define CLR_HIGH_LROGUE		0x505050bf

#define CLR_HIGH		 0xffffff88
#define CLR_HIGH_SELECT  0xa4f042bf
//
#define CLR_CBLOW	0x00000093
#define CLR_CRDS	0x000000bf
#define CLR_SHADOW	0x0000007a

#define CLR_CONSTRUCT(r, g, b, a) (r&0xff)<<24 | (g&0xff)<<16 | (b&0xff)<<8 | (a&0xff)
#define CLR_CONSTRUCT_EX(r, g, b, aPercentage) (r&0xff)<<24 | (g&0xff)<<16 | (b&0xff)<<8 | ((int)(255*aPercentage)&0xff)

#define CLR_ONLINE_ITEM		CLR_CONSTRUCT(99, 225, 252, 255)
#define CLR_OFFLINE_ITEM	CLR_CONSTRUCT(120, 120, 120, 255)
#define CLR_DARK_ITEM		CLR_CONSTRUCT(255, 60, 180, 255)
#define CLR_SELECTED_ITEM	CLR_WHITE

#define CLR_SELECTION_FOREGROUND CLR_CONSTRUCT_EX(123, 252, 164, 1.0)
#define CLR_SELECTION_BACKGROUND CLR_CONSTRUCT_EX(123, 252, 164, 0.33)

#define CLR_MOUSEOVER_FOREGROUND CLR_CONSTRUCT_EX(123, 252, 164, 0.25)
#define CLR_MOUSEOVER_BACKGROUND CLR_CONSTRUCT_EX(123, 252, 164, 0.12)

#define CLR_NORMAL_FOREGROUND CLR_CONSTRUCT_EX(255, 255, 255, 0.25)
#define CLR_NORMAL_BACKGROUND CLR_CONSTRUCT_EX(255, 255, 255, 0.12)

#define CLR_DISABLED_FOREGROUND CLR_CONSTRUCT_EX(127, 127, 127, 0.25)
#define CLR_DISABLED_BACKGROUND CLR_CONSTRUCT_EX(127, 127, 127, 0.12)

#define CLR_GOLD_FOREGROUND CLR_CONSTRUCT_EX(255, 255, 0, 0.25)
#define CLR_ORANGE_FOREGROUND CLR_CONSTRUCT_EX(255, 100, 0, 0.25)
#define CLR_PURPLE_FOREGROUND CLR_CONSTRUCT_EX(118, 0, 147, 0.25)

#define CLR_TAB_INACTIVE	0xaaaaaaff

#define CLR_MISSIONYELLOW	0xdddd00ff

#define CLR_MM_SEARCH_HIGHLIGHT		0xffa500ff
#define CLR_MM_SEARCH_HALLOFFAME	0xcff69bff
#define CLR_MM_SEARCH_DEVCHOICE		0x26acf9ff
#define CLR_MM_GUESTAUTHOR			0xe8ff79ff



#define CLR_MM_TEXT 0xa2e7eeff
#define CLR_MM_TEXT_DARK 0x5db6b4ff
#define CLR_MM_TEXT_ERROR 0xff9501ff
#define CLR_MM_TEXT_ERROR_LIT 0xff9501ff

#define CLR_MM_BACK_ALTERNATE 0x426755cc
#define CLR_MM_TEXT_BACK 0x0c414fff
#define CLR_MM_TEXT_BACK_LIT 0x1c515fff
#define CLR_MM_TEXT_BACK_MOUSEOVER 0x2c616fff

#define CLR_MM_FRAME_BACK 0x2a4d49cc
#define CLR_MM_TITLE_OPEN 0xbff897ff
#define CLR_MM_TITLE_CLOSED 0xb6f7f7ff
#define CLR_MM_TITLE_LIT 0xe6febaff

#define CLR_MM_BAR_OPEN 0x85cbcbff
#define CLR_MM_BAR_CLOSED 0x6a9a9aff
#define CLR_MM_BAR_MOUSEOVER 0xa5ebfbff

#define CLR_MM_BUTTON 0x4f7065ff
#define CLR_MM_BUTTON_LIT 0x96ce83ff

#define CLR_MM_SS_ARROW 0x4A7A6FFF

#define CLR_MM_BUTTON_ERROR 0x7b2609ff
#define CLR_MM_BUTTON_ERROR_LIT 0xb73e13ff
#define CLR_MM_BUTTON_ERROR_TEXT 0xff9058ff
#define CLR_MM_BUTTON_ERROR_TEXT_LIT 0xffff88ff

#define CLR_MM_BACK_DARK  0x0f1817ff
#define CLR_MM_BACK_LIGHT 0x233a34ff

#define CLR_MM_HELP_TEXT 0xc0d6caff
#define CLR_MM_HELP_LIGHT 0x7eb7ecff
#define CLR_MM_HELP_DARK 0x384379ff
#define CLR_MM_HELP_MID 0x435191cff

#define CLR_MM_ERROR_DARK 0x692500ff
#define CLR_MM_ERROR_MID 0xc74223ff


#define CLR_AH_TEXT 0xccccccff
#define CLR_AH_TEXT_LIT 0xffffffff
#define CLR_AH_TEXT_SELECTED 0xffffaaff
#define CLR_AH_TEXT_DARK 0x888888ff;


#define CLR_AH_BACKGROUND	0x333333ff
#define CLR_AH_BACKGROUND_LIT 0x555555ff
#define CLR_AH_BACKGROUND_SELECTED 0x666655ff

#define CLR_AH_BUTTON 0x555555ff
#define CLR_AH_BUTTON_LIT 0x777777ff


enum
{
	D_NONE,
	D_MOUSEOVER,
	D_MOUSEDOWN,
	D_MOUSEHIT,
	D_MOUSELOCKEDHIT,
	D_MOUSEDRAG,
	D_MOUSEDOUBLEHIT
};


enum
{
	BUTTON_LOCKED	= 1 << 0,
	BUTTON_CHECKED	= 1 << 1,
	BUTTON_DIMMED	= 1 << 2,
	BUTTON_ULTRA	= 1 << 3,
};

void build_cboxxy( AtlasTex *spr, CBox *c, float xsc, float ysc, int xp, int yp );
void brightenColor( int *color, int amount );
void wave_vec_from_timer( int timer, int period, float *vec );

int drawStdButton( float x, float y, float z, float wd, float ht, int color, const char *txt, float txtSc, int flags );
#define drawStdButtonTopLeft(x, y, z, wd, ht, color, txt, txtSc, flags) drawStdButton((x)+((wd)/2), (y)+((ht)/2), (z), (wd), (ht), (color), (txt), (txtSc), (flags))
void drawStdButtonMeat( float x, float y, float z, float wd, float ht, int color );

typedef struct UIElemColors
{
	int base, text, text2, frame, fill, highlight, scrollbar;
} UIElemColors;

typedef struct UIColors
{
	UIElemColors standard, selected, clicked, unselectable;
} UIColors;

extern UIColors uiColors;

typedef struct Entity Entity;
typedef enum UISkin UISkin;

UISkin skinFromMapAndEnt(Entity *e);

#endif