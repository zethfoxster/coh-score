/*\
 *
 *	contactIntro.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	functions to handle contact introductions
 *
 */

#include "contact.h"
#include "storyarcprivate.h"
#include "character_level.h"
#include "entity.h"
#include "character_eval.h"
#include "taskforce.h"

// *********************************************************************************
//  Contact relationships
// *********************************************************************************

// returns true if the relationship has changed
int ContactDetermineRelationship(StoryContactInfo* contactInfo)
{
	ContactRelationship old = contactInfo->contactRelationship;
	if(!contactInfo->contact->def->friendCP && !contactInfo->contact->def->confidantCP && !contactInfo->contact->def->completeCP)
		contactInfo->contactRelationship = CONTACT_NO_RELATIONSHIP;
	else if(contactInfo->contactPoints < contactInfo->contact->def->friendCP)
		contactInfo->contactRelationship = CONTACT_ACQUAINTANCE;
	else if (contactInfo->contactPoints < contactInfo->contact->def->confidantCP)
		contactInfo->contactRelationship = CONTACT_FRIEND;
	else
		contactInfo->contactRelationship = CONTACT_CONFIDANT;
	return old != contactInfo->contactRelationship;
}

const char* ContactRelationshipGetName(Entity* player, StoryContactInfo* contactInfo)
{
	switch(contactInfo->contactRelationship)
	{
	case CONTACT_NO_RELATIONSHIP:
		return saUtilLocalize(player,"Norelationship");
	case CONTACT_ACQUAINTANCE:
		return saUtilLocalize(player,"Acquaintance");
	case CONTACT_FRIEND:
		return saUtilLocalize(player,"Friend");
	case CONTACT_CONFIDANT:
		{
			// for display purposes, we pretend that completeCP is all that matters for completion,
			// this is not technically accurate, since we have a level requirement too
			if (contactInfo->contactPoints >= contactInfo->contact->def->completeCP)
				return saUtilLocalize(player,"Complete");
			else
				return saUtilLocalize(player,"Confidant");
		}
	default:
		return saUtilLocalize(player,"Invalid relationship");
	}

	// Never gets here.
	return NULL;
}

// *********************************************************************************
//  New contact picking
// *********************************************************************************

typedef char StatureSetList[CONTACTS_PER_PLAYER][MAX_SA_NAME_LEN];

static void PushStatureSet(StatureSetList list, const char* statureSet)
{
	int i;
	for (i = 0; i < CONTACTS_PER_PLAYER; i++)
	{
		if (!list[i][0])
		{
			strcpy(list[i], statureSet);
			return;
		}
	}
}

// look at the player info, and build a list of all contact stature sets
// they have at a particular stature level
void GetExistingStatureSets(StoryInfo* info, int stature, StatureSetList list)
{
	int i, n;
	memset(list, 0, sizeof(char)*CONTACTS_PER_PLAYER*MAX_SA_NAME_LEN);
	n = eaSize(&info->contactInfos);
	for (i = 0; i < n; i++)
	{
		const ContactDef* contact = info->contactInfos[i]->contact->def;
		if (stature == 0 || stature == contact->stature)
		{
			if (contact->statureSet)
				PushStatureSet(list, contact->statureSet);
		}
	}
}

// Contact down the chain of the introducer is within range for the player
static const ContactDef* ContactIntroducesAcceptableLevelContact(int playerLevel, const ContactDef* introducer)
{
	const ContactDef* nextContact = introducer;
	int sanityCheck = 100;
	while ((nextContact = ContactFromStature(nextContact->nextStatureSet)) && nextContact->nextStatureSet && sanityCheck--)
		if (playerLevel <= nextContact->maxPlayerLevel)
			return nextContact;
	if (sanityCheck <= 0)
	{
		log_printf("storyerrs.log", "Could not determine after 100 iterations if %s will introduce a higher level contact for player level %d, so allowing it(%s)\n", introducer->displayname, playerLevel, introducer->filename);
		return introducer;
	}
	return NULL;
}


