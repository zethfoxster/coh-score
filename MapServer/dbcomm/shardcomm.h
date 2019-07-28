#ifndef _SHARDCOMM_H
#define _SHARDCOMM_H

#include "netio.h"

typedef struct Entity Entity;
typedef struct ClientLink ClientLink;

void shardCommMonitor();
void shardCommSendf(Entity *e, bool check_banned, char *fmt, ...);
void shardCommSend(Entity *e, bool check_banned, char *str);
void shardCommStatus(Entity *e);
void shardCommConnect(Entity *e);
void shardCommLogin(Entity *e, int force);
void shardCommLogout(Entity *e);
int shardMessageCallback(Packet *pak, int cmd, NetLink *link);
void shardCommReconnect();
int chatServerRunning();
int getAuthId(Entity *e);
void shardCommSendInternal(int id, const char *str);
void handleGenericShardCommCmd(ClientLink *client, Entity *e, char *cmd, char *name);
void handleGenericAuthShardCommCmd(ClientLink *client, Entity *e, char *cmd, int authId);
void handleSgChannelInviteHelper(Packet *pak);

#endif
