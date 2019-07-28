#ifndef AIBEHAVIORDEBUG_H
#define AIBEHAVIORDEBUG_H

typedef struct AIBehavior		AIBehavior;
typedef struct AIBehaviorMod	AIBehaviorMod;
typedef struct AIBParsedString	AIBParsedString;
typedef struct AIVarsBase		AIVarsBase;
typedef struct Entity			Entity;

// debug functions
char* aiBehaviorDebugPrint(Entity* e, AIVarsBase* aibase, int stripColors);
char* aiBehaviorGetDebugString(Entity* e, AIVarsBase* aibase, AIBehavior* behavior, char* indent);
int aiBehaviorGetDataChunkSize(void);
int aiBehaviorGetTeamDataChunkSize(void);
char* aiBehaviorPrintMod(AIBehaviorMod* mod);
char* aiBehaviorDebugPrintParse(AIBParsedString* parsed);
// table is aliases and anim lists mixed together
int aiBehaviorDebugGetTableCount(void);
const char* aiBehaviorDebugGetTableEntry(int idx);
int aiBehaviorDebugGetAliasCount(void);
const char* aiBehaviorDebugGetAliasEntry(int idx);
char* aiBehaviorDebugPrintPrevMods(Entity* e, AIVarsBase* ai);
char* aiBehaviorDebugPrintPendingMods(Entity* e, AIVarsBase* ai);

#endif