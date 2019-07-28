
#ifndef ENTAILOG_H
#define ENTAILOG_H

#include "stdtypes.h"
#include "cmdserver.h"	// needed for server_state

typedef struct Entity Entity;

typedef enum {
	AI_LOG_BASIC		= 0,
	AI_LOG_PATH,
	AI_LOG_COMBAT,
	AI_LOG_POWERS,
	AI_LOG_TEAM,
	AI_LOG_WAYPOINT,
	AI_LOG_CAR,
	AI_LOG_THINK,
	AI_LOG_PRIORITY,
	AI_LOG_ENCOUNTER,
	AI_LOG_SEQ,
	AI_LOG_FEELING,
	AI_LOG_SPECIAL, // For special behaviors like DevouringEarth, COT.
	AI_LOG_BEHAVIOR,
	AI_LOG_COUNT,
} AILogFlag;

void aiLog(Entity* e, char* txt, ...);
void aiLogClear(Entity* e);

void aiPointLog(Entity* e, Vec3 pos, int argb, char* txt, ...);
void aiPointLogClear(Entity* e);

const char* aiLogEntName(Entity* e);

extern int aiLogCurFlagBit;

// This is the macro to call for AI logging.
// This prevents a costly function call when AI logging is disabled.
// "params" must be passed in parens because of the variable argument list.
//
// The syntax is: AI_LOG( AI_LOG_FLAGBIT, (e, format, ...) )

//TODO: make this not hang off server_state?

#define AI_LOG_ENABLED(flagBit) (server_state.aiLogFlags & (1 << flagBit))
#define AI_LOG(flagBit, params) {if(AI_LOG_ENABLED(flagBit)){aiLogCurFlagBit = flagBit; aiLog params;}}
#define AI_LOG_ENT_NAME(e) aiLogEntName(e)

// This is the macro to call for AI point logging.
// This prevents a costly function call when AI logging is disabled.
// "params" must be passed in parens because of the variable argument list.
//
// The syntax is: AI_POINTLOG( AI_LOG_FLAGBIT, (e, pos, rgb, format, ...) )

#define AI_POINTLOG_ENABLED(flagBit) (server_state.aiLogFlags & (1 << flagBit))
#define AI_POINTLOG(flagBit, params) {if(AI_POINTLOG_ENABLED(flagBit)){aiLogCurFlagBit = flagBit; aiPointLog params;}}

#endif