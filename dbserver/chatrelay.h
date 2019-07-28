#ifndef _CHATRELAY_H
#define _CHATRELAY_H

#include "netio.h"

void shardChatRelay(Packet *pak,NetLink *link);
void shardChatMonitor();
void shardLogoutStranded(NetLink *link);
void shardChatInit();
void shardChatFlagForSendStatusToMap(NetLink *link);
extern NetLink shard_comm;

#endif
