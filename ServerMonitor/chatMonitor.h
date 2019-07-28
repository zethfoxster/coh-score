#ifndef __CHAT_MONITOR_H
#define __CHAT_MONITOR_H

#include <winsock2.h>
#include <windows.h>
#include "net_structdefs.h"
#include "textparser.h"
#include "listView.h"


#define CHATMON_PROTOCOL_VERSION	( 20050106 )

// Commands from ChatServer to ServerMonitor
enum
{
	CHATMON_STATUS = COMM_MAX_CMD, // Receive full status update from Chatserver
	CHATMON_PROTOCOL_MISMATCH,
};

// Commands from ServerMonitor to ChatServer
enum
{
	SVRMONTOCHATSVR_ADMIN_SENDALL = COMM_MAX_CMD, // Receive full status update from Chatserver
	SVRMONTOCHATSVR_CONNECT,
	SVRMONTOCHATSVR_SHUTDOWN,
};


LRESULT CALLBACK DlgChatMonProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

extern TokenizerParseInfo ChatConNetInfo[];

void chatSetAutoConnect(bool connect);
BOOL chatMonConnected();
BOOL chatMonExpectedConnection();
int chatMonConnect(void);

#endif // __CHAT_MONITOR_H