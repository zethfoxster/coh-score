#ifndef _POWER_STATS_H
#define _POWER_STATS_H

#include "StashTable.h"
#include "common.h"
#include "player_stats.h"

typedef struct DamageStats
{
	int	powerID;
	int time;
//	char key[MAX_NAME_LENGTH];
	PlayerStats * playerOwner;
	int villainOwner;
	int damage[MAX_DAMAGE_TYPES];

} DamageStats;

void trackPowerActivate(int id, char * zone, PlayerStats * po, char * vo, int time);
void trackPowerEffect(int id, char * zone, int dindex, int damount, int time);
void saveDamageStatsToOwner(DamageStats * ds);
void cleanDamageStats(int time);
void saveAllDamageStats();

#endif  // _PLAYER_STATS_H