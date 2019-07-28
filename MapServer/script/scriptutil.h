/*\
 *
 *	scriptutil.h/c - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Utility functions defined in terms of the interface in script.h.
 *  Defines some ease-of-use macros for state management, utility functions
 *	to iterate over players and do things, etc.
 *
 */

#ifndef SCRIPTUTIL_H
#define SCRIPTUTIL_H

#include "script.h"

// pseudonyms
#define OR ||
#define AND &&
#define NOT !

// State machine macros
#define GET_STATE		VarGet(STATE_VAR)
#define SET_STATE(x)	{ VarSet(STATE_VAR, x); VarSetTrue(STATE_ENTRY); VarSetNumber(STATE_TIME, CurrentTime()); }
#define	INITIAL_STATE	if (VarIsEmpty(STATE_VAR)) { SET_STATE(STATE_INITIAL) } \
						if (VarEqual(STATE_VAR, STATE_INITIAL)) {
#define STATE(x)		} else if (VarEqual(STATE_VAR,x)) {
#define END_STATES		}
#define ON_ENTRY		if (VarIsTrue(STATE_ENTRY)) { VarSetEmpty(STATE_ENTRY);
#define END_ENTRY		}
#define STATE_MINS		( ((FRACTION)CurrentTime() - VarGetFraction(STATE_TIME)) / 60.0)

// Standard signals
#define ON_SIGNAL(x)	if (VarIsTrue(PREFIX_SIGNAL x)) { VarSetEmpty(PREFIX_SIGNAL x); {
#define END_SIGNAL		} }
#define ON_STOPSIGNAL	ON_SIGNAL(SIGNAL_STOP)

// simple utility functions
int NumEntitiesInArea(AREA area, TEAM team);
int EntityExists(ENTITY entity);				// really just resolves to NumEntitiesInTeam

char *Script_GetCurrentBlame();

// Zone objectives ride on the back on InstanceVar's, but are limited to one of three states: unset, success and fail.
typedef enum ZoneObjectiveState
{
	ZONE_OBJECTIVE_UNSET = 0,
	ZONE_OBJECTIVE_SUCCESS = 1,
	ZONE_OBJECTIVE_FAIL = 2,
} ZoneObjectiveState;

#endif // SCRIPTUTIL_H