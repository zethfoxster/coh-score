#ifndef _PROCLIST_H
#define _PROCLIST_H

#include "stdtypes.h"
#include "network\netio.h"
#include "launcher_common.h"

#define NUM_TICKS 60
#define MAX_PROCESS_LIST_SIZE	2000	// most number of processes we can track (must be inclusive of all processes running on machine with current algorithm)

typedef struct
{
	U32		process_id;
	U32		time_tables[NUM_TICKS];
	U8		tag;
	U32		count_samples;

	// process memory statistics
	U32		mem_used_phys;	// Working set size in KB (private and shared)
//	U32		mem_priv_phys;	// Private working set size in KB (@todo)
	U32		mem_peak_phys;	// Peak working set size in KB
	U32		mem_used_virt;	// Commit charge of process in KB
//	U32		mem_priv_virt;	// Private commit charge of process in KB (cannot be shared with other processes)
	U32		mem_peak_virt;	// Peak commit charge of process in KB
	U32		thread_count;	// Number of threads used by process

	char	exename[100];

	int		container_id;	// container_id assigned by dbserver
	int		cookie;			// cookie assigned by dbserver
	LaunchType	launch_type;	// Type of launch, assigned by dbserver and implies container type
	int		container_type;	// Type of container, implied by launch_type
	void	*link;			// NetLink this process belongs to
	int		crashed;
	int		ready;			// mapservers can notify us when they are done starting up
} ProcessInfo;

typedef struct
{
	int		num_connected_dbservers;	// total connected dbserver count for reporting purposes
	int		num_static_maps;
	int		num_mission_maps;
	int		num_server_apps;
	int		num_mission_maps_starting;
	int		num_static_maps_starting;
	int		num_mapservers_raw;			// total count of 'mapserver' processes running on this machine, whether we started them or not
	int		num_starts_total;			// total launch requests handled by launcher since startup
	F32		avg_starts_per_sec;			// average launches per second as calculated since the last process update
} ProcessCategories;

typedef struct
{
	ProcessInfo			processes[MAX_PROCESS_LIST_SIZE];
	ProcessInfo			system_info;
	U32					timestamp_tables[NUM_TICKS]; // Array of times (in ms) when sampling occurred
	U32					total_offset; // The number of accumulated msecs from processes who have exited since our sampling began
	int					count;
	ProcessCategories	category_counts;	// count of mission, static, starting, etc.
} ProcessList;

extern ProcessList process_list;

void procSendTrackedInfo(Packet *pak, NetLink *link);
void procGetList(void);
bool trackProcessByContainerId(int container_id,U32 process_id,int cookie,LaunchType launch_type, void *link);
U32 trackProcessByExename(int container_id,int cookie,LaunchType launch_type, const char *command_line, void *link);
bool procRegisterReady(int container_id, U32 process_id, int cookie, int container_type);
void procUpdateTrackingInfo(int num_connected_dbservers, int num_process_starts, F32 elapsed_seconds);
#endif
