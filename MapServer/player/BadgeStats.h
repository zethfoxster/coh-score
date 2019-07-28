/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BADGESTATS_H
#define BADGESTATS_H

#include "stdtypes.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

typedef struct BadgeStatHashes
{	
	StashTable idxStatFromName; // look up the index of a stat by name
	StashTable aIdxBadgesFromName; // look up the array of badge idxs affected by named stat. g_hashBadgeStatUsage
	StashTable statnameFromId; //
	int idxStatMax; // max stat idx
} BadgeStatHashes;

extern BadgeStatHashes g_BadgeStatsSgroup;



#endif //BADGESTATS_H
