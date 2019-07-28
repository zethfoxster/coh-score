#pragma once

#include "ListView.h"
#include "serverMonitorCommon.h"
#include "serverMonitorCmdRelay.h"
#include "netio_core.h"
#include "relay_types.h"
#include "resource.h"

typedef struct 
{
	int size;
	char * data;
} FileAllocInfo;

BOOL OpenAndAllocFile(char * title, char * pattern, FileAllocInfo * file);

void sendBatchFileToClient(ListView *lv, CmdRelayCon *con, FileAllocInfo * file);

char *OpenFileDlg(char * title, char *fileMask,char *fileName);


extern char g_updateServerAddr[512];
extern char g_customCmd[1024];


static VarMap relayMapping[2] = {
	{IDC_COMBO_RELAY_UPDATE_SVR, true, 0, TOK_STRING_X, (int)&g_updateServerAddr, sizeof(g_updateServerAddr)},
	{IDC_COMBO_RELAY_CUSTOM_CMD, true, 0, TOK_STRING_X, (int)&g_customCmd, sizeof(g_customCmd) },
};