#include <stdio.h>
#include "wininclude.h"
#include <tlhelp32.h>
#include "stdtypes.h"
#include <psapi.h>
#include "proclist.h"
#include "network\netio.h"
#include "assert.h"
#include "comm_backend.h"
#include "utils.h"
#include "earray.h"
#include "file.h"
#include "error.h"
#include "StashTable.h"
#include "monitor.h"

extern bool is_launcher_suspended();

ProcessList			process_list;

int get_crash_count(void);

static int launch_type_to_container_type(LaunchType lt)
{
	if (lt == LT_MISSIONMAP || lt == LT_STATICMAP)
	{
		return CONTAINER_MAPS;
	}
	else if (lt == LT_SERVERAPP)
	{
		return CONTAINER_SERVERAPPS;
	}
	else
	{
		devassert(false);	// launch type doesn't have a corresponding container type
		return CONTAINER_TESTDATABASETYPES;
	}
}

static void addFileTime(FILETIME *a,FILETIME *b,FILETIME *c)
{
	LARGE_INTEGER lia, lib, lic;
	lia.LowPart = a->dwLowDateTime;
	lia.HighPart = a->dwHighDateTime;
	lib.LowPart = b->dwLowDateTime;
	lib.HighPart = b->dwHighDateTime;
	lic.QuadPart = lia.QuadPart + lib.QuadPart;
	c->dwHighDateTime = lic.HighPart;
	c->dwLowDateTime = lic.LowPart;
}

static U32 milliSecondsRunning(FILETIME kt,FILETIME ut)
{
FILETIME	tt;
double		ftime,seconds;
U32			t,ts;
#define SEC_SCALE 0.0000001

	addFileTime(&kt,&ut,&tt);
	seconds = tt.dwLowDateTime * SEC_SCALE;
	ftime = tt.dwHighDateTime * SEC_SCALE * 4294967296.0;
	t = ftime * 1000;
	ts = seconds * 1000;
	return t + ts;
}

// update tracking info common to all dbservers served by this launcher
// elapsed_seconds is seconds since we last performed an update
void procUpdateTrackingInfo(int num_connected_dbservers, int num_process_starts, F32 elapsed_seconds)
{
	int	i;
	memset(&process_list.category_counts, 0, sizeof(process_list.category_counts) );

	for(i=0;i<process_list.count;i++)
	{
		ProcessInfo* pi = &process_list.processes[i];
		// count processes we are tracking at the request of a dbserver
		// and that haven't crashed.
		if (pi->container_id >= 0 && pi->crashed == 0)
		{
			// categorize maps, totals for launches from ALL dbservers using this launcher
			if (pi->launch_type == LT_MISSIONMAP)
			{
				process_list.category_counts.num_mission_maps++;
				if (!pi->ready)
				{
					process_list.category_counts.num_mission_maps_starting++;
				}
			}
			else if (pi->launch_type == LT_STATICMAP)
			{
				process_list.category_counts.num_static_maps++;
				if (!pi->ready)
				{
					process_list.category_counts.num_static_maps_starting++;
				}
			}
			else if (pi->launch_type == LT_SERVERAPP)
				process_list.category_counts.num_server_apps++;
		}

		// keep a 'raw' count of mapserver processes which can be used to tell
		// when the launcher has become out of sync with the dbservers it is
		// serving, e.g. launcher was killed  and then later restarted it will
		// still have a bunch of pre-existing maps running on the host but it will
		// only officially report on servers it starts up from then on.
		// @todo this is brittle but better than nothing at the moment
		if (stricmp(pi->exename, "mapserver")==0)
		{
			process_list.category_counts.num_mapservers_raw++;
		}
	}

	// calculate the average arrival rate of launch requests since our last
	// update.
	// generally updated only at a rate of once per second, so elapsed time should
	// never really be too small
	process_list.category_counts.num_starts_total = num_process_starts;	// total launch requests handled by launcher
	if (elapsed_seconds > 0.0001f )
	{
		static int s_process_starts_last_updated;	// for calculating launchers/sec at update time
		int delta_starts = num_process_starts - s_process_starts_last_updated;
		F32 start_rate = delta_starts/elapsed_seconds;
		process_list.category_counts.avg_starts_per_sec = start_rate;
		s_process_starts_last_updated = num_process_starts;
	}

	process_list.category_counts.num_connected_dbservers = num_connected_dbservers;
}

