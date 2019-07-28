/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef STATS_BASE_H__
#define STATS_BASE_H__

// #define STAT_TRACK_VILLAIN_TYPE // define to track every villain type defeat/victory. Very expensive: 550+ stats.
// #define STAT_TRACK_LEVEL        // define to track defeat/victory. Not expensive, but unused.

typedef enum StatCategory
{
	kStat_General,
	kStat_Kills,
	kStat_Deaths,
	kStat_Time,
	kStat_XP,
	kStat_Influence,
	kStat_Wisdom,
	kStat_ArchitectXP,
	kStat_ArchitectInfluence,
	kStat_Count
} StatCategory;

typedef enum StatPeriod
{
	kStatPeriod_Today,
	kStatPeriod_Yesterday,
	kStatPeriod_ThisMonth,
	kStatPeriod_LastMonth,

	kStatPeriod_Count
} StatPeriod;

typedef struct DBStatResult
{
	int *pIDs;
	int *piValues;
} DBStatResult;

typedef struct StatResultDesc
{
	const char *pchItem;
	StatCategory eCat;
	DBStatResult aRes[kStatPeriod_Count];
} StatResultDesc;


int stat_GetStatIndex(const char *pch);
const char *stat_GetStatName(int num);
char *stat_GetPeriodName(int num);
char *stat_GetCategoryName(int num);
int stat_GetStatCount(void);

DBStatResult *stat_GetTable(const char *pchName, StatCategory cat, StatPeriod per);

void InitStats(void);
	// Used to init the stats system. Should only be called once at program
	// start.


#endif /* #ifndef STATS_BASE_H__ */

/* End of File */

