#include "monitor.h"
#include "PerformanceCounter.h"

PerformanceCounter *s_counterNetwork=NULL;
PerformanceCounter *s_counterCPU=NULL;
PerformanceCounter *s_counterMemory=NULL;
MonitorData			s_samples;

MonitorData const*	system_monitor_get_data(void)
{
	return &s_samples;
}

void system_monitor_init(void)
{
	static int inited=0;
	if (!inited)
	{
		inited = 1;
		if (s_counterNetwork = performanceCounterCreate("Network Interface"))
		{
			performanceCounterAdd(s_counterNetwork, "Bytes Sent/sec", &s_samples.net_bytes_sent);
			performanceCounterAdd(s_counterNetwork, "Bytes Received/sec", &s_samples.net_bytes_read);
		}
		if (s_counterCPU = performanceCounterCreate("Processor"))
			performanceCounterAdd(s_counterCPU, "% Processor Time", &s_samples.cpu_usage);
		if (s_counterMemory = performanceCounterCreate("Memory"))
		{
			performanceCounterAdd(s_counterMemory, "Pages Input/sec", &s_samples.pages_input_sec);
			performanceCounterAdd(s_counterMemory, "Pages Output/sec", &s_samples.pages_output_sec);
			performanceCounterAdd(s_counterMemory, "Page Reads/sec", &s_samples.page_reads_sec);
			performanceCounterAdd(s_counterMemory, "Page Writes/sec", &s_samples.page_writes_sec);
		}
	}
}

void system_monitor_sample(void)
{
	if (s_counterNetwork)
		performanceCounterQuery(s_counterNetwork);

	if (s_counterCPU)
		performanceCounterQuery(s_counterCPU);

	if (s_counterMemory)
		performanceCounterQuery(s_counterMemory);
}
