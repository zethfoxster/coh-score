#include <stdio.h>
#include "waitingEntities.h"
#include "container.h"
#include "comm_backend.h"
#include "error.h"
#include "mapxfer.h"
#include "dblog.h"
#include "log.h"

typedef struct WaitingEntity 
{
	U32 db_id;
	EntCon *ent_con;
	U32 map_id;
	int is_map_xfer;
} WaitingEntity;

Array waitingEntities = {0}; // Array of entities waiting for their maps to start

U32	g_waiting_entity_peak_count;	// for monitoring peak size of waiting entities

MP_DEFINE(WaitingEntity);
WaitingEntity* createWaitingEntity(void)
{
	MP_CREATE(WaitingEntity, 16);
	return MP_ALLOC(WaitingEntity);
}
void destroyWaitingEntity(WaitingEntity *we)
{
	MapCon *map_con;
	map_con = containerPtr(dbListPtr(CONTAINER_MAPS), we->map_id);
	if (map_con) {
		map_con->num_players_waiting_for_start--;
	}
	MP_FREE(WaitingEntity, we);
}

int failureCallback(WaitingEntity *we, char *errmsg)
{
	int ret = 0;
	if (we->is_map_xfer) {
		doneWaitingMapXferFailed(we->ent_con, errmsg);
	} else {
		doneWaitingChoosePlayerFailed(we->ent_con, errmsg);
		ret = 1; // This always unloads or deletes it
	}
	return ret;
}

void successCallback(WaitingEntity *we)
{
	if (we->is_map_xfer) {
		doneWaitingMapXferSucceeded(we->ent_con, we->map_id);
	} else {
		finishSetNewMap(we->ent_con, we->ent_con->id, we->map_id, we->is_map_xfer);
		doneWaitingChoosePlayerSucceded(we->ent_con);
	}
}

// return 1 if unloaded
int waitingEntitiesRemove(EntCon *ent_con)
{
	int i;
	int ret = 0;
	for (i=0; i<waitingEntities.size; i++) {
		WaitingEntity *we = (WaitingEntity*)waitingEntities.storage[i];
		if (ent_con == we->ent_con) {
			arrayRemoveAndFill(&waitingEntities, i);
			i--;
			if (ent_con->is_gameclient)
				ret = failureCallback(we, "ClientDisconnect");  // Must be after removing from the array!
			destroyWaitingEntity(we);
		}
	}
	return ret;
}

int waitingEntitiesCheckEnt(int i)
{
	MapCon *map_con;
	WaitingEntity *we = (WaitingEntity*)waitingEntities.storage[i];
	EntCon *ent_con = containerPtr(dbListPtr(CONTAINER_ENTS),we->db_id);

	// DGNOTE 2/11/2010
	// There's some code over in container.c that should have dumped a stack track to the log files just before this assert went off.
	// The point of that is to try and trap who is nuking Ent's while they're waiting to transfer.  Go look at the DGNOTE over there with
	// the same date tag as this one, find the stack backtrace created there in the logs, and get the whole lot to me.
	assert(ent_con == we->ent_con && ent_con->id == we->db_id); // a player was unloaded while logging in. see entlog.log (or containerUnload())

	// check to make sure entity isn't locked if this isn't a mapxfer (entities are always owned during a mapxfer)
	if (!we->is_map_xfer) {
		if (ent_con->lock_id) {
			// This entity is locked by some map already (must be for a lock_and_load otherwise we're in big trouble!)
			return 0;
		}
	}

	map_con = containerPtr(dbListPtr(CONTAINER_MAPS), we->map_id);

	if (!map_con) {
		// Failed to start!
		LOG_OLD( "Map %d failed to start!  Dropping waiting player.\n", we->map_id);
		dbMemLog(__FUNCTION__, "Static map %d failed to start.  Dropping ent %s:%d (%s)", we->map_id, ent_con->ent_name, ent_con->id, we->is_map_xfer?"MapXfer":"GameClient");
		arrayRemoveAndFill(&waitingEntities, i);
		failureCallback(we, "MapFailed"); // Must be after removing from the array!
		destroyWaitingEntity(we);
		return 1;
	}

	if (map_con->starting)
		return 0;

	if (!map_con->active)
	{
		// Failed to start?
		LOG_OLD("Static map %d failed to start #2!  Dropping waiting player.\n", map_con->id);
		dbMemLog(__FUNCTION__, "Static map %d failed to start #2.  Dropping ent %s:%d (%s)", map_con->id, ent_con->ent_name, ent_con->id, we->is_map_xfer?"MapXfer":"GameClient");
		failureCallback(we, "MapFailed");
		arrayRemoveAndFill(&waitingEntities, i);
		destroyWaitingEntity(we);
		return 1;
	}

	dbMemLog(__FUNCTION__, "Map %d ready for ent %s:%d (%s)", map_con->id, ent_con->ent_name, ent_con->id, we->is_map_xfer?"MapXfer":"GameClient");
	arrayRemoveAndFill(&waitingEntities, i);
	successCallback(we);
	destroyWaitingEntity(we);
	return 1;
}


void waitingEntitiesCheck(void)
{
	int i;
	// Loop through them all and see if their map is ready and they're unlocked...
	for (i=waitingEntities.size-1; i>=0; i--) {
		waitingEntitiesCheckEnt(i);
	}
}

void waitingEntitiesAddEnt(EntCon *ent_con, U32 dest_map_id, int is_map_xfer, MapCon *destMap)
{
	WaitingEntity *we = createWaitingEntity();
	we->db_id = ent_con->id;
	we->ent_con = ent_con;
	we->is_map_xfer = is_map_xfer;
	we->map_id = dest_map_id;
	arrayPushBack(&waitingEntities, we);
	dbMemLog(__FUNCTION__, "Added: ent %s:%d moving to %d (%s)", ent_con->ent_name, ent_con->id, dest_map_id, is_map_xfer?"MapXfer":"GameClient");
	if (destMap) {
		destMap->num_players_waiting_for_start++;
	}
	waitingEntitiesCheckEnt(waitingEntities.size-1); // If this entity's map is ready, do it now!
	if ( (U32) waitingEntities.size > g_waiting_entity_peak_count)
		g_waiting_entity_peak_count = waitingEntities.size;
}

void waitingEntitiesDumpStats(void)
{
	int i;
	int xfer_count=0;
	int locked=0;
	for (i=waitingEntities.size-1; i>=0; i--) {
		WaitingEntity *we = waitingEntities.storage[i];
		if (we->is_map_xfer) {
			xfer_count++;
		} else {
			if (we->ent_con->lock_id)
				locked++;
		}
	}
	printf("%d Entities waiting\n", waitingEntities.size);
	printf("\t%d MapXfer\n", xfer_count);
	printf("\t%d GameClient\n", waitingEntities.size - xfer_count);
	printf("\t\t%d Locked\n", locked);
}

int waitingEntitiesIsIDWaiting(int id)
{
	int i;
	WaitingEntity *we;

	for (i = waitingEntities.size - 1; i >= 0; i--)
	{
		we = waitingEntities.storage[i];
		if (we->db_id == id)
		{
			return 1;
		}
	}
	return 0;
}

U32 waitingEntitiesGetPeakSize(bool bReset)
{
	U32 peak = g_waiting_entity_peak_count;
	if (bReset)
	{
		g_waiting_entity_peak_count = 0;
	}
	return peak;
}