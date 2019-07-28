
#include "daily_stats.h"
#include "StashTable.h"
#include "common.h"
#include "player_stats.h"
#include "parse_util.h"


extern StashTable g_AllPlayers;
extern GlobalStats g_GlobalStats;
DailyStats **g_DailyStats = NULL;

static int s_dayOffset = 0;

void DailyStatsInit(DailyStats * pDaily)
{
	memset(pDaily, 0, sizeof(DailyStats));
}


// handles stats that are not incremented w/ every entry (ex: player counts)
void DailyStatsFinalize(DailyStats * pDaily)
{
	StashElement element;
	StashTableIterator it;

    if(!pDaily)
        return;

	// count # of players per level
	stashGetIterator(g_AllPlayers, &it);
	while(stashGetNextElement(&it,&element))
	{
		PlayerStats * pPlayer = stashElementGetPointer(element);
		pDaily->players_per_level[pPlayer->currentLevel][pPlayer->classType]++;
	}
}


int GetDay(int time)
{
	// hack, may need to adjust for timezone & daylight savings time...
	return (time / (60 * 60 * 24));
}

const char * GetDateStrFromDayOffset(int day)
{
	int time = (day + s_dayOffset) * (60 * 60 * 24);

	return sec2Date(time);
}

int GetCurrentDayIndex(int time)
{
	int day;
	if(!s_dayOffset)
	{
		s_dayOffset = GetDay(g_GlobalStats.startTime);
	}

	day = GetDay(time) - s_dayOffset;

	return day;
}

DailyStats * GetCurrentDay(int time)
{
	int day = GetCurrentDayIndex(time);
	static int lastDay = -1;

	// make sure daily stats array is big enough
	int size = eaSizeUnsafe(&g_DailyStats);
	if(day >= size)
	{
		eaSetSize(&g_DailyStats, (day + 1));
		size = eaSizeUnsafe(&g_DailyStats);

		if(lastDay > -1)
		{	
			assert(lastDay < eaSizeUnsafe(&g_DailyStats));
			DailyStatsFinalize(g_DailyStats[lastDay]);
		}
	}



	if( ! g_DailyStats[day])
	{
		g_DailyStats[day] = malloc(sizeof(DailyStats));
		DailyStatsInit(g_DailyStats[day]);
	}

	lastDay = day;

	return (DailyStats*) g_DailyStats[day];
}

void DailyStatsFinalizeAll()
{
	int size = eaSizeUnsafe(&g_DailyStats);

//	assert(size);

	DailyStatsFinalize(g_DailyStats[ (size - 1) ]);
}
