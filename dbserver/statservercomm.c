/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 */
#include <stdio.h>
#include "comm_backend.h"
#include "SgrpStatsCommon.h"
#include "netio.h"
#include "structNet.h"
#include "error.h"
#include "dbdispatch.h"
#include "timing.h"
#include "statservercomm.h"
#include "launchercomm.h" 
#include "log.h"

StatCon g_stat_con = {0};

typedef struct StatServerInfo {
	NetLink* link;
} StatServerInfo;

StatServerInfo stat_state = {0};
NetLinkList stat_links = {0};

NetLink *statLink()
{
	return stat_state.link;
}


static int connectCallback(NetLink *link)
{
	stat_state.link = link;
	dbClientCallback(link);
	g_stat_con.starting = false;
	LOG_OLD( "new stat server link from %s", makeIpStr(link->addr.sin_addr.s_addr));
	return 1;
}

static int disconnectCallback(NetLink* link)
{
	if (link == stat_state.link)
		stat_state.link = NULL;
	netLinkDelCallback(link);
	return 1;
}

void statCommInit(void)
{
	netLinkListAlloc(&stat_links,50,sizeof(DBClientLink),connectCallback);
	if (!netInit(&stat_links,0,DEFAULT_DBSTAT_PORT)) {
		printf("Error binding to port %d!\n", DEFAULT_DBSTAT_PORT);
	}
	stat_links.destroyCallback = disconnectCallback;
	NMAddLinkList(&stat_links, dbHandleClientMsg);
}

int statServerCount(void)
{
	return stat_state.link != NULL? 1: 0;
}


static void sgrpstats_RelayAdjs(Packet *pak, Packet *pakOut)
{
	int nIds = pktGetBitsPack( pak, SGRPSTATS_NIDS_PACKBITS); 
	int i;

	pktSendBitsPack( pakOut, SGRPSTATS_NIDS_PACKBITS, nIds); 
	for( i = 0; i < nIds; ++i ) 
	{
		int idSgrp = pktGetBitsPack( pak, SGRPSTATS_IDSGRP_PACKBITS); 
		int nAdjs = pktGetBitsPack( pak, SGRPSTATS_NADJS_PACKBITS); 
		int j;
		
		pktSendBitsPack( pakOut, SGRPSTATS_IDSGRP_PACKBITS, idSgrp); 
		pktSendBitsPack( pakOut, SGRPSTATS_NADJS_PACKBITS, nAdjs); 

		for( j = 0; j < nAdjs; ++j ) 
		{
			int idStat = pktGetBitsPack( pak, SGRPSTATS_IDSTAT_PACKBITS); 
			int adjAmt = pktGetBitsPack( pak, SGRPSTATS_ADJAMT_PACKBITS);
			
			pktSendBitsPack( pakOut, SGRPSTATS_IDSTAT_PACKBITS, idStat); 
			pktSendBitsPack( pakOut, SGRPSTATS_ADJAMT_PACKBITS, adjAmt);
		}
	}
}

void handleSgrpStatAdj(Packet* pak,NetLink* clientLink)
{
	if( pak && clientLink && stat_state.link )
	{
		// create packet to forward stats to sgrp stat server
		Packet *pakStat = pktCreateEx(stat_state.link,DBSERVER_SGRP_STATSADJ);
		sgrpstats_RelayAdjs( pak, pakStat );
		pktSend(&pakStat,stat_state.link);
	}
}

int statServerSecondsSinceUpdate(void)
{
	if (stat_state.link) {
		return timerCpuSeconds() - stat_state.link->lastRecvTime;
	} else {
		return 9999;
	}
}

void handleMiningDataRelay(Packet *pak)
{
	Packet* pak_out = NULL;
	NetLink *linkStat = statLink();
	int i, count = pktGetBitsPack(pak,1);

	if(linkStat)
	{
		pak_out = pktCreateEx(linkStat,DBSERVER_MININGDATA_RELAY);
		if(pak_out)
			pktSendBitsPack(pak_out,1,count);
	}

	for(i = 0; i < count; i++)
	{
		char *name = pktGetString(pak);
		int j, fields = pktGetBitsPack(pak,1);

		if(pak_out)
		{
			pktSendString(pak_out,name);
			pktSendBitsPack(pak_out,1,fields);
		}

		for(j = 0; j < fields; j++)
		{
			char *field = pktGetString(pak); // static buffer, technically alters 'name', but doesn't matter
			int val = pktGetBitsPack(pak,1);

			if(pak_out)
			{
				pktSendString(pak_out,field);
				pktSendBitsPack(pak_out,1,val);
			}
		}
	}

	if(pak_out)
		pktSend(&pak_out, linkStat);
}
