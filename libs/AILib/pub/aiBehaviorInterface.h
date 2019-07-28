#ifndef AIBEHAVIORINTERFACE_H
#define AIBEHAVIORINTERFACE_H

#include "aiStruct.h"

typedef struct AIBehavior				AIBehavior;
typedef struct AIBehaviorFlag			AIBehaviorFlag;
typedef struct AIBCondition				AIBCondition;
typedef struct AIBParsedString			AIBParsedString;
typedef struct AIBTableEntry			AIBTableEntry;
typedef struct Entity					Entity;
typedef struct PerformanceInfo			PerformanceInfo;
typedef struct StaticDefineInt			StaticDefineInt;
typedef struct StructFunctionCall		StructFunctionCall;
#define TokenizerFunctionCall StructFunctionCall

// BehaviorInfo special flags
#define AIB_NPC_ALLOWED				1 << 0

typedef struct AIBehaviorInfo
{
	char* name;
	void (*startFunc)(Entity* e, AIVarsBase* aibase, AIBehavior* behavior);
	void (*finishFunc)(Entity* e, AIVarsBase* aibase, AIBehavior* behavior);
	void (*setFunc)(Entity* e, AIVarsBase* aibase, AIBehavior* behavior);
	void (*unsetFunc)(Entity* e, AIVarsBase* aibase, AIBehavior* behavior);
	void (*runFunc)(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, AIBehavior* behavior);
	void (*offTickFunc)(Entity* e, AIVarsBase* aibase, AIBehavior* behavior);
	int (*flagFunc)(Entity* e, AIVarsBase* aibase, AIBehavior* behavior, AIBehaviorFlag* flag);
	int (*stringFunc)(Entity* e, AIVarsBase* aibase, AIBehavior* behavior, TokenizerFunctionCall* param);
	void (*debugFunc)(Entity* e, AIVarsBase* aibase, AIBehavior* behavior, char** estr, char* indent);
	U32 structSize;
	U32 teamDataStructSize;
	U32 specialFlags;

	PerformanceInfo* timer;
}AIBehaviorInfo;

typedef struct AIBehavior
{
	AIBehaviorInfo* info;
	void* data;
	AIBehaviorFlag** flags;

	AIBCondition** endConditions;

	U32 teamDataID;
	U32 lastRunTime;
	U32 timeRun;
	U32 id;

	F32 timerTime;

	const char* originalString;

	U32 activated		: 1;
	U32 running			: 1;
	U32 finished		: 1;
	U32 dirty			: 1;
	U32 overrideExempt	: 1;
	U32 uninterruptable	: 1;
}AIBehavior;

typedef enum FlagDataType
{
	FLAG_DATA_ALLOCSTR,
	FLAG_DATA_BOOL,
	FLAG_DATA_ENTREF,
	FLAG_DATA_ESTR,
	FLAG_DATA_FLOAT,
	FLAG_DATA_MULT_STR,
	FLAG_DATA_INT,
	FLAG_DATA_INT3,
	FLAG_DATA_PARSE,
	FLAG_DATA_PERCENT,
	FLAG_DATA_SPECIAL,
	FLAG_DATA_VEC3,
}FlagDataType;

typedef struct AIBehaviorFlagInfo
{
	int type;	// this should be an enum in each client that lets it easily process the flags
	// but the AILib doesn't need to know about the enum

	char* name;
	FlagDataType datatype;
	StaticDefineInt* parseTable;
	U32 allowDuplicates : 1;
}AIBehaviorFlagInfo;

typedef struct AIBehaviorFlag
{
	AIBehaviorFlagInfo* info;

	union
	{
		U32 intarray[3];
		F32 floatarray[3];
		char* strptr;
		const char* allocstr;
		char** strarray;
		U64 entref;
	};
}AIBehaviorFlag;

typedef struct AIBParsedString
{
	TokenizerFunctionCall** string;
}AIBParsedString;

typedef struct AIBehaviorAliasInfo
{
	char* name;
	char* resolveStr;
	AIBParsedString* parsedStr;
}AIBehaviorAliasInfo;

typedef enum FlagReturnVal
{
	FLAG_NOT_PROCESSED,
	FLAG_PROCESSED,
	FLAG_PROCESSED_DONT_REMOVE,
	FLAG_MOVE_TO_END,
}FlagReturnVal;

typedef enum AIBehaviorCombatOverride
{
	AIB_COMBAT_OVERRIDE_AGGRESSIVE = 0,	// this order is counterintuitive but matches with the AIScript priorities
	AIB_COMBAT_OVERRIDE_COLLECTIVE,		// this was inserted here, disregarding the above comment... I can't figure out why this enum needs to match anything else, it appears to be completely independent
	AIB_COMBAT_OVERRIDE_DEFENSIVE,
	AIB_COMBAT_OVERRIDE_PASSIVE,
}AIBehaviorCombatOverride;

