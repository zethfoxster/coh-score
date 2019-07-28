/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef PL_STATS_INTERNAL_H__
#define PL_STATS_INTERNAL_H__

#ifndef STATS_BASE_H__
#include "stats_base.h"
#endif

typedef struct EntPlayer EntPlayer;

int stat_GetStatIndex(const char *pch);
	// Given the name of the statistic, return the index in the stat table.
	// Returns -1 if the stat isn't in the list.

int stat_GetStatCount(void);
	// Return the total number of stats stored per time interval.

char *stat_GetCategoryName(int num);
	// Given the index of a category, return the string representing it.

const char *stat_GetStatName(int num);
	// Given the index of a stat, return the string representing it.

char *stat_ZoneStatName(void);
	// Returns the stat name for the current zone.
char *stat_LevelStatName(int iLevel);
	// Returns the stat name for the given level.

int stat_GetByIdx(EntPlayer *pl, int per, int cat, int idx);
void stat_SetByIdx(EntPlayer *pl, int per, int cat, int idx, int i);
void stat_IncByIdx(EntPlayer *pl, int per, int cat, int idx);
void stat_DecByIdx(EntPlayer *pl, int per, int cat, int idx);
void stat_AddByIdx(EntPlayer *pl, int per, int cat, int idx, int i);
	// Does what they say, given the index of the item in the table.


// The following allow you to modify attribs on the basis of their name.
// They don't do any error checking, so give them good data.
//
static INLINEDBG  int stat_Get(EntPlayer *pl, int ePer, int eCat, char *pch)
	{ return stat_GetByIdx(pl, ePer, eCat, stat_GetStatIndex(pch)); }

static INLINEDBG  void stat_SetWithPeriod(EntPlayer *pl, int ePer, int eCat, const char *pch, int i)
	{ stat_SetByIdx(pl, ePer, eCat, stat_GetStatIndex(pch), i); }

static INLINEDBG  void stat_Set(EntPlayer *pl, int eCat, char *pch, int i)
	{ stat_SetByIdx(pl, kStatPeriod_Today, eCat, stat_GetStatIndex(pch), i); stat_SetByIdx(pl, kStatPeriod_ThisMonth, eCat, stat_GetStatIndex(pch), i);}

static INLINEDBG  void stat_Add(EntPlayer *pl, int eCat, char *pch, int i)
	{ stat_AddByIdx(pl, kStatPeriod_Today, eCat, stat_GetStatIndex(pch), i); stat_AddByIdx(pl, kStatPeriod_ThisMonth, eCat, stat_GetStatIndex(pch), i);}

// Special version of stat_add for setting your best race time
static INLINEDBG  void stat_SetIfGreater(EntPlayer *pl, int eCat, const char *pch, int i)
	{
		int old;

		old = stat_GetByIdx(pl, kStatPeriod_Today, eCat, stat_GetStatIndex(pch));
		if( i > old )
			stat_SetByIdx(pl, kStatPeriod_Today, eCat, stat_GetStatIndex(pch), i);

		old = stat_GetByIdx(pl, kStatPeriod_ThisMonth, eCat, stat_GetStatIndex(pch));
		if( i > old )
			stat_SetByIdx(pl, kStatPeriod_ThisMonth, eCat, stat_GetStatIndex(pch), i);
	}


static INLINEDBG  void stat_Inc(EntPlayer *pl, int eCat, const char *pch)
	{ stat_IncByIdx(pl, kStatPeriod_Today, eCat, stat_GetStatIndex(pch)); stat_IncByIdx(pl, kStatPeriod_ThisMonth, eCat, stat_GetStatIndex(pch));}

static INLINEDBG  void stat_Dec(EntPlayer *pl, int eCat, const char *pch)
	{ stat_DecByIdx(pl, kStatPeriod_Today, eCat, stat_GetStatIndex(pch)); stat_DecByIdx(pl, kStatPeriod_ThisMonth, eCat, stat_GetStatIndex(pch));}

#endif /* #ifndef PL_STATS_INTERNAL_H__ */

/* End of File */

