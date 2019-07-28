
#include "overloadProtection.h"

#include "log.h"
#include "SuperAssert.h"

// @todo move this enum to a new overloadprotection_common.h shared with ServerMonitor code
enum
{
	// Update code/CoH/ServerMonitor/serverMonitorOverload.c, smoverloadFormat() with any changes to these flags
	OVERLOADPROTECTIONFLAG_MANUAL_OVERRIDE		= 1 << 0,
	OVERLOADPROTECTIONFLAG_LAUNCHERS_FULL		= 1 << 1,
	OVERLOADPROTECTIONFLAG_MONITOR_OVERRIDE		= 1 << 2,
	OVERLOADPROTECTIONFLAG_SQL_QUEUE			= 1 << 3,
};

typedef enum
{
	OVERLOADPROTECTIONFLAG_NOCHANGE,
	OVERLOADPROTECTIONFLAG_ENABLED,
	OVERLOADPROTECTIONFLAG_DISABLED,
} OVERLOADPROTECTIONFLAG_STATUS;

static U32 s_OverloadProtection_EngageFlags = 0;

// Set via overloadProtection_SetLauncherWaterMarks
static F32 s_OverloadProtection_LauncherHighWaterMarkPercent = 0.0f;
static F32 s_OverloadProtection_LauncherLowWaterMarkPercent = 0.0f;

// Set via overloadProtection_SetSQLQueueWaterMarks
static int s_OverloadProtection_SQLQueueLowWaterMark = 0;	// disabled
static int s_OverloadProtection_SQLQueueHighWaterMark = 0;	// disabled

// ---------------------------------------------------------------------------
// Function to get the current state of overload protection
// This is sent to the server/shard monitor
int  overloadProtection_GetStatus(void)
{
	return s_OverloadProtection_EngageFlags;
}

// ---------------------------------------------------------------------------
// is any overload condition is in effect?
// used to queue incoming players
bool overloadProtection_IsEnabled(void)
{
	return (s_OverloadProtection_EngageFlags != 0);
}

// ---------------------------------------------------------------------------
// is severity of current overload such that we should prevent map starts?
bool overloadProtection_DoNotStartMaps(void)
{
	int no_map_start_flags = OVERLOADPROTECTIONFLAG_MANUAL_OVERRIDE |
							 OVERLOADPROTECTIONFLAG_MONITOR_OVERRIDE |
							 OVERLOADPROTECTIONFLAG_LAUNCHERS_FULL;

	return ((s_OverloadProtection_EngageFlags&no_map_start_flags) != 0);

}

// ---------------------------------------------------------------------------
// Utility function to set or clear a bit.
static OVERLOADPROTECTIONFLAG_STATUS setBit(U32 bit, bool engage, bool disengage)
{
	// If both engage and disengage are true, then ignore the disengage.
	if (engage && disengage)
		disengage = false;

	if (!(s_OverloadProtection_EngageFlags & bit) && engage)
	{
		s_OverloadProtection_EngageFlags |= bit;
		return OVERLOADPROTECTIONFLAG_ENABLED;
	}
	else if ((s_OverloadProtection_EngageFlags & bit) && disengage)
	{
		s_OverloadProtection_EngageFlags &= ~bit;
		return OVERLOADPROTECTIONFLAG_DISABLED;
	}

	return OVERLOADPROTECTIONFLAG_NOCHANGE;
}

// ---------------------------------------------------------------------------
// Function to set the current state of the manual override
void overloadProtection_ManualOverride(bool engage)
{
	OVERLOADPROTECTIONFLAG_STATUS status = setBit(OVERLOADPROTECTIONFLAG_MANUAL_OVERRIDE, engage, !engage);

	if (status == OVERLOADPROTECTIONFLAG_ENABLED)
	{
		LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_ALERT, LOG_CONSOLE, "Overload Protection manual override engaged (servers.cfg)");
	}
	else if (status == OVERLOADPROTECTIONFLAG_DISABLED)
	{
		LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_ALERT, LOG_CONSOLE, "Overload Protection manual override cleared (servers.cfg)");
	}
}

// ---------------------------------------------------------------------------
// Function to set the current state of the monitor override
void overloadProtection_MonitorOverride(char *message)
{
	bool engage = (atoi(message) > 0);

	OVERLOADPROTECTIONFLAG_STATUS status = setBit(OVERLOADPROTECTIONFLAG_MONITOR_OVERRIDE, engage, !engage);

	if (status == OVERLOADPROTECTIONFLAG_ENABLED)
	{
		LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_ALERT, LOG_CONSOLE, "Overload Protection monitor override engaged");
	}
	else if (status == OVERLOADPROTECTIONFLAG_DISABLED)
	{
		LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_ALERT, LOG_CONSOLE, "Overload Protection monitor override cleared");
	}
}

