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
#include "storyarcprivate.h"
#include "missionspec.h"
#include "dbcomm.h"
#include "missionMapCommon.h"

// *********************************************************************************
//  Mission spec file
// *********************************************************************************
#define MISSION_SPEC_FILENAME "specs/missions.spec"

SHARED_MEMORY MissionSpecFile missionSpecFile = {0};

TokenizerParseInfo ParseEncounterGroupSpec[] =
{
	{	"{",					TOK_START },
	{	"Layouts",				TOK_STRINGARRAY(EncounterGroupSpec, layouts) },
	{	"Number",				TOK_INT(EncounterGroupSpec, count, 1) },
	{	"}",					TOK_END },
	{	"", 0, 0 }
};

TokenizerParseInfo ParseMissionAreaSpec[] =
{
	{	"{",					TOK_START },
	{	"CustomEncounterGroup",	TOK_STRUCT(MissionAreaSpec, encounterGroups, ParseEncounterGroupSpec) },
	{	"MissionEncounters",	TOK_INT(MissionAreaSpec, missionSpawns, 0) },
	{	"Hostages",				TOK_INT(MissionAreaSpec, hostages, 0) },
	{	"ItemWall",				TOK_INT(MissionAreaSpec, wallItems, 0) },
	{	"ItemFloor",			TOK_INT(MissionAreaSpec, floorItems, 0) },
	{	"ItemRoof",				TOK_INT(MissionAreaSpec, roofItems, 0) },
	{	"}",					TOK_END },
	{	"", 0, 0 }
};

TokenizerParseInfo ParseMapMissionSpec[] =
{
	{	"",			TOK_STRUCTPARAM | TOK_FILENAME(MissionSpec, mapFile, 0) },
	{	"{",		TOK_START },
	{	"DisableMissionCheck",	TOK_BOOLFLAG(MissionSpec, disableMissionCheck, 0)	},
	{	"Variable",	TOK_BOOLFLAG(MissionSpec, varMapSpec, 0)					},
	{	"SubMaps",	TOK_STRINGARRAY(MissionSpec, subMapFiles)		},
	{	"Front",	TOK_EMBEDDEDSTRUCT(MissionSpec, front, ParseMissionAreaSpec) },
	{	"Middle",	TOK_EMBEDDEDSTRUCT(MissionSpec, middle,	ParseMissionAreaSpec) },
	{	"Back",		TOK_EMBEDDEDSTRUCT(MissionSpec, back, ParseMissionAreaSpec) },
	{	"Lobby",	TOK_EMBEDDEDSTRUCT(MissionSpec, lobby, ParseMissionAreaSpec) },
	{	"}",		TOK_END },

	// vars from ParseSetMissionSpec have to be here to get written..
	{	"",			TOK_INT(MissionSpec, mapSet,0)	},
	{	"",			TOK_INT(MissionSpec, mapSetTime,0)	},
	{	"", 0, 0 }
};

TokenizerParseInfo ParseSetMissionSpec[] =
{
	{	"",			TOK_STRUCTPARAM | TOK_INT(MissionSpec, mapSet, 0),	ParseMapSetEnum },
	{	"",			TOK_STRUCTPARAM | TOK_INT(MissionSpec, mapSetTime, 0) },
	{	"{",		TOK_START },
	{	"Front",	TOK_EMBEDDEDSTRUCT(MissionSpec, front, ParseMissionAreaSpec) },
	{	"Middle",	TOK_EMBEDDEDSTRUCT(MissionSpec, middle,	ParseMissionAreaSpec) },
	{	"Back",		TOK_EMBEDDEDSTRUCT(MissionSpec, back, ParseMissionAreaSpec) },
	{	"Lobby",	TOK_EMBEDDEDSTRUCT(MissionSpec, lobby, ParseMissionAreaSpec) },
	{	"}",		TOK_END },
	{	"", 0, 0 }
};

TokenizerParseInfo ParseMissionSpecFile[] =
{
	// doing kind of a hack to make the spec file look better.. 
	// Map and MissionSet don't actually refer to different structs
	{	"Map",			TOK_STRUCT(MissionSpecFile, missionSpecs, ParseMapMissionSpec) },
	{	"MissionSet",	TOK_REDUNDANTNAME | TOK_STRUCT(MissionSpecFile, missionSpecs, ParseSetMissionSpec) },
	{	"", 0, 0 }
};

static char* MissionSpecDescriptiveName(const MissionSpec* spec)
{
	static char buf[MAX_PATH];
	if (spec->mapFile)
		strcpy(buf, spec->mapFile);
	else if (spec->mapSet)
		sprintf(buf, "%i minute %s map", spec->mapSetTime, StaticDefineIntRevLookup(ParseMapSetEnum, spec->mapSet));
	else
		sprintf(buf, "(invalid mission spec)");
	return buf;
}

static int VerifyMissionAreaSpec(const MissionSpec* spec, const MissionAreaSpec* area)
{
	const EncounterGroupSpec* group;
	const char** layouts = 0;
	int i, n;
	int s, nums;
	int ret = 1;

	// verify that the encounter groups specify disjoint layouts
	eaCreateConst(&layouts);
	n = eaSize(&area->encounterGroups);
	for (i = 0; i < n; i++)
	{
		group = area->encounterGroups[i];
		nums = eaSize(&group->layouts);
		if (nums == 0)
		{
			ErrorFilenamef(MISSION_SPEC_FILENAME, "CustomEncounterGroup with no encounter layouts in %s",
				MissionSpecDescriptiveName(spec));
			ret = 0;
			break;
		}
		for (s = 0; s < nums; s++)
		{
			if (StringArrayFind(layouts, group->layouts[s]) >= 0)
			{
				ErrorFilenamef(MISSION_SPEC_FILENAME, "Layout %s specified twice in two different CustomEncounterGroups in %s",
					group->layouts[s],
					MissionSpecDescriptiveName(spec));
				ret = 0;
			}
			else
				eaPushConst(&layouts, group->layouts[s]);
		}
	}
	eaDestroyConst(&layouts);
	return ret;
}