int procInfoSend(ProcessInfo *pi,Packet *pak,int num_processors_scale)
{
	F32		cpu_elapsed,clock_elapsed,cpu_usage,cpu_usage60;
	U32		msecs;
	int		end;

	msecs = pi->time_tables[0];
	cpu_elapsed = pi->time_tables[0] - pi->time_tables[1];
	clock_elapsed = process_list.timestamp_tables[0] - process_list.timestamp_tables[1];
	if (process_list.timestamp_tables[1]) {
		cpu_usage = cpu_elapsed / clock_elapsed;
	} else {
		// initial case
		cpu_usage = 0;
	}

	for (end=1; end < NUM_TICKS-1 && process_list.timestamp_tables[end] && pi->time_tables[end]; end++);
	end--;
	cpu_elapsed = pi->time_tables[0] - pi->time_tables[end];
	clock_elapsed = process_list.timestamp_tables[0] - process_list.timestamp_tables[end];
	if (clock_elapsed) {
		cpu_usage60 = cpu_elapsed / clock_elapsed;
	} else {
		cpu_usage60 = 0;
	}

	pktSendBitsPack(pak,1,pi->container_id);
	pktSendBitsPack(pak,1,pi->container_type);
	pktSendBitsPack(pak,10,pi->cookie);
	pktSendBitsPack(pak,1,pi->process_id);
	pktSendBitsPack(pak,1,pi->mem_used_phys);
	pktSendBitsPack(pak,1,pi->mem_peak_phys);
	pktSendBitsPack(pak,1,pi->mem_used_virt);
//	pktSendBitsPack(pak,1,pi->mem_priv_virt);
	pktSendBitsPack(pak,1,pi->mem_peak_virt);
	pktSendBitsPack(pak,1,pi->thread_count);
	pktSendBitsPack(pak,1,msecs);
	pktSendF32(pak,cpu_usage/num_processors_scale);
	pktSendF32(pak,cpu_usage60/num_processors_scale);
//	pktSendF32(pak,0/*cpu_usage3600*/);
	return 1;
}

