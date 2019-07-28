
#ifndef UIWINDOWS_INIT_H
#define UIWINDOWS_INIT_H

#include "wdwbase.h"
#include "uiBox.h"

// This file handles the big messy initialization of windows
#define WINDOW_ICON_SIZE	10
#define WINDOW_HT_OFFSET	-7
#define JELLY_HT			18
#define CLOSE_DEFAULT 0
#define MOVE_DEFAULT  1
#define LOCK_DEFAULT  2

typedef enum WindowHorizAnchor
{
	kWdwAnchor_Left,
	kWdwAnchor_Center,
	kWdwAnchor_Right
} WindowHorizAnchor;

typedef enum WindowVertAnchor
{
	kWdwAnchor_Top,
	kWdwAnchor_Middle,
	kWdwAnchor_Bottom
} WindowVertAnchor;

// sets all the default window values
void windows_initDefaults(int force_size);
void window_initChild( Wdw *wdw, int child, int num, char*name, char *command );

UIBox window_getDefaultSize(WindowName window);

// resets the windows to default values
void windows_ResetLocations(void);

// closes windows specially tagged or marked always_closed
void closeUnimportantWindows(void);

// deales with the client window expanding
void window_HandleResize( int newWd, int newHt, int oldWd, int oldHt );

void resetWindows(void);
// sets up some paramerters for windows loop
int initWindows(void);

bool window_addChild( Wdw *wdw, int child, char*name, char *command );
bool window_removeChild(Wdw *wdw, char *name );

int  windowNeedsSaving(int wdw);
int  windowIsSaved(int wdw);
int window_Isfading( Wdw *win);

void wdwSave(char *pch);
void wdwLoad(char *pch);

void window_getAnchors(int wdw, WindowHorizAnchor *horiz, WindowVertAnchor *vert);

void fixWindowHeightValuesToNewAttachPoints(void);
void fixInspirationTrayToNewAttachPoint(void);

#endif