static int VerifyMissionSpecFile(TokenizerParseInfo pti[], void* structptr)
{
	const MissionSpec** specs = missionSpecFile.missionSpecs;
	int i, n;
	int ret = 1;
	n = eaSize(&specs);
	for (i = 0; i < n; i++)
	{
		if (!specs[i]->mapFile && !specs[i]->mapSet)
		{
			ErrorFilenamef(MISSION_SPEC_FILENAME, "Mission spec with invalid mission set type (%i'th spec)", i+1);
			ret = 0;
		}
		if (!VerifyMissionAreaSpec(specs[i], &specs[i]->front)) ret = 0;
		if (!VerifyMissionAreaSpec(specs[i], &specs[i]->middle)) ret = 0;
		if (!VerifyMissionAreaSpec(specs[i], &specs[i]->back)) ret = 0;
	}
	return ret;
}

static int VerifyMissionSpecsUnique(void)
{
	const MissionSpec** specs = missionSpecFile.missionSpecs;
	int i, j, n;
	int ret = 1;
	n = eaSize(&specs);
	for (i = 0; i < n; i++)
	{
		if (!specs[i]->mapFile)
			continue;
		for (j = i+1; j < n; j++)
		{
			if (specs[j]->mapFile && !stricmp(specs[i]->mapFile, specs[j]->mapFile))
			{
				ErrorFilenamef(MISSION_SPEC_FILENAME, "Duplicate Mission Specs for: %s", specs[i]->mapFile);
				ret = 0;
			}
		}
	}
	return ret;
}

static int VerifyMissionSpecsValid(void)
{
	extern int ValidateSpecListItem(const char *specFile, const char *mapFile);

	const MissionSpec** specs = missionSpecFile.missionSpecs;
	int i, n;
	int ret = 1;
	n = eaSize(&specs);
	for (i = 0; i < n; i++)
	{
		if (!specs[i]->mapFile)
			continue;

		if(specs[i]->varMapSpec)
		{
			int j;
			for(j = eaSize(&(specs[i]->subMapFiles)) - 1; j >= 0; j--)
				ret = ret && ValidateSpecListItem(MISSION_SPEC_FILENAME, specs[i]->subMapFiles[j]);
		}
		else
			ret = ret && ValidateSpecListItem(MISSION_SPEC_FILENAME, specs[i]->mapFile);
	}
	return ret;
}

void MissionSpecReload(void)
{
	int flags = PARSER_SERVERONLY;

	if(missionSpecFile.missionSpecs)
	{
		ParserUnload(ParseMissionSpecFile, &missionSpecFile, sizeof(MissionSpecFile));
		flags |= PARSER_FORCEREBUILD;
	}

	if(!ParserLoadFilesShared("SM_missionspecs.bin", NULL, MISSION_SPEC_FILENAME, "missionspecs.bin", flags, ParseMissionSpecFile, &missionSpecFile, sizeof(MissionSpecFile), NULL, NULL, NULL, NULL, NULL))
	{
		printf("Unable to load missions.spec!\n");
		return;
	}

	VerifyMissionSpecsUnique();
	VerifyMissionSpecsValid();
}

const MissionSpec* MissionSpecFromFilename(const char* mapfile)
{
	int i, n;
	n = eaSize(&missionSpecFile.missionSpecs);

	for (i = 0; i < n; i++)
	{
		if (missionSpecFile.missionSpecs[i]->mapFile && !stricmp(missionSpecFile.missionSpecs[i]->mapFile, mapfile))
			return missionSpecFile.missionSpecs[i];
	}
	// file may have a specific section, or may be part of a mapset spec
	
	
	return NULL;
}

const MissionSpec* MissionSpecFromMapSet(MapSetEnum mapset, int maptime)
{
	int i, n;
	n = eaSize(&missionSpecFile.missionSpecs);
	for (i = 0; i < n; i++)
	{
		if (missionSpecFile.missionSpecs[i]->mapSet == mapset &&
			missionSpecFile.missionSpecs[i]->mapSetTime == maptime)
			return missionSpecFile.missionSpecs[i];
	}
	return NULL;
}

static EncounterGroupSpec** ShallowCopyGroupSpec(const EncounterGroupSpec** orig)
{
	int i, n;
	EncounterGroupSpec** ret;
	eaCreate(&ret);
	n = eaSize(&orig);
	eaSetSize(&ret, n);
	for (i = 0; i < n; i++)
	{
		ret[i] = calloc(sizeof(EncounterGroupSpec), 1);
		ret[i]->count = orig[i]->count;
		ret[i]->layouts = orig[i]->layouts;
	}
	return ret;
}

// enough of a copy to be able to manipulate the counts
static MissionSpec* ShallowCopyMissionSpec(const MissionSpec* orig)
{
	// leaving all the strings as shallow copies
	MissionSpec* ret = calloc(sizeof(MissionSpec),1);
	memcpy(ret, orig, sizeof(MissionSpec));
	ret->front.encounterGroups = ShallowCopyGroupSpec(orig->front.encounterGroups);
	ret->middle.encounterGroups = ShallowCopyGroupSpec(orig->middle.encounterGroups);
	ret->back.encounterGroups = ShallowCopyGroupSpec(orig->back.encounterGroups);
	ret->lobby.encounterGroups = ShallowCopyGroupSpec(orig->lobby.encounterGroups);
	return ret;
}

static void ShallowDestroyMissionSpec(MissionSpec* spec)
{
	// Only valid when ShallowCopyMissionSpec()'d above
	eaDestroyExConst(&spec->front.encounterGroups, NULL);
	eaDestroyExConst(&spec->middle.encounterGroups, NULL);
	eaDestroyExConst(&spec->back.encounterGroups, NULL);
	eaDestroyExConst(&spec->lobby.encounterGroups, NULL);
	free(spec);
}

// *********************************************************************************
//  Verify mission geometry
// *********************************************************************************

