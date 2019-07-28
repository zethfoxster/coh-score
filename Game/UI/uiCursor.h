#ifndef _CURSOR_H
#define	_CURSOR_H

#include "uiInclude.h"
#include "stdtypes.h"
#include "trayCommon.h"
#include "entityRef.h"

typedef	struct DefTracker	DefTracker;
typedef struct GroupDef GroupDef;

typedef enum EWorldTarget
{
	kWorldTarget_None       = 0,
	kWorldTarget_Normal     = 1,
	kWorldTarget_InRange    = 2,
	kWorldTarget_Blocked    = 3,
	kWorldTarget_OutOfRange = 4,
	kWorldTarget_OffGround  = 5,
} EWorldTarget;

typedef struct Cursor
{
	AtlasTex*		base;			// tha actual cursor
	AtlasTex*		dragged_icon;	// The icon
	float			dragged_icon_scale;	// Scale applied to only the dragged_icon
	AtlasTex*		overlay;		// overlay - overlay draws on top of cursor, dragged_icon below
	float			overlay_scale;	// Scale applied to only the overlay

	int				software;	// why two variables to keep track of one state? Well the ShowCursor function is crazy
	int				HWvisible;	// and the two variables ensures it doesn't get messed up.  HWvisible is essentially a private memeber
								// to keep track of state and software is a public variable to set state.

	EWorldTarget	target_world;	// Used to turn on world targeting for locational powers.
	float			target_distance;	// When world targeting, the max targetable distance.

	int				dragging;	// is the cursor dragging
	TrayObj			drag_obj;	// the objoect its dragging, not a pointer because the object that gets dragged may leave its old home

	int				hotX, hotY;

	int				click_move_timer; // timer for detecting double click on entities
	EntityRef 		click_move_last_ent; // last entity clicked
	GroupDef		*click_move_last_def; // last group def clicked

	int				initted;

	int				s_iCursorID;  //ID of the bullseye fx currently in use for targeting, if any
	EWorldTarget	s_iLastWorldTarget;

}Cursor;

extern Cursor cursor;
extern Vec3	current_location_target;
extern Entity	*current_target;
extern Entity	*current_target_under_mouse;
extern EntityRef current_pet; //Right Click on a pet only makes it your current pet, not your current target


void handleCursor();
void setCursor( char *base, char *overlay, int software, int hotX, int hotY );
void destroyCursorHashTable();
void interactWithInterestingGroupDef( Entity * player, GroupDef * def, Vec3 collisionPos, DefTracker * tracker );

#endif