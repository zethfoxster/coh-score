#ifndef UIUTILGAME_H
#define UIUTILGAME_H

#include "uiInclude.h"

// This file contains drawing routines for common in game elements
//--------------------------------------------------------------------------------------------------

typedef	struct CapsuleData
{
	int showNumbers;
	int showText;
	int instant;

	int txt_clr1;
	int txt_clr2;
	float curr;
	char * txt;
	unsigned int drawn : 1;	// 0 if this capsule has never been drawn before.
} CapsuleData;

typedef enum FrameStyle
{
	kFrameStyle_Standard = 0,
	kFrameStyle_3D       = 0,
	kFrameStyle_Flat     = 1<<0,
	kFrameStyle_GradientH = 1<<1,
	kFrameStyle_GradientV = 1<<2,
} FrameStyle;

typedef enum ColorStyle
{
	kStyle_Standard,
	kStyle_Architect,
	kStyle_AuctionHouse,
} ColorStyle;

typedef enum TabDir
{
	kTabDir_None,
	kTabDir_Top,
	kTabDir_Left,
	kTabDir_Bottom,
	kTabDir_Right,
} TabDir;

typedef enum TrayBendType
{
	kBend_LR,   // _|
	kBend_UR,	// -|
	kBend_UL,    // |-
	kBend_LL,  // |_
}TrayBendType;

typedef struct FrameColors
{
	//int colors[9];
	int topLeft;
	int topCenter;
	int topRight;
	int midLeft;
	int midCenter;
	int midRight;
	int botLeft;
	int botCenter;
	int botRight;
	int border;
}FrameColors;

typedef enum FrameParts
{
	kFramePart_UpL = 1 << 0,
	kFramePart_UpC = 1 << 1,
	kFramePart_UpR = 1 << 2,
	kFramePart_MidL = 1 << 3,
	kFramePart_MidC = 1 << 4,
	kFramePart_MidR = 1 << 5,
	kFramePart_BotL = 1 << 6,
	kFramePart_BotC = 1 << 7,
	kFramePart_BotR = 1 << 8,
}FrameParts;
#define FRAMEPARTS_ALL (kFramePart_UpL|kFramePart_UpC|kFramePart_UpR\
					|kFramePart_MidL|kFramePart_MidC|kFramePart_MidR\
					|kFramePart_BotL|kFramePart_BotC|kFramePart_BotR)


void drawHorizontalLine( float beginX, float beginY, float width, int z, float sc, int color );
void drawVerticalLine( float beginX, float beginY, float height, int z, float sc, int color );

typedef struct CBox CBox;
typedef struct UIBox UIBox;
void drawBox( CBox * box, int z, int border_color, int interior_color );
void drawOutlinedBox( CBox * box, int z, int border_color, int interior_color , float width );
void drawBoxEx( CBox * box, int z, int border_color, int interior_color, int above, int right, int below, int left );
void drawUiBox( UIBox * box, int z, int border_color );

// Draws a button with the given sprite, returns true if hit
int drawGenericButton( AtlasTex * spr, float x, float y, float z, float scale, int color, int over_color, int available  );
int drawGenericRotatedButton( AtlasTex * spr, float x, float y, float z, float scale, int color, int over_color, int available, float angle );
int drawGenericButtonEx( AtlasTex * spr, float x, float y, float z, float scaleX, float scaleY, int color, int over_color, int available );
int drawTextButton( char *txt, float x, float y, float z, float scale, int color, int over_color,
					int txt_color, int available, UIBox *dims  );
// Draws the conning window and dock buttons
//
void drawButton( float x, float y, float z, float wd, float scale, int color, int bar_color, int isDown,
				char* txt, int txt_color);

// Draws the border around a capsule
//
void drawCapsuleFrame( float x, float y, float z, float scale, float wd, int color, int back_color );

// Draws a capsule (used for health bars and such)
//
void drawCapsule( float x, float y, float z, float wd, float scale, int color, int back_color, int bar_color,
				 float curr, float total, CapsuleData * data );

void drawCapsuleEx( float x, float y, float z, float wd, float scale, int color, int back_color, int shine_color, int bar_color,
				 float curr, float total, CapsuleData * data );

//Draw an actual capsule for debug purposes
void drawSpinningCapsule( F32 radius, F32 length, const Mat4 collMat, int color );
void drawSpinningCircle( F32 radius, const Mat4  mat, int color );