static void VerifyAndCountEncounter(const MissionSpec* spec, char* mapfile, EncounterGroup* encounter, int* spawnCounts, int* missionSpawns, const EncounterGroupSpec** groupSpecs)
{
	char actual[1000];
	char required[1000];
	static const char** layouts = NULL;
	static char** intersection = NULL;
	EncounterPoint* point;
	int i;
	int s, nums;

	// a couple of cached earrays so we don't have to realloc them
	if (!layouts) eaCreateConst(&layouts);
	eaSetSizeConst(&layouts, 0);
	if (!intersection) eaCreate(&intersection);
	eaSetSize(&intersection, 0);

	// get a list of our layouts first
	for (i = 0; i < encounter->children.size; i++)
	{
		point = encounter->children.storage[i];
		if (StringArrayFind(layouts, point->name) < 0)
			eaPushConst(&layouts, point->name);
	}

	// count mission spawns separately
	if (StringArrayFind(layouts, "Mission") >= 0)
	{
		(*missionSpawns)++;
	}

	// now look for an intersection with one of the specs
	nums = eaSize(&groupSpecs);
	for (s = 0; s < nums; s++)
	{
		const EncounterGroupSpec* groupspec = groupSpecs[s];
		StringArrayIntersectionConst(&intersection, layouts, groupspec->layouts);
		if (eaSize(&intersection) == eaSize(&groupspec->layouts))
		{
			spawnCounts[s]++;
			break;
		}
		else if (eaSize(&intersection))
		{
			StringArrayPrint(actual, 1000, layouts);
			StringArrayPrint(required, 1000, groupspec->layouts);
			ErrorFilenamef(mapfile, "Encounter group %s doesn't fit the list of required layouts in %s\n"
				"Actual layouts %s\nmission.spec requires: %s", encounter->geoname,
				MissionSpecDescriptiveName(spec), actual, required);
			break;
		}
	}
}

static void VerifyMissionPlace(const MissionSpec* spec, char* mapfile, const MissionAreaSpec* area, MissionPlaceEnum place)
{
	int r, numr;
	int i, numi;
	int s, nums;
	int floorItems = 0;
	int wallItems = 0;
	int roofItems = 0;
	int hostages = 0;
	int missionSpawns = 0;

	// count of how many of each of the encounter group types we have..
	static int* spawnCounts = NULL;

	if (!spawnCounts) eaiCreate(&spawnCounts);
	eaiSetSize(&spawnCounts, eaSize(&area->encounterGroups));
	eaiClear(&spawnCounts);

	// count up everything we have found on map
	numr = eaSize(&g_rooms);
	for (r = 1; r < numr; r++)	// don't count anything in NULL room
	{
		RoomInfo* room = g_rooms[r];
		if (place == MISSION_FRONT && !room->startroom) continue;
		if (place == MISSION_BACK && !room->endroom) continue;
		if (place == MISSION_MIDDLE && !(room->startroom == 0 && room->endroom == 0)) continue;
		if ((place == MISSION_LOBBY) ^ (room->lobbyroom)) continue;

		numi = eaSize(&room->items);
		for (i = 0; i < numi; i++)
		{
			if (room->items[i]->loctype == LOCATION_ITEMFLOOR)
				floorItems++;
			else if (room->items[i]->loctype == LOCATION_ITEMWALL)
				wallItems++;
			else if (room->items[i]->loctype == LOCATION_ITEMROOF)
				roofItems++;
			// ignore legacy locations
		}

		hostages += eaSize(&room->hostageEncounters);
		nums = eaSize(&room->standardEncounters);
		for (s = 0; s < nums; s++)
		{
			VerifyAndCountEncounter(spec, mapfile, room->standardEncounters[s], spawnCounts, &missionSpawns, area->encounterGroups);
		}
	}

	// see if we don't have enough of anything
	if (floorItems < area->floorItems)
	{
		ErrorFilenamef(mapfile, "Map doesn't have enough floor items in %s.  %s requires %i, only %i exist",
			StaticDefineIntRevLookup(ParseMissionPlaceEnum, place),
			MissionSpecDescriptiveName(spec), area->floorItems, floorItems);
	}
	if (wallItems < area->wallItems)
	{
		ErrorFilenamef(mapfile, "Map doesn't have enough wall items in %s.  %s requires %i, only %i exist",
			StaticDefineIntRevLookup(ParseMissionPlaceEnum, place),
			MissionSpecDescriptiveName(spec), area->wallItems, wallItems);
	}
	if (roofItems < area->roofItems)
	{
		ErrorFilenamef(mapfile, "Map doesn't have enough roof items in %s.  %s requires %i, only %i exist",
			StaticDefineIntRevLookup(ParseMissionPlaceEnum, place),
			MissionSpecDescriptiveName(spec), area->roofItems, roofItems);
	}
	if (hostages < area->hostages)
	{
		ErrorFilenamef(mapfile, "Map doesn't have enough hostages in %s.  %s requires %i, only %i exist",
			StaticDefineIntRevLookup(ParseMissionPlaceEnum, place),
			MissionSpecDescriptiveName(spec), area->hostages, hostages);
	}
	if (missionSpawns < area->missionSpawns)
	{
		ErrorFilenamef(mapfile, "Map doesn't have enough Mission spawns in %s.  %s requires %i, only %i exist",
			StaticDefineIntRevLookup(ParseMissionPlaceEnum, place),
			MissionSpecDescriptiveName(spec), area->missionSpawns, missionSpawns);
	}
	nums = eaSize(&area->encounterGroups);
	for (s = 0; s < nums; s++)
	{
		if (spawnCounts[s] < area->encounterGroups[s]->count)
		{
			char layout[1000];
			StringArrayPrint(layout, 1000, area->encounterGroups[s]->layouts);
			ErrorFilenamef(mapfile, "Map doesn't have enough %s encounter layouts in %s.  %s requires %i, only %i exist",
				layout, StaticDefineIntRevLookup(ParseMissionPlaceEnum, place),
				MissionSpecDescriptiveName(spec), area->encounterGroups[s]->count, spawnCounts[s]);
		}
	}
}

static void VerifyMissionSpec(const MissionSpec* spec, char* mapfile)
{
	VerifyMissionPlace(spec, mapfile, &spec->front, MISSION_FRONT);
	VerifyMissionPlace(spec, mapfile, &spec->middle, MISSION_MIDDLE);
	VerifyMissionPlace(spec, mapfile, &spec->back, MISSION_BACK);
	VerifyMissionPlace(spec, mapfile, &spec->lobby, MISSION_LOBBY);
}

static int VerifyMapSet(MapSetEnum mapset, int maptime)
{
	const MissionSpec* spec = MissionSpecFromMapSet(mapset, maptime);
	if (spec) VerifyMissionSpec(spec, saUtilCurrentMapFile());
	return 1;
}

