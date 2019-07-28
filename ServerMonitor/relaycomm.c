#define RELAYCOMM_PARSE_INFO_DEFS
#include "relaycomm.h"
#include "error.h"
#include "comm_backend.h"
#include <stdio.h>
#include "assert.h"
#include "structNet.h"
#include "netio_core.h"
#include "dbdispatch.h"
#include "launchercomm.h"
#include "timing.h"
#include "ListView.h"
#include "serverMonitorCmdRelay.h"
#include "file.h"


#define MAX_CLIENTS 50

NetLinkList		cmdRelayLinks;
char * g_CmdRelayFilename = 0;

extern ListView *lvRelays;


int cmdRelayConnectCallback(NetLink *link) 
{
	CmdRelayClientLink	*client;
	CmdRelayCon * con;

	printf("new CmdRelay link at %s\n",makeIpStr(link->addr.sin_addr.S_un.S_addr));
	client = link->userData;
	client->link = link;

	con = malloc(sizeof(CmdRelayCon));
	memset(con, 0, sizeof(CmdRelayCon));

	con->link = link;

	strcpy(con->hostname,	makeHostNameStr(link->addr.sin_addr.S_un.S_addr));
	strcpy(con->ipAddress,	makeIpStr(link->addr.sin_addr.S_un.S_addr));

	client->relayCon = con;	

	listViewAddItem(lvRelays, con);

	return 1;
}


int cmdRelayDelCallback(NetLink *link)
{
	CmdRelayClientLink	*client = link->userData;
	int index;

	index = listViewFindItem(lvRelays, client->relayCon);
	
	assert(index != -1);

	if(index != -1)
	{
		listViewDelItem(lvRelays, index);
	}

	free(client->relayCon);
	client->relayCon = 0;

	return 1;
}


void receiveStatusFromClient(Packet * pak, NetLink * link)
{
	CmdRelayClientLink	*client = link->userData;
	CmdRelayCon * con = client->relayCon;
	CmdRelayType * tdesc;

	// only proceed if the versions match, as future status messages may change in format
	if(con->protocol == CMDRELAY_PROTOCOL_VERSION)
	{
		con->status	= pktGetBitsPack(pak, 1);
		con->type	= pktGetBitsPack(pak, 1);

		con->launcherCount		= pktGetBitsPack(pak, 1);
		con->dbserverCount		= pktGetBitsPack(pak, 1);
		con->mapserverCount		= pktGetBitsPack(pak, 1);
		con->crashedMapCount	= pktGetBitsPack(pak, 1);
		con->beaconizerCount	= pktGetBitsPack(pak, 1);
		con->authserverCount	= pktGetBitsPack(pak, 1);
		con->accountserverCount = pktGetBitsPack(pak, 1);
		con->chatserverCount	= pktGetBitsPack(pak, 1);
		con->auctionserverCount = pktGetBitsPack(pak, 1);
		con->missionserverCount = pktGetBitsPack(pak, 1);

		strcpy(con->version,	pktGetString(pak));
		strcpy(con->lastMsg,	pktGetString(pak));

		// now set text versions
		con->typeStr[0] = 0;
		tdesc = cmdRelayTypes;
		while (!(tdesc->name == 0 && tdesc->type == 0)) {
			if (con->type & tdesc->type) {
				if (con->typeStr[0] != 0)
					strcat(con->typeStr, ", ");
				strcat(con->typeStr, tdesc->name);
			}
			++tdesc;
		}
		strcpy(con->statusStr, StaticDefineIntRevLookup(cmdRelayStatusDef, con->status));

		con->lastUpdate = 0;
		listViewSetItemColor(lvRelays, con, LISTVIEW_DEFAULT_COLOR, LISTVIEW_DEFAULT_COLOR);
	}
}

void requestProtocolFromClient(NetLink * link)
{
	Packet * pak = pktCreate();
	pktSendBitsPack(pak, 1,CMDRELAY_REQUEST_PROTOCOL);
	pktSend(&pak,link);
	lnkFlush(link);
}


