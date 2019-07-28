#include <stdio.h>
#include "comm_backend.h"
#include "netio.h"
#include "structNet.h"
#include "error.h"
#include "dbdispatch.h"
#include "timing.h"
#include "log.h"

typedef struct ArenaServerInfo {
	NetLink* link;
} ArenaServerInfo;

ArenaServerInfo arena_state = {0};
NetLinkList arena_links = {0};

static int connectCallback(NetLink *link)
{
	arena_state.link = link;
	dbClientCallback(link);
	LOG_OLD( "new arena server link from %s", makeIpStr(link->addr.sin_addr.s_addr));
	return 1;
}

static int disconnectCallback(NetLink* link)
{
	if (link == arena_state.link)
		arena_state.link = NULL;
	netLinkDelCallback(link);
	return 1;
}

void arenaCommInit(void)
{
	netLinkListAlloc(&arena_links,50,sizeof(DBClientLink),connectCallback);
	if (!netInit(&arena_links,0,DEFAULT_DBARENA_PORT)) {
		printf("Error binding to port %d!\n", DEFAULT_DBARENA_PORT);
	}
	arena_links.destroyCallback = disconnectCallback;
	NMAddLinkList(&arena_links, dbHandleClientMsg);
}

int arenaServerCount(void)
{
	return arena_state.link != NULL? 1: 0;
}

void handleRequestArena(Packet* pak,NetLink* link)
{
	Packet* ret = pktCreateEx(link,DBSERVER_ARENA_ADDRESS);
	if (arena_state.link && arena_state.link->connected)
	{
		pktSendBits(ret, 1, 1);
		pktSendString(ret, makeIpStr(arena_state.link->addr.sin_addr.S_un.S_addr));
	}
	else
		pktSendBits(ret, 1, 0);
	pktSend(&ret,link);
}

int arenaServerSecondsSinceUpdate(void)
{
	if (arena_state.link) {
		return timerCpuSeconds() - arena_state.link->lastRecvTime;
	} else {
		return 9999;
	}
}
