//
//
// Wrappers for looking at the mouse input buffer
//--------------------------------------------------------------------------------

#include "wininclude.h"
#include "uiInput.h"
#include "uiGame.h"
#include "sprite_base.h"
#include "cmdcommon.h"
#include "entDebug.h"
#include "font.h"
#include "uiCursor.h"
#include "uiWindows.h"
#include "uiScrollBar.h"
#include "input.h"
#include "uiBaseInput.h"
#include "uiOptions.h"

mouse_input gMouseInpBuf[MOUSE_INPUT_SIZE] = { 0 };
mouse_input gMouseInpCur  = {0};
mouse_input gMouseInpLast = {0};

int gInpBufferSize;

int gLastDownX = 0;
int gLastDownY = 0;

// ramp up when you hold the mouse down
static float cursor_repeat_times_fast[] =
{
	8, 15, 21, 26, 30, 34, 38, 42, 46, 50, 54, 57, 60 
};

static float cursor_repeat_times_slow[] =
{
	20, 35, 45, 52, 57, 60
};

int mouseDragging()
{
	if( cursor.dragging || gWindowDragging || gScrollBarDragging ||
		baseEdit_state.surface_dragging == kDragType_Drag ||
		baseEdit_state.detail_dragging == kDragType_Drag ||
		baseEdit_state.room_dragging == kDragType_Drag ||
		baseEdit_state.door_dragging == kDragType_Drag ||
		baseEdit_state.plot_dragging == kDragType_Drag )
	{
		return true;
	}

	return false;
}