BOOL DetermineValidCmdRelayFilename()
{
	// determine location of a valid CmdRelay.exe to use for auto-updating

	static bool s_bCalledOnce = FALSE;

	if(!s_bCalledOnce)
	{
		char * exe = 0;
		char filename[FILENAME_MAX] = "";


		assert(!g_CmdRelayFilename);	// this should not have been allocated yet since this is the first time in this function!

		s_bCalledOnce = TRUE;

		if(!fileExists(exe = ".\\CmdRelay.exe"))	// this one SHOULD always work
		{
			if(g_bRelayDevMode && fileExists(exe = "C:\\util\\CmdRelay.exe"))	// this one is for development
			{
				// do nothing, we just set 'exe' above
			}
			else
			{
				// failsafe - prompt user
				if(OpenFileDlg("Select a source CmdRelay, or 'Cancel' to abort auto-update", "CmdRelay.exe",filename))
				{
					exe = filename;
				}
				else
				{
					MessageBox(NULL, "Unable to locate CmdRelay.exe. Auto-update is disabled", "ERROR", MB_ICONERROR);
					exe = 0;
				}
			}
		}

		if(exe)
			g_CmdRelayFilename = strdup(exe);
		else
			g_CmdRelayFilename = 0;
	}

	return (g_CmdRelayFilename ? TRUE : FALSE);
}


void receiveActionResultFromClient(int res, NetLink * link)
{
	CmdRelayClientLink	*client = link->userData;
	CmdRelayCon * con = client->relayCon;

	con->actionResult = res;

	//now see if we're done
	//for()
}

void receiveProtocolFromClient(Packet * pak, NetLink * link)
{
	CmdRelayClientLink	*client = link->userData;
	CmdRelayCon * con = client->relayCon;
	static BOOL s_bFailure = FALSE;

	if(s_bFailure)
		return;		// we've already failed to locate a CmdRelay.exe for auto-updating, so don't spam

	con->protocol = pktGetBitsPack(pak, 1);

	if(con->protocol != CMDRELAY_PROTOCOL_VERSION)
	{
		// auto-update client
		int size;
		char * data;

		
		if(DetermineValidCmdRelayFilename())
		{
			data = fileAlloc(g_CmdRelayFilename, &size);

			if(!data)
			{
				MessageBox(NULL, "Failed to read file", "ERROR", MB_ICONERROR);
				return;
			}

			sendRelayExe(client, data, size);

			free(data);
		}
	}

	listViewItemChanged(lvRelays, con);	
}

void sendRelayExe(CmdRelayClientLink * client, char * data, int size)
{
	Packet * pak;

	// send the file to each relay
	pak = pktCreate();
	pktSendBitsPack(pak, 1,CMDRELAY_REQUEST_UPDATE_SELF);
	pktSendBitsPack(pak, 1, size);
	pktSendBitsArray(pak, (size * 8), data);
	pktSend(&pak,client->link);
	lnkFlush(client->link);
}


int cmdRelayHandleClientMsg(Packet *pak,int cmd, NetLink *link)
{
	CmdRelayClientLink	*client = link->userData;

	if (!client)
		return 0;

	switch(cmd)
	{
	case CMDRELAY_ANSWER_STATUS:
		receiveStatusFromClient(pak, link);
		break;
	case CMDRELAY_ANSWER_PROTOCOL:
		receiveProtocolFromClient(pak, link);
		break;
	case CMDRELAY_ANSWER_ACTION_SUCCESS:
		receiveActionResultFromClient(CMDRELAY_ACTION_STATUS_SUCCESS, link);
		break;
	case CMDRELAY_ANSWER_ACTION_FAILURE:
		receiveActionResultFromClient(CMDRELAY_ACTION_STATUS_FAILURE, link);
		break;
	default:
		printf("Unknown command from ServerMonitor: %d\n",cmd);
		return 0;
	}
	return 1;
}




void cmdRelayInit()
{		
	// setup networking stuff
	netLinkListAlloc(&cmdRelayLinks,MAX_CLIENTS,sizeof(CmdRelayClientLink),cmdRelayConnectCallback);
	netInit(&cmdRelayLinks,0,DEFAULT_CMDRELAY_PORT);
	cmdRelayLinks.destroyCallback = cmdRelayDelCallback;
	NMAddLinkList(&cmdRelayLinks, cmdRelayHandleClientMsg);
}