// actually going to verify encounter, hostage, and item coverage here
void VerifyMissionEncounterCoverage(void)
{
	const MissionSpec* spec = MissionSpecFromFilename(saUtilCurrentMapFile());
	if (spec) VerifyMissionSpec(spec, saUtilCurrentMapFile());
	MissionMapSetForEach(saUtilCurrentMapFile(), VerifyMapSet);
}


// *********************************************************************************
//  Verify mission script
// *********************************************************************************

static int ProcessAndTestPlaceEncounterArea(MissionAreaSpec* area, const char* spawntype, int forced, int cloned, int count)
{
	if (stricmp(spawntype, "Mission") == 0)
	{
		if (area->missionSpawns > 0 || forced)
		{
			if (!cloned)
				area->missionSpawns -= count;
			return 1;
		}
	}
	else
	{
		int i, n;
		EncounterGroupSpec** groupspec = cpp_const_cast(EncounterGroupSpec**)area->encounterGroups;
		n = eaSize(&groupspec);
		for (i = 0; i < n; i++)
		{
			if (StringArrayFind(groupspec[i]->layouts, spawntype) >= 0)
			{
				if ((groupspec[i]->count >= count && area->missionSpawns >= count) || forced)
				{
					if (!cloned) 
					{
						area->missionSpawns -= count;
						groupspec[i]->count -= count;
					}
					return 1;
				}
				break;
			}
		}
	}
	return 0;
}

static int ProcessAndTestPlaceEncounter(MissionSpec* countdown, const SpawnDef* spawndef, const char* spawntype)
{
	int placed = 0;

	if (spawndef->missionplace == MISSION_FRONT ||
		spawndef->missionplace == MISSION_OBJECTIVE ||
		spawndef->missionplace == MISSION_ANY)
	{
		placed = ProcessAndTestPlaceEncounterArea(&countdown->front, spawntype, spawndef->missionplace == MISSION_FRONT, 
											spawndef->flags & SPAWNDEF_CLONEGROUP, spawndef->missioncount);
	}
	if (!placed &&
		(spawndef->missionplace == MISSION_BACK ||
		spawndef->missionplace == MISSION_OBJECTIVE ||
		spawndef->missionplace == MISSION_ANY))
	{
		placed = ProcessAndTestPlaceEncounterArea(&countdown->back, spawntype, spawndef->missionplace == MISSION_BACK,
											spawndef->flags & SPAWNDEF_CLONEGROUP, spawndef->missioncount);
	}
	if (!placed && 
		(spawndef->missionplace == MISSION_LOBBY ||
		spawndef->missionplace == MISSION_OBJECTIVE ||
		spawndef->missionplace == MISSION_ANY))
	{
		placed = ProcessAndTestPlaceEncounterArea(&countdown->lobby, spawntype, spawndef->missionplace == MISSION_LOBBY, 
			spawndef->flags & SPAWNDEF_CLONEGROUP, spawndef->missioncount);
	}

	if (!placed)
	{
		placed = ProcessAndTestPlaceEncounterArea(&countdown->middle, spawntype, 1,
											spawndef->flags & SPAWNDEF_CLONEGROUP, spawndef->missioncount);
	}
	return placed;
}

static int ProcessAndTestPlaceObjectiveArea(MissionAreaSpec* area, const MissionObjective* objective, int forced)
{
	switch (objective->locationtype)
	{
	case LOCATION_PERSON: 
		if (area->hostages > 0 || forced)
		{
			area->hostages--;
			return 1;
		}
		break;
	case LOCATION_ITEMWALL:
		if (area->wallItems > 0 || forced)
		{
			area->wallItems--;
			return 1;
		}
		break;
	case LOCATION_ITEMFLOOR:
		if (area->floorItems > 0 || forced)
		{
			area->floorItems--;
			return 1;
		}
		break;
	case LOCATION_ITEMROOF:
		if (area->roofItems > 0 || forced)
		{
			area->roofItems--;
			return 1;
		}
	}
	return 0;
}

static void ProcessAndTestPlaceObjective(MissionSpec* countdown, const MissionObjective* objective)
{
	int placed = 0;
	if (objective->room == MISSION_FRONT ||
		objective->room == MISSION_OBJECTIVE ||
		objective->room == MISSION_ANY)
	{
		placed = ProcessAndTestPlaceObjectiveArea(&countdown->front, objective, objective->room == MISSION_FRONT);
	}
	if (!placed &&
		(objective->room == MISSION_BACK ||
		objective->room == MISSION_OBJECTIVE ||
		objective->room == MISSION_ANY))
	{
		placed = ProcessAndTestPlaceObjectiveArea(&countdown->back, objective, objective->room == MISSION_BACK);
	}
	if (!placed &&
		(objective->room == MISSION_LOBBY ||
		objective->room == MISSION_OBJECTIVE ||
		objective->room == MISSION_ANY))
	{
		placed = ProcessAndTestPlaceObjectiveArea(&countdown->lobby, objective, objective->room == MISSION_LOBBY);
	}
	if (!placed)
	{
		placed = ProcessAndTestPlaceObjectiveArea(&countdown->middle, objective, 1);
	}
}

static int pe_ret;
static void PlaceError(const char* filename, const char* logicalname, int required, int actual, const char* itemname, 
					   MissionPlaceEnum place, const MissionSpec* spec)
{
	ErrorFilenamef(filename, "Mission %s requires %i %s in %s of %s, but only %i are available",
		logicalname, required, itemname, 
		StaticDefineIntRevLookup(ParseMissionPlaceEnum, place),
		MissionSpecDescriptiveName(spec), actual);
	pe_ret = 0;
}

