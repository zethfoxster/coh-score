
#ifndef _DAILY_STATS
#define _DAILY_STATS

#include "common.h"

typedef struct PowerStats
{
	int activated;
	int hits;
	int misses;		// redundant, for debugging
	int player_count;
} PowerStats;


typedef struct VillainStats
{
	int player_kills;
	int deaths;
	int created;
	int damage[MAX_DAMAGE_TYPES];

} VillainStats;



typedef struct InfluenceStats
{
	int buyEnhancements;
	int buyInspirations;
	int sellEnhancements;
	int sellInspirations;
	int destroyedByCommand;
	int createdByCommand;
	int earnedByKill;
	int earnedByCompleteMission;
	int earnedbyCompleteTask;
	int unknown;

} InfluenceStats;



typedef struct DailyStats
{
	//int time_per_level[MAX_PLAYER_SECURITY_LEVEL][CLASS_TYPE_COUNT];
	int players_per_level[MAX_PLAYER_SECURITY_LEVEL][CLASS_TYPE_COUNT];
	int deaths_per_level[MAX_PLAYER_SECURITY_LEVEL][CLASS_TYPE_COUNT];
	int kills_per_level[MAX_PLAYER_SECURITY_LEVEL][CLASS_TYPE_COUNT];
	int damage_per_level[MAX_PLAYER_SECURITY_LEVEL][MAX_DAMAGE_TYPES];

	PowerStats power_stats[MAX_POWER_TYPES];
	VillainStats villain_stats[MAX_POWER_TYPES];

	InfluenceStats influence;

} DailyStats;

DailyStats * GetCurrentDay(int time);
const char * GetDateStrFromDayOffset(int day);

void DailyStatsFinalizeAll();

#endif  // _DAILY_STATS
