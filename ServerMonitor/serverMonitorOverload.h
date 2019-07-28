#include <winsock2.h>
#include <windows.h>

// Overload Protection dialog box

typedef struct ServerMonitorState ServerMonitorState;
typedef struct ServerStats ServerStats;

void smoverloadShow(ServerMonitorState *state);
int smoverloadIsVisible();
int smoverloadHook(PMSG pmsg);
void smoverloadUpdate();

void smoverloadFormat(ServerStats *stats, char *string);
bool smoverloadIsManualOverride(ServerStats *stats);
void smoverloadSetManualOverride(bool enable);
