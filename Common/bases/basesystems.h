#ifndef _BASESYSTEMS_H
#define _BASESYSTEMS_H

#include "stdtypes.h"

#define BASE_UPDATE_INTERVAL 10

typedef struct Base			Base;
typedef struct BaseRoom		BaseRoom;
typedef struct RoomDetail	RoomDetail;

typedef struct RoomDetailPosition
{
	RoomDetail *detail;
	Vec3 pos;
} RoomDetailPosition;

// Returns the number of active anchors in this base
int baseCountActiveAnchors(Base *base);

// Returns an earray of items of power and their position in the base
RoomDetailPosition** baseGetItemsOfPower(Base *base);

// Main function for updating the state of the base systems
void baseSystemsTick(float fRate);
void baseSystemsTick_PhaseOne(Base *base, float fRate);

// Make this detail fall back to its default behavior (ignores power/control status)
void detailBehaviorSetToDefault(RoomDetail *detail);

// Request that this detail review its active state and update its behavior
void detailBehaviorUpdateActive(RoomDetail *detail);

// Get a VecName string for this vec3
char *detailAttackVolumesVecName(Vec3 vec);

// Detach and locate auxiliary details
void detachAuxiliary(BaseRoom *room, RoomDetail *auxiliary);
RoomDetail **findAttachedAuxiliary(BaseRoom *base, RoomDetail *detail);
void roomAttachAuxiliary(BaseRoom *room, RoomDetail **ppDetails);

// Let the base know to update itself based on this detail changing in some way
void detailFlagBaseUpdate(RoomDetail *detail);

// Let the base know the destroyed state of this detail changed
void detailSetDestroyed(RoomDetail *detail, bool destroyed, bool rebuild, bool floater);

// Find out if this detail should look unpowered or not
bool roomDetailLookUnpowered(RoomDetail *detail);

// Performs all required base updates immediately after a concluded base raid
void basePostRaidUpdate(Base *base, bool bNoDestroy);

// Returns the number of spaces available in the base for Items of Power (includes IoPs and Mounts)
int baseSpacesForItemsOfPower(Base *pBase);

// Given a supergroup ID, try to find a loaded entity which is in the supergroup
Entity* FindEntityInSupergroup(int supergroup_id);

//This is a temporary function to handle a data bug that allowed players to have more IOP mounts than their plot size shold allow
bool playerIsNowInAnIllegalBase();

void baseCostAndUpkeep(int *cost_out, int *upkeep_out);

#endif
