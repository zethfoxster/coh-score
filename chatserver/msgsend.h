#ifndef _MSGSEND_H
#define _MSGSEND_H

#include "chatdb.h"
#include "netio.h"

void msgFlushPacket(NetLink *link);
void sendMsg(NetLink *link,int auth_id,char *msg);
void resendMsg(User *user);
bool sendUserMsg(User *user,char const *fmt, ...);
bool sendSysMsg(User *user,char *channel_name,int is_error, char const *fmt, ...);
void sendAdminMsg(char * fmt, ...);

#endif