static int VerifyPlaceCount(const MissionAreaSpec* countdown, const MissionAreaSpec* actual, MissionPlaceEnum place, 
							const MissionDef* mission, const char* logicalname, const MissionSpec* spec)
{
	int i, n;
	pe_ret = 1;
	if (mission->mapset==MAPSET_RANDOM)	//random is a special case, so don't check all this stuff for it
		return 1;
	if (countdown->floorItems < 0)
		PlaceError(mission->filename, logicalname, actual->floorItems - countdown->floorItems, actual->floorItems, 
			"Floor items", place, spec);
	if (countdown->hostages < 0)
		PlaceError(mission->filename, logicalname, actual->hostages - countdown->hostages, actual->hostages,
			"Hostages", place, spec);
	if (countdown->roofItems < 0)
		PlaceError(mission->filename, logicalname, actual->roofItems - countdown->roofItems, actual->roofItems, 
			"Roof items", place, spec);
	if (countdown->wallItems < 0)
		PlaceError(mission->filename, logicalname, actual->wallItems - countdown->wallItems, actual->wallItems, 
			"Wall items", place, spec);
	n = eaSize(&countdown->encounterGroups);
	for (i = 0; i < n; i++)
	{
		const EncounterGroupSpec* countdowngroup = countdown->encounterGroups[i];
		const EncounterGroupSpec* actualgroup = actual->encounterGroups[i];
		if (countdowngroup->count < 0)
		{
			char buf[1000]; 
			StringArrayPrint(buf, 1000, countdowngroup->layouts);
			PlaceError(mission->filename, logicalname, actualgroup->count - countdowngroup->count, actualgroup->count, 
				buf, place, spec);
		}
	}
	return pe_ret;
}

static int VerifyAndProcessMissionPlacementAgainst(MissionDef* mission, const char* logicalname, const MissionSpec* spec)
{
	int i, n;
	int ret = 1;
	const char* spawntype;
	MissionSpec* countdown;
	ScriptVarsTable vars = {0};
#define VAR_LOOKUP(spawndef) \
	ScriptVarsTableClear(&vars); \
	ScriptVarsTablePushScope(&vars, &mission->vs); \
	ScriptVarsTablePushScope(&vars, &spawndef->vs); \
	spawntype = ScriptVarsTableLookup(&vars, spawndef->spawntype); \
	if (!spawntype) spawntype = "Mission";

	countdown = ShallowCopyMissionSpec(spec);
	

	// spawns with generic placement have to be mission layout
	n = eaSize(&mission->spawndefs);
	for (i = 0; i < n; i++)
	{
		if (mission->spawndefs[i]->missionplace == MISSION_REPLACEGENERIC)
		{
			VAR_LOOKUP(mission->spawndefs[i])
			if (stricmp(spawntype, "Mission") != 0)
			{
				ErrorFilenamef(mission->filename, "Mission %s: %s encounter is marked as ReplaceGeneric but isn't Mission layout",
					logicalname, spawntype);
				ret = 0;
			}
		}
	}

	// place the encounters
	for (i = 0; i < n; i++)
	{
		// area placement first
		if (mission->spawndefs[i]->missionplace == MISSION_FRONT ||
			mission->spawndefs[i]->missionplace == MISSION_MIDDLE ||
			mission->spawndefs[i]->missionplace == MISSION_BACK ||
			mission->spawndefs[i]->missionplace == MISSION_LOBBY)
		{
			VAR_LOOKUP(mission->spawndefs[i])
			if (!ProcessAndTestPlaceEncounter(countdown, mission->spawndefs[i], spawntype))
			{
				ErrorFilenamef(mission->filename, "Mission %s: %s encounter is not available in %s of %s",
					logicalname, spawntype, 
					StaticDefineIntRevLookup(ParseMissionPlaceEnum, mission->spawndefs[i]->missionplace),
					MissionSpecDescriptiveName(spec));
				ret = 0;
			}
		}
	}
	for (i = 0; i < n; i++)
	{
		// then objective, any placement
		if (mission->spawndefs[i]->missionplace == MISSION_OBJECTIVE ||
			mission->spawndefs[i]->missionplace == MISSION_ANY)
		{
			VAR_LOOKUP(mission->spawndefs[i])
			if (!ProcessAndTestPlaceEncounter(countdown, mission->spawndefs[i], spawntype))
			{
				ErrorFilenamef(mission->filename, "Mission %s: %s encounter is not available on %s",
					logicalname, spawntype, MissionSpecDescriptiveName(spec));
				ret = 0;
			}
		}
	}

	// place the objectives
	n = eaSize(&mission->objectives);
	for (i = 0; i < n; i++)
	{
		// area placement first
		if (mission->objectives[i]->room == MISSION_FRONT ||
			mission->objectives[i]->room == MISSION_MIDDLE ||
			mission->objectives[i]->room == MISSION_BACK ||
			mission->objectives[i]->room == MISSION_LOBBY)
		{
			ProcessAndTestPlaceObjective(countdown, mission->objectives[i]);
		}
	}
	for (i = 0; i < n; i++)
	{
		// then objective, any placement
		if (mission->objectives[i]->room == MISSION_OBJECTIVE ||
			mission->objectives[i]->room == MISSION_ANY)
		{
			ProcessAndTestPlaceObjective(countdown, mission->objectives[i]);
		}
	}

	// verify the count of everything
	if (!VerifyPlaceCount(&countdown->front, &spec->front, MISSION_FRONT, mission, logicalname, spec)) ret = 0;
	if (!VerifyPlaceCount(&countdown->middle, &spec->middle, MISSION_MIDDLE, mission, logicalname, spec)) ret = 0;
	if (!VerifyPlaceCount(&countdown->back, &spec->back, MISSION_BACK, mission, logicalname, spec)) ret = 0;
	if (!VerifyPlaceCount(&countdown->lobby, &spec->lobby, MISSION_LOBBY, mission, logicalname, spec)) ret = 0;

	ShallowDestroyMissionSpec(countdown);
	return 1;
#undef VAR_LOOKUP
}

static const MissionSpec* vmp_spec;

static int FirstMapSet(MapSetEnum mapset, int maptime)
{
	vmp_spec = MissionSpecFromMapSet(mapset, maptime);
	return 0;
}

