/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "string.h"

#include "earray.h"

#include "entity.h"

#include "arena.h"
#ifdef SERVER
#include "arena_server.h"
#endif

ArenaScore **g_ppArenaScores;

/**********************************************************************func*
 * ArenaInit
 *
 */
void ArenaInit(void)
{
	eaCreate(&g_ppArenaScores);
}

/**********************************************************************func*
 * ArenaAddScoreForTeam
 *
 */
ArenaScore *ArenaAddScoreForTeam(int dbid, char *pchName)
{
	ArenaScore *pscore = ArenaFindScoreForTeam(dbid);

	if(!pscore)
	{
		pscore = calloc(sizeof(ArenaScore), 1);
		pscore->dbid = dbid;
		pscore->pchName = strdup(pchName);
		eaPush(&g_ppArenaScores, pscore);
	}

	return pscore;
}

/**********************************************************************func*
 * ArenaFindTeam
 *
 */
ArenaScore *ArenaFindScoreForTeam(int dbid)
{
	int i;
	ArenaScore *pscore = NULL;

	for(i=eaSize(&g_ppArenaScores)-1; i>=0; i--)
	{
		if(g_ppArenaScores[i]->dbid==dbid)
			pscore = g_ppArenaScores[i];
	}

	return pscore;
}

/**********************************************************************func*
 * ArenaRemoveScoreForTeam
 *
 */
void ArenaRemoveScoreForTeam(int dbid)
{
	int i;
	ArenaScore *pscore = NULL;

	for(i=eaSize(&g_ppArenaScores)-1; i>=0; i--)
	{
		if(g_ppArenaScores[i]->dbid==dbid)
		{
			eaRemove(&g_ppArenaScores, i);
			return;
		}
	}
}

/**********************************************************************func*
 * ArenaClearScores
 *
 */
void ArenaClearScores(void)
{
	int i;
	ArenaScore *pscore = NULL;

	for(i=eaSize(&g_ppArenaScores)-1; i>=0; i--)
	{
		g_ppArenaScores[i]->iScore = 0;
	}

#ifdef SERVER
	ArenaSendFullScoreUpdateToAll();
#endif
}

/**********************************************************************func*
 * ArenaRemoveAllScores
 *
 */
void ArenaRemoveAllScores(void)
{
	int i;
	ArenaScore *pscore = NULL;

	for(i=eaSize(&g_ppArenaScores)-1; i>=0; i--)
	{
		free(g_ppArenaScores[i]->pchName);
	}

	eaDestroy(&g_ppArenaScores);

#ifdef SERVER
	ArenaSendFullScoreUpdateToAll();
#endif
}


/* End of File */

