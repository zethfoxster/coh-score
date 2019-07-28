
#include "relay_actions.h"
#include "comm_backend.h"
#include "netio_core.h"
#include <stdio.h>
#include "AppRegCache.h"
#include "AppVersion.h"
#include "relay_util.h"
#include "kill_process.h"
#include "process_util.h"
#include "utils.h"
#include "sysutil.h"
#include "file.h"
#include "EString.h"

extern CmdRelayCmdStatus g_status;
extern int g_relayType;  // whether this is a DBSERVER, MAPSERVER, etc. relay
char g_lastMsg[2000] = "";
extern BOOL g_cancelThread;
extern BOOL g_bUse64;
extern BOOL g_bNoLauncher;
extern BOOL g_bNoReserved;
extern BOOL g_bStartAll;

extern CRITICAL_SECTION g_criticalSection;
extern char gStopAllScript[FILENAME_MAX];
extern char gStartAllScript[FILENAME_MAX];
extern char g_launcherFlags[1024];

void relayWorkerThreadBegin(int cmd, char * data);	// from relay_main.c


void SetError(const char * msg)
{
	SetStatus(CMDRELAY_CMDSTATUS_ERROR, msg);
}

void SetStatus(CmdRelayCmdStatus status, const char * msg)
{
	EnterCriticalSection(&g_criticalSection);

	g_status = status;
	if(msg)
		strcpy(g_lastMsg, msg);
	else
		strcpy(g_lastMsg, "");

	LeaveCriticalSection(&g_criticalSection);
}

void onSendProtocol(NetLink * link)
{
	Packet	*pak = pktCreate();

	pktSendBitsPack(pak, 1,CMDRELAY_ANSWER_PROTOCOL);
	pktSendBitsPack(pak, 1, CMDRELAY_PROTOCOL_VERSION);
	pktSend(&pak,link);
	lnkFlush(link);
}


// keeps a running count in the LPARAM parameter
#define MAX_WINDOW_TITLE_SIZE	500

// criteria for determining a crashed mapserver: exe name is "mapserver.exe" (case-insensitive) and window title is "City Of Heroes"
BOOL CALLBACK EnumProcCountCrashedMapservers(HWND hwnd, LPARAM lParam)
{
	DWORD processID;
	int * count = (int*) lParam; 

	if(GetWindowThreadProcessId(hwnd, &processID))
	{
		if(ProcessNameMatch(processID, "mapserver.exe"))
		{
			char title[MAX_WINDOW_TITLE_SIZE];
			if(GetWindowText(hwnd, title, MAX_WINDOW_TITLE_SIZE))
			{
				if(stricmp(title, "City Of Heroes")==0)
				{
					++(*count);
				}
			}
		}
	}
	return TRUE;	// signal to continue processing further windows
}


int CountCrashedMapservers()
{
	int count = 0;
	EnumWindows(EnumProcCountCrashedMapservers, (LPARAM) &count);
	return count;
}


void onSendStatus(NetLink * link)
{
	const char * version = getExecutablePatchVersion("coh server");
	Packet	*pak;
	pak = pktCreate();

	pktSendBitsPack(pak, 1,CMDRELAY_ANSWER_STATUS);
	pktSendBitsPack(pak, 1, g_status);
	pktSendBitsPack(pak, 1, g_relayType);
	pktSendBitsPack(pak, 1, ProcessCount("launcher.exe"));
	pktSendBitsPack(pak, 1, ProcessCount("dbserver.exe") + ProcessCount("dbserver64.exe"));
	pktSendBitsPack(pak, 1, ProcessCount("mapserver.exe"));
	pktSendBitsPack(pak, 1, CountCrashedMapservers());
	pktSendBitsPack(pak, 1, ProcessCount("beaconizer.exe"));
	pktSendBitsPack(pak, 1, ProcessCount("authserver.exe"));
	pktSendBitsPack(pak, 1, ProcessCount("accountserver.exe"));
	pktSendBitsPack(pak, 1, ProcessCount("chatserver.exe"));
	pktSendBitsPack(pak, 1, ProcessCount("auctionserver.exe"));
	pktSendBitsPack(pak, 1, ProcessCount("missionserver.exe"));
	pktSendString(pak, version ? version : "Unknown");
	pktSendString(pak, g_lastMsg);

	pktSend(&pak,link);
	lnkFlush(link);
}




