
#include <stdio.h>
#include "comm_backend.h"
#include "error.h"
#include "dbinit.h"
#include "timing.h"
#include "svrmoncomm.h"
#include "servercfg.h"
#include "log.h"

// BeaconServer Comm ------------------------------------------------------------------------------

NetLinkList beacon_server_links = {0};

static struct {
	U32			updateTime;
	U32			longestWait;
} beacon_server_stats;

static int beaconServerConnectCallback(NetLink *link)
{
	LOG_OLD("new beacon server link from %s", makeIpStr(link->addr.sin_addr.s_addr));
	svrMonUpdate();
	return 1;
}

static int beaconServerDisconnectCallback(NetLink* link)
{
	svrMonUpdate();
	return 1;
}

static int dbHandleBeaconServerMsg(Packet *pak,int cmd, NetLink *link)
{
	switch(cmd)
	{
		xcase BEACON2DB_BEACONSERVER_STATUS:
		{
			U32 version = pktGetBitsPack(pak, 1);
			
			beacon_server_stats.longestWait = pktGetBitsPack(pak, 1);
			
			beacon_server_stats.updateTime = timerSecondsSince2000();
			
			svrMonUpdate();
		}
	}

	return 1;
}

U32 beaconServerCount(void)
{
	return beacon_server_links.links ? beacon_server_links.links->size : 0;
}

static void beaconServerCommInit(void)
{
	netLinkListAlloc(&beacon_server_links,5,sizeof(DBClientLink),beaconServerConnectCallback);
	if (!netInit(&beacon_server_links,0,DEFAULT_BEACONSERVER_PORT)) {
		printf("Error binding to beacon server port %d!\n", DEFAULT_BEACONSERVER_PORT);
	}
	beacon_server_links.destroyCallback = beaconServerDisconnectCallback;
	NMAddLinkList(&beacon_server_links, dbHandleBeaconServerMsg);
}

U32 beaconServerLongestWaitSeconds()
{
	if(	(	!server_cfg.do_not_launch_master_beacon_server ||
			beacon_server_stats.longestWait)
		&&
		(	!beacon_server_stats.updateTime ||
			timerSecondsSince2000() - beacon_server_stats.updateTime > 10))
	{
		return 9999;
	}
	
	return beacon_server_stats.longestWait;
}

// BeaconClient Comm ------------------------------------------------------------------------------

NetLinkList beacon_client_links = {0};

static int beaconClientConnectCallback(NetLink *link)
{
	LOG_OLD("new beacon client link from %s", makeIpStr(link->addr.sin_addr.s_addr));
	return 1;
}

static int beaconClientDisconnectCallback(NetLink* link)
{
	return 1;
}

static int dbHandleBeaconClientMsg(Packet *pak,int cmd, NetLink *link)
{
	// Don't do anything.
	return 0;
}

U32 beaconClientCount(void)
{
	return beacon_client_links.links ? beacon_client_links.links->size : 0;
}

static void beaconClientCommInit(void)
{
	netLinkListAlloc(&beacon_client_links,50,sizeof(DBClientLink),beaconClientConnectCallback);
	if (!netInit(&beacon_client_links,0,DEFAULT_BEACONCLIENT_PORT)) {
		printf("Error binding to beacon client port %d!\n", DEFAULT_BEACONCLIENT_PORT);
	}
	beacon_client_links.destroyCallback = beaconClientDisconnectCallback;
	NMAddLinkList(&beacon_client_links, dbHandleBeaconClientMsg);
}

// Common BeaconClient and BeaconServer stuff.

void beaconCommInit()
{
	beaconServerCommInit();
	beaconClientCommInit();
}

static void disconnectAtIP(NetLinkList* nll, U32 ip)
{
	S32 i;
	
	if(!nll->links)
	{
		return;
	}
	
	for(i = 0; i < nll->links->size; i++){
		NetLink* link = nll->links->storage[i];
		
		if(	link->connected &&
			!link->disconnected &&
			link->addr.sin_addr.S_un.S_addr == ip)
		{
			netSendDisconnect(link, 0);
		}
	}
}

void beaconCommKillAtIP(U32 ip)
{
	disconnectAtIP(&beacon_server_links, ip);
	disconnectAtIP(&beacon_client_links, ip);
}