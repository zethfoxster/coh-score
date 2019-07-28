/*\
 *
 *	storyinfo.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	handles StoryInfo, StoryContactInfo, StoryTaskInfo, StoryArcInfo structs
 *
 */

#ifndef __STORYINFO_H
#define __STORYINFO_H

#include "storyarcinterface.h"
#include "storytaskinfo.h"

// *********************************************************************************
//  Info struct constructors/destructors
// *********************************************************************************
StoryContactInfo* storyContactInfoAlloc();
void storyContactInfoFree(StoryContactInfo* info);
StoryContactInfo* storyContactInfoCreate(ContactHandle contact);
int storyContactInfoInit(StoryContactInfo* info); // returns success
void storyContactInfoDestroy(StoryContactInfo* info);

StoryArcInfo* storyArcInfoAlloc();
void storyArcInfoFree(StoryArcInfo* info);
StoryArcInfo* storyArcInfoCreate(StoryTaskHandle *storyarc, ContactHandle contact);
int storyArcInfoInit(StoryArcInfo* info); // returns success
void storyArcInfoDestroy(StoryArcInfo* info);

StoryTaskInfo* storyTaskInfoCreate(StoryTaskHandle *sahandle);
int storyTaskInfoInit(StoryTaskInfo* info); // returns success
int storyTaskInfoGetAlliance(StoryTaskInfo* task);

const char **storyTaskHandleGetTeamRequires(StoryTaskHandle* sahandle, const char **outputFilename);
const char *storyTaskHandleGetTeamRequiresFailedText(StoryTaskHandle* sahandle);

StoryInfo* storyInfoCreate();
void storyInfoDestroy(StoryInfo* info);

// *********************************************************************************
//  Locking & task force mode transparency
// *********************************************************************************

StoryInfo* StoryInfoFromPlayer(Entity* player);
int PlayerInTaskForceMode(Entity* ent);
ContactHandle PlayerTaskForceContact(Entity* ent);
Entity *StoryArcInfoIsLocked();
StoryInfo* StoryArcLockInfo(Entity* player);
StoryInfo* StoryArcLockPlayerInfo(Entity* player);
void StoryArcUnlockInfo(Entity* player);
void StoryArcUnlockPlayerInfo(Entity* player);

int StoryArcCountLongArcs(StoryInfo* info);
int StoryArcMaxStoryarcs(StoryInfo* info);
int StoryArcMaxLongStoryarcs(StoryInfo* info);
int StoryArcMaxContacts(StoryInfo* info);
int StoryArcMaxTasks(StoryInfo* info);

#endif //__STORYINFO_H
