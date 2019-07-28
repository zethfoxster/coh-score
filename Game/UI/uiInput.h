#ifndef UI_INPUT_H
#define UI_INPUT_H

#include "uiInclude.h"

typedef enum
{
	MS_NONE,
	MS_DOWN,
	MS_UP,
	MS_CLICK,
	MS_DRAG,
	MS_DBLCLICK,
} mouseState;

enum
{
	MS_LEFT,
	MS_RIGHT,
};

typedef struct mouse_input
{
	int x;
	int y;
	int z;				// mousewheel
	mouseState left;
	mouseState mid;		// middle button or mousewheel button
	mouseState right;
} mouse_input;

#define MOUSE_INPUT_SIZE 256

extern mouse_input gMouseInpBuf[MOUSE_INPUT_SIZE];
extern mouse_input gMouseInpCur;
extern mouse_input gMouseInpLast;

extern int gInpBufferSize;

extern int gLastDownX;
extern int gLastDownY;

int mouseDragging(void);

int mouseDown( int button );
int mouseUp  ( int button );
int isDown   ( int button );
int mouseRightClick();
int mouseLeftClick();
int mouseLeftDrag( CBox *box );

int rightClickCoords( int *x, int *y );
int leftClickCoords( int *x, int *y );

int mouseDownHit  ( CBox *box, int button );
int mouseUpHit    ( CBox *box, int button );
int mouseClickHit( CBox *box, int button );
int mouseDoubleClickHit( CBox *box, int button );
int mouseCollision( CBox *box );

int mouseDownHitNoCol( int button );
int mouseUpHitNoCol( int button );
int mouseClickHitNoCol( int button );

void mouseClear();
void mouseFlush();

int mouseActive();
F32 mouseScrollingScaled();

#endif