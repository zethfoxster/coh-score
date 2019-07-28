/*\
 *  dooranim.h - server functions to handle animating players through doors
 *
 *  Copyright 2003 Cryptic Studios, Inc.
 */

#ifndef __DOORANIM_H
#define __DOORANIM_H

#include "mathutil.h"
#include "dooranimcommon.h"

#define MAX_DOORANIM_BEACON_DIST		15.0	// dist for everything to and from EnterMe/EmergeFromMe

//ENTITY ENTRY ONTO MAP  FLAGS
#define EE_NOFLAGS						 (0)
#define EE_USE_EMERGE_LOCATION_IF_NEARBY (1 << 0)

typedef struct Entity Entity;
typedef struct ClientLink ClientLink;
typedef struct DoorAnimState DoorAnimState;
typedef struct DoorEntry DoorEntry;
typedef struct GridIdxList GridIdxList;

// internals of anim points
typedef enum DoorAnimPointType
{
	DoorAnimPointEnter,
	DoorAnimPointExit,
} DoorAnimPointType;

#define MAX_DOOR_AUX_POINTS	10	// totally arbritrary, but easier than resizing

// one for entry and one for exit
typedef struct DoorAnimPoint
{
	DoorAnimPointType type;
	GridIdxList* grid_idx;
	Vec3 centerpoint;
	Vec3 auxpoints[MAX_DOOR_AUX_POINTS];
	F32 auxpointRadius[MAX_DOOR_AUX_POINTS]; //ought to make this a struct with auxpoints
	int numauxpoints;
	char* animation;			//Bits that you set as soon as you start running into the door
	F32 entryPointRadius;       //How close to the entry point you need to be to decide to go ahead and turn toward the door
	U8 dontRunIntoMe;			//Im not the kind of door you run into at all, for example, the arena door
	U32		*lastTrickOrTreat;	// dbid, not persisted

} DoorAnimPoint;

// top-level functions.  enter and exit doors
int DoorAnimEnter(Entity* player, DoorEntry* door, const Vec3 doorPos, int map_id, char* mapinfo, char* spawnTarget, 
				  MapTransferType xfer, char* errmsg);
void prepEntForEntryIntoMap(Entity *e, int flags, MapTransferType xfer);  // animate entity out of a nearby door if possible
Entity *getTheDoorThisGeneratorSpawned(Vec3 pos); // get the door entity nearby
DoorAnimState *DoorAnimInitState(DoorAnimPoint *entryPoint, const Vec3 pos ); // set up DoorAnimState

// hooks/debug
void DoorAnimLoad();					// on map load
void DoorAnimCheckMove(Entity* player); // called every tick for each player, animates through door
void DoorAnimDebugEnter(ClientLink* client, Entity* player, int toArena); // run enter animation to nearest door
void DoorAnimDebugExit(ClientLink* client, Entity* player, int toArena ); // run exit animation from nearest door
void DoorAnimClientDone(Entity* player); // client done with entry animation

// door find functions
DoorAnimPoint *DoorAnimFindPoint(const Vec3 pos, DoorAnimPointType type, F32 radius);
F32 * DoorAnimFindEntryPoint(Vec3 pos, DoorAnimPointType type, F32 radius);
int DoorAnimFindGroupFromPoint(DoorAnimPoint** doorlist, DoorAnimPoint* sourcedoor, int* numdoors, int maxdoors);
void EntRunOutOfDoor(Entity *e, DoorAnimPoint* runoutdoor, MapTransferType xfer);

// door entry data accessors
//void DoorAnimSetLastTrickOrTreat(DoorAnimPoint *entryPoint, U32 time);
//U32 DoorAnimGetLastTrickOrTreat(DoorAnimPoint *entryPoint);
void DoorAnimAddTrickOrTreat(DoorAnimPoint *entryPoint, Entity *pEnt);
int DoorAnimFindTrickOrTreat(DoorAnimPoint *entryPoint, Entity *pEnt);
void DoorAnimClearTrickOrTreat(void);


#endif // __DOORANIM_H