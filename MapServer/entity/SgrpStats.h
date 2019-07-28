/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SGRPSTATS_H
#define SGRPSTATS_H

#include "stdtypes.h"
#include "SgrpStatsCommon.h"

typedef struct NetLink NetLink;
typedef struct Packet Packet;
typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

bool sgrpstats_StatAdj( int idSgrp, char const *stat, int adjAmt );
void sgrpstats_FlushToDb(NetLink *link);
void sgrpstats_SendQueued(Packet *pak);
void sgrpstats_ReceiveToQueue(Packet *pak);
bool sgrpstats_Changed();
void sgrpstats_SetChanged(bool changed);

#if DBSERVER
void sgrpstats_RelayAdjs(Packet *pak, Packet *packOut); // for relaying between mapservers
#endif // DBSERVER

#if STATSERVER
void sgrpstats_FlushToSgrps();
#endif

#endif //SGRPSTATS_H
