/*
 *
 *	turnstileDb.h - Copyright 2010 NC Soft
 *		All Rights Reserved
 *		Confidential property of NC Soft
 *
 *	Turnstile system for matching players for large raid content
 *
 */

#ifndef TURNSTILEDB_H
#define TURNSTILEDB_H

#include "stdtypes.h"

void turnstileCommInit(void);
int turnstileServerSecondsSinceUpdate();

void turnstileDBserver_handleQueueForEvents(Packet *pak);
void turnstileDBserver_handleRemoveFromQueue(Packet *pak);
void turnstileDBserver_handleEventReadyAck(Packet *pak);
void turnstileDBserver_handleEventResponse(Packet *pak);
void turnstileDBserver_handleEventFailedStart(Packet *pak);
void turnstileDBserver_handleMapID(Packet *pak);
void turnstileDBserver_handleTurnstilePing(Packet *pak, NetLink *link);
void turnstileDBserver_handleShardXferOut(Packet *pak);
void turnstileDBserver_handleShardXferBack(Packet *pak);
void turnstileDBserver_handleGroupUpdate(Packet *pak_in);
void turnstileDBserver_generateCookieRequest(OrderId order_id, U32 dbid, int type, U32 dst_dbid, char *dst_shard);
void turnstileDBserver_handleCookieRequest(Packet *pak_in);
void turnstileDBserver_handleCookieReply(Packet *pak_in);
void turnstileDBserver_generateJumpRequest(U32 dbid);
void turnstileDBserver_handleCloseInstance(Packet *pak_in);
void turnstileDBserver_handleRejoinRequest(Packet *pak_in, NetLink *link);
void turnstileDBserver_handlePlayerLeave(Packet *pak_in);
void turnstileDBserver_removeFromQueueEx(int ent_id, int leader_id, int removeFromGroup);
void turnstileDBserver_handleIncarnateTrialComplete(Packet *pak_in);
void turnstileDBserver_handleQueueForSpecificMissionInstance(Packet *pak_in);
void turnstileDBserver_addBanDBId(Packet *pak_in);
void turnstileDBserver_handleCrashedIncarnateMap(int mapID);
void turnstileDBserver_init();

#endif // TURNSTILEDB_H
