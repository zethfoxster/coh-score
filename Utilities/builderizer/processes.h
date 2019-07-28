#include <windows.h>

PROCESS_INFORMATION *procGetInfo(char *name);
int procGetExitCode(char *name, int *exitCode);
void procWait(HANDLE *proc); // wait for the process to terminate
int procIsRunning(char *name);
int procStore(char *name, PROCESS_INFORMATION *pi);
int procFindProcesses(char *exe, HANDLE **hProcs);
void procKillHandle(HANDLE *proc);
void procKill(PROCESS_INFORMATION *pi);
void procFreeProcesses(HANDLE **hProcs);
