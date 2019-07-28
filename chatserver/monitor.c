#include "monitor.h"
#include "chatMonitor.h"
#include "error.h"
#include "comm_backend.h"
#include "timing.h"
#include "net_linklist.h"
#include "chatdb.h"
#include "reserved_names.h"
#include "shardnet.h"
#include "csr.h"
#include "performance.h"
#include "users.h"
#include "log.h"
#include "chatsqldb.h"

NetLinkList	monitor_links;
extern NetLinkList	net_links;


int monitorCreateCb(NetLink *link)
{
	MonitorLink	*client = link->userData;

	client->link = link;
	netLinkSetMaxBufferSize(link, BothBuffers, 8*1024*1024); // Set max size to auto-grow to
	netLinkSetBufferSize(link, BothBuffers, 1*1024*1024);

	return 1;
}

int monitorDeleteCb(NetLink *link)
{
	return 1;
}


int handleMonitorMsg(Packet *pak,int cmd, NetLink *link)
{
	switch(cmd)
	{
		xcase SVRMONTOCHATSVR_ADMIN_SENDALL:
			csrSendAllAnon(pktGetString(pak));
		xcase SVRMONTOCHATSVR_CONNECT:
			{
 				int protocol = pktGetBits(pak, 32);
 				if(protocol != CHATMON_PROTOCOL_VERSION)
				{
					Packet * pkt2 = pktCreateEx(link, CHATMON_PROTOCOL_MISMATCH);
					pktSendBits(pkt2, 32, CHATMON_PROTOCOL_VERSION);
					pktSend(&pkt2, link);

					lnkBatchSend(link);
					netRemoveLink(link);
				}
			}
		xcase SVRMONTOCHATSVR_SHUTDOWN:
			chatServerShutdown(0,pktGetString(pak));
			break;
		default:
			LOG_OLD_ERR("monitor_errs: Unknown command %d\n",cmd);
			return 0;
	}

	return 1;
}

void chatMonitorInit()
{
	netLinkListAlloc(&monitor_links,10,sizeof(MonitorLink),monitorCreateCb);
	netInit(&monitor_links,0,DEFAULT_CHATMON_PORT);
	monitor_links.destroyCallback = monitorDeleteCb;
	NMAddLinkList(&monitor_links, handleMonitorMsg);
}

void sendStatus(NetLink * link_out)
{	
	int i;
	MEMORYSTATUSEX memoryStatus;
	
	Packet * pak = pktCreateEx(link_out, CHATMON_STATUS);

	pktSendBitsPack(pak, 1, chatUserGetCount());
	pktSendBitsPack(pak, 1, chatChannelGetCount());
	pktSendBitsPack(pak, 1, g_online_count);
	pktSendBitsPack(pak, 1, GetReservedNameCount());


	pktSendF32(pak, g_stats.crossShardRate);
	pktSendF32(pak, g_stats.invalidRate);
	
	pktSendBitsPack(pak, 1, g_stats.send_rate);
	pktSendBitsPack(pak, 1, g_stats.recv_rate);

	pktSendBitsPack(pak, 1, g_stats.sendMsgRate);
	pktSendBitsPack(pak, 1, g_stats.recvMsgRate);

	pktSendBitsPack(pak, 1, monitor_links.links->size);

	// indiv link info
	pktSendBitsPack(pak, 1, net_links.links->size);
	for(i=0; i<net_links.links->size; i++)
	{
		NetLink * link = net_links.links->storage[i];
		ClientLink * client = (ClientLink*)link->userData;
		
		pktSendBitsPack(pak, 1, link->addr.sin_addr.S_un.S_addr);
		pktSendBitsPack(pak, 1, (client->linkType == kChatLink_Shard));

		pktSendBitsPack(pak, 1, client->online);
		pktSendString(pak, client->shard_name);

		pktSendBitsPack(pak, 1, pktRate(&link->sendHistory)>>10);
 		pktSendBitsPack(pak, 1, pktRate(&link->recvHistory)>>10);

//		pktSendBitsPack(pak, 1, client->sendMsgRate);
//		pktSendBitsPack(pak, 1, client->recvMsgRate);
	}

	// Send total memory usage numbers
	ZeroMemory(&memoryStatus,sizeof(MEMORYSTATUSEX));
	memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

	GlobalMemoryStatusEx(&memoryStatus);
	pktSendBits(pak, 32, memoryStatus.ullTotalPhys>>10);
	pktSendBits(pak, 32, memoryStatus.ullAvailPhys>>10);
	pktSendBits(pak, 32, memoryStatus.ullTotalPageFile>>10);
	pktSendBits(pak, 32, memoryStatus.ullAvailPageFile>>10);
//	pktSendBitsPack(pak, 2, num_processors);

	perfSendTrackedInfo(pak);

	pktSend(&pak, link_out);
}

void monitorTick()
{
	static int timer=0;
	static int lock=0;

	if (lock)
		return;
	lock++;

	if (timer==0) {
		timer = timerAlloc();
	}
	if (timerElapsed(timer) > 2.f) {
		timerStart(timer);
		
		perfGetList(); // should move up outside of for-loop
		netForEachLink(&monitor_links, sendStatus);
	}
	lock--;
}