// Checks a contact to see if it is valid, calls recursively once for a next in chain contact if you outleveled it
int ContactIsValidIntroduction(Entity* player, ContactHandle candidate, int stature, StatureSetList excludeSet, const char* exclusiveSet, StoryInfo* info)
{
	int s, level = character_GetExperienceLevelExtern(player->pchar);
	int fail = 0;
	const ContactDef* def = g_contactlist.contactdefs[candidate-1];

	if (CONTACT_IS_DEPRECATED(def) || CONTACT_IS_DONTINTRODUCEME(def))
		return 0;

	// If this contact is not of the correct stature level,
	// it cannot be a candidate.
	if(stature && def->stature != stature)
		return 0;

	if(!def->statureSet)
		return 0;

	// If the contact is in the set of contacts that should be excluded,
	// it cannot be a candidate.
	for (s = 0; s < CONTACTS_PER_PLAYER; s++)
	{
		if (!excludeSet[s][0]) break;
		if (stricmp(def->statureSet, excludeSet[s]) == 0)
		{
			fail = 1;
			break;
		}
	}
	if (fail)
		return 0;

	// If the contact is not in the set of legal contacts,
	// it cannot be a candidate.
	if(exclusiveSet && 0 != stricmp(def->statureSet, exclusiveSet))
		return 0;

	// If the player already knows this contact,
	// it cannot be a candidate.
	if (info && ContactGetInfo(info, candidate))
		return 0;

	// If the contact requires a badge, he cannot be introduced
	if (def->requiredBadge)
	{
		const BadgeDef* required = badge_GetBadgeByName(def->requiredBadge);
		if (required && (required->iIdx >= 0))
		{
			int badgeIndex = required->iIdx;
			int badgeField = player->pl->aiBadges[badgeIndex];
			if (!BADGE_IS_OWNED(badgeField))
				return 0;
		}
	}
	
	//If the contact has a requires statement, he must pass the statement to be introduced
	if(def->interactionRequires && !chareval_Eval(player->pchar, def->interactionRequires, def->filename))
		return 0;

	// Skip introducing contacts in between that serve no purpose to you
	if (level > def->maxPlayerLevel)
	{
		const ContactDef* nextUsable = ContactIntroducesAcceptableLevelContact(level, def);
		if (nextUsable)
		{
			ContactHandle nextContact = ContactGetHandle(nextUsable->filename);
			if (nextContact == candidate)
				return candidate;
			if (ContactIsValidIntroduction(player, nextContact, 0, excludeSet, NULL, info))
				return nextContact;
		}
		return 0;
	}
    return candidate;
}

// produce the set of contacts satisfying the given criteria.
// info is optional - if set, the returned set of contacts will not be already in info
// result should be an eai, already created
// if justYesOrNo is set, all I care about is if there are any contacts.  Once you get the first, the answer is yes, so early out.
void ContactIntroductionSet(Entity* player, int stature, StatureSetList excludeSet, const char* exclusiveSet, StoryInfo* info, ContactHandle** result, int justYesOrNo)
{
	int i;
	if (!result || !result[0])
		return;

	for(i = 0; i < eaSize(&g_contactlist.contactdefs); i++)
	{
		ContactHandle validContact = ContactIsValidIntroduction(player, StoryIndexToHandle(i), stature, excludeSet, exclusiveSet, info);
		if (validContact && eaiFind(result, validContact) == -1)
		{
			eaiPush(result, validContact);
			if (justYesOrNo)
				return;
		}
	}
}

