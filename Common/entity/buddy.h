/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BUDDY_H__
#define BUDDY_H__

typedef struct Entity Entity;
typedef struct Character Character;

typedef enum EBuddyType
{
	kBuddyType_None,
	kBuddyType_Pending,
	kBuddyType_Mentor,
	kBuddyType_Sidekick,
	kBuddyType_Exemplar,
	kBuddyType_Count
} EBuddyType;


typedef struct Buddy // no longer used on the client
{
	int iLastCombatLevel;
		// The combat level of the character on the last tick. Used to
		//   notify players due to changes in combat level.

	int iLastTeamLevel;
		// used to reset pending timers when level changes

	EBuddyType eType;
		// Where the sidekicking is with respect to this character. Updated
		//   each tick on the server.

	U32 uTimeLastMentored;
		// The last time the player was sidekicked. Used to make sure that
		//   players don't manipulate their combat level to tweak the
		//   rewards they get. (number of seconds since 2000)

	int iCombatLevelLastMentored;
		// The combat level of the player when the sidekick duo was broken.

	U32 uTimeStart;
		// The number of seconds since 2000 when the mentor/sidekick
		//   entered mentor/sidekick mode. Used to track badge stats.
} Buddy;

#define PENDING_BUDDY_TIME 20

#define EXEMPLAR_GRACE_LEVELS 5

bool character_IsSidekick(Character *p, int pending_counts );
bool character_IsMentor(Character *p );
bool character_IsExemplar(Character *p, int pending_counts );
bool character_IsPending(Character *p);
int character_TeamLevel(Character *p);

#endif /* #ifndef BUDDY_H__ */

/* End of File */

