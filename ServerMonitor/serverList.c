#include "shardMonitor.h"
#include "shardMonitorConfigure.h"
#include "shardMonitorComm.h"
#include "serverMonitorCommon.h"
#include "serverMonitorNet.h"
#include "serverMonitor.h"
#include "ListView.h"
#include "resource.h"
#include "netio.h"
#include "comm_backend.h"
#include "structNet.h"
#include "entVarUpdate.h"
#include "containerbroadcast.h"
#include "prompt.h"
#include "timing.h"
#include "structHist.h"
#include "winutil.h"
#include "utils.h"

ListView *lvSmServers=NULL;

static void targettedRemoteDesktop(ListView *lv, ShardMonitorConfigEntry *stat, HWND hDlg)
{
	launchRemoteDesktop(stat->ip, stat->name);
}

void serverListInit()
{
	int i;
	listViewDelAllItems(lvSmServers, NULL);
	for (i=0; i<EArrayGetSize(&shmConfig.shardList); i++) {
		listViewAddItem(lvSmServers, shmConfig.shardList[i]);
	}
}

LRESULT CALLBACK DlgServerListProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	LRESULT ret = FALSE;

	switch (iMsg)
	{
	case WM_INITDIALOG:
		if (!lvSmServers)
			lvSmServers = listViewCreate();
		listViewInit(lvSmServers, shardMonitorConfigEntryDispInfo, hDlg, GetDlgItem(hDlg, IDC_LST_SERVERS));
		//listViewSetSortable(lvSmServers, false);

		shardMonLoadConfig("./ServerListConfig.txt");
		serverListInit();
		GetClientRect(hDlg, &rect); 
		doDialogOnResize(hDlg, (WORD)(rect.right - rect.left), (WORD)(rect.bottom - rect.top), IDC_ALIGNME, IDC_UPPERLEFT);
		setDialogMinSize(hDlg, 0, 200);
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
		case IDC_BTN_CONFIGURE:
            shardMonConfigure(g_hInst, hDlg, "./ServerListConfig.txt");
			serverListInit();
			break;
		case IDC_BTN_REMOTEDESKTOP:
			listViewDoOnSelected(lvSmServers, targettedRemoteDesktop, hDlg);
			break;
		}
		break;
	case WM_SIZE:
		{
			WORD w = LOWORD(lParam);
			WORD h = HIWORD(lParam);
			//printf("shard res: %dx%d\n", w, h);
			//SendMessage(hStatusBar, iMsg, wParam, lParam);
			doDialogOnResize(hDlg, w, h, IDC_ALIGNME, IDC_UPPERLEFT);
		}
		break;
	case WM_NOTIFY:
		{
			int idCtrl = (int)wParam;
			int count;
			ret |= listViewOnNotify(lvSmServers, wParam, lParam, NULL);

			count = listViewDoOnSelected(lvSmServers, NULL, NULL);
			EnableWindow(GetDlgItem(hDlg, IDC_BTN_REMOTEDESKTOP), (count == 1));
		}
		break;
	}

	return ret;
}