// if justYesOrNo is set, all I care about is if there are any contacts.  Once you get the first, the answer is yes, so early out.
static ContactHandle* ContactPickNewContactsInternal(Entity* player, int stature, StatureSetList excludeSet, const char* exclusiveSet, const char* exclusiveSet2, StoryInfo* info, StoryContactInfo* contactInfo, int justYesOrNo)
{
	static ContactHandle ret[4];
	ContactHandle* candidates = NULL;
	int i;
	int pickCount = 0;

	eaiCreate(&candidates);
	saSeed(contactInfo->contactIntroSeed);
	ret[0] = ret[1] = ret[2] = ret[3] = 0;

	// Find the first contact
	ContactIntroductionSet(player, stature, excludeSet, exclusiveSet, info, &candidates, justYesOrNo);
	if (eaiSize(&candidates))
	{
		i = saRoll(eaiSize(&candidates));
		ret[0] = candidates[i];

		if (justYesOrNo)
			return ret;

		PushStatureSet(excludeSet, ContactDefinition(ret[0])->statureSet);
	}
	eaiSetSize(&candidates, 0);

	// Then the second contact
	ContactIntroductionSet(player, stature, excludeSet, exclusiveSet2? exclusiveSet2 : exclusiveSet, info, &candidates, justYesOrNo);
	if (eaiSize(&candidates))
	{
		i = saRoll(eaiSize(&candidates));
		ret[1] = candidates[i];

		if (justYesOrNo)
			return ret;

		PushStatureSet(excludeSet, ContactDefinition(ret[1])->statureSet);
	}
	eaiSetSize(&candidates, 0);

	// return
	if (ret[1] && !ret[0])
	{
		ret[0] = ret[1];
		ret[1] = 0;
	}

	if(contactInfo->contact->def->contactsAtOnce < 3)
	{
		if (ret[0]) return ret;
		return NULL;
	}

	// Then the third contact
	ContactIntroductionSet(player, stature, excludeSet, exclusiveSet2? exclusiveSet2 : exclusiveSet, info, &candidates, justYesOrNo);
	if (eaiSize(&candidates))
	{
		i = saRoll(eaiSize(&candidates));
		ret[2] = candidates[i];

		if (justYesOrNo)
			return ret;

		PushStatureSet(excludeSet, ContactDefinition(ret[2])->statureSet);
	}
	eaiSetSize(&candidates, 0);

	// return
	if (ret[2] && !ret[1])
	{
		ret[1] = ret[2];
		ret[2] = 0;
	}

	if(contactInfo->contact->def->contactsAtOnce < 4)
	{
		if (ret[0]) return ret;
		return NULL;
	}

	// Then the fourth contact
	ContactIntroductionSet(player, stature, excludeSet, exclusiveSet2? exclusiveSet2 : exclusiveSet, info, &candidates, justYesOrNo);
	if (eaiSize(&candidates))
	{
		i = saRoll(eaiSize(&candidates));
		ret[3] = candidates[i];
	}

	// return
	if (ret[3] && !ret[2])
	{
		ret[2] = ret[3];
		ret[3] = 0;
	}

	if (ret[0]) return ret;
	return NULL;
}

// returns a pointer to two contact handles.  second or both may be NULL
// if justYesOrNo is set, the return value will be either NULL if there will be no contacts or not NULL if there will be a contact.
//   In this case, the returned contact may not be the contact actually picked once you try for real.
ContactHandle* ContactPickNewContacts(Entity* player, StoryInfo* info, StoryContactInfo* contactInfo, int* nextLevel, int justYesOrNo)
{
	int statureLevel;
	int nextLevelContact = 0;	// introduce to next level, otherwise same level
	const char* exclusiveSet = NULL;
	const char* exclusiveSet2 = NULL;
	int pickCount = 0;
	ContactHandle* newContacts = NULL;
	StatureSetList excludeSet;

	// task force contacts don't do introductions
	if (CONTACT_IS_TASKFORCE(contactInfo->contact->def))
		return NULL;

	// Figure out which kind of contact to introduce.
	//	Try to introduce a current level contact and then a next level contact.
	for(nextLevelContact = 0; nextLevelContact < 2; nextLevelContact++)
	{
		// Introduce a new contact to the player?
		// If no contacts can be picked for this stature level for some reason, try the next stature level.
		if(!ContactTimeForNewContact(player, contactInfo, nextLevelContact, NULL))
			continue;

		// Introduce a contact of which stature level?
		if (nextLevelContact)
			statureLevel = 0;
		else
			statureLevel = contactInfo->contact->def->stature;

		// what contacts do I have at this stature level already?
		GetExistingStatureSets(info, statureLevel, excludeSet);

		exclusiveSet = NULL;
		exclusiveSet2 = NULL;
		if(nextLevelContact)
		{
			exclusiveSet = contactInfo->contact->def->nextStatureSet;
			exclusiveSet2 = contactInfo->contact->def->nextStatureSet2;
		}

		// Pick new contacts of the chosen stature level.
		newContacts = ContactPickNewContactsInternal(player, statureLevel, excludeSet, exclusiveSet, exclusiveSet2, info, contactInfo, justYesOrNo);

		// If it is possible to pick contacts of the this stature level, return the results now.
		if(newContacts)
		{
			if(nextLevel)
				*nextLevel = nextLevelContact;
			return newContacts;
		}

		// Otherwise, try to pick contacts of the next stature level.
	}

	// Unable to pick any contacts at all.
	return NULL;
}

