#include "memlog.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "wininclude.h"
#include "stdtypes.h"
#include "utils.h"
#include "timing.h"

MemLog g_genericlog = {0};

void memlog_init(MemLog* log)
{
	if (!log) log = &g_genericlog;
	memset(log, 0, sizeof(MemLog));
}

void memlog_setCallback(MemLog* log, Printf callback)
{
	if (!log) log = &g_genericlog;
	log->callback = callback;
}

void memlog_enableThreadId(MemLog* log)
{
	if (!log) log = &g_genericlog;
	log->careAboutThreadId = 1;
}


void memlog_printf(MemLog* log, char const *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	memlog_printfv(log,fmt,ap);
	va_end(ap);
}

void memlog_printfv(MemLog* log, char const *fmt, va_list ap)
{
	int old_value;
	char newformat[256];

	PERFINFO_AUTO_START("memlog_printf", 1);
		if (!log) log = &g_genericlog;
		old_value = (InterlockedIncrement(&log->tail)-1)% MEMLOG_NUM_LINES;
		if (log->careAboutThreadId) {
			sprintf_s(SAFESTR(newformat), "[%5d]: %s", GetCurrentThreadId(), fmt);
		} else {
			strcpy(newformat, fmt);
		}
		_vsnprintf(log->log[old_value],MEMLOG_LINE_WIDTH-1,newformat,ap);
		log->log[old_value][MEMLOG_LINE_WIDTH-1] = 0;
		if (log->callback) {
			log->callback("%s", log->log[old_value]);
		}
		if (old_value == MEMLOG_NUM_LINES-1) {
			log->wrapped = true;
		}
		if (0)
			memlog_dump(log);
	PERFINFO_AUTO_STOP();
}

void memlog_dump(MemLog* log)
{
	int i;
	int eff_tail;
	
	if (!log) log = &g_genericlog;
	eff_tail = log->tail % MEMLOG_NUM_LINES;
	OutputDebugStringf("Dumping memlog...\n");
	if (log->wrapped) {
		for (i=eff_tail; i<MEMLOG_NUM_LINES; i++) {
			OutputDebugStringf("%s\n", log->log[i]);
		}
	}
	for (i=0; i<eff_tail; i++) {
		OutputDebugStringf("%s\n", log->log[i]);
	}
	OutputDebugStringf("Done.\n");
}
