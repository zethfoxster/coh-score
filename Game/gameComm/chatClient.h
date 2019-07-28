#ifndef CHAT_CLIENT_H__
#define CHAT_CLIENT_H__

#include "stdtypes.h"

void chatClientInit();
bool processShardCmd(char **args,int count);
int UsingChatServer();

bool canChangeChatHandle();
bool canInviteToChannel(char * channelName);

// debug only (copied from chatserver\channels.c)
typedef struct AtlasTex AtlasTex;
AtlasTex * flagIcon(int flags);


#endif  // CHAT_CLIENT_H__