void procSendTrackedInfo(Packet *pak, NetLink *link)
{
	int		i;

	static int	num_processors=0;
	if (num_processors==0) {
		SYSTEM_INFO sysinfo;
		GetNativeSystemInfo(&sysinfo);
		num_processors = sysinfo.dwNumberOfProcessors;
	}

	// Send information of each tracked process

	// first entry is actually for overall system
	// @todo remove this as it's confusing and we send most of the host system
	// statistics at the end of the process list
	procInfoSend(&process_list.system_info,pak,num_processors); // Scale launcher CPU usage by number of CPUs

	for(i=0;i<process_list.count;i++)
	{
		ProcessInfo* pi = &process_list.processes[i];
		// only send information about processes we are tracking at the request of a dbserver
		if (pi->container_id >= 0)
		{
			// Include the test against the link, this filters this packet to only contain data about mapserver's that the dbserver in question started
			// I have no idea at all what would happen if we reported Freedom's mapservers to Virtue, and I don't intend to find out. :)
			if (pi->link == (void *) link)
			{
				procInfoSend(pi,pak,1); // Send MapServer process info as % of single CPU (only used by ServerMonitor)
			}
		}
	}
	pktSendBitsPack(pak,1,-1);

	//**
	// Send overall host machine stats

	// metrics we can grab easily through windows API
	{
		PERFORMANCE_INFORMATION info;
		U32	page_in_kb;

		ZeroMemory(&info,sizeof(info));
		info.cb = sizeof(info);
		GetPerformanceInfo(&info, sizeof(info));

		// info is reported in page counts, which we convert to KB expected by DBServer/ServerMonitor
		page_in_kb = info.PageSize >> 10;	// page size in KB

		pktSendBits(pak, 32, info.PhysicalTotal*page_in_kb);					// host machine total physical memory in KB
		pktSendBits(pak, 32, info.PhysicalAvailable*page_in_kb);				// host machine available physical memory in KB
		pktSendBits(pak, 32, info.CommitLimit*page_in_kb);						// host machine commit limit in KB
		pktSendBits(pak, 32, (info.CommitLimit - info.CommitTotal)*page_in_kb);	// host machine available address space that can be committed in KB
		pktSendBits(pak, 32, info.CommitPeak*page_in_kb);						// host machine commit peak in KB
		pktSendBits(pak, 32, info.ProcessCount);								// host machine number of processes
		pktSendBits(pak, 32, info.ThreadCount);									// host machine number of threads
		pktSendBits(pak, 32, info.HandleCount);									// host machine number of open handles
	}

	// ...and others that we need to acquire through the PDH performance monitor counter interfaces (which suck...)
	{
		MonitorData const*	perf_info = system_monitor_get_data();
		pktSendBits(pak, 32, perf_info->net_bytes_sent);
		pktSendBits(pak, 32, perf_info->net_bytes_read);
		pktSendBits(pak, 32, perf_info->pages_input_sec);	// hard fault count of 4K page reads from paging store/memory mapped files
		pktSendBits(pak, 32, perf_info->pages_output_sec);	// hard fault count of 4K page writes to paging store/memory mapped files
		pktSendBits(pak, 32, perf_info->page_reads_sec);	// read *transactions* necessary to load "pages input", implies seeks
		pktSendBits(pak, 32, perf_info->page_writes_sec);	// write *transactions* necessary to save "pages output", implies seeks
		pktSendBitsPack(pak, 8, perf_info->cpu_usage);		// system reported CPU usage percentage (from "\Processor Information(_Total)\% Processor Time")
		pktSendBitsPack(pak, 5, num_processors);			// number of CPU cores on host machine
	}
	// we send the launcher manual suspension state with the other metrics as that is used to tell
	// the load balancer not to launch on this machine
	pktSendBits(pak, 1, is_launcher_suspended());								// suspension state
	// Send categorized tracked launch type counts
	pktSendBitsPack(pak, 12, process_list.category_counts.num_static_maps);
	pktSendBitsPack(pak, 12, process_list.category_counts.num_mission_maps);
	pktSendBitsPack(pak, 12, process_list.category_counts.num_server_apps);
	pktSendBitsPack(pak, 12, process_list.category_counts.num_static_maps_starting);
	pktSendBitsPack(pak, 12, process_list.category_counts.num_mission_maps_starting);
	// send raw count of mapserver processes whether tracked or not (good for sanity check and detect when launcher is not in sync)
	pktSendBitsPack(pak, 12, process_list.category_counts.num_mapservers_raw);
	pktSendBits(pak, 32, process_list.category_counts.num_starts_total);
	pktSendF32(pak, process_list.category_counts.avg_starts_per_sec);
	pktSendBitsPack(pak, 12, process_list.category_counts.num_connected_dbservers);
	pktSendBitsPack(pak, 12, get_crash_count());	// supply number of crashed processes
	
}

static void calcTimers(ProcessInfo *pi,U32 msecs)
{
int		j,d;

	if (!pi->count_samples)
	{
		for(j=0;j<NUM_TICKS;j++)
			pi->time_tables[j] = msecs;
	} else {
		memmove(&pi->time_tables[1],&pi->time_tables[0],(NUM_TICKS-1) * sizeof(U32));
		d = pi->time_tables[0] - msecs;
		if (d > 0)
		{
			// this will (probably) help when 60 days worth of CPU time (2^32 millis) has passed
			for(j=1;j<NUM_TICKS;j++)
				pi->time_tables[j] -= d;
		}
		pi->time_tables[0] = msecs;
	}
}

// Because we've gone asynchronous, it's now possible to reach here with either a link that's NULL, or that's not connected.
// So we test both of those cases.
void notifyProcessClosed(U32 process_id, int container_id, int cookie, int container_type, NetLink *link)
{
	if (link && link->connected)
	{
		Packet *pak = pktCreateEx(link, LAUNCHERANSWER_PROCESS_CLOSED);
		pktSendBitsPack(pak, 1, container_id);
		pktSendBitsPack(pak, 1, container_type);
		pktSendBitsPack(pak, 10, cookie);
		pktSendBitsPack(pak, 1, process_id);
		pktSend(&pak, link);
	}
}

void notifyProcessCrashed(U32 process_id, int container_id, int cookie, int container_type, int unhandled_crash, NetLink *link)
{
	if (link && link->connected)
	{
		Packet *pak = pktCreateEx(link, LAUNCHERANSWER_PROCESS_CRASHED);
		pktSendBitsPack(pak, 1, container_id);
		pktSendBitsPack(pak, 1, container_type);
		pktSendBitsPack(pak, 10, cookie);
		pktSendBitsPack(pak, 1, process_id);
		pktSendBitsPack(pak, 1, unhandled_crash);
		pktSend(&pak, link);
	}
}

