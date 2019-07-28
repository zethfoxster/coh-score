#include "builderizer.h"
#include "builderizerclient.h"
#include "utils.h"
#include "builderizercomm.h"
#include "winutil.h"
#include "builderizerconfig.h"
#include "buildstatus.h"
#include "resource.h"
#include "ListView.h"
#include <commctrl.h>

NetComm * net_comm = NULL;
NetLink * buildServerLink = NULL;
char serverIP[256] = {0};
int buildIsInProgress = 0;
int buildIsPaused = 0;
int buildFailed = 0;

static ListView *lvStatus;
static ListView *lvConfigs;
static ListView *lvVariables;

RequiredVariable **clientRequiredVars = NULL;
RequiredVariable **allocatedRequiredVars = NULL;
StashTable reqListsStash = NULL;

HWND hClientDlg = NULL;

// HACK:
// i dont like the fact that i had to do this, but i have a 
// separate wndproc for the config and variable listviews because
// the timing of the messages to update them was causing bugs
WNDPROC orig_ConfigListViewProc = NULL;
WNDPROC orig_VariableListViewProc = NULL;
int updateConfigListView = 0;
int updateVariableListView = 0;


#define CMD_IS(x) (LOWORD(wParam) == (x))


static ParseTable tpiBuildConfigListView[] =
{
	{"Available Builds", TOK_STRING(BuilderizerConfig, name, 0), 0, TOK_FORMAT_LVWIDTH(205)},
	{0}
};


static ParseTable tpiReqVarsListView[] =
{
	{"Required Variables", TOK_STRING(RequiredVariable, name, 0), 0, TOK_FORMAT_LVWIDTH(170)},
	{"Value", TOK_STRING(RequiredVariable, value, 0), 0, TOK_FORMAT_LVWIDTH(100)},
	{0}
};



void setBuildServerIP(char *ipstr)
{
	strncpyt(serverIP, ipstr, 255);
}

int buildclientConnect(void)
{
	int retryAttempts = 1;

	bcfgFreeConfigs();
	buildStatusFreeAll();

	net_comm = commCreate(1, 1);
	if(buildServerLink)
		linkRemove(&buildServerLink);
	buildServerLink = commConnect(net_comm, BUILDERIZER_FLAGS, serverIP, BUILDERIZER_PORT, handleNetMsg, NULL, NULL, 0);
	while ( retryAttempts-- )
	{
		if ( linkConnectWait(buildServerLink, 2.0f) )
		{
			Packet	*pak = pktCreate(buildServerLink, BUILDCOMM_REQUESTCONFIGS);
			//pktSendString(pak, "C:/builderizer/demo.bs");
			pktSend(&pak);
			linkFlush(buildServerLink);

			// get available configs from server
			{
				int msg = waitForServerMsg(&pak);
				while ( msg == BUILDCOMM_DISCONNECTED )
				{
					linkRemove(&buildServerLink);
					buildServerLink = commConnect(net_comm, BUILDERIZER_FLAGS, serverIP, BUILDERIZER_PORT, handleNetMsg, NULL, NULL, 0);
					if ( linkConnectWait(buildServerLink, 2.0f) )
					{
						msg = waitForServerMsg(&pak);
					}
					else
					{
						printf("Failed to connect to %s:%d...\n", serverIP, BUILDERIZER_PORT);
						Sleep(1000);
					}
				}
				if ( msg == BUILDCOMM_SENDCONFIGS )
				{
					int i, j;
					
					buildIsInProgress = pktGetBitsPack(pak, 1);
					i = pktGetBitsPack(pak, 1);

					// unload existing configs
					if ( loadedConfigs )
						bcfgFreeConfigs();

					bcfgInit();

					for ( ; i > 0; --i )
					{
						BuilderizerConfig *bcfg;
						
						//bcfg = (BuilderizerConfig*)malloc(sizeof(BuilderizerConfig));
						ParserSetTableInfo(tpiBuildConfig, sizeof(BuilderizerConfig), "BuildConfig", NULL, __FILE__);
						bcfg = StructAllocRaw(tpiBuildConfig);
						//ZeroStruct(bcfg);
						ParserRecv(tpiBuildConfig, pak, bcfg);
						eaPush(&loadedConfigs, bcfg);

						for ( j = 0; j < eaSize(&bcfg->vars); ++j )
						{
							if ( bcfg->vars[j]->isActuallyList )
								bcfg->vars[j]->value = NULL;
						}
					}
				}
				pktFree(&pak);
			}
			return 1;
		}
		else
		{
			printf("Failed to connect to %s:%d...\n", serverIP, BUILDERIZER_PORT);
			Sleep(10);
		}
	}
	return 0;
}