// determine if the contact is ready to introduce to a new contact
// nextLevel determines which contact we are checking for
int ContactTimeForNewContact(Entity* player, StoryContactInfo* contactInfo, int nextLevel, int* needLevel)
{
	if (PlayerInTaskForceMode(player)) 
		return 0;

	if (needLevel) 
		*needLevel = 0;

	if(nextLevel)
	{
		// if we don't do complete introductions, advance past it
		if (CONTACT_IS_NOCOMPLETEINTRO(contactInfo->contact->def))
		{
			contactInfo->contactsIntroduced |= CI_NEXT_LEVEL;
			return 0;
		}

		// if past failsafe level, just do the intro
		if (!(contactInfo->contactsIntroduced & CI_NEXT_LEVEL) &&
			(ContactPlayerPastFailsafeLevel(contactInfo, player) || 
			character_GetExperienceLevelExtern(player->pchar) > contactInfo->contact->def->maxPlayerLevel))
			return 1;

		// Is it time for this contact to introduce to the player to a higher level contact?
		if( !(contactInfo->contactsIntroduced & CI_NEXT_LEVEL) &&
			contactInfo->contactPoints >= contactInfo->contact->def->completeCP &&
			character_GetSecurityLevelExtern(player->pchar) >= contactInfo->contact->def->completePlayerLevel)
		{
			return 1;
		}
		// Would it be time to introduce the next contact if I was right level?
		else if (needLevel && 
			!(contactInfo->contactsIntroduced & CI_NEXT_LEVEL) &&
			contactInfo->contactPoints >= contactInfo->contact->def->completeCP)
		{
			*needLevel = contactInfo->contact->def->completePlayerLevel;
			return 0;
		}
	}
	else
	{
		// If a new contact introduction is flagged due to a task reward, do it
		if (contactInfo->rewardContact)
			return 1;

		// if we don't do friend introductions, advance past it
		if (CONTACT_IS_NOFRIENDINTRO(contactInfo->contact->def))
		{
			contactInfo->contactsIntroduced |= CI_SAME_LEVEL; 
			return 0;
		}

		// if past failsafe level, don't intro to same level
		if (ContactPlayerPastFailsafeLevel(contactInfo, player) || 
			character_GetExperienceLevelExtern(player->pchar) > contactInfo->contact->def->maxPlayerLevel)
			return 0;

		// Is it time to introduce a new contact?
		if( !(contactInfo->contactsIntroduced & CI_SAME_LEVEL) &&
			contactInfo->contact->def->friendCP && contactInfo->contactPoints >= contactInfo->contact->def->friendCP)
		{
			return 1;
		}
	}

	return 0;
}

int ContactPlayerPastFailsafeLevel(StoryContactInfo* contactInfo, Entity* player)
{
	if (TaskForceIsOnFlashback(player))
		return 0; // flash backs are always level safe

	if(character_GetExperienceLevelExtern(player->pchar) > contactInfo->contact->def->failsafePlayerLevel)
		return 1;
	return 0;
}

// *********************************************************************************
//  Debugging contact introductions
// *********************************************************************************

void DebugShowIntroHelper(ClientLink* client, ContactHandle** list)
{
	int i, n;
	n = eaiSize(list);
	for (i = 0; i < n; i++)
	{
		ContactHandle contact = (*list)[i];
		const ContactDef* def = ContactDefinition(contact);
		conPrintf(client, "\t%i\t%s\t(%i,%s)\n", contact, ContactDisplayName(def, 0), def->stature, def->statureSet);
	}
}

