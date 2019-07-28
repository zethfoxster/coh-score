#ifndef _GLOBAL_STATS_H
#define _GLOBAL_STATS_H

#include "StashTable.h"
#include "Statistic.h"
#include "process_entry.h"
#include "common.h"





typedef struct GlobalStats
{
	int totalEntries;
	int totalConnections;
	int totalLogins;
	int totalConnectionTime;
	int totalMapTypeConnectionTime[MAP_TYPE_COUNT];
	int totalLevelTime[MAX_PLAYER_SECURITY_LEVEL];
	int startTime;
	int lastTime;
	int logStartTime;	// first entry time in log file currently being processed
	int totalLogTime;

	StashTable mapTimes;	// stash of 'Statistic' structs
	StashTable chatCommands;	//stash of ints

	StashTable encounterGroupsByMap;	// stash of stashes of 'EncounterGroupStats' :)

	int originsSelected[ORIGIN_TYPE_COUNT];
	int classesSelected[CLASS_TYPE_COUNT];
	int originClassCombinations[ORIGIN_TYPE_COUNT][CLASS_TYPE_COUNT];

	int minimumXPforLevel[MAX_PLAYER_SECURITY_LEVEL];

	Statistic completedLevelTimeStatistics[MAX_PLAYER_SECURITY_LEVEL];

	Statistic itemsBought[MAX_ITEM_TYPES];
	Statistic itemsSold[MAX_ITEM_TYPES];


	int xpPerLevel[MAX_PLAYER_SECURITY_LEVEL];
	int debtPayedPerLevel[MAX_PLAYER_SECURITY_LEVEL];
	int influencePerLevel[MAX_PLAYER_SECURITY_LEVEL][INCOME_TYPE_COUNT];
	int influenceSpentPerLevel[MAX_PLAYER_SECURITY_LEVEL][EXPENSE_TYPE_COUNT];
	int deathsPerLevel[MAX_PLAYER_SECURITY_LEVEL];
	int killsPerLevel[MAX_PLAYER_SECURITY_LEVEL];

	StashTable missionTimesStash;
	StashTable missionMapTimesStash;
	StashTable missionTypeTimesStash;

	StashTable actionCounts;

} GlobalStats;

extern GlobalStats g_GlobalStats;

void InitGlobalStats();


struct EncounterGroupStats
{
	int spawns;
	int completions;
};


typedef struct HourlyStats {
	// Play-hours being tracked
	float playTime;
	
	// Hourly xp and debt tracking
	int xpNormal[XPGAIN_TYPE_COUNT];
	int xpDebt[XPGAIN_TYPE_COUNT];
	int debt;
	int deaths;
	
	// Damage tracking
	int damage[MAX_DAMAGE_TYPES];

	// Hourly influence in/out tracking
	int influenceIncome[INCOME_TYPE_COUNT];
	int influenceExpense[EXPENSE_TYPE_COUNT];
} HourlyStats;

extern HourlyStats g_HourlyStats[MAX_PLAYER_SECURITY_LEVEL][CLASS_TYPE_COUNT];

void InitHourlyStats();

typedef struct LevelCompletionStats {
	// Per-level time to complete
	int time;

	// Per-level XP debt accumulation
	int debt;
	int deaths;

	// Per-level influence in/out tracking
	int influenceIncome[INCOME_TYPE_COUNT];
	int influenceExpense[EXPENSE_TYPE_COUNT];

} LevelCompletionStats;

typedef struct MissionCompletionStats {
	// Time spent on the mission
	int timeSuccess, timeFailure;

	// Number of missions run
	int countSuccess, countFailure;

} MissionCompletionStats;


#endif  // _GLOBAL_STATS_H
