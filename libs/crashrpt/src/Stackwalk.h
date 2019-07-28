#ifndef STACKWALK_H
#define STACKWALK_H

#include "CrashRptConf.h"

typedef struct _CONTEXT *PCONTEXT;

CREXTERN int CREXPORT	swReady();
typedef int (CREXPORT *pfnSWReady)();

CREXTERN int CREXPORT	swStartup();
typedef int (CREXPORT *pfnSWStartup)();

CREXTERN void CREXPORT	swShutdown();
typedef void (CREXPORT *pfnSWShutdown)();

CREXTERN void CREXPORT	swDumpStack();
typedef void (CREXPORT *pfnSWDumpStack)();

CREXTERN void CREXPORT	swDumpStackToBuffer(char* buffer, int bufferSize, PCONTEXT stack);
typedef void (CREXPORT *pfnSWDumpStackToBuffer)(char* buffer, int bufferSize, PCONTEXT stack);

#endif