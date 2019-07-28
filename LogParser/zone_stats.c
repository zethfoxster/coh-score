#include "zone_stats.h"
#include "player_stats.h"
#include <assert.h>
#include <stdlib.h>
#include <memory.h>


StashTable g_AllZones;
extern StashTable g_AllPlayers;



void ZoneStatsInit(ZoneStats * pStats)
{
	memset(pStats, 0, sizeof(ZoneStats));

	pStats->encounterActivityStash = stashTableCreateWithStringKeys(300, StashDeepCopyKeys);

	eaiCreate(&pStats->periodicSpawnedEncounters);
	eaiCreate(&pStats->periodicActiveEncounters);
	eaiCreate(&pStats->periodicActivePlayers);   

	eaiCreate(&pStats->periodicTotalConnections);
	eaiCreate(&pStats->periodicTotalDisconnects);   

	eaiCreate(&pStats->periodicTotalSpawns);
	eaiCreate(&pStats->periodicTotalCompletions);   
}


ZoneStats * GetZoneStats(const char * mapName)
{
	ZoneStats * pStats;
	

	if(!stashFindPointer(g_AllZones, mapName, &pStats))
	{
		pStats = malloc(sizeof(ZoneStats));
		ZoneStatsInit(pStats);
		stashAddPointer(g_AllZones, mapName, pStats, true);
	}

	assert(pStats);

	return pStats;	
}



void ZoneStatsUpdatePeriodStats(const char * mapName, ZoneStats * pStats, int time, int period)
{
	int activePlayers = 0;

	// update stats
	eaiPush(&pStats->periodicActiveEncounters, pStats->currentEncounterActiveCount);
	eaiPush(&pStats->periodicSpawnedEncounters, pStats->currentEncounterSpawnCount);

	activePlayers = CountActivePlayersInZone(mapName, time);
//	printf("\n*\n*\n*\nActive Players in %s: %d (full recount %d)\n*\n*\n*",mapName,pStats->activePlayerCount, activePlayers);
	eaiPush(&pStats->periodicActivePlayers,    activePlayers);
}


int CountActivePlayersInZone(const char * mapName, int time)
{
	StashElement element;
	StashTableIterator it;
	int count = 0;

	stashGetIterator(g_AllPlayers, &it);
	while(stashGetNextElement(&it, &element))
	{
		PlayerStats * pStats = stashElementGetPointer(element);
		if(!strcmp(mapName, pStats->currentMap)
			&& pStats->isPlaying
			&& ((time - pStats->lastEntryTime) < (60 * 5)))
		{
			count++;
		}
	}	

	return count;
}
