/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef STATDB_H
#define STATDB_H

#include "stdtypes.h"

typedef struct Supergroup Supergroup;
typedef struct Packet Packet;

void statdb_Init();
void statdb_Save(bool force); // save the database
void stat_sendToAllSgrpMembers(int db_id, char *msg);
void stat_sendToEnt(int idEnt, char *msg);
void statdb_SetSgStatDirty(int idSgrp);
Supergroup *stat_sgrpLockAndLoad(int idSgrp);
void stat_sgrpUpdateUnlock(int idSgrp, Supergroup *sg);

void miningdata_Save(float duration);
void miningdata_Receive(Packet *pak);


#endif //STATDB_H