//////////////////////////////////////////////////////////////////////////
// Functions needed for client AI internals
//////////////////////////////////////////////////////////////////////////

AIVarsBase* aiSystemGetBase();
void aiSystemDestroyBase(AIVarsBase* base);
AITeamBase* aiSystemGetTeamData();
void aiSystemDestroyTeamData(AITeamBase* base);


// Runs one tick of the behavior system for the given entity
void aiBehaviorProcess(Entity* e, AIVarsBase* aibase, AIBehavior*** behavior);

// Runs the offtick function for this entity if the current behavior has requested this. Normally the AI runs at 1/15th
// the speed of the normal game, offtick functions will get called the ticks when think doesn't.
void aiBehaviorProcessOffTick(Entity* e, AIVarsBase* ai);

typedef enum AIBEntryType
{
	AIB_BEHAVIOR,
	AIB_FLAG,
	AIB_ALIAS,
	AIB_CUSTOM,
}AIBEntryType;

typedef void (*aiBehaviorReloadCallback)(void);
typedef void (*aiBehaviorFlagReloadCallback)(void);
typedef void (*aiBehaviorAliasReloadCallback)(void);
typedef void (*aiBehaviorTableCustomReloadCallback)(void);

typedef void (*aiBehaviorCustomTableEntryCallback)(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors,
	AIBTableEntry* entry, const char* origStr, const char* processedStr, TokenizerFunctionCall* param);

typedef char* (*aiBehaviorDefaultBehaviorCallback)(Entity* e, AIVarsBase* aibase);
typedef void (*aiBehaviorStringPreprocessCallback)(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, 
												   TokenizerFunctionCall* string, char** resultStr, int* addSelfAsString, int* continueProcessing);
typedef const char* (*aiBehaviorBlameFileCallback)(Entity* e);
typedef int (*aiBehaviorSpecialFlagProcessingCallback)(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, 
													   AIBehavior* behavior, AIBehaviorFlag* flag, TokenizerFunctionCall* param, char** estr);
typedef void (*aiBehaviorFlagProcessCallback)(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, AIBehavior* behavior);

typedef void (*aiBehaviorReinitCallback)(Entity* e, AIVarsBase* aibase);

typedef bool (*aiBehaviorCheckCombatOverrideCallback)(Entity* e, AIVarsBase* aibase,
													  AIBehavior*** behaviors, AIBehavior* behavior, int override);

typedef U32 (*aiBehaviorGetABS_TIMECallback)(void);

typedef int (*aiBehaviorConditionValueCallback)(Entity* e, AIVarsBase* aibase, int condtype);
typedef U32 (*aiBehaviorConditionParseCallback)(const char* condstring);

extern bool g_TestingBehaviorAliases;

void aiBehaviorSetBehaviorReloadCallback(aiBehaviorReloadCallback callback);
void aiBehaviorSetBehaviorFlagReloadCallback(aiBehaviorFlagReloadCallback callback);
void aiBehaviorSetBehaviorAliasReloadCallback(aiBehaviorAliasReloadCallback callback);
void aiBehaviorSetBehaviorTableCustomReloadCallback(aiBehaviorTableCustomReloadCallback callback);

void aiBehaviorSetBehaviorCustomTableEntryCallback(aiBehaviorCustomTableEntryCallback callback);
void aiBehaviorSetDefaultBehaviorCallback(aiBehaviorDefaultBehaviorCallback callback);
void aiBehaviorSetSpecialFlagProcessingCallback(aiBehaviorSpecialFlagProcessingCallback callback);
void aiBehaviorSetFlagProcessCallback(aiBehaviorFlagProcessCallback callback);
void aiBehaviorSetStringPreprocessCallback(aiBehaviorStringPreprocessCallback callback);
void aiBehaviorSetBlameFileCallback(aiBehaviorBlameFileCallback callback);

void aiBehaviorSetCheckCombatOverrideCallback(aiBehaviorCheckCombatOverrideCallback callback);
void aiBehaviorSetReinitCallback(aiBehaviorReinitCallback callback);
void aiBehaviorSetConditionParseCallback(aiBehaviorConditionParseCallback callback);
void aiBehaviorSetConditionValueCallback(aiBehaviorConditionValueCallback callback);

void aiBehaviorSetGetABS_TIMECallback(aiBehaviorGetABS_TIMECallback callback);

