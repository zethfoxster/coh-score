#ifndef _DBLOG_H
#define _DBLOG_H

#include "netio.h"

typedef struct MemLog MemLog;
void dbMemLog(const char* func, const char* s, ...);
void dbMemLogEcho(const char* s, ...);

#endif