static BOOL CALLBACK EnumProcCheckIfCrashed(HWND hwnd, LPARAM lParam)
{
	int *crashed = (int*)lParam;
	char text[500];
	if(GetWindowText(hwnd, text, ARRAY_SIZE(text))) {
		if (0==stricmp(text, "&Send Error Report") ||
			0==stricmp(text, "De&bug") ||
			0==stricmp(text, "Debug") ||
			0==stricmp(text, "&Abort"))
		{
			*crashed=1;
		}
	}
	return !*crashed;
}

static StashTable htCrashes;

static BOOL CALLBACK EnumProcFindCrashedServers(HWND hwnd, LPARAM lParam)
{
	DWORD processID;

	if(GetWindowThreadProcessId(hwnd, &processID))
	{
		char title[500];
		if(GetWindowText(hwnd, title, ARRAY_SIZE(title)))
		{
			int is_crashed = 0;
			if(stricmp(title, "City Of Heroes")==0)
			{
				is_crashed = 1;
			} else if (strEndsWith(title, ".exe") ||
				strStartsWith(title, "Microsoft Visual C"))
			{
				int crashed=0;
				// Might be a crash window
				EnumChildWindows(hwnd, EnumProcCheckIfCrashed, (LPARAM)&crashed);
				if (crashed) {
					is_crashed = 2; // unhandled!
				}
			}
			if (is_crashed) {
				stashIntAddInt(htCrashes, processID, is_crashed, false);
			}
		}
	}
	return TRUE;	// signal to continue processing further windows
}

static void findCrashedServers()
{
	if (!htCrashes)
		htCrashes = stashTableCreateInt(16);
	else
		stashTableClear(htCrashes);

	EnumWindows(EnumProcFindCrashedServers, (LPARAM)NULL);
}

int get_crash_count(void)
{
	int count = 0;
	if (htCrashes)
		count = stashGetValidElementCount(htCrashes);
	return count;
}

int isProcessCrashed(int process_id)
{
	int iResult;
	if ( stashIntFindInt(htCrashes, process_id, &iResult))
		return iResult;
	return 0;
}

