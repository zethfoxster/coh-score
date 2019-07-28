/***************************************************************************
 *     Copyright (c) 2006-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * $Workfile: $
 * $Revision: 1.36 $
 * $Date: 2006/02/07 06:13:59 $
 *
 * Module Description:
 *
 * Revision History:
 *
 ***************************************************************************/
#ifndef AUCTIONCMDS_H
#define AUCTIONCMDS_H

#include "stdtypes.h"

typedef struct GenericHashTableImp *GenericHashTable;
typedef struct HashTableImp *HashTable;
typedef struct AuctionEnt AuctionEnt;
typedef struct Xaction Xaction;

void AuctionServer_HandleSlashCmd(AuctionEnt *ent, int message_dbid, char *str);

// AuctionServer_HandleLoggedSlashCmd should only be called by XactServer.c
// void AuctionServer_HandleLoggedSlashCmd(Xaction *xact, AuctionEnt *ent, char *str, XactServerMode svr_mode);

#endif //AUCTIONCMDS_H