BuilderizerConfig *curCfg = NULL;
RequiredVariable *curReqVar = NULL;

void OnConfigSelect(ListView *lv, void *structptr, void *data)
{
	int i;//, j;
	curCfg = ((BuilderizerConfig*)structptr);

	//if ( clientRequiredVars )
	//{
	//	eaClear(&clientRequiredVars);
	//	//clientRequiredVars = NULL;
	//	// clean up memory we allocated
	//	for ( i = 0; i < eaSize(&allocatedRequiredVars); ++i )
	//	{
	//		free(allocatedRequiredVars[i]);
	//		allocatedRequiredVars[i] = NULL;
	//	}
	//	eaClear(&allocatedRequiredVars);
	//}
	//eaCreate(&clientRequiredVars);

	SetWindowText(GetDlgItem(hClientDlg, IDC_CONFIGDESCRIPTION), curCfg->disabled ? "***THIS BUILD IS DISABLED***" : curCfg->text);
	SetWindowText(GetDlgItem(hClientDlg, IDC_VARIABLEDESCRIPTION), "");

	if ( reqListsStash )
	{
		stashTableDestroy(reqListsStash);
		reqListsStash = NULL;
	}
	reqListsStash = stashTableCreateWithStringKeys(10, 0);

	listViewDelAllItems(lvVariables, NULL);
	for ( i = 0; i < eaSize(&curCfg->vars); ++i )
	{
		int varDefined = curCfg->vars[i]->value && strlen(curCfg->vars[i]->value);
		//if ( curCfg->vars[i]->isActuallyList )
		//{
		//	for ( j = 0; j < eaSize(&curCfg->lists); ++j )
		//	{
		//		if ( !stricmp(curCfg->lists[j]->name, curCfg->vars[i]->name) )
		//		{
		//			if ( !stashFindElement(reqListsStash, curCfg->lists[j]->name, NULL) )
		//				stashAddPointer(reqListsStash, curCfg->vars[i]->name, curCfg->lists[j], 0);
		//			else
		//				assert("List added twice to hashtable!"==0);
		//		}
		//	}
		//}

		listViewAddItem(lvVariables, curCfg->vars[i]);
		listViewSetItemColor(lvVariables, curCfg->vars[i], varDefined ? RGB(0,0,255) : RGB(255,0,0), RGB(255,255,255));
	}

	//for ( i = 0; i < eaSize(&curCfg->lists); ++i )
	//{
	//	int varDefined = curCfg->lists[i]->values[0] && strlen(curCfg->lists[i]->values[0]);
	//	RequiredVariable *rv = (RequiredVariable*)malloc(sizeof(RequiredVariable));

	//	if ( !stashFindElement(reqListsStash, curCfg->lists[i]->name, NULL) )
	//		stashAddPointer(reqListsStash, curCfg->lists[i]->name, curCfg->lists[i], 0);
	//	else
	//		assert("List added twice to hashtable!"==0);

	//	rv->name = curCfg->lists[i]->name;
	//	rv->value = curCfg->lists[i]->values ? curCfg->lists[i]->values[0] : 0;
	//	rv->isActuallyList = 1;
	//	eaPush(&allocatedRequiredVars, rv);
	//	eaPush(&clientRequiredVars, rv);

	//	listViewAddItem(lvVariables, curCfg->vars[i]);
	//	listViewSetItemColor(lvVariables, curCfg->vars[i], varDefined ? RGB(0,0,255) : RGB(255,0,0), RGB(255,255,255));
	//}

	curReqVar = NULL;
}


LRESULT CALLBACK GetListProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	updateVariableListView = 0;

	switch( iMsg )
	{	
	case WM_INITDIALOG:
		{
			int i;
			//RequiredList *rl;
			char *text = NULL;
			//stashFindPointer(reqListsStash, curReqVar->name, &rl);
			for ( i = 0; i < eaSize(&curReqVar->values); ++i )
			{
				estrConcatf(&text, "%s\r\n", curReqVar->values[i]);
			}
			SetWindowText(GetDlgItem(hDlg, IDC_GETLIST),text);
			estrDestroy(&text);
			
		}
	xcase WM_COMMAND:
		if ( CMD_IS(IDOK) )
		{
			int i, len = GetWindowTextLength(GetDlgItem(hDlg, IDC_GETLIST));
			char *text = (char*)malloc(sizeof(char)*len+2);
			char *line,*context;
			//RequiredList *rl;
			//stashFindPointer(reqListsStash, curReqVar->name, &rl);
			GetWindowText(GetDlgItem(hDlg, IDC_GETLIST), text, len+1);

			for ( i = 0; i < eaSize(&curReqVar->values); ++i )
			{
				free(curReqVar->values[i]);
			}
			eaDestroy(&curReqVar->values);
			curReqVar->values = NULL;
			
			line = strtok_s(text,"\r\n",&context);
			while ( line )
			{
				eaPush(&curReqVar->values, strdup(line));
				line = strtok_s(NULL,"\r\n",&context);
			}
			free(text);

			if ( curReqVar->value )
				free(curReqVar->value);
			curReqVar->value = curReqVar->values ? strdup(curReqVar->values[0]) : NULL;
			//if ( rl->values )
			//	curReqVar->value = strdup(rl->values[0]);
			EndDialog(hDlg, 1);
			return TRUE;
		}
		else if ( CMD_IS(IDCANCEL) )
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}

	}
	return FALSE;
}