// @todo
// This code and the 'crashed map' management code uses process IDs to match up process list entries
// without accounting for the fact that windows can reuse process IDs.
// Identifying a process should involve a comparison of process id and a process creation time.
// This code should be refactored.
void procGetList()
{
	PROCESSENTRY32 pe;
	BOOL retval;
	HANDLE hSnapshot;
	ProcessInfo	*pi;
	int			i,launcher_count=0;
	FILETIME	total_time = {0,0},zero_time = {0,0};
	FILETIME	current_time;
	SYSTEMTIME	current_system_time;
	U32			current_time_millis;
	int			reset_stats=0;

	GetSystemTime(&current_system_time); // gets current time
	SystemTimeToFileTime(&current_system_time, &current_time);  // converts to file time format
	current_time_millis = milliSecondsRunning(current_time, zero_time);

	hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if ((int)hSnapshot==-1)
	{
		printf("ERROR: procGetList: System cannot create process snapshot. No process updates will go to dbservers\n");
		return;
	}

	pe.dwSize=sizeof(PROCESSENTRY32);
	retval=Process32First(hSnapshot,&pe);
	while(retval)
	{
		retval=Process32Next(hSnapshot,&pe);

		// 07-12-2011
		// Windows 7/Windows Server 2008 r2 will spawn a conhost.exe process to accompany any process with its
		// own console in order to handle system related features such as themes, drag and drop, etc.
		// This means that we actually get two processes running for every server we start.
		// There is no reason to be tracking and sending information on these auxiliary processes to the dbserver
		// @todo
		// It would be better to have an inclusion list so that we only track processes that we have launched
		// or were pre-existing that we are actually interested in, in order to save processing and bandwidth here
		// and at the dbserver.
		// Or at least have an exclusion list of the system processes which get lots of instances
		if (stricmp(pe.szExeFile, "conhost.exe")==0)
		{
			continue;
		}

		for(i=0;i<process_list.count;i++)
		{
			if (process_list.processes[i].process_id == pe.th32ProcessID)
			{
				if (process_list.processes[i].tag)
					process_list.processes[i].count_samples++;
				process_list.processes[i].tag = 0;
				break;
			}
		}
		if (i >= process_list.count)
		{
			if (process_list.count < ARRAY_SIZE(process_list.processes))
			{
				pi = &process_list.processes[process_list.count++];
				memset(pi,0,sizeof(*pi));
				pi->process_id = pe.th32ProcessID;
				pi->container_id = -1;
				pi->container_type = 0;
				pi->cookie = 0;
				pi->link = NULL;
				pi->thread_count = pe.cntThreads;	// @todo Seems that this is only ever 1, so not a good source of thread count...may need to walk the process
				// store filename of exe from pe, convenient
				{
					char* s;
					Strncpyt(pi->exename, pe.szExeFile);	
					if (s = strrchr(pi->exename, '.'))		// trim extension
						*s = 0;
				}

			}
			else
			{
				printf("ERROR: procGetList: process_list too small to handle the current number of processes.\n");
				break;
			}

		}
	}
	CloseHandle(hSnapshot);

	findCrashedServers();

	for(i=0;i<process_list.count;i++)
	{
		pi = &process_list.processes[i];
		if (pi->tag)
		{
			// Process has exited
			if (pi->container_id>=0 || pi->cookie) {
				// We should notify the DbServer
				// Since we blindly pass in the link saved with the process, process that weren't explicitly started by any DbServer
				// will not have a valid link, hence the test at the top of notifyProcessClosed(...).  In addition, if the DbServer
				// has disconnected between starting this mapserver and it closing, the link will show as disconnected. Thus we test
				// that conditions as well.
				notifyProcessClosed(pi->process_id, pi->container_id, pi->cookie, pi->container_type, pi->link);
			}

			// Add to process_list.total_offset!
			process_list.total_offset += pi->time_tables[0];
			memmove(pi,&pi[1],sizeof(ProcessInfo) * (process_list.count - i - 1));
			process_list.count--;
			i--;
			continue;
		}
		else
		{
			HANDLE		p;
			U32			msecs;
			FILETIME	cr,ex,kt,ut;
			PROCESS_MEMORY_COUNTERS_EX mem_counters;

			pi->tag = 1;
			p = OpenProcess(PROCESS_QUERY_INFORMATION| PROCESS_VM_READ,FALSE,pi->process_id);
			if (!p) {
				// Unable to query the process, reset stats, otherwise they'll be very messed up when this process comes back!
				//Apparently on some systems we get processes we can *never* query, so let's not reset the stats every tick,
				// and just deal with the wacky values when we get them =(
				//This also happens if we get a list of process IDs, and the process is
				// closed before we can query it, treat it as closed now then!  Otherwise
				// the DbServer will assume it's closed because it was not reported upon.
				//reset_stats = 1;
				if (pi->container_id>=0 || pi->cookie)
				{
					printf("Error querying process (ct: %d, cid: %d, pid: %d, %s), assuming closed\n", pi->container_type, pi->container_id, pi->process_id, pi->exename);
					// We should notify the DbServer
					notifyProcessClosed(pi->process_id, pi->container_id, pi->cookie, pi->container_type, pi->link);
				}
				// Treat as closed for this tick
				process_list.total_offset += pi->time_tables[0];
				memmove(pi,&pi[1],sizeof(ProcessInfo) * (process_list.count - i - 1));
				process_list.count--;
				i--;
				continue;
			}
			GetProcessTimes(p,&cr,&ex,&kt,&ut);
			mem_counters.cb = sizeof(mem_counters);
			if (GetProcessMemoryInfo(p,(PROCESS_MEMORY_COUNTERS*)&mem_counters,sizeof(mem_counters)))
			{
				pi->mem_used_phys = mem_counters.WorkingSetSize >> 10;			// Working set size in KB (private and shared)
				pi->mem_peak_phys = mem_counters.PeakWorkingSetSize >> 10;		// Peak working set size in KB
				pi->mem_used_virt = mem_counters.PrivateUsage >> 10;			// Commit charge of process in KB (@todo currently always same value as PagefileUsage)
//				pi->mem_priv_virt = mem_counters.PrivateUsage >> 10;			// Private commit charge of process in KB (cannot be shared with other processes)
				pi->mem_peak_virt = mem_counters.PeakPagefileUsage >> 10;		// Peak private commit charge of process in KB
				// @todo currently private working set is only available from the PDH
				// "Process" performance counters but its not obvious how to specify the
				// correct instance since you have to do it by enumerated name and can't use the process ID
			}
			// We use the process name populated when walking the process entries.
			CloseHandle(p);

			msecs = milliSecondsRunning(kt,ut);
			addFileTime(&kt,&total_time,&total_time);
			addFileTime(&ut,&total_time,&total_time);

			calcTimers(pi,msecs);
			if (!pi->crashed && (pi->crashed = isProcessCrashed(pi->process_id))) {
				notifyProcessCrashed(pi->process_id, pi->container_id, pi->cookie, pi->container_type, pi->crashed == 2, pi->link);
			}
		}

	}

	// @todo kind of pointless since the information can be derived from the overall system information we pass on
	// the dbservers
	{
		MEMORYSTATUSEX memoryStatus;
		ZeroMemory(&memoryStatus,sizeof(MEMORYSTATUSEX));
		memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

		GlobalMemoryStatusEx(&memoryStatus);
		process_list.system_info.mem_used_phys = (memoryStatus.ullTotalPhys - memoryStatus.ullAvailPhys) >> 10;
		process_list.system_info.mem_used_virt = (memoryStatus.ullTotalPageFile - memoryStatus.ullAvailPageFile) >> 10;
	}

	// Update timestamp_tables
	if (process_list.system_info.count_samples==0) {
		memset(process_list.timestamp_tables, 0, sizeof(process_list.timestamp_tables));
	}
	memmove(&process_list.timestamp_tables[1], &process_list.timestamp_tables[0], (NUM_TICKS-1)*sizeof(U32));
	process_list.timestamp_tables[0] = current_time_millis;
	calcTimers(&process_list.system_info,milliSecondsRunning(total_time,zero_time) + process_list.total_offset);
	process_list.system_info.count_samples++;

	if (reset_stats) {
		process_list.system_info.count_samples = 0;
	}
	
}

