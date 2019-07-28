/*\
 *
 *	mission.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	top level mission handling functions
 *
 */

#ifndef __MISSION_H
#define __MISSION_H

#include "storyarcinterface.h"
#include "encounterPrivate.h"

#define MISSION_SHUTDOWN_DELAY_MINS 20 // how many minutes we wait to shutdown if no one on map
#define MISSION_NOLOGIN_DELAY_MINS 3 // how many minutes we will wait for the first person to log in

// *********************************************************************************
//  Active Mission Information
// *********************************************************************************

extern MissionInfo* g_activemission;
extern int g_ArchitectTaskForce_dbid;
extern int g_ArchitectCheated;
extern struct TaskForce *g_ArchitectTaskForce;

int MissionObjectiveExpressionEval(const char **expr);
int MissionObjectiveIsSetComplete(char** set, int success);
int MissionObjectiveIsComplete(const char* objective, int success);
const SpawnDef * getGenericMissionSpawn(void);
const SpawnDef * getMissionSpawnByLogicalName(const char *logicalName);
void MissionGrantSecondaryReward(int db_id, const char** reward, VillainGroupEnum vgroup, int baseLevel, int levelAdjust, ScriptVarsTable* vars);
 
void MissionSendEntryText(Entity* player);
void MissionWaypointSendAll(Entity* player);
void MissionWaypointCreate(MissionObjectiveInfo* objectiveInfo);
void MissionWaypointDestroy(MissionObjectiveInfo* objectiveInfo);

StoryTaskInfo *MissionGetCurrentMission(Entity *e);
bool MissionCanGetToMissionMap(Entity *e, StoryTaskInfo * pSTI);

void mission_StoreCurrentServerMusic(const char *musicName, float volumeLevel);

// mission.h includes everything mission related
#include "missiondef.h"
#include "missionset.h"
#include "missiongeoCommon.h"
#include "missionobjective.h"
#include "missionteamup.h"
#include "missionplacement.h"
#include "missionspec.h"
#include "missionscript.h"

#endif // __MISSION_H