/*\
 *
 *	scriptconstants.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Constants that have to be shared with other systems in the game.
 *
 */

#ifndef SCRIPTCONSTANTS_H
#define SCRIPTCONSTANTS_H

// encounters
typedef enum ENCOUNTERSTATE
{
	ENCOUNTER_ASLEEP,		// not near any players, not polled
	ENCOUNTER_AWAKE,		// players getting close, polled for EP_WORKING change
	ENCOUNTER_WORKING,		// creatures spawned
	ENCOUNTER_INACTIVE,		// someone has heard encounter
	ENCOUNTER_ACTIVE,		// creatures have been alerted
	ENCOUNTER_COMPLETED,	// we are logically completed, still have NPC's with stuff to do
	ENCOUNTER_OFF,			// went off, either didn't spawn anything, or I'm closed after completion
} ENCOUNTERSTATE;

// AI
typedef enum PRIORITY {
	UNSET_PRIORITY,
	LOW_PRIORITY,
	MEDIUM_PRIORITY,
	HIGH_PRIORITY,
} PRIORITY;

typedef enum DayNightCycleEnum {
	DAYNIGHT_NORMAL,
	DAYNIGHT_ALWAYSNIGHT,
	DAYNIGHT_ALWAYSDAY,
	DAYNIGHT_FAST,
} DayNightCycleEnum;

#endif // SCRIPTCONSTANTS_H