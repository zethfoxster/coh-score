/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SGRPBADGES_H
#define SGRPBADGES_H

typedef struct Supergroup Supergroup;
typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

void sgrpbadge_ServerInit(void);
bool sgrp_BadgeStatAdd(Supergroup *sg, const char *pchStat, int iAdd);
void sgrpbadges_TickOverHash(StashTable sgrpFromId);
bool sgrpbadge_GetIdxFromName(const char *pch, int *pi);
int sgrp_BadgeStatGet(Supergroup *sg, const char *pchStat);
bool sgrpbadge_GetIdxFromName(const char *pch, int *pi);
U32 *sgrpbadge_eaiStatesFromId(int id);

#endif //SGRPBADGES_H
