/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef ARENA_H__
#define ARENA_H__

typedef struct Entity Entity;

typedef struct ArenaScore
{
	int iScore;
	int dbid;
	char *pchName;
} ArenaScore;

extern ArenaScore **g_ppArenaScores;

void ArenaInit(void);
void ArenaClearScores(void);
void ArenaRemoveAllScores(void);

ArenaScore *ArenaAddScoreForTeam(int dbid, char *pchName);
ArenaScore *ArenaFindScoreForTeam(int dbid);

#endif /* #ifndef ARENA_H__ */

/* End of File */

