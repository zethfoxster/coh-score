#include "prompt.h"
#include "utils.h"
#include "resource.h"
#include "netio.h"
#include <CommCtrl.h>

static DWORD dwValue;
static char cpValue[128];

BOOL CALLBACK ShardMonConfigureNewShardDlgProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

int shardMonConfigureNewShard(HINSTANCE hinst, HWND hwnd, char *name, U32 *ip)
{
	int res;

	dwValue = *ip;
	Strncpyt(cpValue, name);
	res = DialogBox (hinst, MAKEINTRESOURCE (IDD_DLG_NEWSHARD), hwnd, (DLGPROC)ShardMonConfigureNewShardDlgProc);

	if (res) {
		strcpy(name, cpValue);
		*ip = dwValue;
	}

	return res;
}


static BOOL CALLBACK ShardMonConfigureNewShardDlgProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	char buffer[256] = {0};
	switch (iMsg) {
		case WM_INITDIALOG:
			if (dwValue) {
				SetDlgItemText(hDlg, IDC_IP, makeIpStr(dwValue));
			}
			if (cpValue[0]) {
				SetDlgItemText(hDlg, IDC_NAME, cpValue);
			}
			break;

		case WM_COMMAND:

			switch (LOWORD (wParam))
			{
			case IDOK: // Syncronize Data
				GetDlgItemText(hDlg, IDC_NAME, cpValue, ARRAY_SIZE(cpValue)-1);
				GetDlgItemText(hDlg, IDC_IP, buffer, ARRAY_SIZE(buffer)-1);
				dwValue = ipFromString(buffer);
				EndDialog(hDlg, 1);
				return TRUE;
			case IDCANCEL:
				EndDialog(hDlg, 0);
				return TRUE;
			case IDC_IP:	// auto-insert ip address into 
			{
				if(HIWORD (wParam) == EN_SETFOCUS)
				{
					strcpy(buffer, "");
					GetDlgItemText(hDlg, IDC_NAME, buffer, ARRAY_SIZE(buffer) -1);
					if(strlen(buffer))// && !strlen(buf2))
					{
						U32 ip = ipFromString(buffer);
						if(ip != INADDR_NONE)
						{
							SetDlgItemText(hDlg, IDC_IP, makeIpStr(ip));
						}
					}
				}
			}
			break;
		}
		break;
	}
	return FALSE; //DefWindowProc(hDlg, iMsg, wParam, lParam);
}

