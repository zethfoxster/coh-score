#include <winsock2.h>
#include <windows.h> 
#include "relaycomm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include "RegistryReader.h"
#include "earray.h"
#include <Shellapi.h>
#include "ListView.h"
#include "textparser.h"
#include "resource.h"
#include "assert.h"
#include "timing.h"
#include "utils.h"
#include "container.h"
#include "MemoryMonitor.h"
#include "winutil.h"
#include "relay_utils.h"
#include "relay_util.h"
#include <CommCtrl.h>
#include "prompt.h"
#include "serverMonitorCommon.h"
#include "netio_core.h"
#include <direct.h>
#include "sysutil.h"
#include "shardMonitorComm.h"
#include "serverMonitor.h"
#include "serverMonitorNet.h"
#include "net_linklist.h"
#include "sock.h"
#include "chatMonitor.h"
#include "mathutil.h"
#include "file.h"
#include "serverMonitorCmdRelay.h"



HWND g_hRelayStatusBar = NULL;
ListView *lvRelays = NULL;
BOOL g_bRelayDevMode = FALSE;

extern NetLinkList	cmdRelayLinks; // TODO: put all in same file, eliminate this 'extern'
extern NetLinkList	svrMonShardMonLinks;

char g_shardRelayClientStatus[128];

int g_pendingShardCommand;


// called my main server monitor tick loop to update stats sent to shard monitor
void updateServerRelayStats(ServerStats *stats)
{
	int i;

	stats->ms_relays = 0;
	stats->ds_relays = 0;
	stats->custom_relays = 0;
	stats->auth_relays = 0;
	stats->acct_relays = 0;
	stats->chat_relays = 0;
	stats->auc_relays = 0;
	stats->ma_relays = 0;
	stats->crashed_mscount = 0;
	stats->maxLastUpdate = 0;


	if (cmdRelayLinks.links) { // JE: Added this because it was crashing when you serverMonitored from a shardMonitor
		for(i=cmdRelayLinks.links->size-1;i>=0;i--)
		{
			CmdRelayClientLink * client = ((NetLink*)cmdRelayLinks.links->storage[i])->userData;
			CmdRelayCon * con = client->relayCon;
			if(client->link)
			{
				if(con->type & RELAY_TYPE_MAPSERVER)
					stats->ms_relays++;
				if(con->type & RELAY_TYPE_DBSERVER)
					stats->ds_relays++;
				if(con->type & RELAY_TYPE_CUSTOM)
					stats->custom_relays++;
				if(con->type & RELAY_TYPE_AUTHSERVER)
					stats->auth_relays++;
				if(con->type & RELAY_TYPE_ACCOUNTSERVER)
					stats->acct_relays++;
				if(con->type & RELAY_TYPE_CHATSERVER)
					stats->chat_relays++;
				if(con->type & RELAY_TYPE_AUCTIONSERVER)
					stats->auc_relays++;
				if(con->type & RELAY_TYPE_MISSIONSERVER)
					stats->ma_relays++;

				stats->crashed_mscount += con->crashedMapCount;
			}

			stats->maxLastUpdate = MAX(stats->maxLastUpdate, con->lastUpdate);
		}
	}

	strcpy(stats->shardrelay_status, g_shardRelayClientStatus);
}

void cmdRelayCmdToAllClients(ListViewCallbackFunc callback)
{
	assert(callback);
	listViewForEach(lvRelays, callback, 0);
}


void onRunBatchFile()
{
	FileAllocInfo file;

	if(OpenAndAllocFile("Select Batch File", "*.bat", &file))
	{
		listViewDoOnSelected(lvRelays, (ListViewCallbackFunc) sendBatchFileToClient, &file);
		free(file.data);
	}
}




void onUpdateAllRelays()
{
	int i;

	FileAllocInfo file;

	if(OpenAndAllocFile("Select source CmdRelay", "CmdRelay.exe", &file))
	{
		for(i=cmdRelayLinks.links->size-1;i>=0;i--)
		{
			CmdRelayClientLink * client = ((NetLink*)cmdRelayLinks.links->storage[i])->userData;
			if(client->link)
			{
				sendRelayExe(client, file.data, file.size);
			}
		}
		free(file.data);
	}
}


