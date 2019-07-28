#ifndef SERVER_MONITOR_COMMON_H
#define SERVER_MONITOR_COMMON_H

//#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include "textparser.h"
#include "RegistryReader.h"
#include <commctrl.h>

typedef enum VarMapFlags {
	VMF_READ=1,		// Do not update, just grab
	VMF_WRITE=2,	// Just update, don't grab
	VMF_READWRITE=VMF_READ|VMF_WRITE,
} VarMapFlags;

typedef struct VarMap {
	int id;
	VarMapFlags flags;
	TokenizerParseInfo parse;
	TokenizerParseInfo endtable; // leave NULL so that parse is correctly terminated
} VarMap;

void initText(HWND hDlg, void *base, VarMap mapping[], int count);
void getText(HWND hDlg, void *base, VarMap mapping[], int count);
void loadValuesFromReg(HWND hDlg, void *base, VarMap mapping[], int count);
void saveValuesToReg(HWND hDlg, void *base, VarMap mapping[], int count);

// REGISTRY STUFF
void initRR();
const char *regGetString(const char *key, const char *deflt);
void regPutString(const char *key, const char *value);
void getNameList(const char * category);
void addNameToList(const char * category, char *name);
void removeNameFromList(const char * category, char * name);

extern int name_count;
extern char namelist[256][256];

extern HINSTANCE g_hInst; // instance handle

extern int g_shardmonitor_mode;


void setStatusBar(HWND hStatusBar, int elem, const char *fmt, ...);

void serverMonitorMessageLoop(void);
bool serverMonitorExiting(void);


#endif		// SERVER_MONITOR_COMMON_H