// returns true if the mouse was pressed down
int mouseDown( int button )
{
	int i;

	if( control_state.canlook || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

	for( i = 0; i < gInpBufferSize; i++ )
	{
		if ( ( button == MS_LEFT  && gMouseInpBuf[i].left  == MS_DOWN ) ||
			 ( button == MS_RIGHT && gMouseInpBuf[i].right == MS_DOWN ) )
		{
			return TRUE;
		}
	}

	return FALSE;

}

// returns true if the mouse was released 
int mouseUp( int button )
{
	int i;

	if( control_state.canlook || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

	for( i = 0; i < gInpBufferSize; i++ )
	{
		if ( ( button == MS_LEFT  && gMouseInpBuf[i].left  == MS_UP ) ||
			 ( button == MS_RIGHT && gMouseInpBuf[i].right == MS_UP ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// returns true if the mouse button is currently down
int isDown( int button )
{
	if( control_state.canlook )
		return FALSE;

	if( ( button == MS_LEFT && gMouseInpCur.left == MS_DOWN ) ||
		( button == MS_RIGHT && gMouseInpCur.right == MS_DOWN ) )
	{
		return TRUE;
	}

	return FALSE;
}

// returns true if the right mouse button was clicked
int mouseRightClick()
{
	int i;
 
 	if( entDebugMenuVisible() || control_state.canlook || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

 	for( i = 0; i < gInpBufferSize; i++ )
	{
		if ( gMouseInpBuf[i].right == MS_CLICK )
		{
			return TRUE;	
		}
	}

	return FALSE;
}

int rightClickCoords( int *x, int *y )
{
	int i;

	if( entDebugMenuVisible() || control_state.canlook || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

	for( i = 0; i < gInpBufferSize; i++ )
	{
		if ( gMouseInpBuf[i].right == MS_CLICK )
		{
			*x = gMouseInpBuf[i].x;
			*y = gMouseInpBuf[i].y;
			reversePointToScreenScalingFactori(x, y);
			return TRUE;	
		}
	}

	return FALSE;
}

// returns true if the left mouse button was clicked
int mouseLeftClick()
{
	int i;

	if( entDebugMenuVisible() || control_state.canlook || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

	for( i = 0; i < gInpBufferSize; i++ )
	{
		if ( gMouseInpBuf[i].left == MS_CLICK )
		{
			return TRUE;
		}
	}

	return FALSE;
}

int leftClickCoords( int *x, int *y )
{
	int i;

	if( entDebugMenuVisible() || control_state.canlook || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

	for( i = 0; i < gInpBufferSize; i++ )
	{
		if ( gMouseInpBuf[i].left == MS_CLICK )
		{
			*x = gMouseInpBuf[i].x;
			*y = gMouseInpBuf[i].y;
			reversePointToScreenScalingFactori(x, y);
			return TRUE;	
		}
	}

	return FALSE;
}

// returns true if the left mouse button is dragging
int mouseLeftDrag( CBox *box )
{
	int i;

 	if( entDebugMenuVisible() || control_state.canlook || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

	for( i = 0; i < gInpBufferSize; i++ )
	{
		if ( gMouseInpBuf[i].left == MS_DRAG )
		{
			int xp = gMouseInpBuf[i].x;
			int yp = gMouseInpBuf[i].y;

			if( scaled_point_cbox_clsn( xp, yp, box ) && !outside_scissor( xp, yp ) )
				return TRUE;
		}
	}

	return FALSE;
}

// returns true if the mouse button was clicked while over a given box
int mouseDownHit( CBox *box, int button )
{
 	int i;

	if( entDebugMenuVisible() || control_state.canlook || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

 	for ( i = 0; i < gInpBufferSize; i++ )
	{
		if ( ( button == MS_LEFT  && gMouseInpBuf[i].left  == MS_DOWN ) ||
			 ( button == MS_RIGHT && gMouseInpBuf[i].right == MS_DOWN ) )
		{
			int xp = gMouseInpBuf[i].x;
			int yp = gMouseInpBuf[i].y;

 			if ( scaled_point_cbox_clsn( xp, yp, box ) && !outside_scissor( xp, yp ) )
				return TRUE;
		}
	}

	return FALSE;
}

// returns true if the mouse button was released while over a given box
int mouseUpHit( CBox *box, int button )
{
	int i;

	if( entDebugMenuVisible() || control_state.canlook || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

	for ( i = 0; i < gInpBufferSize; i++ )
	{
		if ( ( button == MS_LEFT  && gMouseInpBuf[i].left  == MS_UP ) ||
			( button == MS_RIGHT && gMouseInpBuf[i].right == MS_UP ) )
		{
    		float xp = gMouseInpBuf[i].x;
			float yp = gMouseInpBuf[i].y;

			if ( scaled_point_cbox_clsn( xp, yp, box ) && !outside_scissor( xp, yp ) )
				return TRUE;
		}
	}
	return FALSE;
}

// returns true if the mouse button was clicked while over a given box
int mouseClickHit( CBox *box, int button )
{
	int i;

	if( entDebugMenuVisible() || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

	for ( i = 0; i < gInpBufferSize; i++ )
	{
		if ( ( button == MS_LEFT  && gMouseInpBuf[i].left  == MS_CLICK ) ||
			( button == MS_RIGHT && gMouseInpBuf[i].right == MS_CLICK) )
		{
			float xp = gMouseInpBuf[i].x;
			float yp = gMouseInpBuf[i].y;

			if ( scaled_point_cbox_clsn( xp, yp, box ) && !outside_scissor( xp, yp ) )
				return TRUE;
		}
	}

	return FALSE;
}

int mouseDoubleClickHit( CBox *box, int button )
{
	int i;

	if( entDebugMenuVisible() || collisions_off_for_rest_of_frame || mouseDragging() )
		return FALSE;

	for ( i = 0; i < gInpBufferSize; i++ )
	{
		if ( ( button == MS_LEFT  && gMouseInpBuf[i].left  == MS_DBLCLICK ) ||
			( button == MS_RIGHT && gMouseInpBuf[i].right == MS_DBLCLICK) )
		{
			float xp = gMouseInpBuf[i].x;
			float yp = gMouseInpBuf[i].y;

			if ( scaled_point_cbox_clsn( xp, yp, box ) && !outside_scissor( xp, yp ) )
				return TRUE;
		}
	}

	return FALSE;
}

// returns true if the mouse did collide
//
int mouseCollision( CBox * box )
{
	float xp, yp;

  	if( entDebugMenuVisible() || control_state.canlook ||
   		 collisions_off_for_rest_of_frame || gWindowDragging || gScrollBarDragging )
		return FALSE;

   	xp = gMouseInpCur.x;
 	yp = gMouseInpCur.y;

 	if ( scaled_point_cbox_clsn( xp, yp, box ) && !outside_scissor( xp, yp ) )
		return TRUE;

	//if ( point_cbox_clsn( xp, yp, box ) && !outside_scissor( xp, yp ))
	//	return TRUE;

	return FALSE;
}

// returns true if the mouse button was pressed
int mouseDownHitNoCol( int button )
{
	int i;

	if( entDebugMenuVisible() || control_state.canlook || mouseDragging() )
		return FALSE;

	for ( i = 0; i < gInpBufferSize; i++ )
	{
		if ( ( button == MS_LEFT  && gMouseInpBuf[i].left  == MS_DOWN ) ||
			( button == MS_RIGHT && gMouseInpBuf[i].right == MS_DOWN ) )
			return true;
	}
	return FALSE;
}

// returns true if the mouse button was released
int mouseUpHitNoCol( int button )
{
	int i;

	if( entDebugMenuVisible() || control_state.canlook || mouseDragging() )
		return FALSE;

	for ( i = 0; i < gInpBufferSize; i++ )
	{
		if ( ( button == MS_LEFT  && gMouseInpBuf[i].left  == MS_UP ) ||
			( button == MS_RIGHT && gMouseInpBuf[i].right == MS_UP ) )
			return true;
	}
	return FALSE;
}

// returns true if the mouse button was clicked
int mouseClickHitNoCol( int button )
{
	int i;

	if( entDebugMenuVisible() || control_state.canlook || mouseDragging() )
		return FALSE;

	for ( i = 0; i < gInpBufferSize; i++ )
	{
		if ( ( button == MS_LEFT  && gMouseInpBuf[i].left  == MS_CLICK ) ||
			( button == MS_RIGHT && gMouseInpBuf[i].right == MS_CLICK ) )
			return true;
	}
	return FALSE;
}


// Returns true if mouse is doing something like dragging or looking
int mouseActive()
{
	if( entDebugMenuVisible() || control_state.canlook || 
		collisions_off_for_rest_of_frame || gWindowDragging || gScrollBarDragging )
		return TRUE;
	return FALSE;
}

void mouseFlush()
{
	memset( &gMouseInpBuf, 0, sizeof(mouse_input) * MOUSE_INPUT_SIZE );
	gInpBufferSize = 0;
	MouseClearButtonState();
}

void mouseClear()
{
 	int i;
	int found=0;
 	for( i = 0; i < gInpBufferSize; i++ )
	{
		if( gMouseInpBuf[i].right == MS_UP ) {
			memcpy( &gMouseInpBuf, &gMouseInpBuf[i+1], sizeof(mouse_input)*(gInpBufferSize-(i+1)) );
			gInpBufferSize -= i;
			found=1;
		}
	}
	if (!found) {
		mouseFlush();
	}
}

F32 mouseScrollingScaled()
{
	if( !optionGet(kUO_DisableMouseScroll) )
	{
		int scroll_wheel = gMouseInpCur.z - gMouseInpLast.z;
		if(scroll_wheel < 0)
		{
			return optionGetf(kUO_MouseScrollSpeed);
		}
		else if (scroll_wheel > 0)
		{
			return -optionGetf(kUO_MouseScrollSpeed);
		}
	}
	return 0;
}
