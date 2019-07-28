/*\
 *
 *	taskforce.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	deals with taskforce-specific aspects of storyarc stuff
 *
 */

#ifndef __TASKFORCE_H
#define __TASKFORCE_H

#include "storyarcinterface.h"

typedef enum TFParamDeaths TFParamDeaths;

void TaskForceSelectTask(Entity* leader);
void TaskForcePatchTeamTask(Entity* member, int new_db_id);
void TaskForceCompleteStatus(Entity *player);
void TaskForceStartStatus(Entity *player);
int TaskForceIsOnFlashback(Entity *player);
int TaskForceIsScalable(Entity *player);
void TaskForceResetDesaturate(Entity *player);
void TaskForceMissionEntryDesaturate(Entity *player);
int TaskForceJoin(Entity* player, char* targetPlayerName);
void TaskForceMissionCompleteDesaturate(Entity *player);
int TaskForceIsOnArchitect(Entity *player);
void TaskForceLogCharacter(Entity *player);
void teamMateEnterArchitectTaskforce(Entity *teammate, int taskforce_id, int owner_id );
#endif // __TASKFORCE_H