// ---------------------------------------------------------------------------
// Set configuration values for launcher availability low/high water marks
void overloadProtection_SetLauncherWaterMarks(F32 lowWaterMarkPercent, F32 highWaterMarkPercent)
{
	s_OverloadProtection_LauncherLowWaterMarkPercent = lowWaterMarkPercent;
	s_OverloadProtection_LauncherHighWaterMarkPercent = highWaterMarkPercent;

	LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_VERBOSE, LOG_CONSOLE, "Overload Protection launcher low water mark = %f, high water mark = %f",
			lowWaterMarkPercent, highWaterMarkPercent);
}

// ---------------------------------------------------------------------------
// Function to set the current state of the launchers
void overloadProtection_SetLauncherStatus(int numAvailableLaunchers, int numImpossibleLaunchers)
{
	F32 percentImpossible;
	int numLaunchers = numAvailableLaunchers + numImpossibleLaunchers;
	OVERLOADPROTECTIONFLAG_STATUS status;

	// Handle this here to avoid divide by zero
	if (numLaunchers < 1)
	{
		s_OverloadProtection_EngageFlags &= ~OVERLOADPROTECTIONFLAG_LAUNCHERS_FULL;
		return;
	}

	percentImpossible = (100.0f * numImpossibleLaunchers) / (1.0f * numLaunchers);

	// Engage at 100%, disengage when it falls to 90%
	status = setBit(OVERLOADPROTECTIONFLAG_LAUNCHERS_FULL, percentImpossible >= s_OverloadProtection_LauncherHighWaterMarkPercent,
		   												   percentImpossible <= s_OverloadProtection_LauncherLowWaterMarkPercent);

	if (status == OVERLOADPROTECTIONFLAG_ENABLED)
	{
		LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_ALERT, LOG_CONSOLE, "Automatic Overload Protection ENGAGED: %d of %d launchers are at capacity or suspended (%6.2f%%) which is at or above the high water mark of %6.2f%%.\n", numImpossibleLaunchers, numLaunchers, percentImpossible, s_OverloadProtection_LauncherHighWaterMarkPercent );
	}
	else if (status == OVERLOADPROTECTIONFLAG_DISABLED)
	{
		LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_ALERT, LOG_CONSOLE, "Automatic Overload Protection disengaged: %d of %d launchers are at capacity or suspended (%6.2f%%) which is at or below the low mark of %6.2f%%.\n", numImpossibleLaunchers, numLaunchers, percentImpossible, s_OverloadProtection_LauncherLowWaterMarkPercent );
	}
}

// ---------------------------------------------------------------------------
// Set configuration values for SQL load low/high water marks
void overloadProtection_SetSQLQueueWaterMarks(int lowWaterMark, int highWaterMark)
{
	if (lowWaterMark >= 0 && highWaterMark>=lowWaterMark)
	{
		s_OverloadProtection_SQLQueueLowWaterMark = lowWaterMark;
		s_OverloadProtection_SQLQueueHighWaterMark = highWaterMark;

		LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_VERBOSE, LOG_CONSOLE, "Overload Protection SQL queue low water mark = %d, high water mark = %d",
			lowWaterMark, highWaterMark);
	}
	else
	{
		// invalid settings just leave what we had
		LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_IMPORTANT, LOG_CONSOLE, "Error: SQL queue low water mark (%d) must be positive and <= high water mark (%d)",
			lowWaterMark, highWaterMark);
	}
}

// ---------------------------------------------------------------------------
// Function to set the current state of dbserver SQL load 
// sql_queue_count comes from sqlFifoWorstQueueDepth()
// which is currently called every 10 seconds. So it is the peak over that
// 10s interval.
void overloadProtection_SetSQLStatus(int sql_queue_count)
{
	bool can_engage = s_OverloadProtection_SQLQueueHighWaterMark > 0;
	bool engage = (sql_queue_count >= s_OverloadProtection_SQLQueueHighWaterMark) && can_engage;
	bool disengage = (sql_queue_count <= s_OverloadProtection_SQLQueueLowWaterMark) || !can_engage;

	OVERLOADPROTECTIONFLAG_STATUS status = setBit(OVERLOADPROTECTIONFLAG_SQL_QUEUE, engage, disengage);

	if (status == OVERLOADPROTECTIONFLAG_ENABLED)
	{
		LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_ALERT, LOG_CONSOLE, "Automatic Overload Protection ENGAGED: SQL queue size is at or above high water mark: %d >= %d)", sql_queue_count, s_OverloadProtection_SQLQueueHighWaterMark );
	}
	else if (status == OVERLOADPROTECTIONFLAG_DISABLED)
	{
		LOG(LOG_OVERLOAD_PROTECTION, LOG_LEVEL_ALERT, LOG_CONSOLE, "Automatic Overload Protection disabled: SQL queue size is at or below low water mark: %d <= %d)", sql_queue_count, s_OverloadProtection_SQLQueueLowWaterMark );
	}
}

