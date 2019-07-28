/*\
 *
 *	scripthookecnounter.h - Copyright 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Provides all the functions CoH script files should be referencing -
 *		essentially a list of hooks into game systems.
 *
 *	This file contains all functions related to encounters
 *
 */

#ifndef SCRIPTHOOKENCOUNTER_H
#define SCRIPTHOOKENCOUNTER_H


#define ENCOUNTER_KEYVILLAIN	30  // generally, the central villain is here
#define ENCOUNTER_KEYVICTIM		40  // generally, the key victim is here

//For FindEncounter (Possibly violating the spirit of the script separtaion, but copying these flags is a pain)
typedef enum SEFindEGFlags
{
	SEF_UNOCCUPIED_ONLY				= (1 << 0),		// Available for a new encounter spawn
	SEF_OCCUPIED_ONLY				= (1 << 1),		// Has current occupants
	SEF_SORTED_BY_NEARBY_PLAYERS	= (1 << 2),		// Fewest first
	SEF_SORTED_BY_DISTANCE			= (1 << 3),		// Nearest first
	SEF_HAS_PATH_TO_LOC				= (1 << 4),		// Path exists from group's position to the Location parameter
	SEF_HAS_FLIGHT_PATH_TO_LOC		= (1 << 5),		// Path exists for fliers
	SEF_FULL_LIST					= (1 << 6),		// Return all encounter groups that fulfill requirements (default returns only the best fit.)
	SEF_GENERIC_ONLY				= (1 << 7),		// Return only encounter groups that are generic mission groups
} SEFindEGFlags;

// Encounters
LOCATION Spawn(int num, TEAM teamName, STRING spawndef, ENCOUNTERGROUP encounter, STRING layout, int level, int size); // level and size are optional
void DetachSpawn(TEAM team); // releases entities from spawn point
void AttachSpawn(ENCOUNTERGROUP encounter, TEAM team); // attaches entity to running encounter
STRING GetSpawnLayout(STRING spawnfile);

ENCOUNTERGROUP MyEncounter();		// returns encounter this script is attached to (SCRIPT_ENCOUNTER only)
ENCOUNTERGROUP FindEncounterGroup(AREA area, STRING layout, int inactiveonly, int deserted); // randomly picks a matching encounter group
ENCOUNTERGROUP FindEncounter( LOCATION loc, FRACTION innerRadius, FRACTION outerRadius, AREA area, STRING missionEncounterName, STRING layout, STRING groupName, SEFindEGFlags flags ); // randomly picks a matching encounter group
ENCOUNTERGROUP FindEncounterGroupByRadius(LOCATION location, int innerRadius, int outerRadius, STRING layout, int inactiveonly, int deserted);
ENCOUNTERGROUP CloneEncounter( ENCOUNTERGROUP group );

void CloseMyEncounterAndEndScript(void);
void CloseEncounter(ENCOUNTERGROUP encounter);	// closes given encounter group (free'ing any entities still attached)
void ReserveEncounter(ENCOUNTERGROUP encounter); // this encounter will be reserved for my use until script ends
ENCOUNTERSTATE EncounterState(ENCOUNTERGROUP encounter);	// what state this encounter is in
void SetEncounterComplete(ENCOUNTERGROUP encounter, NUMBER success);

ENTITY ActorFromEncounterPos(ENCOUNTERGROUP encounter, int position);
LOCATION EncounterPos(ENCOUNTERGROUP encounter, int position);	// get location of actor position
int EncounterLevel(ENCOUNTERGROUP encounter);					// if active, gives level encounter was set up at
int EncounterTeamSize(ENCOUNTERGROUP encounter);				// if active, gives team size encounter was set up at
int EncounterSize(ENCOUNTERGROUP encounter);					// if active, gives the current count of villains in this encounter
ENTITY ActorFromEncounterActorName(ENCOUNTERGROUP encounter, STRING actorName );
void RegenerateEncounter( ENCOUNTERGROUP encounter );

void SetDisableMapSpawns(NUMBER flag);
void StopReservingAllEncounters(void);

#define ENCOUNTER_ITERATOR_MAX 1000

typedef struct ENCOUNTERGROUPITERATOR
{
	NUMBER index;
	NUMBER count;
	NUMBER handles[ENCOUNTER_ITERATOR_MAX];
} ENCOUNTERGROUPITERATOR;

void EncounterGroupsInitIterator(
						ENCOUNTERGROUPITERATOR *iter,
						LOCATION loc,			//Center point to start search from
						FRACTION innerRadius,		//No nearer than this
						FRACTION outerRadius,		//No farther than this
						STRING area,			//Inside this area
						STRING layout,			//Layout required in the EncounterGroup
						STRING groupname		//Name of the encounter group ( Property GroupName )
						);

ENCOUNTERGROUP EncounterGroupsFindNext(ENCOUNTERGROUPITERATOR *iter);

#endif