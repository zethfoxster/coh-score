#ifndef UITOOLTIP_H
#define UITOOLTIP_H

#include "wdwbase.h"

// A tool tip parent is a bounding box where tool tips will continue to persist while the mouse
// is over the bounds
typedef struct ToolTipParent
{
	CBox box;
} ToolTipParent;

typedef struct SMFBlock SMFBlock;

#define TT_NOTRANSLATE  1
#define TT_FIXEDPOS  2
#define TT_MISSIONMAKER  4
#define TT_HIDE	 8

// tool tip structure
typedef struct ToolTip
{
	int				window;			// the window enum, only nessecary if menu = MENU_GAME
	int				menu;			// the menu enum
	int				timer;			// If this is set, the tip will use this instead of the global slider
	int				constant;		// If this is set, the tip does not need to be updated with window coordinates
	int				reparse;		// Should we reparse the smf
	int				updated;		// if this is set, we've already processed this tooltip this frame
	int				flags;			// Flags, like no translate
	CBox			bounds;			// the actual activation area
	CBox			rel_bounds;		// the bounds relative to the window coordinates (only for constant tips)
	char			*txt;			// the short text
	char			*sourcetxt;		// before translation
	ToolTipParent	*parent;		// pointer to parent, can be null
	SMFBlock		*smf;
	bool			allocated_self;			// can be free'd
	int				constant_wd;
	int				back_color;
	float			x;
	float			y;
	bool			disableScreenScaling;
}ToolTip;

void setToolTip( ToolTip * tip, CBox * box, const char * txt, ToolTipParent *parent, int menu, int window );
//
// this keeps the the tool tip up to date, should be called every frame for windows and movable items
// if the tool tip is entirely static, only needs to be called once.  The window parameter is only nessecary if the 
// menu is MENU_GAME

void setToolTipEx( ToolTip * tip, CBox * box, const char * txt, ToolTipParent *parent, int menu, int window, int timer, int flags);
//
// Allows the setting of the timer and flags

ToolTip * addWindowToolTip( float x, float y, float wd, float ht, const char* txt, ToolTipParent *parent, int menu, int window, int timer );
//
// This tool tip takes coordinates relative to the window, should only be called once
// It will update its own bounds
// This is how it always should've been, but I don't want to touch all the old tips


ToolTip * addToolTipEx( float x, float y, float wd, float ht, const char* txt, ToolTipParent *parent, int menu, int window, int timer );
//
// This tool tip takesabsolute coordinates and needs to be updated, but it creates the tip for you

void clearToolTipEx( const char * txt );
//
// clearToolTip for tooltips under the new system that do not require static pointers.
// used to explicitly deactivate a tooltip. This usually not required (the tooltip loop will do the checking for you)
// Used when different tooltips share the same area depending on circumstance


void clearToolTip( ToolTip *tip );
//
// used to explicitly deactivate a tooltip. This usually not required (the tooltip loop will do the checking for you)
// Used when different tooltips share the same area depending on circumstance

void freeToolTip( ToolTip *tip );
//
// free a tooltip

void addToolTip( ToolTip * tip );
//
// Add the tooltip to the array of tooltip pointers.  Can be called every frame without breaking if you don't feel like
// having a special one-time only initialization

void removeToolTip( ToolTip * tip );
// removed specified tooltip from the array of tooltip pointers. 

void updateToolTips();
//
// master tooltip function called in the main UI function, executes and handles all the tooltips

void enableToolTips(int enable);
//
// used to explicity turn tool tips on/off

void repressToolTips();
// Repress display of tooltips for one frame.

void forceDisplayToolTip(ToolTip *tip, CBox * box, const char * txt, int menu, int window);
// used to explicitly force a tool tip to be displayed

#endif