void cmdRelayTick(HWND hDlg)
{
	// update list view - will be grayed out if no relays are connected
	static int s_lastCount = -1;
	static int lock=0;
	int count;

	if (lock!=0)
		return;
	lock++;


	// handle networking stuff
	if(cmdRelayLinks.links)
	{
		int i;

		NMMonitor(1);

		// ask links for protocol (if necessary)
		for(i=cmdRelayLinks.links->size-1;i>=0;i--)
		{

			CmdRelayClientLink * client = ((NetLink*)cmdRelayLinks.links->storage[i])->userData;
			CmdRelayCon * con = client->relayCon;

			if(    client->link 
 				&& con
				&& con->protocol != CMDRELAY_PROTOCOL_VERSION)
			{
				requestProtocolFromClient(client->link);
			}

			if(con->lastUpdate > 10)
				listViewSetItemColor(lvRelays, con, RGB(211,211,211), RGB(178,34,34));

			listViewItemChanged(lvRelays, client->relayCon);

			con->lastUpdate += RELAY_TICK_SEC;
		}

		{
			char * str;
			switch(g_pendingShardCommand)
			{
				case SVRMONSHARDMON_RELAY_RUN_BATCH_FILE:
					str = "Running Batch File...";
					break;
				default:
					str = "Ready";
					break;
			}
			setStatusBar(g_hRelayStatusBar, 0, str);
			strcpy(g_shardRelayClientStatus, str);
		}

		if(g_pendingShardCommand)
		{
			// see if the command has finished, or if there were any errors
			bool busy = false;
			bool failure = false;
			for(i=cmdRelayLinks.links->size-1;i>=0;i--)
			{
				CmdRelayClientLink * client = ((NetLink*)cmdRelayLinks.links->storage[i])->userData;
				if(client->relayCon)
				{
					int res = client->relayCon->actionResult;
					if(res == CMDRELAY_ACTION_STATUS_FAILURE)
					{
						failure = true;
						break;	// early exit, since this is the worst-case scenario
					}
					else if(res == CMDRELAY_ACTION_STATUS_BUSY)
					{
						busy = true;
					}
				}
			}

			if(!busy)
			{

				if(failure)
				{
		//			sendResultToShardMon(CMDRELAY_ACTION_STATUS_FAILURE);
				}
				else
				{
					//success!!

			//		sendResultToShardMon(CMDRELAY_ACTION_STATUS_SUCCESS);

					g_pendingShardCommand = 0;
				}

				// reset status of all relays
				for(i=cmdRelayLinks.links->size-1;i>=0;i--)
				{
					CmdRelayClientLink * client = ((NetLink*)cmdRelayLinks.links->storage[i])->userData;
					if(client->relayCon)
					{
						client->relayCon->actionResult = CMDRELAY_ACTION_STATUS_NOTASK;
					}
				}

			}
		}


		// update dialog

		count = listViewGetNumItems(lvRelays);
		if(count != s_lastCount)
		{
			SetDlgItemInt(hDlg, IDC_RELAY_CONNECTED_COUNT, count, TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_LST_RELAY_CLIENTS), (count != 0));
			s_lastCount = count;
		}

		UpdateWindow(GetDlgItem(hDlg, IDC_LST_RELAY_CLIENTS));
	}
	lock--;
}



void SendCommand(CmdRelayCon * con, int cmd, int relayType)
{
	if(con && (con->type & relayType))
	{
		Packet	*pak = pktCreateEx(con->link,cmd);
		pktSend(&pak, con->link);
		lnkFlush(con->link);
	}
}


void onRelayStartDbServer(ListView *lv, void *structptr, int cmd)
{
	SendCommand((CmdRelayCon*) structptr, CMDRELAY_REQUEST_START_DBSERVER, RELAY_TYPE_DBSERVER);
}

void onRelayStartLauncher(ListView *lv, void *structptr, int cmd)
{
	SendCommand((CmdRelayCon*) structptr, CMDRELAY_REQUEST_START_LAUNCHER, RELAY_TYPE_ALL);
}

void onRelayKillAllLauncher(ListView *lv, void *structptr, int cmd)
{
	SendCommand((CmdRelayCon*) structptr, CMDRELAY_REQUEST_KILL_ALL_LAUNCHER, RELAY_TYPE_ALL);
}

void onRelayKillAllMapserver(ListView *lv, void *structptr, int cmd)
{
	SendCommand((CmdRelayCon*) structptr, CMDRELAY_REQUEST_KILL_ALL_MAPSERVER, RELAY_TYPE_ALL);
}

