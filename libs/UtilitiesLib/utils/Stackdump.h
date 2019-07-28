/* Provides simplified way to call corresponding functions in CrashRpt.dll. 
 *
 */

#ifndef STACKDUMP_H
#define STACKDUMP_H

typedef struct _CONTEXT *PCONTEXT;

int sdReady(void);
int sdStartup(void);
void sdShutdown(void);
void sdDumpStack(void);
void sdDumpStackToBuffer(char* buffer, int bufferSize, PCONTEXT stack);

#endif