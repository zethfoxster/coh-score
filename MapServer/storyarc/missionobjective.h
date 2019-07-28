/*\
 *
 *	missionobjective.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	- handles creating, interacting with, destroying mission items
 *	- functions for handling objective info structs - objective infos
 *	  are placed on mission items and hostage ents that are objectives
 *
 */
 
#ifndef __MISSIONOBJECTIVE_H
#define __MISSIONOBJECTIVE_H

#include "storyarcinterface.h"

typedef struct ScriptClosure ScriptClosure;
typedef ScriptClosure** ScriptClosureList;

// *********************************************************************************
//  Mission Objective Infos - put on anything that is logically an objective
// *********************************************************************************

#define OBJECTIVE_COOLDOWN_PERIOD	5	// seconds of slack between two simultaneous objectives

typedef enum
{
	MOS_NONE,			// invalid state.
	MOS_WAITING_INITIALIZATION,
	MOS_INITIALIZED,
	MOS_REQUIRES,		// Initalized, but has a requires so not glowie
	MOS_INTERACTING,
	MOS_COOLDOWN,		// only for simultaneous objectives
	MOS_SUCCEEDED,		// ONLY use this & MOS_FAILED for animation, look at bits for detailed info
	MOS_FAILED,
	MOS_REMOVE,			// used by script objectives that should be removed after completion
} MissionObjectiveState;

typedef struct MissionObjectiveInfo
{
	char*					name;
	char*					description;
	char*					singulardescription;
	const MissionObjective*	def;
	MissionObjectiveState	state;
	unsigned int			succeeded:1;	// success & failure bits can both be set for encounters
	unsigned int			failed:1;	
	unsigned int			script:1;		// created by script
	unsigned int			reusable:1;		// can this object be reused?
	unsigned int			optional:1;		// Is this a side mission(not required for mission complete)?
	unsigned int			zowie:1;		// Is this a zowie?
	unsigned int			missionWaypointID;	// ID of the mission waypoint that was sent to the server(0 if no waypoint)

	// Mission Item objective related fields.
	EntityRef				objectiveEntity;		// Which entity am I in the world?

	EntityRef				interactPlayerRef;	// Who is interacting with me?
	int						interactPlayerDBID;

	EntityRef				forceFieldEntity;
	U32						forceFieldTimer;		// dbseconds when force field should respawn

	__int64					lastInteractCheckTime;
	float					interactTimer;			// How long has the player been interacting with me?

	ScriptClosureList		onCompleteHandler;		// script callbacks for after completing the objective

	// Mission hostage encounter related fields.
	EncounterGroup*			encounter;
} MissionObjectiveInfo;

extern MissionObjectiveInfo** objectiveInfoList;		// All known mission objective infos
extern int g_playersleftmap;							// signal that all players left map after it started
extern int g_bSendItemLocations;
#define OVERRIDE_STATUS_LEN 200
extern char g_overridestatus[OVERRIDE_STATUS_LEN];		// will override the normal objective string if set

MissionObjectiveInfo* MissionObjectiveInfoCreate();
void MissionObjectiveInfoDestroy(MissionObjectiveInfo* info);
int MissionObjectiveFindIndex(const MissionDef* mission, const MissionObjective* objective);
const MissionObjective* MissionObjectiveFromIndex(const MissionDef* mission, int index);
void MissionObjectiveComplete(Entity* player, MissionObjectiveInfo* info, int success); // when any info completes
void MissionObjectiveAttachEncounter(const MissionObjective* objective, EncounterGroup* group, const SpawnDef* spawndef); // add an info to the encounter group
int MissionObjectiveIsEnabled(const MissionObjective* objective);
int MissionObjectiveIsSideMission(const MissionObjective* objective);
void MissionObjectiveResetSideMissions(void);
void RecheckActivateRequires();
void MissionObjectiveCheckThreshold();

// these functions work on every objective with the same logical name at the same time
int EveryObjectiveInState(const char* objectivename, MissionObjectiveState state);
int NumObjectivesInState(const char* objectivename, MissionObjectiveState state);
void MissionItemStopEachInteraction(const char* objectivename);
MissionObjectiveInfo *MissionObjectiveCreate(const char* objectiveName, const char* state, const char* singularState);
void MissionObjectiveCompleteEach(const char* objectivename, int CreateIfNotFound, int success);
void MissionObjectiveSetTextEach(const char* objectivename, const char *description, const char *singularDescription);
void MissionItemGiveEachCompleteString(const char* objectivename);

void MissionObjectiveSendLocations( Entity *player );

// *********************************************************************************
//  Mission Items
// *********************************************************************************

#define MISSION_ITEM_FX "GENERIC/MissionObjective.fx"

void MissionItemProcessInteraction();
void MissionDestroyAllObjectives();

void MissionItemSetState(MissionObjectiveInfo* info, MissionObjectiveState newstate); // sets state, changes effects
void MissionItemSetVarsTable(ScriptVarsTable* vars);

// *********************************************************************************
//  Mission Key Clues
// *********************************************************************************

// Key clues are mission-scope clues.  They have the same lifetime as objectives, so they 
// work as you would expect a key on a map to work.  If the mission gets reset, they 
// disappear.
//
// Key clues are kept in a global table for the mission, like objectives.  They are also
// sent to the teamup, so any team connected to the mission will get the current state of 
// the key clues.

extern U32 g_keyclues; // global bitfield of key clues

void KeyDoorNewPlayer(Entity* player);
int MissionAddRemoveKeyClue(Entity* player, const char* cluename, int add); // returns 1 if clue was a key clue

void TutorialUsedInspirationHook(void);

#endif //__MISSIONOBJECTIVE_H