#ifndef RELAY_UTIL_H
#define RELAY_UTIL_H


#include "relay_types.h"

BOOL execAndQuit(char *exe_name, char * cmd);
BOOL safeRenameFile(char *oldnamep,char *newnamep);
BOOL copyAndRestartInTempDir();

#endif // RELAY_UTIL_H