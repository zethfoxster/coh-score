/*\
 *
 *	container_server.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Common functions for container servers (raidserver, etc.)
 *
 */

#ifndef CONTAINER_SERVER_H
#define CONTAINER_SERVER_H

#include "container.h"

typedef struct ContainerStore ContainerStore;
typedef struct Packet Packet;

void csvrInit(ContainerStore** stores);	// null terminated list of stores
int csvrRegisterSync(void);	// waits indefinitely
void csvrGetNotifications(U32 notify_mask);
ContainerStore* csvrStore(U32 list_id);
void csvrDbConnect(void);	// make initial connection to DB, exit()'s if it fails
void csvrLoadContainers(void);
void csvrDbMessage(int cmd,Packet* pak);

extern ContainerStore** g_storelist;

void dealWithNotify(U32 list_id, U32 cid, U32 dbid, int add); // must be provided by container server

#endif // CONTAINER_SERVER_H