BOOL onUpdateSelf(Packet * pak)
{
	FileWrapper * file = 0;
	char * data;
	int count;

	char oldFilename[2000];
	char newFilename[2000];

	printf("Updating self...\n");

	// rename currently running executable
	strcpy(oldFilename, getExecutableName());
	sprintf(newFilename, "%s.bak", oldFilename);

	printf("Renaming '%s' to '%s'\n", oldFilename, newFilename);
	safeRenameFile(oldFilename, newFilename);

	count = pktGetBitsPack(pak, 1);

	data = malloc(sizeof(char) * count);

	pktGetBitsArray(pak, count * 8, data);

	file = fopen(oldFilename, "wb");
	if(file)
	{
		BOOL res;

		if(count != fwrite(data, sizeof(char), count, file))
		{
			// error writing file - revert file back to old name
			safeRenameFile(newFilename, oldFilename);
			printf("Renaming '%s' to '%s'\n", newFilename, oldFilename);
			free(data);
			SetError("Unable auto-update: Failed to write new file");
			return FALSE;
		}
		fclose(file);	
		free(data);

		res = execAndQuit(oldFilename, GetCommandLine());	// will terminate inside this function on success
		if(!res)
			SetError("Unable to start new version of CmdRelay?");	
		return res;
	}
	else
	{
		printf("Unknown error opening output file '%s' while attempting auto-update\n", oldFilename);
		free(data);	
		SetError("Unable auto-update: Failed to open new file");
		return FALSE;
	}
}


BOOL ShouldCancelThread()
{
	BOOL res;
	EnterCriticalSection(&g_criticalSection);
	res = g_cancelThread;
	LeaveCriticalSection(&g_criticalSection);
	return res;
}

// cancel all worker threads by setting their "cancel" flag
void onCancelAll()
{
	printf("Cancelling all commands...\n");
	EnterCriticalSection(&g_criticalSection);
	g_cancelThread	= TRUE;
	g_status		= CMDRELAY_CMDSTATUS_READY;
	LeaveCriticalSection(&g_criticalSection);
}

BOOL StartProgram(const char * filename, const char * params)
{
	BOOL ret;
	char *actualFilename = NULL;
	extern int g_are_executables_in_bin;
	const char *format = g_are_executables_in_bin ? ".\\bin\\%s" : "%s";
	estrConcatf(&actualFilename, format, filename);
	printf("Starting %s %s\n", actualFilename, params ? params : "");
	// returns less than 32 on error
	ret = 32 < (int)ShellExecute(0, "open", actualFilename, params, NULL, SW_SHOW);
	estrDestroy(&actualFilename);
	return ret;
}

BOOL onStartLauncher()
{
	if (g_bNoLauncher)
		return TRUE;

	ShutdownProgram("launcher.exe", 5);
	return StartProgram("launcher.exe", g_launcherFlags);
}

BOOL onStartDbServer()
{
	const char *dbserver = "dbserver.exe";
	int ret;

	if (g_bUse64)
		dbserver = "dbserver64.exe";

	ShutdownProgram("dbserver.exe", 90);	// just in case...
	ShutdownProgram("dbserver64.exe", 90);	// just in case...
	ret = onStartLauncher();
	ret |= StartProgram(dbserver, g_bStartAll ? "-startall" : 0);
	return ret;
}

BOOL onStartAuthServer()
{
	ShutdownProgram("authserver.exe", 30);	// just in case...
	return onStartLauncher() | StartProgram("authserver.exe", 0);
}

BOOL onStartAccountServer()
{
	ShutdownProgram("accountserver.exe", 30);	// just in case...
	return onStartLauncher() | StartProgram("accountserver.exe", 0);
}

BOOL onStartChatServer()
{
	ShutdownProgram("chatserver.exe", 30);	// just in case...
	return onStartLauncher() | StartProgram("chatserver.exe", g_bNoReserved ? "-noreserved" : 0);
}

BOOL onStartAuctionServer()
{
	ShutdownProgram("auctionserver.exe", 30);	// just in case...
	return onStartLauncher() | StartProgram("auctionserver.exe", 0);
}

BOOL onStartMissionServer()
{
	ShutdownProgram("missionserver.exe", 60);	// just in case...
	return onStartLauncher() | StartProgram("missionserver.exe", 0);
}

BOOL onKillAllBeaconizer()
{
	ShutdownProgram("Beaconizer.exe", 10);
	return TRUE;
}

// terminate all running mapservers & launchers
BOOL onKillAllLauncher()
{
	ShutdownProgram("launcher.exe", 5);
	return TRUE;
}

BOOL onKillAllMapserver()
{
	ShutdownProgram("mapserver.exe", 30);
	onKillAllBeaconizer();
	return TRUE;
}

BOOL onStopDbServer()
{
	ShutdownProgram("dbserver.exe", 90);
	ShutdownProgram("dbserver64.exe", 90);
	return TRUE;
}

BOOL onStopAuthServer()
{
	ShutdownProgram("AuthServer.exe", 30);
	return TRUE;
}

BOOL onStopAccountServer()
{
	ShutdownProgram("AccountServer.exe", 30);
	return TRUE;
}

BOOL onStopChatServer()
{
	ShutdownProgram("ChatServer.exe", 30);
	return TRUE;
}

BOOL onStopAuctionServer()
{
	ShutdownProgram("AuctionServer.exe", 30);
	return TRUE;
}

BOOL onStopMissionServer()
{
	ShutdownProgram("MissionServer.exe", 60);
	return TRUE;
}

