/*\
 *
 *	missionspec.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 * 	functions to handle the mission.spec file and verify both
 *  maps and mission scripts against the spec.
 *
 */

#include "mission.h"
#include "svr_base.h"
#include "entity.h"
#include "storyarcprivate.h"

#ifndef __MISSIONSPEC_H
#define __MISSIONSPEC_H

#include "storyarcinterface.h"

// *********************************************************************************
//  Mission spec file
// *********************************************************************************

typedef struct EncounterGroupSpec
{
	const char**	layouts;
	int				count;
} EncounterGroupSpec;

typedef struct MissionAreaSpec
{
	const EncounterGroupSpec** encounterGroups;
	int				missionSpawns;
	int				hostages;
	int				wallItems;
	int				floorItems;
	int				roofItems;
} MissionAreaSpec;

typedef struct MissionSpec
{
	const char*		mapFile;		// either a spec for a specific file
	MapSetEnum		mapSet;			//  .. or for a mapset and time
	int				mapSetTime;

	MissionAreaSpec front;		
	MissionAreaSpec	middle;
	MissionAreaSpec back;
	MissionAreaSpec lobby;

	bool disableMissionCheck;	// Skip validation of this map versus mission spawns - used when building and testing missions
	bool varMapSpec;			// This mapspec is being used as a var on a mission
	const char** subMapFiles;	// list of mapFiles which are actually accessible via the var this map is used in
} MissionSpec;

typedef struct
{
	const MissionSpec** missionSpecs;
} MissionSpecFile;

extern SHARED_MEMORY MissionSpecFile missionSpecFile;

void MissionSpecReload(void);
const MissionSpec* MissionSpecFromFilename(const char* mapfile);
const MissionSpec* MissionSpecFromMapSet(MapSetEnum mapset, int maptime);

// *********************************************************************************
//  Verify mission spec
// *********************************************************************************

void VerifyMissionEncounterCoverage(void);
int VerifyAndProcessMissionPlacement(MissionDef* mission, const char* logicalname);
void MissionDebugCheckAllMapStats(ClientLink *client, Entity *player);

#endif //__MISSIONSPEC_H