// connecting lines:  a segmented line connecting two arbitrary points in 2d space where the two parts are aligned to the axis-grid with a curved corner between them
void drawConnectingLine( int size, int radius, float x1, float y1, float x2, float y2, float z, int upFromBelowPoint, float sc, int color );
float drawConnectingLineStyle( int size, int radius, float x1, float y1, float x2, float y2, float z, int upFromBelowPoint, float sc, int color, FrameStyle style );

// standard frame builders
//
void drawFrameBox( int size, int radius, CBox * box, float z, int color, int back_color );
void drawFlatFrameBox( int size, int radius, CBox * box, float z, int color, int back_color );
void drawFrame( int size, int radius, float x, float y, float z, float wd, float ht, float sc, int color, int back_color );
void drawFlatFrame( int size, int radius, float x, float y, float z, float wd, float ht, float sc, int color, int back_color );
void drawFlatThreeToneFrame( int size, int radius, float x, float y, float z, float wd, float ht, float sc, int colorLeft, int colorCenter, int colorRight );
void drawFlatMultiToneFrame( int size, int radius, float x, float y, float z, float wd, float ht, float sc, FrameColors colors );
void drawFlatSectionFrame( int size, int radius, float x, float y, float z, float wd, float ht, float sc, int color, int sectionFlags );
void drawSectionFrame( int size, int radius, float x, float y, float z, float wd, float ht, float sc, int color, int back_color, int sectionFlags );
#define drawFlatFrameTopLeft(size, radius, x, y, z, wd, ht, sc, color, back_color) drawFlatFrame(size, radius, (x)+((wd)/2), (y)+((ht)/2), z, wd, ht, sc, color, back_color)
void drawFrameStyle( int size, int radius, float x, float y, float z, float wd, float ht, float sc, int color, int back_color, FrameStyle style );
void drawFrameStyleColor( int size, int radius, float x, float y, float z, float wd, float ht, float sc, FrameColors colors, FrameStyle style, int sectionFlags);
void drawTabbedFlatFrameBox( int size, int radius, CBox * box, float z, float sc, int color, int back_color, TabDir dir );
void drawTabbedFrameStyle( int size, int radius, float x, float y, float z, float wd, float ht, float sc, int color, int back_color, FrameStyle style, TabDir dir );

void drawBustedOutFrame( int style, int size, int radius, float x, float y, float z, float sc, float wd, float mid_ht, float top_ht, float bot_ht, int color, int bcolor, int fallback );


typedef struct UIBox UIBox;

void drawFrameWithBounds(int size, int radius, float x, float y, float z, float wd, float ht, int color, int back_color, UIBox* bounds);
void drawJoinedFrame2Panel( int size, int radius, float x, float y, float z, float sc, float wd, float ht, float ht2,
							int color, int back_color, UIBox* bounds1, UIBox* bounds2);
void drawJoinedFlatFrame2Panel( int size, int radius, float x, float y, float z, float sc, float wd, float ht, float ht2,
								int color, int back_color, UIBox* bounds1, UIBox* bounds2);
void drawJoinedFrame2PanelStyle( int size, int radius, float x, float y, float z, float sc, float wd, float ht, float ht2,
								 int color, int back_color, UIBox* bounds1, UIBox* bounds2, FrameStyle style);

// builds a 3 panel frame (used for chat window)
//
void drawJoinedFrame3Panel( int size, int radius, float x, float y, float z, float sc, float wd, float ht, float ht2, float ht3,
							int color, int back_color );
void drawJoinedFlatFrame3Panel( int size, int radius, float x, float y, float z, float sc, float wd, float ht, float ht2, float ht3,
								int color, int back_color );
void drawJoinedFrame3PanelStyle( int size, int radius, float x, float y, float z, float sc, float wd, float ht, float ht2, float ht3,
								 int color, int back_color, FrameStyle style );

void drawFadeBar( float x, float y, float z, float wd, float ht, int color );

typedef struct ToolTipParent ToolTipParent;
#define drawSmallCheckbox(wdw, x, y, text, on, callback, selectable, tiptxt, tipParent) drawSmallCheckboxEx((wdw), (x), (y), 0, 1, (text), (on), (callback), (selectable), (tiptxt), (tipParent), false, 0)
int drawSmallCheckboxEx( int wdw, float x, float y, float z, float sc, char *text, int *on, void (*callback)(), int selectable, char *tiptxt, ToolTipParent *tipParent, int bBigFont, int color );

