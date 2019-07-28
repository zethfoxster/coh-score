#ifndef _RENDERPERF_H
#define _RENDERPERF_H

typedef struct
{
	float maxThroughput;
	float minTimePerCall;
	int numTests;
} PerfTestResultsHeader;

typedef struct
{
	int trisPerModel;
	float avgTimePerCall;
	float avgThroughput;
	float avgCalls;
} PerfTestResult;

typedef struct
{
	PerfTestResultsHeader header;
	PerfTestResult **testResults;
} PerfTestResults;

void freePerfTestResults(PerfTestResults *results);

void renderPerfTest(int test_num, int num_runs, PerfTestResults *results);
void renderPerfTestSuite(int num_runs);

#endif