BOOL runScript(char * batchFile)
{
	char * commandStr;

	if(!fileExists(batchFile))
	{
		SetError("Unknown error - custom script file doesn't exist, but should!");
		printf("Error: Custom script file '%s' does not exist!\n", batchFile);
		return FALSE;
	}

	commandStr = strdup(batchFile);	// this will be deallocated by thread
	relayWorkerThreadBegin(CMDRELAY_REQUEST_RUN_BATCH_FILE, commandStr);

	return TRUE;
}

BOOL onStopAll()
{
	if (!g_bNoLauncher)
		onKillAllLauncher();
	
	// Just in case...
	ShutdownProgram("cityofheroes.exe", 60);

	if(g_relayType & RELAY_TYPE_MAPSERVER) {
		onKillAllMapserver();
	}
	if(g_relayType & RELAY_TYPE_DBSERVER) {
		onStopDbServer();
		ShutdownProgram("queueserver.exe", 15);
		ShutdownProgram("turnstileserver.exe", 15);
		ShutdownProgram("arenaserver.exe", 15);
		ShutdownProgram("raidserver.exe", 15);
		ShutdownProgram("statserver.exe", 30);
		ShutdownProgram("logserver.exe", 30);
	}
	if(g_relayType & RELAY_TYPE_ACCOUNTSERVER)
		onStopAccountServer();
	if(g_relayType & RELAY_TYPE_CHATSERVER)
		onStopChatServer();
	if(g_relayType & RELAY_TYPE_AUCTIONSERVER)
		onStopAuctionServer();
	if(g_relayType & RELAY_TYPE_MISSIONSERVER)
		onStopMissionServer();
	if(g_relayType & RELAY_TYPE_AUTHSERVER)
		onStopAuthServer();
	if(g_relayType & RELAY_TYPE_CUSTOM)
	{
		if(gStopAllScript[0])
		{
			return runScript(gStartAllScript);
		}
	}
		
	return TRUE;
}

BOOL onStartAll()
{
	if(g_relayType & RELAY_TYPE_AUTHSERVER)
		onStartAuthServer();
	if(g_relayType & RELAY_TYPE_MAPSERVER)
		onStartLauncher();
	if(g_relayType & RELAY_TYPE_DBSERVER)
		onStartDbServer();
	if(g_relayType & RELAY_TYPE_ACCOUNTSERVER)
		onStartAccountServer();
	if(g_relayType & RELAY_TYPE_CHATSERVER)
		onStartChatServer();
	if(g_relayType & RELAY_TYPE_AUCTIONSERVER)
		onStartAuctionServer();
	if(g_relayType & RELAY_TYPE_MISSIONSERVER)
		onStartMissionServer();
	if(g_relayType & RELAY_TYPE_CUSTOM)
	{
		if(gStartAllScript[0])
		{
			return runScript(gStartAllScript);
		}
	}

	return TRUE;
}




BOOL startProcessAndWait(char * exeName, char * params)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	BOOL bRes = FALSE;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	// Start the child process. 
	if( !CreateProcess( exeName, // No module name (use command line). 
		params, // Command line. 
		NULL,             // Process handle not inheritable. 
		NULL,             // Thread handle not inheritable. 
		FALSE,            // Set handle inheritance to FALSE. 
		CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,             
		NULL,             // Use parent's environment block. 
		NULL,             // Use parent's starting directory. 
		&si,              // Pointer to STARTUPINFO structure.
		&pi )             // Pointer to PROCESS_INFORMATION structure.
		) 
	{
		SetError( "CreateProcess failed." );
		return FALSE;
	}

	// loop until CohUpdater finishes, or the thread has been "cancelled"
	while(1)
	{
		DWORD r = WaitForSingleObject(pi.hProcess, 2000); 

		if(    r != WAIT_TIMEOUT )
		{
			printf("Process finished normally\n");
			bRes = TRUE;
			break;
		}
		else if(ShouldCancelThread())
		{
			KillProcessEx(pi.dwProcessId, TRUE);	// kill entire process tree
			printf("Aborting process...");
			bRes = TRUE;
			break;
		}
	}

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	return bRes;
}

BOOL onExecuteBatchFile(char * commandStr)
{
	SetStatus(CMDRELAY_CMDSTATUS_CUSTOM_CMD, 0);

	return startProcessAndWait(commandStr, 0);
}

// execute a custom shell command in the background worker thread
BOOL onExecuteCustomCommand(char * commandStr)
{
	char params[10000];

	if(!strcmp(commandStr, ""))
	{
		SetError("Empty command string!");
		return FALSE;
	}

	sprintf(params, "/C \"%s\"", commandStr);		// must surround entire command in quotes
	printf("Executing custom command '%s'...\n", params);
	SetStatus(CMDRELAY_CMDSTATUS_CUSTOM_CMD, 0);

	return startProcessAndWait("cmd.exe", params);
}
