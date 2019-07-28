/*\
 *
 *	raidinfo.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Some information about the raid game saved to supergroups
 *
 */

#ifndef RAIDINFO_H
#define RAIDINFO_H

#include "stdtypes.h"

typedef struct Packet Packet;
typedef struct SupergroupRaidInfo SupergroupRaidInfo;

void RaidInfoUpdate(U32 sgid, int deleting);
void RaidInfoDestroy(U32 sgid);
SupergroupRaidInfo* RaidInfoGetAdd(U32 sgid);

void handleRequestSetWindow(Packet* pak, U32 listid, U32 cid);
void handleBaseSettings(Packet* pak, U32 listid, U32 cid);
void handleRequestRaidList(Packet* pak, U32 listid, U32 cid);
void handleNotifyVillainSG(Packet* pak, U32 listid, U32 cid);

// info for raids, etc.
int SGHasOpenMount(U32 sgid);
int SGWindowIsScheduled(U32 sgid, U32 windowstart, U32 windowend);
void SGGetOpenRaidWindows(U32 sgid, U32* first, U32* second);
int SGCanBeRaided(U32 sgid, U32 attackersg, U32 time, U32 raidsize, char* err); // all params other than sgid are optional

#endif // RAIDINFO_H