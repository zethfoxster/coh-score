/*\
 *
 *	ArenaPlayer.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Arena server handling of all ArenaPlayer structures -
 *  keeps track of each player's rating, etc.
 *
 */

#include "arenaplayer.h"
#include "StashTable.h"
#include "earray.h"
#include "error.h"
#include "dbcontainer.h"
#include "dbcomm.h"
#include "containerArena.h"
#include "arenaserver.h"
#include "arenaupdates.h"
#include "arenaevent.h"
#include "log.h"

#define PLAYER_DISCONNECT_TIMEOUT	30	// seconds we wait after a player has disconnected before kicking him from events
										// this must at a minimum cover the lag between two mapservers claiming ownership

ArenaPlayer** g_players;
StashTable g_playerHash;

// return existing record or create new one
ArenaPlayer* PlayerGetAdd(U32 dbid, int initialLoad)
{
	char id[10];
	ArenaPlayer* result;

	if (!dbid)
	{
		LOG_OLD_ERR("Arena PlayerGetAdd passed dbid of zero");
		return NULL;
	}
	result = PlayerGet(dbid, !initialLoad);
	if (!result)
	{
		result = ArenaPlayerCreate();
		PlayerInit(result, dbid);
		eaPush(&g_players, result);
		sprintf(id, "%i", dbid);
		stashAddPointer(g_playerHash, id, result, false);
	}
	return result;
}

ArenaPlayer* PlayerGet(U32 dbid, int req_new)
{
	char id[10];
	ArenaPlayer *ap;

	if (!g_players)
		eaCreate(&g_players);
	if (!g_playerHash)
		g_playerHash = stashTableCreateWithStringKeys(100, StashDeepCopyKeys | StashCaseSensitive );

	sprintf(id, "%i", dbid);
	ap = stashFindPointerReturnPointer(g_playerHash, id);

	if(!ap && req_new)
	{
		dbSyncContainerRequest(CONTAINER_ARENAPLAYERS, dbid, CONTAINER_CMD_LOCK_AND_LOAD, 0);
		ap = stashFindPointerReturnPointer(g_playerHash, id);
		if (ap && ap->connected > ARENAPLAYER_LOGOUT)
		{
			ap->connected = ARENAPLAYER_DISCONNECTED;
			ap->lastConnected = dbSecondsSince2000();
		}
		return ap;
	}
	else 
		return ap;
}

ArenaPlayer** PlayerGetAll()
{
	return g_players;
}

// propagate to dbserver, clients
void PlayerSave(U32 dbid)
{
	char* data;
	ArenaPlayer* ap;

	ap = PlayerGet(dbid,1);
	if (!ap)
	{
		LOG_OLD_ERR( "Arena PlayerSave - received bad dbid %i", dbid);
		return;
	}
	data = packageArenaPlayer(ap);
	dbAsyncContainerUpdate(CONTAINER_ARENAPLAYERS, dbid, CONTAINER_CMD_CREATE_MODIFY, data, 0);
	free(data);
	ClientUpdatePlayer(dbid);
	UpdateArenaTitle();
}

int PlayerTotal(void)
{
	return eaSize(&g_players);
}

// do opening initialization on a new player
void PlayerInit(ArenaPlayer* ap, U32 dbid)
{
	int i;

	ap->dbid = dbid;
	for (i = 0; i < ARENA_NUM_RATINGS; i++)
	{
		ap->ratingsByRatingIndex[i] = ARENA_DEFAULT_RATING;
		ap->provisionalByRatingIndex[i] = 1;
	}
	ap->connected = ARENAPLAYER_CONNECTED;
}

// all players just got loaded
void PlayerListInit(void)
{
	// we just restarted, so everyone is disconnected and gets a fresh timer
	U32 now = dbSecondsSince2000();
	int i;
	for (i = 0; i < eaSize(&g_players); i++)
	{
		ArenaPlayer* ap = g_players[i];
		if (ap->connected > ARENAPLAYER_LOGOUT)
		{
			ap->connected = ARENAPLAYER_DISCONNECTED;
			ap->lastConnected = now;
		}
	}
}

void PlayerListTick(void)
{
	int i;
	U32 now = dbSecondsSince2000();

	// timeout players as needed
	for (i = 0; i < eaSize(&g_players); i++)
	{
		ArenaPlayer* ap = g_players[i];
		if (ap->connected == ARENAPLAYER_DISCONNECTED)
		{
			if (now - ap->lastConnected > PLAYER_DISCONNECT_TIMEOUT)
			{
				ap->connected = ARENAPLAYER_LOGOUT;
				EventPlayerLoggedOut(ap->dbid);
			}
		}
	}
}

// deal with player connection state
void PlayerSetConnected(U32 dbid, int connected)
{
	ArenaPlayer* ap = PlayerGet(dbid,1);
	if (ap)
	{
		ap->connected = connected? ARENAPLAYER_CONNECTED: ARENAPLAYER_DISCONNECTED;
		ap->lastConnected = dbSecondsSince2000();
	}
}

