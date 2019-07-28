/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef STAT_SGRPBADGES_H
#define STAT_SGRPBADGES_H

#include "stdtypes.h"

typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct Supergroup Supergroup;

void sgrpbadges_TickOverHash(StashTable sgrpFromId);
void supergroup_MarkModifiedSgrpBadges(Supergroup *sg, char *category);
bool sgrpbadges_Locked();
char *badgestates_estrCmdFromStates(U32 *eaiStates);

#endif //STAT_SGRPBADGES_H
