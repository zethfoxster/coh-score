#ifndef windows_h
#define	windows_h

#include "uiInclude.h"
#include "stdtypes.h"

typedef struct Wdw Wdw;

// window accessors
int  windowUp( int wdw );
void window_openClose( int w );
void window_setMode( int wdw, int mode );
int  window_getMode( int wdw );
float window_getMaxScale(void);
void window_setScale(int wdw, float sc );
void window_SetScaleAll( float sc );

void window_set_edit_mode( int wdw, int editor);

// getUpperLeftPos ignores the current window mode, use window_getDims for actual display data
// does take loc_minimized into account
void window_getUpperLeftPos(int wdw, int *xp, int *yp);
// pass in -1 to not set that value
void window_setUpperLeftPos(int wdw, float xp, float yp);

// getDims takes the current window mode into account, use window_getUpperLeftPos for internal data
// does take loc_minimized into account
int window_getDimsEx(int wdw, float *xp, float *yp, float *zp, float* wd, float *ht, float *scale, int * color, int * back_color, bool useAnchor);
int window_getDims(int wdw, float *x, float *y, float *zp, float *wd, float *ht, float *scale, int * color, int * back_color);
void window_setDims(int wdw, float  x, float  y, float width, float height);

void window_permSetDims(int wdw);
void window_UpdateServer(int wdw);
void window_UpdateServerAllWindows();

void childButton_getCenter( Wdw *wdw, int child_id,  float *x, float *y );

// Functions to manipulation a window by string name (for slash commands)
void window_Names(void);
Wdw *windows_Find(char *pch, bool bAllowNumbers, int reason);
const char* windows_GetNameByNumber(int num);
void windows_Show(char *pch);
void windows_ShowDbg(char *pch);
void windows_Hide(char *pch);
void windows_Toggle(char *pch);

void window_closeAlways(void);

// Main window loop
void serveWindows();
int displayWindow( int i );
void window_bringToFront( int i );
int window_isFront( int i );
// color
extern int gWindowRGBA[4];
void windows_ChangeColor(void);
void window_SetColor( int wdw, int color );
int window_GetColor( int wdw );
float window_GetOpacity( int wdw );

bool window_collision(void);

void window_SetTitle(int wdw, const char *title);
void window_EnableCloseButton(int wdw, int enableClose);
void window_SetFlippage(int wdw, int flip);
int window_IsBlockingCollison( int wdw );

extern int gWindowDragging;
extern int gCurrentWindefIndex;
extern int custom_window_count;

#define WINDOW_BUTTON_WIDTH 25
#define WINDOW_TAB_HT	10

#define WDW_FIND_ANY 0
#define WDW_FIND_OPEN 1
#define WDW_FIND_CLOSE 2
#define WDW_FIND_SCALE 3

#endif
