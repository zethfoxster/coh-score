#pragma once

#include <winsock2.h>
#include <windows.h>

LRESULT CALLBACK DlgShardRelayProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

void shardRelayInit();