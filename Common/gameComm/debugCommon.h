/*\
 *
 *	debugCommon.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Just some defines for debug communications
 *
 */

#ifndef DEBUGCOMMON_H
#define DEBUGCOMMON_H

// predefined variable types
#define STATE_VAR		"__State"
#define STATE_ENTRY		"__StateEntry"
#define STATE_TIME		"__StateStarted"
#define STATE_INITIAL	"__Initial"
#define PREFIX_SIGNAL	"__Signal_"
#define PREFIX_TIMER	"__Timer_"
#define SIGNAL_STOP		PREFIX_SIGNAL "STOP"

#define SP_MULTIVALUE			(1 << 0)
#define SP_OPTIONAL				(1 << 1)
#define SP_DONTDISPLAYDEBUG		(1 << 2)

enum {
	SCRIPTDEBUGFLAG_DEBUGON		=	1 << 0,
	SCRIPTDEBUGFLAG_SHOWVARS	=	1 << 1,
};

typedef enum ScriptType 
{
	SCRIPT_MISSION,
	SCRIPT_ENCOUNTER,
	SCRIPT_LOCATION,
	SCRIPT_ZONE,
	SCRIPT_ENTITY,
} ScriptType;

typedef enum ScriptRunState
{
	SCRIPTSTATE_RUNNING,
	SCRIPTSTATE_PAUSED,
} ScriptRunState;

#endif // DEBUGCOMMON_H