AIBehaviorInfo* aiBehaviorInfoFromString(const char* name);
AIBehavior* aiBehaviorGetCurBehaviorInternal(Entity* e, AIVarsBase* aibase);
AIBehavior* aiBehaviorCreateSCOPEVAR(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, int idx);
void aiBehaviorProcessFlag(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, AIBehavior* in_behavior, const char* string);
void aiBehaviorFlagDestroy(Entity* e, AIVarsBase* aibase, AIBehaviorFlag* flag);
void aiBehaviorTableAddBehavior(AIBehaviorInfo* info);
void aiBehaviorTableAddFlag(AIBehaviorFlagInfo* info);
void aiBehaviorTableAddAlias(AIBehaviorAliasInfo* info);
void aiBehaviorTableAddCustom(void* info, int type, const char* name);
void aiBehaviorRebuildLookupTable(void);
void aiBehaviorSortDebugAliasTable(void);
AIBehaviorFlagInfo* aiBehaviorFlagInfoFromString(const char* name);
int aiBehaviorGetHandle(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, AIBehavior* behavior);
AIBehavior* aiBehaviorGetCurBehaviorInternal(Entity* e, AIVarsBase* aibase);

AIBCondition* aiBehaviorParseCondition(TokenizerFunctionCall* parse);
AIBParsedString* aiBehaviorParseString(const char* string);
F32 aiBehaviorParseFloat(TokenizerFunctionCall* param);
int aiBehaviorParseInt(TokenizerFunctionCall* param);
void aiBehaviorParseFlags(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, AIBehavior* behavior, TokenizerFunctionCall** tokens);
void aiBehaviorProcessString(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, const char* origStr, AIBParsedString* parsed, bool uninterruptable);
AIBehavior* aiBehaviorAdd(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, AIBehaviorInfo* info, int insertAt);

int aiBehaviorPropagateStringToChildren(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, AIBehavior* startAfter, TokenizerFunctionCall* param);
void aiBehaviorPropagateFlagToChildren(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, AIBehavior* startAfter, const AIBehaviorFlag* flag);

void aiBehaviorReinitEntity(Entity* e, AIVarsBase* aibase);

// convenience macros
#define ERRORF_DESTROY(string)\
	Errorf("Wrong number of parameters to %s: %d %s",\
	flag->info->name, eaSize(&param->params), string);\
	aiBehaviorFlagDestroy(e, aibase, flag);\
	return false;

#define WRONG_PARAMETERS(num, string)\
	if(eaSize(&param->params) != (num))\
{\
	ERRORF_DESTROY(string)\
}

#define ERRORF_NODESTROY(string)\
	Errorf("Wrong number of parameters to %s: %d %s",\
	flag->info->name, eaSize(&param->params), string);\
	return false;

#define WRONG_PARAMETERS_NODESTROY(num, string)\
	if(eaSize(&param->params) != (num))\
{\
	ERRORF_NODESTROY(string)\
}

#define aiBehaviorPush(e, ai, behaviors, info) aiBehaviorAdd(e, ai, behaviors, info, eaSize(behaviors))

//////////////////////////////////////////////////////////////////////////
// Prototype Macros
//////////////////////////////////////////////////////////////////////////
#define BEHAVIOR_DEBUG_FUNC(name)	void aiBF##name##DebugStr(Entity* e, AIVarsBase* aibase, AIBehavior* behavior, char** estr, char* indent)
#define BEHAVIOR_FINISH_FUNC(name)	void aiBF##name##Finish(Entity* e, AIVarsBase* aibase, AIBehavior* behavior)
#define BEHAVIOR_FLAG_FUNC(name)	int aiBF##name##Flag(Entity* e, AIVarsBase* aibase, AIBehavior* behavior, AIBehaviorFlag* flag)
#define BEHAVIOR_OFFTICK_FUNC(name)	void aiBF##name##OffTick(Entity* e, AIVarsBase* aibase, AIBehavior* behavior)
#define BEHAVIOR_RUN_FUNC(name)		void aiBF##name##Run(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors, AIBehavior* behavior)
#define BEHAVIOR_SET_FUNC(name)		void aiBF##name##Set(Entity* e, AIVarsBase* aibase, AIBehavior* behavior)
#define BEHAVIOR_START_FUNC(name)	void aiBF##name##Start(Entity* e, AIVarsBase* aibase, AIBehavior* behavior)
#define BEHAVIOR_STRING_FUNC(name)	int aiBF##name##String(Entity* e, AIVarsBase* aibase, AIBehavior* behavior, TokenizerFunctionCall* param)
#define BEHAVIOR_UNSET_FUNC(name)	void aiBF##name##Unset(Entity* e, AIVarsBase* aibase, AIBehavior* behavior)

//////////////////////////////////////////////////////////////////////////
// Alias
//////////////////////////////////////////////////////////////////////////
BEHAVIOR_DEBUG_FUNC(Alias);
BEHAVIOR_FINISH_FUNC(Alias);
BEHAVIOR_FLAG_FUNC(Alias);
BEHAVIOR_RUN_FUNC(Alias);
BEHAVIOR_SET_FUNC(Alias);
BEHAVIOR_STRING_FUNC(Alias);

typedef struct AIBDAlias
{
	const char* aliasName;
	AIBehavior** behaviors;
}AIBDAlias;

#endif