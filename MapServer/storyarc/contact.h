/*\
 *
 *	contact.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	main file for contacts
 *	deals mainly with maintaining ContactInfo records on players
 *
 *	Contacts:
 *		- issue missions and tasks
 *		- issue storyarcs
 *		- contain stores
 *		- have extensive dialog
 *		- introduce other contacts
 *
 */

#ifndef __CONTACT_H
#define __CONTACT_H

#include "storyarcinterface.h"

// *********************************************************************************
//  Contact info functions
// *********************************************************************************

StoryContactInfo* ContactGetInfo(StoryInfo* info, ContactHandle handle);
StoryContactInfo* ContactGetInfoFromDefinition(StoryInfo* info, const ContactDef *def);
const ContactDef *ContactGetLastContact(Entity* player, int seed, int statureLevel); // get a reasonable referral target for the player
int ContactAdd(Entity* player, StoryInfo* info, ContactHandle handle, const char *desc);
int ContactRemove(StoryInfo* info, ContactHandle handle); // removes from storyinfo, internal use only, use ContactDismiss() instead
int ContactDismiss(Entity *player, ContactHandle handle, int dismissEvenIfOnTask); // handles lots of edge cases, should be used externally
int ContactCanBeDismissed(Entity *player, StoryContactInfo *contactInfo);

// *********************************************************************************
//  Contact/Task interaction
// *********************************************************************************

const StoryTask* ContactTaskDefinition(const StoryTaskHandle *sahandle);
TaskSubHandle ContactGetTaskSubHandle(ContactHandle contacthandle, StoryTask* taskdef);
StoryTaskInfo* ContactGetAssignedTask(int player_id, StoryContactInfo* contactInfo, StoryInfo* info);

// *********************************************************************************
//  Contact debugging
// *********************************************************************************

void ContactDebugShowAll(ClientLink* client);
void ContactDebugDatamine(ClientLink* client, bool detailed);
void ContactDebugShow(ClientLink* client, StoryInfo* info, StoryContactInfo* contact, int detailed);
void ContactDebugShowHelper(char ** estr, ClientLink * client, StoryInfo* info, StoryContactInfo* contact, int detailed);
void ContactDebugAddAll(ClientLink* client, Entity* e);
void ContactDebugOutputFileForFlowchart(void);
int ContactDebugPlayerHasFinishedTask(Entity *e, char *fileName, char *taskName);

// *********************************************************************************
//  Contact Misc
// *********************************************************************************
void ContactMarkAllDirty(Entity *e, StoryInfo* info);
void ContactRegisterScriptContact(char* name, Mat4 location);
void ContactGatherScriptContacts();
void ContactGetValidList(Entity *e, StoryContactInfo ***contactsDst, StoryContactInfo ***contactsSrc);
int ContactPlayerHasContact(Entity *e, char *contactName);

#define CONTACT_IS_AVAILABLE(e, contact)\
	(ContactCorrectAlliance(SAFE_MEMBER(e->pl,playerType), contact->alliance) && \
	ContactCorrectUniverse(SAFE_MEMBER(e->pl,praetorianProgress), contact->universe) && \
	(!contact->interactionRequires || chareval_Eval(e->pchar, contact->interactionRequires, contact->filename)))

// contact.h includes everything contact related
#include "contactdef.h"
#include "contactInteraction.h"
#include "contactDialog.h"
#include "contactIntro.h"
#include "ContactStore.h"

#endif // __CONTACT_H
