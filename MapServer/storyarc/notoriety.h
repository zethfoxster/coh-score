/*\
 *
 *	notoriety.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 * 		Confidential property of Cryptic Studios
 *  
 *	deals with notoriety adjustment of tasks - allows players
 *  to go to special 'notoriety' contacts and change the level tasks
 *  are generated at
 *
 */

#ifndef NOTORIETY_H
#define NOTORIETY_H

#include "storyarcinterface.h"

#define MIN_LEVEL_ADJUST -1
#define MAX_LEVEL_ADJUST 4

// Praetorians in Praetoria can only have groups of 4. If they're in Pocket D
// or after they come to primal Earth they get the normal max size of 8.
#define MIN_TEAM_SIZE 1
#define MAX_TEAM_SIZE 8
#define MAX_PRAETORIAN_TEAM_SIZE 4


void difficultyLoad(void);

typedef struct StoryDifficulty StoryDifficulty;

StoryDifficulty *playerDifficulty(Entity* player);

void difficultyCopy( StoryDifficulty *pDifficultyDest, StoryDifficulty *pDifficultySrc );
void difficultySet( StoryDifficulty *pDifficulty, int levelAdjust, int teamSize, int alwaysAV, int dontReduceBoss );
void difficultySetForPlayer(Entity* player, int levelAdjust, int teamSize, int alwaysAV, int dontReduceBoss );
void difficultySetFromPlayer(StoryDifficulty *pDifficulty, Entity* player );

int difficultyDoAVReduction(StoryDifficulty * pDifficulty, int teamSize);
int difficultyAVLevelAdjust(StoryDifficulty * pDifficulty, int avLevel, int teamSize);

int difficultyContact(Entity* player, Contact* contact, int link, ContactResponse* response);
void difficultyUpdateTask(Entity* player, StoryTaskHandle *sahandle);

F32 exemplarXPMod(void);


#endif // NOTORIETY_H