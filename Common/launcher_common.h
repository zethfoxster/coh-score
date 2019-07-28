#ifndef _LAUNCHER_COMMON_H
#define _LAUNCHER_COMMON_H

typedef enum LaunchType
{
	LT_MISSIONMAP,
	LT_STATICMAP,
	LT_SERVERAPP,
	LT_UNKNOWN,
} LaunchType;

typedef enum LaunchSuspensionFlags
{
	kLaunchSuspensionFlag_Capacity		= 1 << 0,	// heuristics say host is over capacity
	kLaunchSuspensionFlag_Trouble		= 1 << 1,	// host is crashing maps or otherwise in trouble
	kLaunchSuspensionFlag_Manual		= 1 << 2,	// host launcher was manually suspended by keyboard command input ('s', after 'd' 'd')
	kLaunchSuspensionFlag_ServerMonitor	= 1 << 3,	// host launcher was suspended through ServerMonitor
} LaunchType;

typedef enum LauncherFlags
{
	kLauncherFlag_Monitor = 1 << 0,
} LauncherFlags;

#endif	// _LAUNCHER_COMMON_H