void OnVariableSelect(ListView *lv, void *structptr, void *data)
{
	curReqVar = ((RequiredVariable*)structptr);
	if ( curReqVar->isActuallyList )
	{
		DialogBox(winGetHInstance(), (LPCTSTR) (IDD_GETLIST), hClientDlg, (DLGPROC)GetListProc);
		if ( curReqVar->values && eaSize(&curReqVar->values) )
			SetWindowText(GetDlgItem(hClientDlg, IDC_VARIABLEVALUE), curReqVar->values[0]);
	}
	else
	{
		SetWindowText(GetDlgItem(hClientDlg, IDC_VARIABLEDESCRIPTION), curReqVar->text);
		SetWindowText(GetDlgItem(hClientDlg, IDC_VARIABLEVALUE), curReqVar->value);
	}
}

int AllReqVarsAreDefined()
{
	if ( curCfg )
	{
		int i;
		for ( i = 0; i < eaSize(&curCfg->vars); ++i )
		{
			if ( curCfg->vars[i]->notRequired )
				continue;

			if (!curCfg->vars[i]->value || !strlen(curCfg->vars[i]->value))
				return 0;
		}
	}
	return 1;
}

void SetReqVariableValue()
{
	char val[256];

	if ( curReqVar )
	{
		if ( curReqVar->isActuallyList )
		{
			if ( curReqVar->value )
				free(curReqVar->value);
			curReqVar->value = curReqVar->values ? strdup(curReqVar->values[0]) : NULL;
		}
		else
		{
			GetWindowText(GetDlgItem(hClientDlg, IDC_VARIABLEVALUE), val, 256);
			if ( curReqVar->value )
				free(curReqVar->value);
			curReqVar->value = strdup(val);
		}

		if ( curReqVar->value && strlen(curReqVar->value) )
			listViewSetItemColor(lvVariables, curReqVar, RGB(0,0,255), RGB(255,255,255));
		listViewItemChanged(lvVariables, curReqVar);
	}
}

LRESULT CALLBACK ConfigListViewProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch( iMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_SETFOCUS:
		updateConfigListView = 1;
		break;
	}
	return CallWindowProc(orig_ConfigListViewProc,hDlg, iMsg, wParam, lParam);
}

LRESULT CALLBACK VariableListViewProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch( iMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_SETFOCUS:
		updateVariableListView = 1;
		break;
	}
	return CallWindowProc(orig_VariableListViewProc,hDlg, iMsg, wParam, lParam);
}

void buildclientInitListViews()
{
	int i;

	if ( hClientDlg )
	{
		if ( lvStatus )
			listViewDestroy(lvStatus);
		if ( lvConfigs )
			listViewDestroy(lvConfigs);
		if ( lvVariables )
			listViewDestroy(lvVariables);

		lvStatus = listViewCreate();
		listViewInit(lvStatus, tpiBuildStepStatus, hClientDlg, GetDlgItem(hClientDlg, IDC_STATUS));

		lvConfigs = listViewCreate();
		listViewInit(lvConfigs, tpiBuildConfigListView, hClientDlg, GetDlgItem(hClientDlg, IDC_CONFIGS));
		for ( i = 0; i < eaSize(&loadedConfigs); ++i )
		{
			listViewAddItem(lvConfigs, loadedConfigs[i]);
		}

		orig_ConfigListViewProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(GetDlgItem(hClientDlg, IDC_CONFIGS),
			GWLP_WNDPROC, (LONG_PTR)ConfigListViewProc);

		lvVariables = listViewCreate();
		listViewInit(lvVariables, tpiReqVarsListView, hClientDlg, GetDlgItem(hClientDlg, IDC_VARIABLES));
		orig_VariableListViewProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(GetDlgItem(hClientDlg, IDC_VARIABLES),
			GWLP_WNDPROC, (LONG_PTR)VariableListViewProc);
	}
}

