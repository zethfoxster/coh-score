/*\
 *
 *	storySend.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	all functions to send contacts, tasks, clues to client
 *
 */

#ifndef __STORYSEND_H
#define __STORYSEND_H

#include "storyarcinterface.h"

// *********************************************************************************
//  Contact status
// *********************************************************************************

void ContactMarkDirty(Entity* player, StoryInfo* info, StoryContactInfo* contactInfo);
void ContactMarkSelected(Entity* player, StoryInfo* info, StoryContactInfo* contactInfo);
void ContactSetNotify(Entity* player, StoryInfo* info, StoryContactInfo* contactInfo, int newState);
int ContactFindLocation(ContactHandle contact, int *mapID, Vec3 *coord);
void ContactDestroyLocations(void);

void ContactStatusSendAll(Entity* e);
void ContactStatusSendChanged(Entity* e);
void ContactStatusAccessibleSendAll(Entity *e);

void ContactTaskConstructStatus(char* str, Entity* owner, StoryTaskInfo* task, const ContactDef* contactdef);
StoryLocation* ContactTaskGetLocations(Entity* player, StoryTaskInfo* task, int* numlocs);

// *********************************************************************************
//  Task status
// *********************************************************************************

int TaskIsRunningMission(Entity* player, StoryTaskInfo* task);
void TaskStatusSend(Entity* player, StoryTaskInfo* task, const ContactDef* contactdef, Packet* pak);
void TaskStatusSendAll(Entity* e);
void TaskStatusSendUpdate(Entity* e);
void TaskStatusSendNotorietyUpdate(Entity* e);
void SendSelectTask(Entity* e, int sel);

// *********************************************************************************
//  Mission Sending functions - weird versions of contact send and task send
// *********************************************************************************

void SendMissionStatus(Entity* e);
void ClearMissionStatus(Entity* e);

// *********************************************************************************
//  Visit locations
// *********************************************************************************

void ContactVisitLocationSendAll(Entity* e);
void ContactVisitLocationSendChanged(Entity* e);

// *********************************************************************************
//  Task detail page
// *********************************************************************************

bool TaskSendDetail(Entity* player, int targetDbid, StoryTaskHandle *sahandle);

#endif // STORYSEND_H