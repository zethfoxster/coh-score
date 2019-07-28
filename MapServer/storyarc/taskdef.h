/*\
 *
 *	taskdef.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	handles static task definitions and initialization checks
 *	on these
 *
 */

#ifndef __TASKDEF_H
#define __TASKDEF_H

#include "storyarcinterface.h"

typedef struct ScriptVarsScope ScriptVarsScope;

typedef struct StoryTaskSetList {
	const StoryTaskSet** sets;
} StoryTaskSetList;

extern SHARED_MEMORY StoryTaskSetList g_tasksets;

// *********************************************************************************
//  Parse definitions
// *********************************************************************************

int VerifyRewardBlock(const StoryReward *reward, const char *rewardName, const char *filename, int allowSuperGroupName, int allowLastContactName, int allowRescuer);
int TaskPreprocess(StoryTask* task, const char* filename, int allowSuperGroupName, int inStoryArc);
void TaskPostprocess(StoryTask* task, const char* filename);
void TaskValidate(StoryTask* task, const char* filename);
int TaskProcessAndCheckLevels(StoryTask* task, const char* filename, int minlevel, int maxlevel);
int TaskSetTotalTasks(void);

// *********************************************************************************
//  Utility functions for task defs
// *********************************************************************************

const StoryTask* TaskParentDefinition(const StoryTaskHandle *sahandle);
const StoryTask* TaskChildDefinition(const StoryTask* parent, int compoundPos);
const StoryTask* TaskDefinition(const StoryTaskHandle *sahandle);
const StoryTask *TaskDefFromLogicalName(char* logicalname);
TaskType TaskStartingType(StoryTask* taskdef);
TaskSubHandle TaskFromLogicalName(const ContactDef* contactdef, const char* logicalname);
void TaskSetRemerge(char* filename);
const MissionDef* TaskMissionDefinition(const StoryTaskHandle *sahandle);
const MissionDef* TaskMissionDefinitionFromStoryTask(const StoryTask* def);
const SpawnDef* TaskSpawnDef(const StoryTask* def);

const char* TaskFileName(TaskHandle context);
TaskHandle TaskGetHandle(const char* filename);
int TaskSanityCheck(const StoryTask* taskdef);
const StoryClue** TaskGetClues(StoryTaskHandle *sahandle);
int TaskContainsMission(StoryTaskHandle *sahandle);
int TaskDefIsFlashback(const StoryTask *pTask);

StoryTask *TaskDefStructCopy(const StoryTask *pTask);

// *********************************************************************************
//  Task sets
// *********************************************************************************

void TaskSetPreload();
const StoryTaskSet* TaskSetFind(const char* filename);
int TaskSetCountTasks(char* filename);

#endif //__TASKDEF_H