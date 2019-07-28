#ifndef _MEMLOG_H
#define _MEMLOG_H

#define MEMLOG_NUM_LINES 256
#define MEMLOG_LINE_WIDTH 256

typedef void (*Printf)(FORMAT, ...);
typedef struct MemLog 
{
	char log[MEMLOG_NUM_LINES][MEMLOG_LINE_WIDTH];
	volatile long tail;
	int wrapped;
	Printf callback;
	int careAboutThreadId;
} MemLog;

void memlog_init(MemLog* log);
void memlog_enableThreadId(MemLog* log);
void memlog_printf(MemLog* log, FORMAT fmt, ...);
void memlog_printfv(MemLog* log, char const *fmt, va_list args);
void memlog_dump(MemLog* log);
void memlog_setCallback(MemLog* log, Printf callback);

#endif