int VerifyAndProcessMissionPlacement(MissionDef* mission, const char* logicalname)
{
	// figure out the spec to use
	if (mission->mapfile)
	{
		vmp_spec = MissionSpecFromFilename(mission->mapfile);
		if (!vmp_spec)
			MissionMapSetForEach(mission->mapfile, FirstMapSet);
		if (!vmp_spec)
		{
			ErrorFilenamef(mission->filename, "Mission %s specifies an invalid map %s (map needs to be in missions.spec)",
				logicalname, mission->mapfile);
			return 0;
		}
	}
	else if (mission->mapset!=MAPSET_RANDOM)
	{
		vmp_spec = MissionSpecFromMapSet(mission->mapset, mission->maplength);
		if (!vmp_spec)
		{
			ErrorFilenamef(mission->filename, "Mission %s specifies an invalid mapset (%s,%i) - (mapset needs to be in missions.spec)",
				logicalname, StaticDefineIntRevLookup(ParseMapSetEnum, mission->mapset), mission->maplength);
			return 0;
		}
	}
	else if (mission->mapset == MAPSET_RANDOM)
	{
		// We can't really verify placement of a random mission so....
		return 1;
	}

	if(vmp_spec->disableMissionCheck && isDevelopmentMode()) // designer override of the check for development
		return 1;
	else if(vmp_spec->varMapSpec) // mapname is actually a VAR
	{
			int i, ret = 1;
			for(i = eaSize(&vmp_spec->subMapFiles) - 1; i >= 0; i--)
			{
				const MissionSpec* sub_spec = MissionSpecFromFilename(vmp_spec->subMapFiles[i]);
				if(!sub_spec)
				{
					ErrorFilenamef(mission->filename, "Variable MapSpec %s specifies an invalid map %s (map needs to be in missions.spec)",
						mission->mapfile, vmp_spec->subMapFiles[i]);
					return 0;
				}
				ret = ret && VerifyAndProcessMissionPlacementAgainst(mission, logicalname, sub_spec);
			}
			return ret;
	}
	return VerifyAndProcessMissionPlacementAgainst(mission, logicalname, vmp_spec);
}

static void WriteString(FILE* out, char* str, int tabs, int eol)
{
	int i;
	for (i = 0; i < tabs; i++) fwrite("\t", 1, sizeof(char), out);
	fwrite(str, strlen(str), sizeof(char), out);
	if (eol) fwrite("\r\n", 2, sizeof(char), out);
}

extern int minGenericDifference;
extern int minHostageDifference;
extern int minWallDifference;
extern int minFloorDifference;
extern int minRoofDifference;

static void MissionPlaceStats_checkAgainstSpec(MissionPlaceStats *missionPlaceStats, const MissionAreaSpec *missionAreaSpec, FILE *file, FILE *lobbyFile, MissionPlaceStats* minDiffs, char *mapName)
{
	char buffer[256];
	int genericDifference, hostageDifference, wallDifference, floorDifference, roofDifference;
	int customGroupIndex, customGroupCount;
	int totalCount = 0;

	genericDifference = missionPlaceStats->genericEncounterOnlyCount - missionAreaSpec->missionSpawns;
	if (genericDifference < 0)
	{
		sprintf(buffer, "Mission %s doesn't have enough generic encounters in %s: has %i but requires %i",
			mapName, StaticDefineIntRevLookup(ParseMissionPlaceEnum, missionPlaceStats->place), missionPlaceStats->genericEncounterOnlyCount, missionAreaSpec->missionSpawns);
		WriteString(file, buffer, 0, 1);
	}
	else if (genericDifference < minDiffs->genericEncounterOnlyCount)
	{
		minDiffs->genericEncounterOnlyCount = genericDifference;
	}
	totalCount += missionPlaceStats->genericEncounterOnlyCount;

	hostageDifference = missionPlaceStats->hostageLocationCount - missionAreaSpec->hostages;
	if (hostageDifference < 0)
	{
		sprintf(buffer, "Mission %s doesn't have enough hostage encounters in %s: has %i but requires %i",
			mapName, StaticDefineIntRevLookup(ParseMissionPlaceEnum, missionPlaceStats->place), missionPlaceStats->hostageLocationCount, missionAreaSpec->hostages);
		WriteString(file, buffer, 0, 1);
	}
	else if (hostageDifference < minDiffs->hostageLocationCount)
	{
		minDiffs->hostageLocationCount = hostageDifference;
	}
	totalCount += missionPlaceStats->hostageLocationCount;

	wallDifference = missionPlaceStats->wallObjectiveCount - missionAreaSpec->wallItems;
	if (wallDifference < 0)
	{
		sprintf(buffer, "Mission %s doesn't have enough wall objectives in %s: has %i but requires %i", 
			mapName, StaticDefineIntRevLookup(ParseMissionPlaceEnum, missionPlaceStats->place), missionPlaceStats->wallObjectiveCount, missionAreaSpec->wallItems);
		WriteString(file, buffer, 0, 1);
	}
	else if (wallDifference < minDiffs->wallObjectiveCount)
	{
		minDiffs->wallObjectiveCount = wallDifference;
	}
	totalCount += missionPlaceStats->wallObjectiveCount;

	floorDifference = missionPlaceStats->floorObjectiveCount - missionAreaSpec->floorItems;
	if (floorDifference < 0)
	{
		sprintf(buffer, "Mission %s doesn't have enough floor objectivesin %s: has %i but requires %i", 
			mapName, StaticDefineIntRevLookup(ParseMissionPlaceEnum, missionPlaceStats->place), missionPlaceStats->floorObjectiveCount, missionAreaSpec->floorItems);
		WriteString(file, buffer, 0, 1);
	}
	else if (floorDifference < minDiffs->floorObjectiveCount)
	{
		minDiffs->floorObjectiveCount = floorDifference;
	}
	totalCount += missionPlaceStats->floorObjectiveCount;

	roofDifference = missionPlaceStats->roofObjectiveCount - missionAreaSpec->roofItems;
	if (roofDifference < 0)
	{
		sprintf(buffer, "Mission %s doesn't have enough roof objectives in %s: has %i but requires %i", 
			mapName, StaticDefineIntRevLookup(ParseMissionPlaceEnum, missionPlaceStats->place), missionPlaceStats->roofObjectiveCount, missionAreaSpec->roofItems);
		WriteString(file, buffer, 0, 1);
	}
	else if (roofDifference < minDiffs->roofObjectiveCount)
	{
		minDiffs->roofObjectiveCount = roofDifference;
	}
	totalCount += missionPlaceStats->roofObjectiveCount;

	customGroupCount = eaSize(&missionAreaSpec->encounterGroups);
	for (customGroupIndex = 0; customGroupIndex < customGroupCount; customGroupIndex++)
	{
		const EncounterGroupSpec *customSpec = missionAreaSpec->encounterGroups[customGroupIndex];
		int layoutIndex, layoutCount;
		int statsCount = 0;
		int statsDiff;
		char *combinedLayoutName;

		estrCreate(&combinedLayoutName);

		layoutCount = eaSize(&customSpec->layouts);
		for (layoutIndex = 0; layoutIndex < layoutCount; layoutIndex++)
		{
			const char *layoutName = customSpec->layouts[layoutIndex];
			int customStatsIndex, customStatsCount;

			estrConcatCharString(&combinedLayoutName, layoutName);
			estrConcatChar(&combinedLayoutName, ' ');

			customStatsCount = eaSize(&missionPlaceStats->customEncounters);
			for (customStatsIndex = 0; customStatsIndex < customStatsCount; customStatsIndex++)
			{
				MissionCustomEncounterStats *customStats = missionPlaceStats->customEncounters[customStatsIndex];
				if (!stricmp(customStats->type, layoutName))
				{
					statsCount += customStats->countOnly;
				}
			}
		}

		statsDiff = statsCount - customSpec->count;
		if (statsDiff < 0)
		{
			sprintf(buffer, "Mission %s doesn't have enough '%s' encounters in %s: has %i but requires %i",
				mapName, StaticDefineIntRevLookup(ParseMissionPlaceEnum, missionPlaceStats->place), combinedLayoutName, statsCount, customSpec->count);
			WriteString(file, buffer, 0, 1);
		}
		else
		{
			int minDiffCustomIndex, minDiffCustomCount;
			bool foundItInMinDiffList = false;

			minDiffCustomCount = eaSize(&minDiffs->customEncounters);
			for (minDiffCustomIndex = 0; minDiffCustomIndex < minDiffCustomCount; minDiffCustomIndex++)
			{
				MissionCustomEncounterStats *minDiffCustomStats = minDiffs->customEncounters[minDiffCustomIndex];
				if (!stricmp(minDiffCustomStats->type, combinedLayoutName))
				{
					if (statsDiff < minDiffCustomStats->countOnly)
					{
						minDiffCustomStats->countOnly = statsDiff;
					}
					foundItInMinDiffList = true;
				}
			}

			if (!foundItInMinDiffList)
			{
				MissionCustomEncounterStats *newMinDiffStats = StructAllocRaw(sizeof(MissionCustomEncounterStats));
				newMinDiffStats->type = StructAllocString(combinedLayoutName);
				newMinDiffStats->countOnly = statsDiff;
				eaPush(&minDiffs->customEncounters, newMinDiffStats);
			}
		}
		totalCount += statsCount;

		estrDestroy(&combinedLayoutName);
	}

	if (lobbyFile)
	{
		sprintf(buffer, "%d in Lobby in map %s", totalCount, mapName);
		WriteString(lobbyFile, buffer, 0, 1);
	}
}


