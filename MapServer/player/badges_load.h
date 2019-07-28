/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BADGES_LOAD_H
#define BADGES_LOAD_H

#include "stdtypes.h"
typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

int LoadBadgeStatNames(char *fileName, char *altFileName, StashTable hashBadgeStatNames, StashTable ghNameFromIdx );
StashTable LoadBadgeStatUsage(StashTable hashBadgeStatUsage, const BadgeDefs *badgeDefs, char *dataFilename);


#endif //BADGES_LOAD_H
