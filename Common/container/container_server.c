/*\
 *
 *	container_server.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Common functions for container servers (raidserver, etc.)
 *
 */

#include "container_server.h"
#include "container_store.h"
#include "container_util.h"
#include "dbcomm.h"
#include "comm_backend.h"
#include "error.h"
#include "log.h"

ContainerStore** g_storelist; // null-terminated list of container stores

void csvrInit(ContainerStore** stores)
{
	g_storelist = stores;
}

ContainerStore* csvrStore(U32 list_id)
{
	ContainerStore** s = g_storelist;
	while (*s)
	{
		if ((*s)->list_id == list_id)
			return *s;
		s++;
	}
	return NULL;
}

void csvrDbConnect(void)
{
#define DB_CONNECT_TRIES 5
	int i;

	dbCommStartup();
	for (i = 0; i < DB_CONNECT_TRIES; i++)
	{
		if (dbTryConnect(15.0, DEFAULT_DB_PORT)) break;
		if (i < DB_CONNECT_TRIES-1)
		{
			printf("failed to connect to dbserver, retrying..\n");
			Sleep(60);
		}
	}
	if (i == DB_CONNECT_TRIES)
	{
		printf("failed to connect to dbserver, exiting..\n");
		exit(1);
	}
#undef DB_CONNECT_TRIES
}

void csvrSaveContainer(ContainerStore* cstore, U32 cid)
{
	/*
	char* data;
	ScheduledBaseRaid* br;

	br = BaseRaidGet(dbid);
	if (!br)
	{
		log_printf("arenaerrs", "BaseRaidSave - received bad dbid %i", dbid);
		return;
	}
	data = packageBaseRaid(br);
	dbAsyncContainerUpdate(CONTAINER_BASERAIDS, dbid, CONTAINER_CMD_CREATE_MODIFY, data, 0);
	free(data);
	UpdateConsoleTitle();
	*/
}

static int g_registered_list = 0;
int csvrRegisterSync(void)
{
	static PerformanceInfo* perfInfo;
	ContainerStore** s = g_storelist;
    Packet	*pak;
	int success = 1;

	do {
		printf("Registering to serve %s..\n", (*s)->name);
		pak = pktCreateEx(&db_comm_link,DBCLIENT_REGISTER_CONTAINER_SERVER);
		pktSendBitsPack(pak, 1, (*s)->list_id);
		pktSend(&pak, &db_comm_link);

		g_registered_list = 0;
		dbMessageScanUntil(__FUNCTION__, &perfInfo);
		success = success && (g_registered_list == (*s)->list_id);
		printf(" %s.\n", success? "succeeded": "failed");
		s++;
	} while (*s && success);
	return success;
}

void csvrGetNotifications(U32 notify_mask)
{
	Packet* pak = pktCreateEx(&db_comm_link, DBCLIENT_REGISTER_CONTAINER_SERVER_NOTIFY);
	pktSendBitsPack(pak, 1, notify_mask);
	pktSend(&pak, &db_comm_link);
}

void csvrRegisterAck(Packet* pak)
{
	ContainerStore* store;
	U32 list_id, max_session_id;

	list_id = pktGetBitsPack(pak, 1);
	max_session_id = pktGetBitsPack(pak, 1);
	store = csvrStore(list_id);
	if (store)
		store->max_session_id = max_session_id;
	g_registered_list = list_id;
}

void csvrNotify(Packet* pak)
{
	U32 list_id, cid, dbid;
	int add;

	list_id = pktGetBitsPack(pak, 1);
	cid = pktGetBitsPack(pak, 1);
	dbid = pktGetBitsPack(pak, 1);
	add = pktGetBits(pak, 1);
	dealWithNotify(list_id, cid, dbid, add);
}

void csvrDbMessage(int cmd,Packet* pak)
{
	switch (cmd)
	{
		case DBSERVER_CONTAINER_SERVER_ACK:
			csvrRegisterAck(pak);
		xcase DBSERVER_CONTAINER_SERVER_NOTIFY:
			csvrNotify(pak);
		xdefault:
			Errorf("Unknown server command: %i", cmd);
	}
}

U32 dealWithContainer(ContainerInfo *ci,int type)
{
	ContainerStore** s = g_storelist;
	while (*s)
	{
		if ((*s)->list_id == type)
		{
			cstoreHandleUpdate(*s, ci);
			return 1;
		}
		s++;
	}
	LOG_OLD_ERR("dealWithContainer: got update for type %i", type);
	return 0;
}

void csvrLoadContainers(void)
{
	ContainerStore** s = g_storelist;
	while (*s)
	{
		dbSyncContainerRequest((*s)->list_id, 0, CONTAINER_CMD_LOAD_ALL, 0);
		printf("%i %s loaded\n", cstoreCount(*s), (*s)->name);
		s++;
	}
}


