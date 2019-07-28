#ifndef _XBOX

#include "wininclude.h"

// Win32 Tab Control Utilities

typedef struct DLGHDR_TAB {
	HWND hwndDisplay;
	DLGTEMPLATE *apRes; 
	void * dlgProc;
	char title[256];
} DLGHDR_TAB;

typedef struct tag_dlghdr { 
	HWND hwndTab;       // tab control 
	HWND hwndCurrent;   // current child dialog box
	HWND hwndParent;	// Parent dialog box
	DLGHDR_TAB **eaTabs;
} DLGHDR; 

VOID WINAPI TabDlgInit(HINSTANCE hInst, HWND hwndDlg);
void TabDlgFinalize(HWND hwndDlg);
HWND TabDlgInsertTab(HINSTANCE hInst, DLGHDR *pHdr, int resource, void *dlgProc, char *title, bool showme, void * param);
void TabDlgRemoveTab(DLGHDR *pHdr, HWND hDlg);
VOID WINAPI TabDlgOnSelChanged(HWND hwndDlg); 
bool TabDlgActivateTabIfAvailable(DLGHDR *pHdr, char *title);

#endif