void onRelayKillAllBeaconizer(ListView *lv, void *structptr, int cmd)
{
	SendCommand((CmdRelayCon*) structptr, CMDRELAY_REQUEST_KILL_ALL_BEACONIZER, RELAY_TYPE_ALL);
}

void onRelayCancelAll(ListView *lv, void *structptr, int cmd)
{
	SendCommand((CmdRelayCon*) structptr, CMDRELAY_REQUEST_CANCEL_ALL, RELAY_TYPE_ALL);
}

void onRelayStartAll(ListView *lv, void *structptr, int cmd)
{
	SendCommand((CmdRelayCon*) structptr, CMDRELAY_REQUEST_START_ALL, RELAY_TYPE_ALL);
}

void onRelayStopAll(ListView *lv, void *structptr, int cmd)
{
	SendCommand((CmdRelayCon*) structptr, CMDRELAY_REQUEST_STOP_ALL, RELAY_TYPE_ALL);
}


BOOL IsSelected(CmdRelayCon * con)
{
	return listViewIsSelected(lvRelays, con);
}

void onCustomCmd(BOOL bAll)
{
	int i;



	if(!strlen(g_customCmd))
		return;

	addNameToList("CustomCmd", g_customCmd);

	for(i=cmdRelayLinks.links->size-1;i>=0;i--)
	{
		Packet * pak;
		CmdRelayClientLink * client = ((NetLink*)cmdRelayLinks.links->storage[i])->userData;
		if(client->link)
		{
			if(    ! bAll 
				&& ! IsSelected(client->relayCon))
			{
				continue;
			}

			pak = pktCreateEx(client->link,CMDRELAY_REQUEST_CUSTOM_CMD);
			pktSendString(pak, g_customCmd);
			pktSend(&pak, client->link);
			lnkFlush(client->link);
		}
	}

}

void onCustomCmdDelete(HWND hDlg)
{
	int i;

	getText(hDlg, NULL, relayMapping, ARRAY_SIZE(relayMapping));

	SetDlgItemText(hDlg, IDC_COMBO_RELAY_CUSTOM_CMD, "");
	removeNameFromList("CustomCmd", g_customCmd);
	i = SendMessage(GetDlgItem(hDlg, IDC_COMBO_RELAY_CUSTOM_CMD), CB_FINDSTRING, 0, (WPARAM) &g_customCmd[0]);
	if(i != CB_ERR)
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_RELAY_CUSTOM_CMD), CB_DELETESTRING, (WPARAM) i, 0);
}


