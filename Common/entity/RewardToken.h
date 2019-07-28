/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef REWARDTOKEN_H
#define REWARDTOKEN_H

#include "stdtypes.h"

#define TOKEN_NONE			(0)
#define TOKEN_AQUIRED		(1<<0)
#define TOKEN_USED			(1<<1)



typedef struct RewardToken
{
	const char *	reward;
	int				val;	// arbitrary value associated with this string
	U32				timer;	// arbitrary timer associated with this string, usually last rewarded time
}RewardToken;


RewardToken* rewardtoken_Create(void);
RewardToken* rewardtoken_Copy(RewardToken *srcToken);
void rewardtoken_Destroy( RewardToken *item );

int rewardtoken_Award(RewardToken ***hTokens, char const * rewardtokenPiece, int val); // always updates timestamp
int rewardtoken_AwardEx(RewardToken ***hTokens, char const * rewardtokenPiece, int val, bool updateTime); // choose to update timestamp or not
bool rewardtoken_Unaward( RewardToken ***hTokens, char const * rewardtokenPiece);
int rewardtoken_IdxFromName( RewardToken ***hTokens, char const *rewardtokenPiece); // -1 if not found

typedef struct Entity Entity;

RewardToken * getRewardToken( Entity * e, const char * rewardTokenName );

#endif //REWARDTOKEN_H
