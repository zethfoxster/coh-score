/*\
 *  dooranimcommon.h - animation functions that have to run on both client and server
 *
 *  Copyright 2003 Cryptic Studios, Inc.
 */

#ifndef __DOORANIMCOMMON_H
#define __DOORANIMCOMMON_H

#include "mathutil.h"
typedef struct Entity Entity;
typedef int MapTransferType;

#define INTERACT_DISTANCE	15
#define SERVER_INTERACT_DISTANCE	20	// useful for server to be slightly more relaxed

typedef __int64 EntityRef;

/////////////////////////////////////// states for DoorAnimState.state

typedef enum DoorAnimAnimState
{
// before the animation actually starts
	DOORANIM_WAITDOOROPEN = 1,
// normal walk through, get to final point
	DOORANIM_BEGIN,
	DOORANIM_ORIENT,
	DOORANIM_TOENTRY,
	DOORANIM_TOFINAL,
// custom animation differs after DOORANIM_TOENTRY
	DOORANIM_ORIENTTOFINAL,
	DOORANIM_ANIMATION,
	DOORANIM_ANIMATIONRUNNNING,
// after either animation is done
	DOORANIM_DONE,
	DOORANIM_ACK
} DoorAnimAnimState;

// keeps track of the state of a player walking through a door.  
// same on client and server
typedef struct DoorAnimState {
	DoorAnimAnimState state;
	Vec3 entrypos;			// a point in front of the door, may be equal to targetpos
	Vec3 targetpos;			// the target point on the threshold
	Vec3 finalpos;			// a point beyond the target (calculated based on other points)
	F32 speed;				// current player speed
	char* animation;		// bits to play while entering the door
	int clientack		: 1;
	int	pastthreshold	: 1; 
	int entryPointRadius;
#ifdef SERVER
	Mat4 initialpos;		// only for debug dooranimenter command
	EntityRef entrydoor;
	EntityRef exitdoor;
	U32 starttime;			// ABS when animation was started

	U8 dontRunIntoMe;
	// destination fields, after animation is done
	int map_id;				
	char mapinfo[256];		// mapinfo to create
	char spawnTarget[256];	// name of spawn target to go to once animation is done
	bool spawnClosest;
	MapTransferType xfer;
#endif // SERVER
} DoorAnimState;

int DoorAnimMove(Entity* player, DoorAnimState* anim); // do one tick of movement with current anim, returns 0 when done
bool closeEnoughToInteract( Entity * player, Entity * target_ent, F32 interactDist );

#endif // __DOORANIMCOMMON_H