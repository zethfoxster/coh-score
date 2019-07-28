#ifndef _SHARDCOMMTEST_H
#define _SHARDCOMMTEST_H

typedef struct Packet Packet;
typedef struct NetLink NetLink;

void setShardCommTestParams(int clients, int startIdx);
void shardCommTestTick();
int shardTestMessageCallback(Packet *pak,int cmd,NetLink *link);
void shardCommTestReconnect();

extern int gChatServerTestClients;

#endif
