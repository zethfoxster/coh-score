#ifndef PROCESS_UTIL_H
#define PROCESS_UTIL_H

#include "wininclude.h"

int getParentPIDIfLauncher();
int killParentIfLauncher();
int kill(DWORD pid);
void killall(const char * module);
void KillAllEx(const char * module, bool bSkipSelf);

// fills passed in array with the directory of the "Coh Server" project (from registry)
int ProcessCount(char * procName);

BOOL ProcessNameMatch( DWORD processID , char * targetName);

void procMemorySizes(size_t *wsbytes, size_t *peakwsbytes, size_t *pfbytes, size_t *peakpfbytes);

#endif // PROCESS_UTIL_H