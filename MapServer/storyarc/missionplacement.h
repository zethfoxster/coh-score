/*\
 *
 *	missionplacement.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	handles rules for placing encounters and objectives on a mission map
 *	deals mainly with logical rooms, and then random placement within those
 *	rooms
 *
 */

#ifndef __MISSIONPLACEMENT_H
#define __MISSIONPLACEMENT_H

#include "storyarcinterface.h"

typedef struct RoomInfo RoomInfo;

void ObjectiveDoPlacement(void);
void EncounterDoPlacement(void);
void SkillDoPlacement(void);

// for scripting - layout is optional
EncounterGroup* MissionFindEncounter(RoomInfo* room, const char* layout, int inactiveonly);

// getting a replacement generic spawndef from the mission definition
const SpawnDef* MissionSpawnReplaceGeneric(const MissionDef* def);

#endif //__MISSIONPLACEMENT_H