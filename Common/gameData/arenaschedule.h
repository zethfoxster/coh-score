/*\
 *
 *	arenaschedule.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Scheduled events started by the arena server
 *
 */

#ifndef ARENASCHEDULE_H
#define ARENASCHEDULE_H

#include "arenastruct.h"

typedef enum EventRecurrence {
	EVENTRECUR_ASAP,
	EVENTRECUR_HOURLY,
	EVENTRECUR_DAILY,
	EVENTRECUR_WEEKLY,
	EVENTRECUR_MONTHLY,
	EVENTRECUR_NEVER,
} EventRecurrence;

typedef struct ScheduledEvent {
	char* description;
	ArenaWeightClass weightClass;
	ArenaTeamStyle teamStyle;
	ArenaTeamType teamType;
	ArenaVictoryType victoryType;
	int victoryValue;
	ArenaTournamentType tournamentType;
	int entryfee;
	int minplayers;
	int maxplayers;
	bool deprecated;
	bool demandplay;						// players can force start by hitting maxplayers

	EventRecurrence recurs;
	int year, month, day, dayofweek, hour;

	U32 scheduled : 1;					// mark during processing
	U32 expired : 1;					// to stop processing non-recurring events
} ScheduledEvent;

void AddScheduledEvent(int handle);
void LoadScheduledEvents(void);
void ScheduledEventsTick(void);
int IncrementScheduledEvent(ArenaEvent* event); // returns 0 if event should be deleted

extern int g_disableScheduledEvents;	// if set, starting new scheduled events is disabled (debugging)

#endif // ARENASCHEDULE_H