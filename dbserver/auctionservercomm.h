/***************************************************************************
 *     Copyright (c) 2006-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef AUCTIONSERVERCOMM_H
#define AUCTIONSERVERCOMM_H

#include "account/AccountTypes.h"

typedef struct NetLink NetLink;
typedef struct Packet Packet;

int auctionServerSecondsSinceUpdate(void);
int auctionServerCount(void);
void auctionMonitorInit(void);
void auctionMonitorTick(void);

void auctionServerShardXfer(OrderId order_id, int dbid, char *dst_shard, int dst_dbid);

void handleAuctionXactReq(Packet *pak_in, NetLink *link);
void handleAuctionXactMultiReq(Packet *pak_in, NetLink *link);
void handleAuctionAddItm(Packet *pak_in, NetLink *link);
void handleAuctionClientReqInv(Packet *pak_in, NetLink *link);
void handleAuctionClientReqHistInfo(Packet *pak_in, NetLink *link);
void handleAuctionXactUpdate(Packet *pak_in, NetLink *link);
void handleSendAuctionCmd(Packet *pak_in, NetLink *link);
void handleRequestShardJump(Packet *pak_in, NetLink *link);
void handleAuctionPurgeFake(Packet *pak_in, NetLink *link);

NetLink *auctionLink(bool bThrottle);

#endif //AUCTIONSERVERCOMM_H
