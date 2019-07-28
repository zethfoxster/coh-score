/*\
 *
 *	missionteamup.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	handles the mission-teamup interaction (tricky)
 *	deals with how to start and stop mission maps and when
 *	to send updates to teamups.  
 *
 *	also handles tracking active players on the map,
 *	and kicking players from the map
 *
 */

#ifndef __MISSIONTEAMUP_H
#define __MISSIONTEAMUP_H

#include "storyarcinterface.h"

// *********************************************************************************
//  Teamup list broadcasts - send events to every teamup this mission has seen
// *********************************************************************************

extern int* g_teamupids;		// list of all the id's I've seen on this map

void MissionTeamupListAdd(Entity* player);
void MissionTeamupListBroadcast(char* message);

// *********************************************************************************
//  Mission objective broadcasts
// *********************************************************************************

char* MissionConstructObjectiveString(int owner, StoryTaskHandle *sahandle, int objnum, int success);
int MissionDeconstructObjectiveString(char* str, int* owner, StoryTaskHandle *sahandle, int* objnum, int* success);
void MissionObjectiveInfoSave(const MissionObjective* def, int succeeded);
void MissionHandleObjectiveInfo(Entity* player, int cpos, int objnum, int success);

// *********************************************************************************
//  Mission-owner interaction
// *********************************************************************************
void MissionSendToOwner(char* cmd);
void MissionChangeOwner(int new_owner, StoryTaskHandle *sahandle);

// *********************************************************************************
//  Mission-teamup interaction
// *********************************************************************************

void MissionRefreshTeamupStatus(Entity* e);		// Update teamup info of a single player with the status of the mission.
void MissionRefreshAllTeamupStatus(void);			// Update all teamups on mapserver with the status of the mission. Called whenever objectives are met and status needs to be updated.
void MissionSetTeamupComplete(Entity* e, int success);
void MissionConstructStatus(char* result, Entity* player, StoryTaskInfo* info);
void MissionCountAllPlayers(int ticks);
void MissionCountCheckPlayersLeft(void);
void MissionPlayerEnteredMap(Entity* ent);

// *********************************************************************************
//  Keeping track of active players
// *********************************************************************************

void MissionRewardActivePlayers(int success);
int MissionEarnedTeamComplete(Entity *player);
void MissionRewardActivePlayersSpecific(const char *reward);

// *********************************************************************************
//  Kicking players system
// *********************************************************************************

void MissionCheckKickedPlayers();
void MissionCheckForKickedPlayers(Entity* player);


#endif //__MISSIONTEAMUP_H