bool procRegisterReady(int container_id,U32 process_id,int cookie,int container_type)
{
	int		i,retry;

	for(retry=0;retry<2;retry++)
	{
		for(i=0;i<process_list.count;i++)
		{
			if (process_list.processes[i].process_id == process_id && process_list.processes[i].cookie == cookie)
			{
				process_list.processes[i].ready = true;
				return true;
			}
		}
		procGetList();
	}
	return false;
}

bool trackProcessByContainerId(int container_id,U32 process_id,int cookie,LaunchType launch_type, void *link)
{
	int		i,retry;

	for(retry=0;retry<2;retry++)
	{
		for(i=0;i<process_list.count;i++)
		{
			if (process_list.processes[i].process_id == process_id)
			{
				process_list.processes[i].container_id = container_id;
				process_list.processes[i].cookie = cookie;
				process_list.processes[i].launch_type = launch_type;
				process_list.processes[i].container_type = launch_type_to_container_type(launch_type);
				if (link != NULL) // process launch passes in the DbServer's link.
				{
					process_list.processes[i].link = link;
				}
				return true;
			}
		}
		procGetList();
	}
	return false;
}

U32 trackProcessByExename(int container_id,int cookie,LaunchType launch_type, const char *command_line, void *link)
{
	char *exe_name, *s;
	char name[MAX_PATH];
	int		i;

	Strncpyt(name, command_line);

	exe_name = strrchr(name,'\\');
	if (!exe_name++)
		exe_name = name;
	if (s = strrchr(exe_name, '.'))
		*s = 0;

	for(i=0;i<process_list.count;i++)
	{
		if (stricmp(process_list.processes[i].exename, exe_name)==0)
		{
			process_list.processes[i].container_id = container_id;
			process_list.processes[i].cookie = cookie;
			process_list.processes[i].launch_type = launch_type;
			process_list.processes[i].container_type = launch_type_to_container_type(launch_type);
			// This is currently only ever called from the startup handler.  However I'm putting this in here in case it ever becomes necessary to
			// call here from the register handler.
			if (link != NULL)
			{
				process_list.processes[i].link = link;
			}
			return process_list.processes[i].process_id;
		}
	}
	return 0;
}
