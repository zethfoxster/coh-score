#include "ListView.h"
#include "textparser.h"

extern HINSTANCE   g_hInst;			        // instance handle
extern HWND        g_hWnd;				        // window handle

typedef enum {
	STATUS_TERMINATED = 0, //Process is not running
	STATUS_LOADING, // Loading resources
	STATUS_RUNNING, // Running in the game normally
	STATUS_CONNECTING, // Connecting to a server
	STATUS_QUEUED, // Waiting in queue
	STATUS_ERROR, // Error has occured
	STATUS_DEAD, // Character is dead (waiting to resurrect, etc)
	STATUS_SLAVE_LAUNCHER,
	STATUS_NOPIPE, // Pipe has been disconnected
	STATUS_RANDOM_DISCONNECT, // Testclient intentionally randomly disconnected
	STATUS_RECONNECTING, // Testclient intentionally disconnected and is reconnecting
	STATUS_CRASH, // Crashed and exception caught
} ChildStatus;

#define BATCH_BUF_SIZE 0x100


typedef struct ChildInfo {
	HANDLE process_handle;
	PROCESS_INFORMATION pinfo;
	char *host; // Computer name if a remote host
	int list_index;
	int uid; // For communicating with the PipeServer
	ChildStatus status;
	char status_name[128];
	char *player;
	char *authname;
	char *mapname;
	float hp[2];
	HWND hwnd;
	char *target;
	char *random_disconnect_stage;
	int lost_sent, lost_recv; // network stats
	HWND tab_hwnd;
	FILE *batchFile;
	char batchbuf[BATCH_BUF_SIZE];
	int batchOffset;
	int batchMaxOffset;
} ChildInfo;


ChildInfo* launchTextClient(wchar_t *params, HWND tab);

void launchNewClient(int notunique);
void launchNewClients(int num, int delay_in_ms);
void tick_periodic_launch(void);

void launchNewClientOnSlaves(void);
#define launchNewClientsOnSlaves(num) { int ii; for (ii=0; ii<num; ii++) launchNewClientOnSlaves(); }
int checkChildren(void);
void doPeriodicUpdate(void);
void killAllChildren(void);
void showChild(ListView *lv, ChildInfo *ci, int cmd); // cmd is SW_SHOW or SW_HIDE
void killChild(ListView *lv, ChildInfo *ci, void *junk);
void sendCommand(ListView *lv, ChildInfo *ci, void *junk);
void setNextChild(ListView *lv, ChildInfo *ci, void *junk);
void grabConsole(ListView *lv, ChildInfo *ci, void *junk);
void quitChild(ListView *lv, ChildInfo *ci, void *junk);
void quitAllChildren(void);

void doInit(HWND hDlg);
void doInitPipeServer(); // Initial start up (may write to the log window)
void doInitListView(HWND hDlg); // Initial start up (may write to the log window)
void doNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);
void doOnSelected(HWND hDlg, ListViewCallbackFunc func, void *data);

char *getComputerName(void);
HWND hwndFromProcessID(DWORD dwProcessID);

int regGetInt(const char *key, int deflt);
const char *regGetString(const char *key, const char *deflt);
void regPutInt(const char *key, int value);
void regPutString(const char *key, const char *value);

// --------------------
// TCHAR helpers

int CharTCharStrcmp(char *lhs, TCHAR *rhs);
TCHAR* TStrDupFromChar(char *src);
TCHAR* TCharFromChar(TCHAR *dest, char const *src, int maxLen);
char *CharFromTChar(char *dest, TCHAR const *src, int maxLen);
TCHAR *TCharFromCharStatic(char const *src);

