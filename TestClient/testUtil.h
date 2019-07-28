#pragma once
#include "StashTable.h"

void setBadNetwork(void);
void clearBadNetwork(void);
int testClientSafeError(const char *s);

void addToChatMsgs(	char *txt, int msg, void * filter);

typedef enum {
	TCLOG_FATAL,
	TCLOG_SUSPICIOUS,
	TCLOG_DEBUG,
	TCLOG_STATS,
} TestClientLogLevel;

void testClientLog(TestClientLogLevel level, const char *fmt, ...);

void modePush(void);
void modePop(void);

void checkProximity();
extern StashTable playerNames;