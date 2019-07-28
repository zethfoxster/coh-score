#include "wininclude.h"
// Normal muticies must be released in the same thread as what grabbed it, these
//  get acquired/released in a utility thread.
//  2.7x as much time compared to just a single-thread mutex (approx 0.05ms to acquire and release)
HANDLE acquireThreadAgnosticMutex(const char *name);
void releaseThreadAgnosticMutex(HANDLE hMutex);

// usage: put return testThreadAgnosticMutex(argc, argv); in main()
int testThreadAgnosticMutex(int argc, char **argv);


// Functions to acquire and count global mutexes.
//
// Returns: valid handle if acquired, NULL otherwise.
//
// prefix: can be NULL.
// suffix: can be NULL.
// pid: if -1, uses current pid.

HANDLE acquireMutexHandleByPID(const char* prefix, S32 pid, const char* suffix);
S32 countMutexesByPID(const char* prefix, const char* suffix);
