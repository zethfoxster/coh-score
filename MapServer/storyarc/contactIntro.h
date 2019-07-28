/*\
 *
 *	contactIntro.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	functions to handle contact introductions
 *
 */

#ifndef __CONTACTINTRO_H
#define __CONTACTINTRO_H

#include "storyarcinterface.h"

// *********************************************************************************
//  Contact relationships
// *********************************************************************************

int ContactDetermineRelationship(StoryContactInfo* contactInfo); // returns true if changed
const char* ContactRelationshipGetName(Entity* player, StoryContactInfo* contactInfo);

// *********************************************************************************
//  New contact picking
// *********************************************************************************

int ContactPlayerPastFailsafeLevel(StoryContactInfo* contactInfo, Entity* player);
int ContactTimeForNewContact(Entity* player, StoryContactInfo* contactInfo, int nextLevel, int* needLevel);
ContactHandle* ContactPickNewContacts(Entity* player, StoryInfo* info, StoryContactInfo* contactInfo, int* nextLevel, int justYesOrNo);

#endif //__CONTACTINTRO_H