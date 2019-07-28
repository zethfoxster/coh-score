#ifndef AISTRUCT_H
#define AISTRUCT_H

typedef struct AIBehavior			AIBehavior;
typedef struct AIBehaviorMod		AIBehaviorMod;
typedef struct AIBehaviorTeamData	AIBehaviorTeamData;
typedef struct Entity				Entity;
typedef struct StashTableImp		StashTableImp;

typedef struct AITeamBase
{
	AIBehaviorTeamData** behaviorData;
	int behaviorTeamDataAllocated;
}AITeamBase;

typedef struct AIVarsBase
{
	struct {
		U32 doProximity							: 1;
		U32 doBScript							: 1;
	}flags;

	Entity** proxEnts;

	AIBehavior** behaviors;
	AIBehavior* curBehavior;
	int behaviorCombatOverride;

	AIBehaviorMod** behaviorMods;
	AIBehaviorMod** behaviorPrevMods;

	int behaviorStructsAllocated;
	int behaviorDataAllocated;
	int behaviorFlagsAllocated;

	StashTableImp* messages;

	U32 isNPC								: 1;
	U32 behaviorRequestOffTickProcessing	: 1;
	U32 overriddenCombat					: 1;
}AIVarsBase;


#endif AISTRUCT_H