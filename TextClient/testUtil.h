#pragma once
#include "StashTable.h"

int testClientSafeError(const char *s);

void addToChatMsgs(	char *txt, int msg, void * filter);

typedef enum {
	TCLOG_FATAL,
	TCLOG_SUSPICIOUS,
	TCLOG_DEBUG,
	TCLOG_STATS,
} TestClientLogLevel;

void testClientLog(TestClientLogLevel level, const char *fmt, ...);
