#pragma once

#define  WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

LRESULT CALLBACK DlgProcessMonProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

typedef enum ProcessMonitorStatus {
	PMS_NOTRUNNING,
	PMS_RUNNING,
	PMS_CRASHED,
	PMS_NOTMONITORED,
} ProcessMonitorStatus;

const char *pmsToString(ProcessMonitorStatus pms);

typedef struct ProcessMonitorEntry {
	// Order of these elements is important!
	// UI/visible elements
	char exename[128];
	char cmdline[128];
	bool monitor;
	bool start;
	bool restart;
	U32 seconds;

	ProcessMonitorStatus oldstatus;
	ProcessMonitorStatus status;

	int idcStatus;

	// Internal elements
	U32 firstSeenCrashed;
	DWORD processID;
	int temp_start; // flag set after issuing a kill to restart a crashed server
	int crashCount;
	int running; // These two are not thread-safe, they're cleared/reset while processing
	int crashed; // These two are not thread-safe, they're cleared/reset while processing
} ProcessMonitorEntry;

extern ProcessMonitorEntry processmonitors[];

enum {
	PME_DBSERVER,
	PME_LAUNCHER,
};

int executeServer(char *cmd, int minimized);
