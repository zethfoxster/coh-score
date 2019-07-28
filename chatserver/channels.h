#ifndef _CHANNELS_H
#define _CHANNELS_H

#include "chatdb.h"

void channelDeleteIfEmpty(Channel *channel);
void channelOnline(Channel *channel,User *user,int refresh);
void channelOffline(Channel *channel,User *user);
void channelJoin(User *user,char *channel_name,char *option);
void channelCreate(User *user,char *channel_name, char *option);
void channelLeave(User *user,char *channel_name, int isKicking);
void channelInviteDeny(User *user, char *channel_name, char *inviter_name);
void channelKill(User *user,char *channel_name);
void channelInvite(User *user,char **args, int count);
void channelCsrInvite(User *user,char **args, int count);

void channelSend(User *user,char *channel_name,char *msg);
void channelSendMessage(User *user, char *channel_name, char *msg, bool xmpp);
void channelSetUserAccess(User *user,char *channel_name,char *user_name,char *option);
void channelSetAccess(User *user,char *channel_name,char *option);
void channelCsrMembersAccess(User *user, char *channel_name, char *option);
void watchingList(User *user);
void channelList(User *user,char *channel_name);
void channelListMembers(User *user,char *channel_name);
void channelSetMotd(User *user,char *channel_name,char *motd);
void channelSetDescription(User *user, char *channel_name, char *description);
void channelSetTimout(User *user, char *channel_name, char *timeout);
void channelFind(User *user,char *mode,char *substring);
int channelIdx(User *user,Channel *channel);

bool sendUserMsgJoin(User *user,User *member,Channel *channel,int refresh);

Channel *channelFindByName(char *channel_name);
#endif