LRESULT CALLBACK DlgCmdRelayProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int i=0;

	switch (iMsg)
	{
	case WM_INITDIALOG:
		{
			if (lvRelays) listViewDestroy(lvRelays);
			lvRelays = listViewCreate();
			listViewInit(lvRelays, CmdRelayConNetInfo, hDlg, GetDlgItem(hDlg, IDC_LST_RELAY_CLIENTS));

			getNameList("CustomCmd");
			for (i=0; i<name_count; i++) 
				SendMessage(GetDlgItem(hDlg, IDC_COMBO_RELAY_CUSTOM_CMD), CB_ADDSTRING, 0, (LPARAM)namelist[i]);


			initText(hDlg, NULL, relayMapping, ARRAY_SIZE(relayMapping));

			// Status bar stuff
			g_hRelayStatusBar = CreateStatusWindow( WS_CHILD | WS_VISIBLE, "Status bar", hDlg, 1);
			{
				int temp[4];
				temp[0]=100;
				temp[1]=200;
				temp[2]=300;
				temp[3]=-1;
				SendMessage(g_hRelayStatusBar, SB_SETPARTS, ARRAY_SIZE(temp), (LPARAM)temp);
			}

			strcpy(g_shardRelayClientStatus, "Ready");

			SetTimer(hDlg, 0, RELAY_TICK_SEC * 1000, NULL);
			cmdRelayInit();
		}
		return FALSE;

	case WM_TIMER:
		cmdRelayTick(hDlg);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			PostQuitMessage(0);
			return TRUE;

		xcase IDC_RELAY_START_ALL:
			listViewDoOnSelected(lvRelays, (ListViewCallbackFunc) onRelayStartAll, 0);
			return TRUE;
		xcase IDC_RELAY_STOP_ALL:
			listViewDoOnSelected(lvRelays, (ListViewCallbackFunc) onRelayStopAll, 0);
			return TRUE;
		xcase IDC_RELAY_KILLALL_MAPSERVER:
			listViewDoOnSelected(lvRelays, (ListViewCallbackFunc) onRelayKillAllMapserver, 0);
			return TRUE;
		xcase IDC_RELAY_KILLALL_LAUNCHER:
			listViewDoOnSelected(lvRelays, (ListViewCallbackFunc) onRelayKillAllLauncher, 0);
			return TRUE;
		xcase IDC_RELAY_KILLALL_BEACONIZER:
			listViewDoOnSelected(lvRelays, (ListViewCallbackFunc) onRelayKillAllBeaconizer, 0);
			return TRUE;
		xcase IDC_RELAY_START_LAUNCHER:
			listViewDoOnSelected(lvRelays, (ListViewCallbackFunc) onRelayStartLauncher, 0);
			return TRUE;
		xcase IDC_RELAY_START_DBSERVER:
			listViewDoOnSelected(lvRelays, (ListViewCallbackFunc) onRelayStartDbServer, 0);
			return TRUE;
		xcase IDC_BUTTON_RELAY_CANCEL_ALL:
			listViewDoOnSelected(lvRelays, (ListViewCallbackFunc) onRelayCancelAll, 0);
			return TRUE;
		xcase IDC_BUTTON_RELAY_CUSTOM_CMD_ALL:
			getText(hDlg, NULL, relayMapping, ARRAY_SIZE(relayMapping));
			onCustomCmd(TRUE);
			return TRUE;
		xcase IDC_BUTTON_RELAY_CUSTOM_CMD_EXECUTE:
			getText(hDlg, NULL, relayMapping, ARRAY_SIZE(relayMapping));
			onCustomCmd(FALSE);
			return TRUE;
		xcase IDC_BUTTON_RELAY_CUSTOM_CMD_DELETE:
			onCustomCmdDelete(hDlg);
			return TRUE;
		xcase IDC_RELAY_UPDATE_SELF:
			onUpdateAllRelays();
			return TRUE;
		xcase IDC_RELAY_SELECT_ALL:
			listViewSelectAll(lvRelays, TRUE);
			return TRUE;
		xcase IDC_RELAY_SELECT_NONE:
			listViewSelectAll(lvRelays, FALSE);
			return TRUE;
		xcase IDC_RELAY_RUN_BATCH_FILE:
			onRunBatchFile();
			return TRUE;
		}
		break;
	case WM_SIZE:
		{
			WORD w = LOWORD(lParam);
			WORD h = HIWORD(lParam);
			//SendMessage(hStatusBar, iMsg, wParam, lParam);
			doDialogOnResize(hDlg, w, h, IDC_RELAY_ALIGNME, IDC_RELAY_UPPERLEFT);
		}
		break;
	case WM_NOTIFY:
		{
			int ret=FALSE;
			if(lvRelays)
			{
				int idCtrl = (int)wParam;
				int count;
				ret = listViewOnNotify(lvRelays, wParam, lParam, NULL);
				// disable buttons that are not applicable if nothing is selected
				count = listViewDoOnSelected(lvRelays, NULL, NULL);
				EnableWindow(GetDlgItem(hDlg, IDC_RELAY_START_LAUNCHER), (count >= 1));
				EnableWindow(GetDlgItem(hDlg, IDC_RELAY_START_DBSERVER), (count >= 1));
				EnableWindow(GetDlgItem(hDlg, IDC_RELAY_KILLALL_LAUNCHER), (count >= 1));
				EnableWindow(GetDlgItem(hDlg, IDC_RELAY_KILLALL_BEACONIZER), (count >= 1));
				EnableWindow(GetDlgItem(hDlg, IDC_RELAY_START_ALL), (count >= 1));
				EnableWindow(GetDlgItem(hDlg, IDC_RELAY_STOP_ALL), (count >= 1));
				EnableWindow(GetDlgItem(hDlg, IDC_RELAY_KILLALL_MAPSERVER), (count >= 1));
				EnableWindow(GetDlgItem(hDlg, IDC_RELAY_RUN_BATCH_FILE), (count >= 1));
				EnableWindow(GetDlgItem(hDlg, IDC_RELAY_UPDATE_SELF), (count >= 1));
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_RELAY_CANCEL_ALL), (count >= 1));
				EnableWindow(GetDlgItem(hDlg, IDC_RELAY_SELECT_NONE), (count > 0));
			}

			return ret;
		}
		break;
	}
	return FALSE;
}











