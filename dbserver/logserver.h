#ifndef _LOGSERVER_H
#define _LOGSERVER_H

#include "netio.h"

int logNetInit();
void logMain();
void updateLogStats();
void logServerShutdown();
void setZMQsocketConnectState(int flag);
void storeZMQStatusInString(char* stringArray, int arrayLength);

typedef struct
{
	NetLink		*link;
	U32			*linked_auth_ids;
	U32			needs_chat_status:1;
} LogClientLink;

#endif
