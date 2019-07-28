#include "power_stats.h"
#include "villain_stats.h"
#include "daily_stats.h"
#include "StashTable.h"
 
void trackPowerActivate(int id, char * zone, PlayerStats * po, char * vo, int time) {
	char cid[64];
	DamageStats *ds;
	char villainTypeName[MAX_NAME_LENGTH];

	sprintf(cid,"%d_%s",id,zone);

	if (!stashFindPointer(g_DamageTracker,cid,&ds)) {
		// This ID wasn't already in the table
		ds = (DamageStats*)malloc(sizeof(DamageStats));
		stashAddPointer(g_DamageTracker,cid,ds,true);
	}
	else {
		// This ID was already in the table
		saveDamageStatsToOwner(ds);
	}

	// Clean it up
	ds->powerID = id;
	ds->playerOwner = po;
	ds->time = time;

	if (vo && GetVillainTypeFromName(vo,villainTypeName)) {
		char petname[1000];
		PetStats * petstats = NULL;
		sprintf(petname,"%s_%s",zone,vo);
		if (stashFindPointer(g_PetTracker,petname,&petstats)) {
			ds->playerOwner = petstats->playerOwner;
			ds->villainOwner = petstats->villainOwner;
		}
		else {
			ds->villainOwner = GetIndexFromString(&g_VillainIndexMap, villainTypeName);
		}
	}
	else ds->villainOwner = -1;
	memset(ds->damage,0,MAX_DAMAGE_TYPES*sizeof(int));

	return;
}

void trackPowerEffect(int id, char * zone, int dindex, int damount, int time) {
	char cid[64];
	DamageStats * ds;

	sprintf(cid,"%d_%s",id,zone);

	if (!stashFindPointer(g_DamageTracker,cid,&ds)) return;

	// Update the damage and the last time this effect did anything
	ds->damage[dindex] += damount;
	ds->time = time;

}

void saveDamageStatsToOwner(DamageStats * ds) {
	int i;
	if (!ds) return;
	if (ds->playerOwner) {
		// Was a power owned by a player
		for (i=0; i<MAX_DAMAGE_TYPES; i++) {
			// Categorize as damage or heal and save it to the player's stats
			if (ds->damage[i] < 0) ds->playerOwner->damageDealtPerLevel[ds->playerOwner->currentLevel] -= ds->damage[i];
			else if (ds->damage[i] > 0) ds->playerOwner->healDealtPerLevel[ds->playerOwner->currentLevel] += ds->damage[i];
		}
	}
	else {
		// Was a power owned by a villain (or, currently, a pet)
		DailyStats * pDaily = GetCurrentDay(ds->time);
		for (i=0; i<MAX_DAMAGE_TYPES; i++) 
			pDaily->villain_stats[ds->villainOwner].damage[i] += ds->damage[i];
	}
}

// Makes a new damage tracking StashTable
//  All damage in the current table is either saved to its owner, if it is old, or
//   copied to the new table if it was recently active.
void cleanDamageStats(int time) {
	StashTable newStash;
	StashElement element;
	StashTableIterator it;

	newStash = stashTableCreateWithStringKeys(50000, StashDeepCopyKeys);

	stashGetIterator(g_DamageTracker, &it);
	while(stashGetNextElement(&it,&element))
	{
		DamageStats * ds = stashElementGetPointer(element);
		if (ds->time < time) {
			// It's old; save the damage to it's owner and free
			saveDamageStatsToOwner(ds);
			free(ds);
		}
		else {
			// It's not dead yet.  Copy it to the new stashtable
			stashAddPointer(newStash, stashElementGetKey(element), ds, true);
		}
	}
	stashTableDestroy(g_DamageTracker);
	g_DamageTracker = newStash;
}

void saveAllDamageStats() {
	StashElement element;
	StashTableIterator it;

	stashGetIterator(g_DamageTracker, &it);
	while(stashGetNextElement(&it, &element))
	{
		DamageStats * ds = stashElementGetPointer(element);
		saveDamageStatsToOwner(ds);
		// Clean out the damage
		memset(ds->damage,0,MAX_DAMAGE_TYPES*sizeof(int));
	}
}


