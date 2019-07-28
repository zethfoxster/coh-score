#include "stdtypes.h"

typedef enum ServerRoles {
	ROLE_NONE =		0,
	ROLE_ZONE =		1<<0,
	ROLE_MISSION =	1<<1,
	ROLE_MONITOR =	1<<2,
	ROLE_BEACONIZER =	1<<3,
} ServerRoles;

typedef struct RunServer {
	const char *command;
	const char *appname_cfg;
	U32 ip; // IP to run this on, for serverAutoStart
	char appname[32]; // Short name, for serverAutoStart
	U32 last_run_time; // Last time this server was attempted to be started
	U32 last_kill_time; // Last time this server crashed and was attempted to be killed
	U32 last_crash_time; // Last time this server crashed
	U32	track_by_id;
	U32 kill_and_restart;
	U32 monitor;
} RunServer;

typedef struct ServerRoleDescription
{
	U32 iprange[2];
	ServerRoles primaryRole;
	ServerRoles secondaryRole;
	RunServer **runServers;
	S32 requestBeaconServerCount; // How many Request-BeaconServers to run on this machine.
} ServerRoleDescription;

// @todo At the moment these 'strategy' and 'metric' values are combined and they really aren't independent flags
// but it was the easiest path to take to allow named values in the load balancing configuration
// files using structparsers 'flag' handling and also error checking on incorrect settings.
// This could be made less confusing and cleaner at some point.
typedef enum LoadBalanceFlags
{
	// balancing strategy flags
	LOADBALANCE_SEQUENTIAL			= 1 << 0, // balance by round robin assignment to the set of available launchers
	LOADBALANCE_RANDOM				= 1 << 1, // balance by randomly choosing amongst available launchers
	LOADBALANCE_RANDOM_CHOICE		= 1 << 2, // balance by randomly choosing a set of launchers and selecting the one with minimum heuristic
	LOADBALANCE_SEARCH				= 1 << 3, // balance by walking the set of launchers and selecting the one with minimum heuristic
	LOADBALANCE_STRATEGY_MASK		= 0x000F,

	// type of 'heuristic' to use in comparisons
	LOADBALANCE_UTILIZATION			= 1 << 8, // a measure of host machine resource utilization (i.e., cpu, etc)
	LOADBALANCE_TOTAL_OCCUPANCY		= 1 << 9, // total hosted server count
	LOADBALANCE_TYPE_OCCUPANCY		= 1 << 10, // total hosted server count of a given type
	LOADBALANCE_HEURISTIC_MASK		= 0x0F00,
};

typedef struct LoadBalanceConfig {

	U32	balanceModeZone;			// flag which determines strategy to use in load balancing static/zone mapservers
	U32	balanceModeMission;			// flag which determines strategy to use in load balancing mission mapservers

	// factors for determining host utilization metric and per launch load heuristic
	U32 minAvailPhysicalMemory; // in MB: 1000
	F32 minAvailPhysicalMemoryBias; // in CPU 0-1: 0.10
	U32 pagingLoadLow; // hard faults in pages/sec: 100
	U32 pagingLoadHigh; // hard faults in pages/sec: 3000
	U32 startingStaticMemBias; // in MB: 500
	F32 startingStaticCPUBias; // in CPU 0-1: 0.30
	U32 startingMissionMemBias; // in MB: 150
	F32 startingMissionCPUBias; // in CPU 0-1: 0.01
	F32 secondaryRoleBias; // in CPU 0-1: 0.5
	F32 staticCPUBias; // in CPU 0.0-1.0: .00
	U32	usedLauncherBias;		// load bias/penalty on a used launcher to prefer using a different launcher before we receive an update on status of all the launchers (so we don't keep hammering the same launcher)
	U32 delinkLoadBias;			// if there have been delinks do we bias this launcher load (if it hasn't been outright suspended yet)

	// factors that affect suspensions
	U32 maxMapservers; // # machine limit due to exposed ports, desired stability and QoS :200 
	U32 minAvailVirtualMemory; // in MB: 3000
	U32 troubleSuspensionTime; // in seconds: 1800
	U32	maxHostUtilization;		// 0-200+, utilization metric level at which we should suspend launching: 250

	U32 maxMapservers_lowWaterMark;			// low-water mark at which overload protection will disengage
	U32 maxHostUtilization_lowWaterMark;	// low-water mark at which overload protection will disengage
	U32 minAvailVirtualMemory_lowWaterMark;	// low-water mark at which overload protection will disengage

	ServerRoleDescription **serverRoles;
} LoadBalanceConfig;

extern LoadBalanceConfig load_balance_config;



bool loadBalanceConfigLoad(void); // Returns FALSE if configured incorrectly
ServerRoleDescription *getNextServerRole(ServerRoleDescription* curRole, U32 ip);
ServerRoleDescription *getServerRole(U32 ip);
void loadBalanceCheckReload(void);
