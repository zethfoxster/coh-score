/*\
 *
 *	clue.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	functions relating to story clues
 *
 *  general story reward stuff is here right now too
 *
 */

#ifndef __CLUE_H
#define __CLUE_H

#include "storyarcinterface.h"
#include "storyarcCommon.h"

// *********************************************************************************
//  Clues
// *********************************************************************************

int ClueFindIndexByName(const StoryClue** clues, const char* clueName);
char* ClueFindDisplayNameByName(const StoryClue** clues, const char* clueName);
const char* ClueIconFile(const StoryClue* clue);
void ClueAddRemove(Entity *player, StoryInfo* info, const char* cluename, const StoryClue** clues, U32* clueStates, StoryTaskInfo* task, int *needintro, int add);
void ClueAddRemoveForCurrentMission(Entity *player, const char* cluename, int add);
int CluePlayerHasByName(Entity *pPlayer, const char *cluename);

// *********************************************************************************
//  Story Rewards
// *********************************************************************************

int StoryRewardApply(const StoryReward* reward, Entity* player, StoryInfo *info, StoryTaskInfo* task, StoryContactInfo* contactInfo, ContactResponse* response, Entity* speaker);
	// lots of weird stuff depending on how many parameters are filled out
void StoryRewardBonusTime(int bonusTime);
void StoryRewardFloatText(Entity* player, const char* floatText);
void StoryRewardHandleTokens(Entity *player, const char **addToPlayer, const char **addToAll, const char **removeFromPlayer, const char **removeFromAll);
void StoryRewardHandleSounds(Entity *player, const StoryReward *reward);

// clue.h includes everything clue related
#include "SouvenirClue.h"

#endif //__CLUE_H