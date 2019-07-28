/*\
 *
 *	raidserver.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Handles central server functions for scheduled base raids
 *
 */

#ifndef RAIDSERVER_H
#define RAIDSERVER_H

#include "raidstruct.h"
#include "comm_backend.h"

void UpdateConsoleTitle(void);

ScheduledBaseRaid* BaseRaidGetAdd(U32 dbid, int initialLoad);
ScheduledBaseRaid* BaseRaidGet(U32 dbid);
void BaseRaidSave(U32 dbid);
int BaseRaidTotal(void);
int RaidServerHandleMsg(Packet *pak,int cmd, NetLink *link);
void RemoveAllScheduledRaids( int issueRefund );
void RemoveSGsScheduledRaids(U32 sgid, int removeAttacks, bool removeDefends, bool issueRefund );
void VerifyAllExistingRaids();
#endif // RAIDSERVER_H