static void MissionMapStats_checkAgainstSpec(MissionMapStats *missionMapStats, const MissionSpec *missionSpec, FILE *file, FILE *lobbyFile, MissionMapStats *minDiffs)
{
	int i;
	//char buffer[256];

	//sprintf(buffer, "Checking mission %s...", missionMapStats->pchMapName);
	//WriteString(file, buffer, 3, 1);

	for (i = 0; i < missionMapStats->placeCount; i++)
	{
		if (missionMapStats->places[i]->place == MISSION_FRONT)
		{
			MissionPlaceStats_checkAgainstSpec(missionMapStats->places[i], &missionSpec->front, file, 0, minDiffs->places[i], missionMapStats->pchMapName);
		}
	}
	for (i = 0; i < missionMapStats->placeCount; i++)
	{
		if (missionMapStats->places[i]->place == MISSION_BACK)
		{
			MissionPlaceStats_checkAgainstSpec(missionMapStats->places[i], &missionSpec->back, file, 0, minDiffs->places[i], missionMapStats->pchMapName);
		}
	}
	for (i = 0; i < missionMapStats->placeCount; i++)
	{
		if (missionMapStats->places[i]->place == MISSION_MIDDLE)
		{
			MissionPlaceStats_checkAgainstSpec(missionMapStats->places[i], &missionSpec->middle, file, 0, minDiffs->places[i], missionMapStats->pchMapName);
		}
	}
	for (i = 0; i < missionMapStats->placeCount; i++)
	{
		if (missionMapStats->places[i]->place == MISSION_LOBBY)
		{
			MissionPlaceStats_checkAgainstSpec(missionMapStats->places[i], &missionSpec->lobby, file, lobbyFile, minDiffs->places[i], missionMapStats->pchMapName);
		}
	}
}

