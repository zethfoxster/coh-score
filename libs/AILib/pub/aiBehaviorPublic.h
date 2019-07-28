#ifndef AIBEHAVIOR_H
#define AIBEHAVIOR_H

typedef struct AIVarsBase				AIVarsBase;
typedef struct AIBehavior				AIBehavior;
typedef struct AIBehaviorInfo			AIBehaviorInfo;
typedef struct AIBehaviorFlag			AIBehaviorFlag;
typedef struct AIBehaviorMod			AIBehaviorMod;
typedef struct AIBTableEntry			AIBTableEntry;
typedef struct AIPriorityManager		AIPriorityManager;
typedef struct AIScript					AIScript;
typedef struct Entity					Entity;
typedef struct StructFunctionCall		StructFunctionCall;
#define TokenizerFunctionCall StructFunctionCall

// main external behavior functions

// Adds a flag to a specific behavior or the top behavior if handle is 0.
void aiBehaviorAddFlag(Entity* e, AIVarsBase* aibase, int handle, const char* string);

// Adds a perm flag (a flag which will be active no matter what behavior is running.
void aiBehaviorAddPermFlag(Entity* e, AIVarsBase* aibase, const char* string);

// Adds a string to be processed by the behavior system. This is the main way to add behaviors.
void aiBehaviorAddStringEx(Entity* e, AIVarsBase* aibase, const char* string, bool uninterruptable);
void aiBehaviorAddString(Entity* e, AIVarsBase* aibase, const char* string);

// Returns whether the currently running behavior matches the type specified by a string.
int aiBehaviorCurMatchesName(Entity* e, AIVarsBase* aibase, const char* name);

// Destroys all behaviors and does it instantly. Only for entity cleanup.
void aiBehaviorDestroyAll(Entity* e, AIVarsBase* aibase, AIBehavior*** behaviors);

// Destroys all behavior history and the queue of mods that have not yet been applied (usually none). This
// is also for cleanup only.
void aiBehaviorDestroyMods(Entity* e, AIVarsBase* ai);

// Lets the behavior system know that combat has finished. This makes combat overrides work.
void aiBehaviorCombatFinished(Entity* e, AIVarsBase* ai);

// Returns a handle to the current behavior for use with things like AddFlag.
int aiBehaviorGetCurBehavior(Entity* e, AIVarsBase* ai);

// Returns a handle to the top behavior of a certain type.
int aiBehaviorGetTopBehaviorByName(Entity* e, AIVarsBase* aibase, const char* name);

// Marks all behaviors (optionally including the permvar behavior) as finished. This is the normal way of
// clearing an entity's behaviors.
void aiBehaviorMarkAllFinished(Entity* e, AIVarsBase* aibase, int finishPermvar);

// Marks the specified behavior as finished.
void aiBehaviorMarkFinished(Entity* e, AIVarsBase* aibase, int handle);

extern int architectbuffer_choosenOneId;
#endif