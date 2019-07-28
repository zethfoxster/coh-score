#ifndef _ZONE_STATS_H
#define _ZONE_STATS_H

#include "StashTable.h"
#include "EArray.h"
#include "common.h"

#define	NUMBER_OF_BLOCKS	100


typedef struct EncounterStats
{
	BOOL isSpawned;
	BOOL isActive;
	int totalSpawns;
	int totalActive;
	int totalComplete;

} EncounterStats;


#define ZONE_STRUCT_VERSION 1
typedef struct ZoneStats
{
	StashTable encounterActivityStash;
	int deathsPerBlock[NUMBER_OF_BLOCKS][NUMBER_OF_BLOCKS];

	int activePlayerCount;
	int activeEncounterCount;

	int currentEncounterSpawnCount;
	int currentEncounterActiveCount;

	int totalPlayerConnects;
	int totalPlayerDisconnects;

	int timeSpentPerLevel[MAX_PLAYER_SECURITY_LEVEL / 5];  // increments of 5

	EArrayIntHandle periodicSpawnedEncounters;
	EArrayIntHandle periodicActiveEncounters;
	EArrayIntHandle periodicActivePlayers;   

	EArrayIntHandle periodicTotalConnections;
	EArrayIntHandle periodicTotalDisconnects;   

	EArrayIntHandle periodicTotalSpawns;
	EArrayIntHandle periodicTotalCompletions;   

} ZoneStats;


void ZoneStatsInit(ZoneStats * pStats);
ZoneStats * GetZoneStats(const char * mapName);
void ZoneStatsUpdatePeriodStats(const char * mapName, ZoneStats * pStats, int time, int period);
int CountActivePlayersInZone(const char * mapName, int time);


#endif  // _ZONE_STATS_H