LRESULT CALLBACK buildclientDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int i;
	static BuildStepStatus **buildStepStats = NULL;
	static StashTable buildStatsHash = NULL;

	switch (iMsg)
	{
		case WM_INITDIALOG:
			{
				hClientDlg = hDlg;

				if ( !linkConnected(buildServerLink) )
				{
					buildclientConnect();
					buildclientInitListViews();
				}

				if ( buildIsInProgress )
					EnableWindow(GetDlgItem(hDlg, IDC_STARTBUILD), FALSE);

				// this is how frequently the client requests updates from the server
				SetTimer(hDlg, 0, 1000, NULL);

				// this is how frequently the variable input window refreshes the variable it is modifying
				SetTimer(hDlg, 1, 10, NULL);

				return FALSE;
			}
		xcase WM_COMMAND:
			if ( CMD_IS(IDCANCEL) )
			{
				EndDialog(hDlg, 0);
				return TRUE;
			}
			else if ( CMD_IS(IDC_BUTTON1) )
			{
				Packet *pak;

				if ( !linkConnected(buildServerLink) )
				{
					buildclientConnect();
					buildclientInitListViews();
				}

				// start build
				if ( !buildIsInProgress )
				{
					for ( i = 0; i < eaSize(&loadedConfigs); ++i )
					{
						if ( listViewIsSelected(lvConfigs, loadedConfigs[i]) )
						{
							printf("Starting Build %s...\n", loadedConfigs[i]->name);
							pak = pktCreate(buildServerLink, BUILDCOMM_STARTBUILD);
							ParserSend(tpiBuildConfig, pak, NULL, loadedConfigs[i], 1, 0, 0, 0, NULL);
							pktSend(&pak);
							linkFlush(buildServerLink);
							buildIsInProgress = 1;
							buildFailed = 0;
							break;
						}
					}
				}
				// stop build
				else
				{
					pak = pktCreate(buildServerLink, BUILDCOMM_STOPBUILD);
					pktSend(&pak);
					linkFlush(buildServerLink);
				}
			}
			else if ( CMD_IS(IDC_BUTTON2) )
			{
				Packet *pak;

				if ( !linkConnected(buildServerLink) )
				{
					buildclientConnect();
					buildclientInitListViews();
				}

				// pause build
				if ( buildIsInProgress && !buildIsPaused )
				{
					pak = pktCreate(buildServerLink, BUILDCOMM_PAUSEBUILD);
					pktSend(&pak);
					linkFlush(buildServerLink);
					buildIsPaused = 1;
				}
				// resume build
				else if ( buildIsInProgress )
				{
					pak = pktCreate(buildServerLink, BUILDCOMM_RESUMEBUILD);
					pktSend(&pak);
					linkFlush(buildServerLink);
					buildIsPaused = 0;
				}
				// restart build
				else if ( !buildIsInProgress )
				{
					pak = pktCreate(buildServerLink, BUILDCOMM_RESTARTBUILD);
					pktSend(&pak);
					linkFlush(buildServerLink);
					buildIsInProgress = 1;
					buildFailed = 0;
				}
			}
			else if ( CMD_IS(IDC_VARIABLEVALUE) )
			{
				SetReqVariableValue();
			}
		xcase WM_TIMER:
			{
				// the server update timer
				if ( wParam == 0 )
				{
					Packet *pak = pktCreate(buildServerLink, BUILDCOMM_REQUESTSTATUS);

					if ( !linkConnected(buildServerLink) )
					{
						buildclientConnect();
						buildclientInitListViews();
					}

					pktSend(&pak);
					linkFlush(buildServerLink);

					if ( waitForServerMsg(&pak) )
					{
						if ( !buildStatsHash )
							buildStatsHash = stashTableCreateWithStringKeys(50, StashDeepCopyKeys);

						buildIsInProgress = pktGetBitsPack(pak, 1);
						{
							int idx = 0;
							//BuildStatus *bstats = (BuildStatus*)malloc(sizeof(BuildStatus));
							BuildStatus *bstats;

							ParserSetTableInfo(tpiBuildStatus, sizeof(BuildStatus), "BuildStatus", NULL, __FILE__);
							bstats = StructAllocRaw(tpiBuildStatus);
							ZeroStruct(bstats);
							ParserRecv(tpiBuildStatus, pak, bstats);

							if ( bstats->build_failed )
							{
								//EnableWindow(GetDlgItem(hDlg, IDC_RESUMEBUILD), TRUE);
								SetWindowText(GetDlgItem(hDlg, IDC_BUTTON2), "Restart Build");
								buildFailed = 1;
							}
							else
							{
								if ( !buildIsInProgress )
									EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
								else
								{
									if ( buildIsPaused )
										SetWindowText(GetDlgItem(hDlg, IDC_BUTTON2), "Unpause Build");
									else
										SetWindowText(GetDlgItem(hDlg, IDC_BUTTON2), "Pause Build");
								}
							}

							for ( i = 0; i < eaSize(&bstats->stepStatus); ++i )
							{
								BuildStepStatus *bstep = bstats->stepStatus[i];
								
								if ( !bstep->step )
									continue;

								if ( stashFindInt(buildStatsHash, bstep->step, &idx) )
								{
									buildStepStats[idx]->error = bstep->error;
									buildStepStats[idx]->in_progress = bstep->in_progress;
									buildStepStats[idx]->time_in_seconds = bstep->time_in_seconds;
									bstep = buildStepStats[idx];
								}
								else
								{
									BuildStepStatus *newBstep = (BuildStepStatus*)malloc(sizeof(BuildStepStatus));
									newBstep->step = strdup(bstep->step);
									idx = eaSize(&buildStepStats);
									eaPush(&buildStepStats,newBstep);
									stashAddInt(buildStatsHash, newBstep->step, idx, 1);
									newBstep->error = bstep->error;
									newBstep->in_progress = bstep->in_progress;
									newBstep->time_in_seconds = bstep->time_in_seconds;
									bstep = newBstep;
									listViewAddItem(lvStatus, newBstep);
								}

								if ( bstep->error )
									listViewSetItemColor(lvStatus, bstep, RGB(255,0,0), RGB(255,255,255));
								else if ( bstep->in_progress )
									listViewSetItemColor(lvStatus, bstep, RGB(0,0,180), RGB(255,255,255));
								else if ( bstep->time_in_seconds != 0.0f )
									listViewSetItemColor(lvStatus, bstep, RGB(0,180,0), RGB(255,255,255));
								else
									listViewSetItemColor(lvStatus, bstep, RGB(100,100,100), RGB(255,255,255));
							
								if ( listViewFindItem(lvStatus, bstep) != -1 )
									listViewItemChanged(lvStatus, bstep);
							}

							StructDestroy(tpiBuildStatus, bstats);
						}
						pktFree(&pak);
					}
				}
				// the "me trying to get my list views and input boxes to behave properly" timer
				if ( wParam == 1 )
				{
					//if ( curReqVar && !curReqVar->isActuallyList )
					//	listViewItemChanged(lvVariables, curReqVar);
					if ( updateConfigListView )
					{
						listViewDoOnSelected(lvConfigs, OnConfigSelect, NULL);
						updateConfigListView = 0;
					}
					if ( updateVariableListView )
					{
						listViewDoOnSelected(lvVariables, OnVariableSelect, NULL);
						updateVariableListView = 0;
					}
				}
			}
		xcase WM_NOTIFY:
			{
				int idCtrl = (int)wParam;

				if ( curReqVar )
					EnableWindow(GetDlgItem(hDlg, IDC_VARIABLEVALUE), TRUE);
				else
					EnableWindow(GetDlgItem(hDlg, IDC_VARIABLEVALUE), FALSE);

				if ( buildIsInProgress )
				{
					//EnableWindow(GetDlgItem(hDlg, IDC_STARTBUILD), FALSE);
					SetWindowText(GetDlgItem(hDlg, IDC_BUTTON1), "Stop Build");
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), TRUE);
				}
				else if ( !buildIsInProgress )
				{
					if ( !AllReqVarsAreDefined() || (curCfg && curCfg->disabled) )
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
					else
					{
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
						if ( buildFailed )
							EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), TRUE);
					}
					SetWindowText(GetDlgItem(hDlg, IDC_BUTTON1), "Start Build");
				}

				return (lvConfigs ? listViewOnNotify(lvConfigs, wParam, lParam, NULL) : 0) |
					(lvVariables ? listViewOnNotify(lvVariables, wParam, lParam, NULL) : 0) |
					(lvStatus ? listViewOnNotify(lvStatus, wParam, lParam, NULL) : 0);
			}
			break;
	}
	//return DefWindowProc(hDlg, iMsg, wParam, lParam);
	return FALSE;
}

void buildclientRun()
{
	DialogBox(winGetHInstance(), (LPCTSTR) (IDD_BUILDCLIENTDLG), NULL, (DLGPROC)buildclientDlgProc);
	bcfgFreeConfigs();
}
