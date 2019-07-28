/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BUDDY_SERVER_H__
#define BUDDY_SERVER_H__

typedef struct Character Character;
typedef struct TeamMembers TeamMembers;

int character_CalcCombatLevel(Character *p);
	// Calculates the level the character fights at. If a sidekick, then
	//   it's based on the mentor's level. If not, then it's based on the
	//   character's actual XP.

void character_HandleCombatLevelChange(Character *p);
	// If the combat level has changed since the last time this function was
	//   called, update the character's HP, etc and send them a message.

void Buddy_Tick(Character *p, float fRate);

bool character_IsTFExemplar(Character *p);


void character_AcceptLevelChange(Character *p, int levelAccepted);

void character_updateTeamLevel(Character *p);

#endif 

/* End of File */

