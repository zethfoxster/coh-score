/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BASEUPKEEP_H
#define BASEUPKEEP_H

#include "stdtypes.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct Supergroup Supergroup;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

typedef struct RentRange
{
 	int nRentMax;
	F32 taxRate;
} RentRange;

typedef struct BaseUpkeepPeriod
{
	int period;
	bool shutBaseDown;
 	bool denyBaseEntry;
} BaseUpkeepPeriod;

typedef struct BaseUpkeep
{
	const char *filenameData;
	int periodRent;				// seconds
	const RentRange **ppRanges;		// always in order by prestige

	int periodResetRentDue;		// period at which rent due date resets to cur_time + period	
	const BaseUpkeepPeriod **ppPeriods; // info about what a late-paying sgrp can do 
} BaseUpkeep;

extern SHARED_MEMORY BaseUpkeep g_baseUpkeep;

void baseupkeep_Load();
F32 baseupkeep_PctFromPrestige(int Prestigetaxable);

#if SERVER || STATSERVER
int sgroup_nUpkeepPeriodsLate(Supergroup *sg); 
const BaseUpkeepPeriod *sgroup_GetUpkeepPeriod(Supergroup *sg);
#endif

#endif //BASEUPKEEP_H
