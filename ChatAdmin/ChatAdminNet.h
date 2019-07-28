#ifndef __CHAT_ADMIN_NET_H__
#define __CHAT_ADMIN_NET_H__

#include "wininclude.h"

typedef struct Packet Packet;

void setLoginInfo(char * server, char * handle, char * password);

bool chatAdminConnected();
int chatAdminConnect();
int chatAdminDisconnect();
void chatAdminNetTick();
VOID CALLBACK UpdateNetRateProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

void ChatAdminInitNet();

void chatCmdSendf(char * cmd, char * fmt, ...);

char * flagStr(int flags);

typedef struct CAUser CAUser;
typedef struct CAChannel CAChannel;

typedef struct AdminClient {

	// for login purposes
	char		server[256];
	char		handle[100];
	U32			hash[4];	// contains hash of password, used to verify identity on chatserver

	bool		wasConnected;

	CAUser		*user;
	int			accessLevel;

	bool		canChangeHandle;

	CAUser		** ignores;
	CAUser		** friends;

	bool		disableFeedback;	// turn off feedback when receiving full update (via "Login" command)
	bool		invisible;

	bool		uploadComplete;

} AdminClient;

extern AdminClient gAdminClient;

#endif //__CHAT_ADMIN_NET_H__