// HolidayEvent.h
//
// Shared values between the base script running in city zones and the scripts running in the winter event zones

#include "timing.h"					// for timerSecondsSince2000()

// Map Data Token name for the current target zone index
#define		ZONE_TARGET_TOKEN			"HolidayEventZoneTarget"
#define		ZONE_ENTRYTIME_TOKEN		"HolidayEventZoneEntryTime"
#define		HEARTBEAT_TOKEN				"HolidayEventBaseHeartbeat"
#define		PLAYER_COOKIE_TOKEN			"HolidayEventPlayerCookie"

// Reward tioken used to gate how long a player can stay in a zone
#define		PLAYER_TICKET_TOKEN			"Holiday_Event_Entry"

// Number of unique zones
#define		WINTER_EVENT_ZONECOUNT		5

// Temporal offset between adjacent zones in minutes.  Adjust this to change the rate at which things happen
#define		WINTER_EVENT_OFFSET			4

// Cycle duration is the time in minutes for the entire run
#define		WINTER_EVENT_CYCLE_DURATION	(WINTER_EVENT_ZONECOUNT * WINTER_EVENT_OFFSET)

#define		SECONDS_PER_MINUTE			60