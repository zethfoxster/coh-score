// @todo this file is no longer used by launcher.
// Still used by chatserver...move there or to common
#define LAUNCHERCOMM_PARSE_INFO_DEFS
#include "performance.h"
#include "PerformanceCounter.h"
#include "structNet.h"
#include "network/netio.h"

PerformanceCounter *counterNetwork=NULL;
PerformanceCounter *counterCPU=NULL;
TrackedPerformance perfInfo;

void perfGetList(void)
{
	static int inited=0;
	if (!inited) {
		inited = 1;
		if (counterNetwork = performanceCounterCreate("Network Interface"))
		{
			performanceCounterAdd(counterNetwork, "Bytes Sent/sec", &perfInfo.bytesSent);
			performanceCounterAdd(counterNetwork, "Bytes Received/sec", &perfInfo.bytesRead);
		}
		if (counterCPU = performanceCounterCreate("Processor"))
			performanceCounterAdd(counterCPU, "% Processor Time", &perfInfo.cpuUsage);
	}

	if (counterNetwork)
		performanceCounterQuery(counterNetwork);

	if (counterCPU)
		performanceCounterQuery(counterCPU);

}

static bool firstTime = true;
void perfSendReset()
{
	firstTime = true;
}

void perfSendTrackedInfo(Packet *pak)
{
	static TrackedPerformance lastInfo;
	if (firstTime) {
		sdPackDiff(TrackedPerformanceInfo, pak, NULL, &perfInfo, true, false, false, 0, 0);
		firstTime = false;
	} else {
		sdPackDiff(TrackedPerformanceInfo, pak, &lastInfo, &perfInfo, false, true, false, 0, 0);
	}
	ParserCopyStruct(TrackedPerformanceInfo, &perfInfo, sizeof(perfInfo), &lastInfo);
}