void uiReset();

enum
{
	GLOW_NONE,
	GLOW_FADE_IN,
	GLOW_MOVE,
	GLOW_MOVE_NONE, // move the healthbar, but don't bother glowing
	GLOW_FADE_OUT,
};

typedef struct MultiBar
{
	int need_mode;
	float need_percent;
	float need_glow_scale;

	int have_mode;
	float have_percent;
	float have_glow_scale;

	float pulse_timer;
}MultiBar;

void handleGlowState( int * mode, float * last_percentage, float percentage, float * glow_scale, float speed );
void drawBar( float x, float y, float z, float width, float scale, float percentage, float glow_scale, int color, int negative );
void drawMultiBar( MultiBar * bar, float have, float need, float x, float y, float z, float sc, float wd, int color );

void drawFrameOpenTopStyle( int size, int radius, float x, float y, float z, float wd, float ht, float xa, float xb, float sc, int color, int back_color, FrameStyle style );
void drawFrameTabStyle( int size, int radius, float x, float y, float z, float wd, float ht, float sc, int color, FrameStyle style );

void drawBendyTrayFrame( float x, float y, float z, float wd, float ht, float sc, int color, int back_color, TrayBendType bend_type );

int drawCloseButton( F32 x, F32 y, F32 z, F32 sc, int back_color );

int drawMMBar( char * pchTitle, F32 x, F32 y, F32 z, F32 wd, F32 sc, int active, int selectable );
void drawWaterMarkFrameCorner( F32 x, F32 y, F32 z, F32 sc, F32 wd, F32 ht, F32 headerHeight, F32 contentWidth, int left_color, int right_color, int back_color, int mark);
void drawWaterMarkFrame_ex( F32 x, F32 y, F32 z, F32 sc, F32 wd, F32 ht, int left_color, int right_color, int back_color, int mark);
#define drawWaterMarkFrame(x, y, z, sc, wd, ht, mark ) (drawWaterMarkFrame_ex(x, y, z, sc, wd, ht, CLR_MM_BACK_DARK, 0x233a34ff, 0x8c9ea0ff, mark ))

int drawTextEntryFrame( F32 x, F32 y, F32 z, F32 wd, F32 ht, F32 sc, int selected );


#define MMBUTTON_LOCKED				(1<<0)
#define MMBUTTON_ERROR				(1<<1)
#define MMBUTTON_CREATE				(1<<2)
#define MMBUTTON_DEPRESSED			(1<<3)
#define MMBUTTON_SMALL				(1<<4)
#define MMBUTTON_ICONRIGHT			(1<<5)
#define MMBUTTON_ICONALIGNTOTEXT	(1<<6)
#define MMBUTTON_USEOVERRIDE_COLOR	(1<<7)
#define MMBUTTON_GRAYEDOUT			(1<<8)
#define MMBUTTON_COLOR			    (1<<9)

int drawMMButton( char * txt, char * icon1, char * icon2, F32 cx, F32 cy, F32 z, F32 wd, F32 sc, int flags, int color );
int drawMMArrow( F32 cx, F32 cy, F32 z, F32 sc, int dir, int lit );
int drawMMArrowButton( char * pchText, F32 x, F32 y, F32 z, F32 sc, F32 wd, int dir );
void drawMMProgressBar( F32 x, F32 y, F32 z, F32 sc, F32 wd, F32 percent );

int drawMMFrameTabs( F32 x, F32 y, F32 z, F32 sc, F32 wd, F32 ht, F32 tab_ht, int style, char ***ppTabNames, int selected );
int drawMMCheckBox( F32 x, F32 y, F32 z, F32 sc, const char *pchText, int *val, int txt_color, int button_color, int mouse_over_txt_color, int mouse_over_button_color, F32 *wd, F32 *ht );
int drawMMBulbButton( char *pchText, char * pchIcon, char* pchIconGlow, F32 x, F32 y, F32 wd, F32 z, F32 sc, int flags );
int drawMMBulbBar( char *pchText, char * pchIcon, char * pchIconGlow, F32 x, F32 y, F32 wd, F32 z, F32 sc, int open, int flags, F32 *popout );
int drawListButton(char *text, float x, float y, float z, float width, float height, float scale);
#endif
