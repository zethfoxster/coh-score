/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SGRPREWARDS_H
#define SGRPREWARDS_H

#include "stdtypes.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct Entity Entity;

void sgrprewards_PrestigeAdj(Entity *e, int adjAmt );
void sgrprewards_Tick(bool flush);

#endif //SGRPREWARDS_H
