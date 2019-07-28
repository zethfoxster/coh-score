#ifndef KILL_PROCESS_H
#define KILL_PROCESS_H

#include <windows.h>

#include <Tlhelp32.h>


BOOL KillProcess(DWORD dwProcessId);
BOOL KillProcessEx(DWORD dwProcessId, BOOL bTree);


void ShutdownProgram(const char* module, int seconds);
BOOL ShutdownProcess(DWORD dwProcessId, int seconds);

#endif // KILL_PROCESS_H
