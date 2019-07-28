#include <windows.h>
#include "DlgMain.h"
#include "resource.h"

HINSTANCE   g_hInst=NULL;			        // instance handle
HWND        g_hWnd=NULL;				        // window handle

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;

	DlgMainDoDialog();

	return 0;
}

void killAllChildren() {
}

int checkChildren() {
	addLineToEnd(IDC_LOG, "checkingcheckingcheckingcheckingcheckingcheckingcheckingcheckingcheckingcheckingcheckingcheckingchecking");
	updateLine(IDC_LIST2, 3, "new data!");
	return 0;
}

void launchNewClient() {
	addLineToEnd(IDC_LIST2, "testingtestingtestingtestingtestingtestingtestingtestingtestingtestingtestingtestingtestingtestingtestingtesting");
}

void doPeriodicUpdate() {
}

void doInit(HWND hDlg) {
}

void doNotify(HWND hDlg, WPARAM wParam, LPARAM lParam) {
}

void showChild(int n) {
}

void killChild(int n) {
}

void sendCommand(int n) {
}

void doOnSelected(HWND hDlg, void* func, void *data) {
}

void setNextChild(void *lv, void *ci, void *junk) {
}

void grabConsole(void *lv, void *ci, void *junk) {
}


void launchNewClientOnSlaves(int n) {
}

/*
char* getComputerName(){
	static char buffer[1024];
	int bufferSize = 1024;

	GetComputerName(buffer, &bufferSize);
	buffer[bufferSize] = '\0';
	return buffer;
}*/

int regGetInt(const char *key, int deflt) {
	return deflt;
}
const char *regGetString(const char *key, const char *deflt) {
	return deflt;
}
void regPutInt(const char *key, int value) {
}
void regPutString(const char *key, const char *value) {
}
