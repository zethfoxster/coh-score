// @todo this file is no longer used by launcher.
// Still used by chatserver...move there or to common
#ifndef _PERFORMANCE_H
#define _PERFORMANCE_H

#include "textparser.h"

typedef struct Packet Packet;

typedef struct TrackedPerformance {
	long bytesSent;
	long bytesRead;
	long cpuUsage;
} TrackedPerformance;

void perfSendTrackedInfo(Packet *pak);
void perfGetList(void);
void perfSendReset(void);

extern TokenizerParseInfo TrackedPerformanceInfo[];
extern TokenizerParseInfo TrackedPerformanceInfoNet[];
extern TokenizerParseInfo TrackedPerformanceInfoCPU[];

#endif

#ifdef LAUNCHERCOMM_PARSE_INFO_DEFS

#ifndef _PERFORMANCE_H_2 
#define _PERFORMANCE_H_2

#include "ListView.h"
#include "textparser.h"

// List sent to DbServer from launcher
TokenizerParseInfo TrackedPerformanceInfo[] = 
{
	{ "BytesSent/sec",	TOK_PRECISION(14) | TOK_INT(TrackedPerformance, bytesSent, 0), },
	{ "BytesRead/sec",	TOK_PRECISION(14) | TOK_INT(TrackedPerformance, bytesRead, 0), },
	{ "CPU Usage",		TOK_PRECISION(8) | TOK_INT(TrackedPerformance, cpuUsage, 0), 0, TOK_FORMAT_LVWIDTH(45) | TOK_FORMAT_PERCENT},
	{0}
};

// Lists used for displaying on ServerMonitor
TokenizerParseInfo TrackedPerformanceInfoNet[] = 
{
	{ "BytesSent/sec",	TOK_PRECISION(14) | TOK_INT(TrackedPerformance, bytesSent, 0), },
	{ "BytesRead/sec",	TOK_PRECISION(14) | TOK_INT(TrackedPerformance, bytesRead, 0), },
	{0}
};
TokenizerParseInfo TrackedPerformanceInfoCPU[] = 
{
	{ "CPU Usage",		TOK_PRECISION(8) | TOK_INT(TrackedPerformance, cpuUsage, 0), 0, TOK_FORMAT_LVWIDTH(45) | TOK_FORMAT_PERCENT},
	{0}
};

#endif
#endif