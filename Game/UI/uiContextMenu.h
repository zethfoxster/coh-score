#ifndef UICONTEXTMENU_H
#define UICONTEXTMENU_H

#include "stdtypes.h"
#include "uiInclude.h"

typedef struct ContextMenu ContextMenu;
typedef struct KeyBindProfile KeyBindProfile;
typedef struct ContextMenuElement ContextMenuElement;


typedef enum CMVisType
{
	CM_HIDE,				// don't draw it
	CM_VISIBLE,				// draw it, but don't allow selection
	CM_CHECKED_UNAVAILABLE,	// draw a checkmark, but don't allow selection
	CM_AVAILABLE,			// draw it and let people click it
	CM_CHECKED,				// draw a checkmark, let people click it
} CMVisType;

typedef enum CMAlign
{
	CM_TOP,
	CM_BOTTOM,
	CM_LEFT,
	CM_RIGHT,
	CM_CENTER,
} CMAlign;

static int boolOffOn[] = { 0, 1 };

int alwaysAvailable( void * );
int alwaysVisible( void * );
int neverAvailable( void * );

typedef int (*CMVisible)(void*);
typedef void(*CMCode)(void*);
typedef char*(*CMText)(void*);

//allocates a context menu structure, returns pointer to menu
// destroyConditions is a function that will close the menu if it returns true
// (catch all for conditions that make a context menu invalid)
ContextMenu *contextMenu_Create( int (*destroyConditions)() );

// adds a divider (extra space) to a context menu
void contextMenu_addDivider( ContextMenu *cm );

// adds a conditionally visible divider
void contextMenu_addDividerVisible( ContextMenu *cm, int(*visible)(void*) );

// adds a title to context menu, (Un-clickable text)
void contextMenu_addTitle( ContextMenu *cm, const char * txt );

// adds a variable title to context menu, (Un-clickable text)
void contextMenu_addVariableText( ContextMenu *cm, const char *(*textCode)(void*), void *textData );
// adds a variable title to context menu, (Un-clickable text)
void contextMenu_addVariableTextVisible( ContextMenu *cm, int(*visible)(void*), void *visData, const char *(*textCode)(void*), void *textData );
void contextMenu_addVariableTitleVisibleNoTrans( ContextMenu *cm, int(*visible)(void*), void *visibleData, const char *(*textCode)(void*), void *textData );

// adds a varible text that may or may not be translated (NULL is always translate)
void contextMenu_addVariableTextTrans( ContextMenu *cm, const char *(*textCode)(void*), void *textData, int(*translate)(void*), void *translateData );

void contextMenu_addVariableTitle( ContextMenu *cm, const char *(*textCode)(void*), void *textData );
void contextMenu_addVariableTitleVisible( ContextMenu *cm, int(*visible)(void*), void *visibleData, const char *(*textCode)(void*), void *textData );

// (NULL is always translate)
void contextMenu_addVariableTitleTrans( ContextMenu *cm, const char *(*textCode)(void*), void *textData, int(*translate)(void*), void *translateData  );

// adds a checkbox to context menu.
void contextMenu_addCheckBox( ContextMenu *cm, int(*visible)(void*), void *visData, void (*code)(void*), void *codeData, const char *txt );
void contextMenu_addIconCheckBox( ContextMenu *cm, int(*visible)(void*), void *visData, void (*code)(void*), void *codeData, const char *txt, AtlasTex * icon );
void contextMenu_addVariableTextCheckBox( ContextMenu *cm, int(*visible)(void*), void *visData, void (*code)(void*), void *codeData, const char *(*textCode)(void*), void *textData );

// all function pointers are same for this, except code which does not need to change anything
// sub is a sub menu, if you set a sub menu be sure to pass NULL in for code
void contextMenu_addCode( ContextMenu *cm, int(*visible)(void*), void *visData, void (*code)(void*), void *codeData, const char *txt, ContextMenu *sub  );
void contextMenu_addIconCode( ContextMenu *cm, int(*visible)(void*), void *visData, void (*code)(void*), void *codeData, const char *txt, ContextMenu *sub,AtlasTex * icon);
void contextMenu_setCode( ContextMenu* cm, int(*visible)(void*), void *visData, void (*code)(void*), void *codeData, const char *txt, ContextMenu *sub  );

// just in case you want to set it all
void contextMenu_addVariableTextCode( ContextMenu *cm, int(*visible)(void*), void *visData, void (*code)(void*), void *codeData, const char *(*textCode)(void*), void *textData, ContextMenu *sub );

// this function is for cmdParse to execute a ContextMenu element though keybind, you should never use this
void contextMenu_activateElement( int i );

// This function sets the current contextmenu (setting it = displays it), use closeAll to close, do not set to null
void contextMenu_set( ContextMenu * cm, float x, float y, void *data );
void contextMenu_setAlign( ContextMenu * cm, float x, float y, CMAlign alignHoriz, CMAlign alignVert, void *data );
void contextMenu_display(ContextMenu * cm);	// displays context menu under mouse.
void contextMenu_displayEx(ContextMenu * cm, void * data );

// Force all context menus closed - shouldn't be nessecary with catch all
void contextMenu_closeAll();

// see if a context menu is currently active
bool contextMenu_IsActive(void);

// set or unset the no_translate flag on all elements
void contextMenu_SetNoTranslate(ContextMenu *cm, int no_translate);

// set custom colors
void contextMenu_SetCustomColors(ContextMenu *cm, int color, int bcolor);
void contextMenu_UnsetCustomColors(ContextMenu *cm);

// set screen scale disabling
void contextMenu_SetDisableScreenScaling(ContextMenu *cm);

// This function is for the main loop, you should never use it
void displayCurrentContextMenu();

#endif