void MissionDebugCheckAllMapStats(ClientLink *client, Entity *player)
{
	// Loops over all top level items in mission.specs
	char buffer[256];
	const MissionSpec** missionSpecs = missionSpecFile.missionSpecs;
	int missionSpecIndex, missionSpecCount;
	FILE* file, *lobbyFile;
	
	MissionMapPreload();

	file = fileOpen("C:/missionSpecValidationResults.txt", "wb");
	lobbyFile = fileOpen("C:/missionSpecLobbyCounts.txt", "wb");

	missionSpecCount = eaSize(&missionSpecs);
	for (missionSpecIndex = 0; missionSpecIndex < missionSpecCount; missionSpecIndex++)
	{
		const MissionSpec *missionSpec = missionSpecs[missionSpecIndex];
		if (missionSpec->mapSet == MAPSET_NONE)
		{
			MissionMapStats *missionMapStats = missionMapStat_Get(missionSpec->mapFile);
			if (missionMapStats)
			{
				int i;
				MissionMapStats *minDiffs;

				minDiffs = missionMapStats_initialize("");
				for (i = 0; i < 5; i++)
				{
					minDiffs->places[i]->genericEncounterOnlyCount = 99999;
					minDiffs->places[i]->genericEncounterGroupedCount = 99999;
					minDiffs->places[i]->hostageLocationCount = 99999;
					minDiffs->places[i]->wallObjectiveCount = 99999;
					minDiffs->places[i]->floorObjectiveCount = 99999;
					minDiffs->places[i]->roofObjectiveCount = 99999;
				}

				MissionMapStats_checkAgainstSpec(missionMapStats, missionSpec, file, lobbyFile, minDiffs);

				sprintf(buffer, "Minimum Differences for Individual Mission %s:", missionMapStats->pchMapName);
				WriteString(file, buffer, 0, 1);

				for (i = 1; i < 4; i++)
				{
					int customIndex, customCount;

					sprintf(buffer, "Place: %s", StaticDefineIntRevLookup(ParseMissionPlaceEnum, minDiffs->places[i]->place));
					WriteString(file, buffer, 1, 1);

					if (minDiffs->places[i]->genericEncounterOnlyCount)
					{
						sprintf(buffer, "Minimum Generic Difference: %i", minDiffs->places[i]->genericEncounterOnlyCount);
						WriteString(file, buffer, 2, 1);
					}

					if (minDiffs->places[i]->hostageLocationCount)
					{
						sprintf(buffer, "Minimum Hostage Difference: %i", minDiffs->places[i]->hostageLocationCount);
						WriteString(file, buffer, 2, 1);
					}

					if (minDiffs->places[i]->wallObjectiveCount)
					{
						sprintf(buffer, "Minimum Wall Difference: %i", minDiffs->places[i]->wallObjectiveCount);
						WriteString(file, buffer, 2, 1);
					}

					if (minDiffs->places[i]->floorObjectiveCount)
					{
						sprintf(buffer, "Minimum Floor Difference: %i", minDiffs->places[i]->floorObjectiveCount);
						WriteString(file, buffer, 2, 1);
					}

					if (minDiffs->places[i]->roofObjectiveCount)
					{
						sprintf(buffer, "Minimum Roof Difference: %i", minDiffs->places[i]->roofObjectiveCount);
						WriteString(file, buffer, 2, 1);
					}

					customCount = eaSize(&minDiffs->places[i]->customEncounters);
					for (customIndex = 0; customIndex < customCount; customIndex++)
					{
						MissionCustomEncounterStats *customStats = minDiffs->places[i]->customEncounters[customIndex];
						if (customStats->countOnly)
						{
							sprintf(buffer, "Minimum '%s' Difference: %i", customStats->type, customStats->countOnly);
							WriteString(file, buffer, 2, 1);
						}
					}
				}
				missionMapStats_destroy(minDiffs);
			}
			else
			{
				sprintf(buffer, "MissionMapStats could not be found for mapFile %s!", missionSpec->mapFile);
				WriteString(file, buffer, 0, 1);
			}
		}
		else
		{
			// Loops over all missions in mission.mapspecs (only within sets)
			int s, sCount;
			int t, tCount;
			int m, mCount;
			MissionMap** maps;
			
			sCount = eaSize(&g_missionmaplist.missionmapsets);
			for (s = 0; s < sCount; s++)
			{
				const MissionMapSet *missionmapset = g_missionmaplist.missionmapsets[s];
				if (missionmapset->mapset != missionSpec->mapSet)
				{
					continue;
				}

				tCount = eaSize(&missionmapset->missionmaptimes);
				for (t = 0; t < tCount; t++)
				{
					int i;
					MissionMapStats *minDiffs;
					const MissionMapTime *missionmaptime = missionmapset->missionmaptimes[t];
					if (missionmaptime->maptime != missionSpec->mapSetTime)
					{
						continue;
					}

					minDiffs = missionMapStats_initialize("");
					for (i = 0; i < 5; i++)
					{
						minDiffs->places[i]->genericEncounterOnlyCount = 99999;
						minDiffs->places[i]->hostageLocationCount = 99999;
						minDiffs->places[i]->wallObjectiveCount = 99999;
						minDiffs->places[i]->floorObjectiveCount = 99999;
						minDiffs->places[i]->roofObjectiveCount = 99999;
					}

					maps = missionmaptime->maps;
					mCount = eaSize(&maps);
					for (m = 0; m < mCount; m++)
					{
						MissionMapStats *missionMapStats = missionMapStat_Get(maps[m]->mapfile);
						if (missionMapStats)
						{
							MissionMapStats_checkAgainstSpec(missionMapStats, missionSpec, file, 0, minDiffs);
						}
						else
						{
							sprintf(buffer, "MissionMapStats could not be found for mapFile %s!", maps[m]->mapfile);
							WriteString(file, buffer, 0, 1);
						}
					}

					sprintf(buffer, "Set: %s, Time: %i", StaticDefineIntRevLookup(ParseMapSetEnum, missionSpec->mapSet), missionSpec->mapSetTime);
					WriteString(file, buffer, 0, 1);
						
					for (i = 1; i <= 4; i++)
					{
						int customIndex, customCount;

						sprintf(buffer, "Place: %s", StaticDefineIntRevLookup(ParseMissionPlaceEnum, minDiffs->places[i]->place));
						WriteString(file, buffer, 1, 1);

						if (minDiffs->places[i]->genericEncounterOnlyCount)
						{
							sprintf(buffer, "Minimum Generic Difference: %i", minDiffs->places[i]->genericEncounterOnlyCount);
							WriteString(file, buffer, 2, 1);
						}

						if (minDiffs->places[i]->hostageLocationCount)
						{
							sprintf(buffer, "Minimum Hostage Difference: %i", minDiffs->places[i]->hostageLocationCount);
							WriteString(file, buffer, 2, 1);
						}

						if (minDiffs->places[i]->wallObjectiveCount)
						{
							sprintf(buffer, "Minimum Wall Difference: %i", minDiffs->places[i]->wallObjectiveCount);
							WriteString(file, buffer, 2, 1);
						}

						if (minDiffs->places[i]->floorObjectiveCount)
						{
							sprintf(buffer, "Minimum Floor Difference: %i", minDiffs->places[i]->floorObjectiveCount);
							WriteString(file, buffer, 2, 1);
						}

						if (minDiffs->places[i]->roofObjectiveCount)
						{
							sprintf(buffer, "Minimum Roof Difference: %i", minDiffs->places[i]->roofObjectiveCount);
							WriteString(file, buffer, 2, 1);
						}

						customCount = eaSize(&minDiffs->places[i]->customEncounters);
						for (customIndex = 0; customIndex < customCount; customIndex++)
						{
							MissionCustomEncounterStats *customStats = minDiffs->places[i]->customEncounters[customIndex];
							if (customStats->countOnly)
							{
								sprintf(buffer, "Minimum '%s' Difference: %i", customStats->type, customStats->countOnly);
								WriteString(file, buffer, 2, 1);
							}
						}
					}
					missionMapStats_destroy(minDiffs);
				}
			}
		}
	}
}