/*\
 *
 *	missionset.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	handles mission map sets and mission spawn sets
 *  - basically lists of generic stuff we can use to fill out missions
 *
 *  we also handle assigning a mission door from here
 *
 */

#ifndef __MISSIONSET_H
#define __MISSIONSET_H

#include "storyarcinterface.h"
typedef struct Entity Entity;
typedef struct StoryTask StoryTask;

// *********************************************************************************
//  Mission map sets
// *********************************************************************************

const char* MissionGetMapName(Entity * ent, const StoryTaskInfo *sti, const ContactHandle contacthandle);
void MissionDoorListFromTask(ClientLink* client, Entity* player, char* taskname);
char* MissionMapPick(MapSetEnum mapset, int maptime, unsigned int seed);

// *********************************************************************************
//  Mission spawn sets
// *********************************************************************************

void MissionSpawnPreload();
const SpawnDef* MissionSpawnGet(VillainGroupEnum group, int level);
const SpawnDef* MissionHostageSpawnGet(VillainGroupEnum group, int level);

// *********************************************************************************
//  Mission doors
// *********************************************************************************

int AssignMissionDoor(StoryInfo* info, StoryTaskInfo* task, int isVillain);
int VerifyCanAssignMissionDoor(const char* doortype, int level);

// *********************************************************************************
//  Mission doors
// *********************************************************************************
int DebugMissionMapMeetsRequirements(char * mapname, int mapset, int maptime, char ** fullName);



#endif //__MISSIONSET_H