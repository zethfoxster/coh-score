#ifndef _SYSTEM_MONITOR_H
#define _SYSTEM_MONITOR_H

typedef struct MonitorData
{
	U32 net_bytes_sent;
	U32 net_bytes_read;
	U32 cpu_usage;
	U32 pages_input_sec;	// hard fault count of 4K page reads from paging store/memory mapped files
	U32 pages_output_sec;	// hard fault count of 4K page writes to paging store/memory mapped files
	U32 page_reads_sec;		// read *transactions* necessary to load "pages input", implies seeks
	U32 page_writes_sec;	// write *transactions* necessary to save "pages output", implies seeks
} MonitorData;
	
void				system_monitor_init(void);
void				system_monitor_sample(void);
MonitorData const*	system_monitor_get_data(void);

#endif