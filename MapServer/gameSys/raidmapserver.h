/*\
 *
 *	raidmapserver.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Mainly networking functions on the mapserver side for communicating
 *  about base raids
 *
 */

#ifndef RAIDMAPSERVER_H
#define RAIDMAPSERVER_H

#include "net_structdefs.h"
typedef struct Entity Entity;
typedef struct ContainerReflectInfo ContainerReflectInfo;
typedef struct Packet Packet;
typedef struct ClientLink ClientLink;

#define SGBASE_SHUTDOWN_TIMEOUT		300		// seconds without a player logged in

// raid client/server IO
void RaidFullyUpdateClient(Entity* ent);
void RaidHandleReflection(int* ids, int deleting, Packet* pak, ContainerReflectInfo** reflectinfo);
void ItemOfPowerHandleReflection(int* ids, int deleting, Packet* pak, ContainerReflectInfo** reflectinfo);
void ItemOfPowerGameHandleReflection(int* ids, int deleting, Packet* pak, ContainerReflectInfo** reflectinfo);
int RaidClientCallback(Packet *pak, int cmd, NetLink *link);
int RaidChallenge(Entity* e, U32 sgid);
int RaidDebugChallenge(Entity* e, char* sgname);
void RaidDebugSetVar(ClientLink* client, char* var, char* value);
int RaidAcceptInstantInvite(Entity* e, U32 inviter_dbid, U32 inviter_sgid);
void RaidRequestList(Entity* e);
void RaidHandleReceipt(U32 cmd, U32 listid, U32 cid, U32 user_cmd, U32 user_data, U32 err, char* str, Packet* pak);
void RaidClientError(U32 dbid, U32 err, char* str);
int RaidSGCanScheduleInstant(Entity* player); // does another raid prevent this player from scheduling an instant raid?
void RaidRequestParticipate(Entity* player, U32 raidid, int join);
void RaidRequestParticipateAndTransfer(Entity* player, U32 raidid, char* transfer_string);
void RaidSetBaseInfo(U32 sgid, int max_raid_size, int open_mount);
void RaidNotifyPlayerSG(Entity* e); // tell raid server what player type this supergroup is
void RaidSchedule(Entity* e, U32 sgid, U32 time, U32 raidsize);
void RaidCancel(Entity* e, U32 raidid);
void RaidSetWindow(Entity* e, U32 daybits, U32 hour);

// participation - valid on any mapserver, returns raid id or zero
U32 RaidSGIsAttacking(U32 sgid);	// if true, player may still not be participating
U32 RaidSGIsDefending(U32 sgid);
U32 RaidSGIsScheduledAttacking(U32 sgid);	// Has this raid scheduled a raid?
U32 RaidSGIsScheduledDefending(U32 sgid);
U32 RaidPlayerIsAttacking(Entity* player); // specifically, is this player participating?
U32 RaidPlayerIsDefending(Entity* player);
U32 RaidGetDefendingSG(U32 raidid);
U32 RaidHasOpenSlot(U32 sgid);				// does the supergroups's current running raid have an open slot?

// supergroup base stuff
int OnSGBase(void);
U32 SGBaseId(void);
void SGBaseSave(void);
void SGBaseSetInfoString(char* info);
void RaidSetInfoString(char *info);
void SGBaseEnter(Entity* ent, char *spawnlocation, char* errmsg);
void SGBaseEnterFromSgid(Entity* ent, char* spawnlocation, int idSgrp, char* errmsg);

//Item of Power Game Stuff
int IsCathedralOfPainOpen();
int ItemOfPowerGrantNew(Entity* e, int sgid);
int ItemOfPowerGrant( Entity* e, int sgid, char * itemOfPowerName );
int ItemOfPowerRevoke( Entity* e, int sgid, char * itemOfPowerName, int timeCreated );
int ItemOfPowerTransfer( Entity* e, int sgid, int newSgid, char * itemOfPowerName, int timeCreated );
int ItemOfPowerSGSynch( int sgid );
int ItemOfPowerGameSetState( Entity * e, char * newState );
void requestItemOfPowerGameStateUpdate();
void ShowItemOfPowerGameStatus( ClientLink* client );

// 2.0 IoP Game
int ItemOfPowerGrant2(Entity* e, char *iopName, int expireTime);
int ItemOfPowerRevoke2( Entity* e, const char *itemOfPowerName );

#endif // RAIDMAPSERVER_H
