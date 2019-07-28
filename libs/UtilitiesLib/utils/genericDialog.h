#ifndef _XBOX
#pragma once

#include "wininclude.h"

// OkToAll functions return this if OkToAll was selected
#define IDOKTOALL 3

// IDs for the various dialog elements, for use in the generic dialog callback
#define IDD_MB_OKTOALL                  105
#define IDD_DLG_REQUEST_STRING          106
#define IDC_OKTOALL                     1029
#define IDC_OKTOALL_EDIT                1030
#define IDC_EDIT_REQUEST_STRING         1033
#define IDC_EDIT_INPUT_STRING           1034
#define IDC_OKTOALLCANCEL_EDIT			1035


// handles extended functionality for generic dialog boxes
typedef int(*GenericDialogCallback)(HWND, UINT, WPARAM, LPARAM); 

// reutrns IDOKTOALL if they hit OKTOALL, IDOK if they just hit OK, and IDCANCEL if they hit cancel
int okToAllCancelDialog(const char *dialogText, const char *caption);
int okToAllCancelDialogEx(char *dialogText, char *caption, GenericDialogCallback callback_proc);

// reutrns IDOKTOALL if they hit OKTOALL, or IDOK if they just hit OK
int okToAllDialog(const char *dialogText, const char *caption);
int okToAllDialogEx(char *dialogText, char *caption, GenericDialogCallback callback_proc);

// reutrns IDOK if they hit OK, and IDCANCEL if they hit cancel
int okCancelDialog(const char *dialogText, const char *caption);

// returns the input string of the dialog box
char *requestStringDialog(char *dialogText, char *caption);
char *requestStringDialogEx(char *dialogText, char *caption, GenericDialogCallback callback_proc);

// flash the toolbar of the given window
void flashWindow(HWND hWnd);

// progress dialog box
bool startProgressDialog(HWND parent, wchar_t * title, wchar_t * cancel);
bool updateProgressDialog(unsigned long long progress, unsigned long long total, wchar_t * text, wchar_t * path);
void stopProgressDialog();

#endif