/*\
 *
 *	contactdef.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	handles static contact definitions and initialization checks
 *	on these
 *
 */


#ifndef __CONTACTDEF_H
#define __CONTACTDEF_H

#include "storyarcinterface.h"
#include "storyarcprivate.h"

// *********************************************************************************
//  Contact defs and handles
// *********************************************************************************

#define ACCESSIBLE_UNIVERSE_COUNT 2
#define ACCESSIBLE_ALLIANCE_COUNT 2
#define ACCESSIBLE_LEVEL_COUNT 50

typedef struct ContactList
{
	const ContactDef** contactdefs;	
	cStashTable contactFromStature;
	cStashTable contactHandleFromName;
	const ContactDef** accessibleContactDefLists[ACCESSIBLE_UNIVERSE_COUNT * ACCESSIBLE_ALLIANCE_COUNT * ACCESSIBLE_LEVEL_COUNT];
} ContactList;
extern SHARED_MEMORY ContactList g_contactlist;
extern Contact* g_contacts;

typedef struct DesignerContactTip
{
	const char *rewardTokenName;
	const char *tokenTypeStr;
	int tipLimit;
	bool revokeOnAlignmentSwitch;
}DesignerContactTip;

typedef struct DesignerContactTipTypes
{
	const DesignerContactTip **designerTips;
}DesignerContactTipTypes;

extern StaticDefineInt ParseContactTipType[];
extern SHARED_MEMORY DesignerContactTipTypes g_DesignerContactTipTypes;

void ContactPreload();									// load all the contact defs
void ContactValidateAllManually(char **errorString);
void ContactValidateEntityAll(Entity *e, char **errorString);
const ContactDef* ContactDefinition(ContactHandle handle);
Contact* GetContact(ContactHandle handle);
const ContactDef* ContactFromStature(const char* stature);
ContactHandle ContactGetHandle(const char *filename);
ContactHandle ContactGetHandleLoose(const char* filename);	// for debug commands
ContactHandle ContactGetHandleFromDefinition(const ContactDef* definition);
const char *ContactFileName(ContactHandle handle);
int ContactNumDefinitions();							// handles will be 1 - n
ContactHandle ContactGetNewspaper(Entity* player);
void ContactReattachTaskSets(StoryTaskSet* taskset);
void ContactGetValidTips(Entity *e, int **handlesEai, TipType type);

// *********************************************************************************
//  Utility functions using only a handle
// *********************************************************************************

const char *ContactDisplayName(const ContactDef* contact, Entity* player);
//const char *ContactDisplayName(ContactHandle handle, Entity* player); // use this instead?  tbd, post-merge cleanup
int ContactNPCNumber(const ContactDef *cdef, Entity* player);					// for a costume index
void ContactLevelRange(ContactHandle handle, int* minlevel, int* maxlevel);
char *ContactAlignmentName(int alliance);

const ContactDef *AccessibleContacts_getCurrent(Entity *player, int *countOutput);
void AccessibleContacts_DebugOutput(Entity *player);
int AccessibleContacts_FindNext(Entity *player);
int AccessibleContacts_FindPrevious(Entity *player);
int AccessibleContacts_FindFirst(Entity *player);
void AccessibleContacts_ShowCurrent(Entity *player);
void AccessibleContacts_TeleportToCurrent(ClientLink *client, Entity *player);
void AccessibleContacts_SelectCurrent(ClientLink *client, Entity *player);

#define GRTIP_STREAKBREAKER_COUNT 7
#define GRTIP_STREAKBREAKER_REDNAME "recentRedTip"
#define GRTIP_STREAKBREAKER_BLUENAME "recentBlueTip"

#endif // __CONTACTDEF_H