// show the introduction links for the specified contact
void ContactDebugShowIntroductions(ClientLink* client, char* filename)
{
	ContactHandle contact = ContactGetHandleLoose(filename);
	const ContactDef* def = ContactDefinition(contact);
	ContactHandle* others = NULL;
	StatureSetList exclude;
	memset(exclude, 0, sizeof(char)*CONTACTS_PER_PLAYER*MAX_SA_NAME_LEN);

	if (!contact || !def)
	{
		conPrintf(client, localizedPrintf(0,"CouldntFindContact", filename));
		return;
	}

	// friends
	eaiCreate(&others);
	if (!CONTACT_IS_NOFRIENDINTRO(def))
	{
		strcpy(exclude[0], def->statureSet);
		ContactIntroductionSet(client->entity, def->stature, exclude, NULL, NULL, &others, 0);
		conPrintf(client, "For friends %s will introduce:\n", ContactDisplayName(def,0));
		DebugShowIntroHelper(client, &others);
	}
	else
	{
		conPrintf(client, "%s will not do friend introductions\n", ContactDisplayName(def,0));
	}

	// completed
	eaiSetSize(&others, 0);
	exclude[0][0] = 0;

	if (CONTACT_IS_NOCOMPLETEINTRO(def))
	{
		conPrintf(client, "%s will not do a complete introduction\n", ContactDisplayName(def,0));
	}
	else if (def->nextStatureSet2)
	{
		ContactIntroductionSet(client->entity, 0, exclude, def->nextStatureSet, NULL, &others, 0);
		conPrintf(client, "Once completed %s gives a choice between:\n", ContactDisplayName(def,0));
		DebugShowIntroHelper(client, &others);
		eaiSetSize(&others, 0);
		ContactIntroductionSet(client->entity, 0, exclude, def->nextStatureSet2, NULL, &others, 0);
		conPrintf(client, "And:\n");
		DebugShowIntroHelper(client, &others);
	}
	else // only one next stature set
	{
		ContactIntroductionSet(client->entity, 0, exclude, def->nextStatureSet, NULL, &others, 0);
		conPrintf(client, "Once completed %s will introduce:\n", ContactDisplayName(def,0));
		DebugShowIntroHelper(client, &others);
	}

	eaiDestroy(&others);
}

void ContactDebugGetIntroduction(ClientLink* client, char* filename, int nextlevel)
{
	ContactHandle handle = ContactGetHandleLoose(filename);
	const ContactDef* def = ContactDefinition(handle);
	StoryInfo* info;
	StoryContactInfo* contactInfo;

	if (!handle || !def)
	{
		conPrintf(client, "Couldn't find contact %s\n", filename);
		return;
	}

	// add or get contact info
	info = StoryArcLockInfo(client->entity);
	contactInfo = ContactGetInfo(info, handle);
	if (!contactInfo)
	{
		if (!ContactAdd(client->entity, info, handle, "debug command"))
		{
			conPrintf(client, "Couldn't add contact %s\n", filename);
		}

		contactInfo = ContactGetInfo(info, handle);
		if (!contactInfo)
		{
			StoryArcUnlockInfo(client->entity);	
			return;
		}
	}

	// force conditions to be right for introduction
	if (nextlevel)
	{
		if (character_GetSecurityLevelExtern(client->entity->pchar) < contactInfo->contact->def->completePlayerLevel)
		{
			conPrintf(client, "You have to be level %i first\n", contactInfo->contact->def->completePlayerLevel);
		}
		else
		{
			contactInfo->contactsIntroduced = CI_SAME_LEVEL;
			contactInfo->contactRelationship = CONTACT_CONFIDANT;
			contactInfo->contactPoints = contactInfo->contact->def->completeCP;
		}
	}
	else
	{
		contactInfo->contactsIntroduced = CI_NONE;
		contactInfo->contactRelationship = CONTACT_FRIEND;
		contactInfo->contactPoints = contactInfo->contact->def->friendCP;
	}
	StoryArcUnlockInfo(client->entity);

	// start the contact dialog
	ContactDebugInteract(client, filename, 0);
}


