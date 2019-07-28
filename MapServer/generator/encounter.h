// encounter.h - random encounter system handling.  spawns villians, etc from
// encounter spawn points

#ifndef __ENCOUNTER_H
#define __ENCOUNTER_H

#include "stdtypes.h"
#include "patrol.h"

typedef struct SpawnDef SpawnDef;
typedef struct EncounterGroup EncounterGroup;
typedef struct EncounterEntityInfo EncounterEntityInfo;
typedef struct Entity Entity;
typedef struct ClientLink ClientLink;
typedef struct ScriptVarsTable ScriptVarsTable;
//typedef struct Volume Volume;

////////////////////////////////////////////////// Encounters
// This is a random encounter system.  It works with spawndefs and
// story arcs.  It allows scripted encounters with random variables
// and actors to occur at preplanned locations on the map.
// 

// processing hooks
void EncounterPreload(void);// single call on startup
void EncounterProcess(void);// main loop hook
void EncounterLoad(void);	// load the per-map info
void EncounterUnload(void);	// debug only way of shutting down encounters

enum EncounterAlerts {
	ENCOUNTER_ALERTED,		// I noticed a player and am being alerted
	ENCOUNTER_THANKHERO,	// I'm an NPC, and I ran up to a player
	ENCOUNTER_NOTALERTED,	// I stopped targetting a player
};


typedef enum eEPFlags
{
	STEALTH_CAMERA_ARC		= (1<<0),
	STEALTH_CAMERA_FROZEN	= (1<<1),
	STEALTH_CAMERA_ROTATE	= (1<<2),
	STEALTH_CAMERA_REVERSEROTATE = (1<<3),
	STEALTH_CAMERA_REVERSEARC = (1<<4)
} eEPFlags;

	// clones an encounter group and adds it to the encounter group list
	//	Allocates a new encounter and adds it to the global list, cleaned when the encounter group
	//	list is freed	
EncounterGroup *EncounterGroupClone(EncounterGroup* group);

void EncounterAISignal(Entity* ent, int what);
	// callbacks from entities to encounter system for specific events

Entity* EncounterWhoSavedMe(Entity* ent); 
	// function for an NPC that wants to know what hero completed the encounter
	// returns NULL on failure

Entity* EncounterWhoInstigated(Entity* actor);
	// find out who instigated the encounter that spawned this actor

Entity *EncounterNearestPlayer(Entity* ent);
Entity *EncounterNearestPlayerByPos(Mat4 pos);

void EncounterGetWanderRange(Entity* ent, Vec3 *pos, int *range);
	// find the wander range for a particular entity, returns values in pos and range.
	// if NOT in an encounter and therefore not restricted, 0 is returned in range

void EncounterAddWhoKilledMyActor(Entity *victim, int victor);

PatrolRoute* EncounterPatrolRoute(Entity* actor); 
	// give back the patrol route attached to this encounter (if any)

void EncounterEntUpdateState(Entity* ent, int deleting);
	// should be updated if team changes, death state changes, or alerted state changes

void EncounterEntAddPet(Entity* parent, Entity* pet);
	// add the pet to the encounter of the parent

void EncounterEntDetach(Entity* ent);
	// when the entity is free'd

void EncounterPlayerCreate(Entity* ent);
	// called when a new player entity is created

int EncounterAllGroupsComplete(void);
	// returns 1 if all encounter groups have been conquered (for a mission map)

void EncounterSpawnMissionNPCs(int count);
	// spawn <count> random NPCs who will just run around the map
	
void EncounterDoZDrop(Vec3 pos_in, Vec3 pos_out);
	// snaps pos_in to the ground, puts that position into pos_out

const SpawnDef* SpawnDefFromFilename(const char* filename);
	// look up a spawndef definition by filename (to spawn an encounter)

const SpawnDef* EncounterGetMyResponsibleSpawnDef(Entity* ent);

int EncounterGetMatchingPoint(EncounterGroup* group, const char* spawntype);
	// find a spawn point in this group that matches the passed spawntype.
	// if none found, return -1.  otherwise, returns index of point within
	// group.children

typedef struct ScriptDef ScriptDef;
U32 StartEncounterScriptFromDef(const ScriptDef *script, EncounterGroup* group);
U32 StartEncounterScriptFromFunction(const char *scriptFunction, EncounterGroup* group);

void EncounterNotifyScriptStopping(EncounterGroup* group, int scriptid);
	// callback for script system

void EncounterNotifyInterestedEncountersThatObjectiveIsComplete( char * objectiveName, Entity * completer );
	//For encounters created or activated by objective success

void EncounterForceReset(ClientLink* client, char* groupname);	
	// a reset command from the console - reset an individual encounter, or all
	// both params are optional

void EncounterSetMinAutostart(ClientLink* client, int min);
void EncounterSetScriptControlsSpawning(int flag);
int EncounterGetScriptControlsSpawning();

int EncounterAllowProcessing(int on);
	// defaults to ON, should only be used for server panic - stops all encounter
	// processing on the server (spawning and cleanup)
	//    returns previous value, if passed -1, no change will be made

int EncounterInMissionMode(void);
void EncounterSetMissionMode(int on);
	// mission mode changes several aspects of how encounters are processed

void DebugPrintEncounterCoverage(ClientLink* client);
	// print a list of what villains and layouts this city zone can produce

//  Nictus hunter adds to spawns
int IsKheldian(Entity* e);

//For scripts to set an encounter active
void setEncounterActive( EncounterGroup * group );
void setEncounterInactive( EncounterGroup * group );

void encounterApplySetsOnReactorBits( Entity * e );

//Respawns any dead actors in the encounter.
void EncounterRespawnActors( EncounterGroup * group );
void EncounterClose(EncounterGroup* group);

int EncounterRespawnActorByLocation(EncounterGroup* group, int actorPos);

void EncounterSpawn(EncounterGroup* group);

void notifyEncounterItsVolumeHasBeenEntered( DefTracker * volumeTracker );

void EncounterOverrideTeamSize(int size);
void EncounterOverrideMobLevel(int level);
void EncounterRespectManualSpawn(int flag);

extern int g_encounterMinAutostartTime;		// in seconds, for debugging

void outputEncounterGroups(); // for debugging

ScriptVarsTable* EncounterVarsFromEntity( Entity * e );

enum
{
	LOCATION_SEND_NONE,
	LOCATION_SEND_NEW,
	LOCATION_SEND_NEW_SENT,
	LOCATION_SENT,
};


#endif __ENCOUNTER_H

