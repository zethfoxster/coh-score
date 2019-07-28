/*\
 *
 *	missionset.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	handles mission map sets and mission spawn sets
 *  - basically lists of generic stuff we can use to fill out missions
 *
 *  we also handle assigning a mission door from here
 *
 */

#include "mission.h"
#include "storyarcprivate.h"
#include "dbdoor.h"
#include "dbcomm.h"
#include "staticMapInfo.h"
#include "AppVersion.h"
#include "taskRandom.h"
#include "entity.h"
#include "teamCommon.h"
#include "logcomm.h"
#include "comm_game.h"
#include "missionMapCommon.h"

// *********************************************************************************
//  Mission map sets
// *********************************************************************************

const char* MissionGetMapName(Entity * ent, const StoryTaskInfo *sti, const ContactHandle contacthandle)
{
	const MissionDef* mission = TaskMissionDefinition(sti ? &sti->sahandle : NULL);
	ScriptVarsTable vars = {0};

	if (!mission)
	{
		return NULL;
	}

	// Potential mismatch
	saBuildScriptVarsTable(&vars, ent, sti->assignedDbId, contacthandle, sti, NULL, NULL, NULL, NULL, 0, 0, false);

	if (mission->mapfile)
		return ScriptVarsTableLookup(&vars, mission->mapfile);
	else
	{
		// perturb the seed by subhandle and compound position so that we get different maps for each subtask
		if (mission->mapset==MAPSET_RANDOM)
			return MissionMapPick(sti->mapSet, mission->maplength, sti->seed + sti->sahandle.context + 100 * sti->sahandle.subhandle + 10000 * sti->sahandle.compoundPos);
		else 
			return MissionMapPick(mission->mapset, mission->maplength, sti->seed + sti->sahandle.context + 100 * sti->sahandle.subhandle + 10000 * sti->sahandle.compoundPos);
	}
}

// randomly selects a mission based on passed criteria
// if it fails, still returns a valid map, and logs the error
char* MissionMapPick(MapSetEnum mapset, int maptime, unsigned int seed)
{
	int nummaps;
	MissionMap** maps;
	const char* mapstr;

	maps = MissionMapGetList(mapset, maptime);
	nummaps = eaSize(&maps);
	if (nummaps)
	{
		int sel;
		sel = saSeedRoll(seed, nummaps);
		return maps[sel]->mapfile;
	}

	// otherwise, we failed, log an error
	mapstr = StaticDefineIntRevLookup(ParseMapSetEnum, mapset);
	if (!mapstr)
		mapstr = "(Invalid map set)";
	LOG_OLD( "Failed to find map to match %s, %i", mapstr, maptime);

	maps = MissionMapGetList(MAPSET_OFFICE, 30);
	return maps[0]->mapfile; // assume we have one map anyway
}

// *********************************************************************************
//  Mission spawn sets
// *********************************************************************************

typedef struct MissionSpawnLevels
{
	int					minlevel;
	int					maxlevel;
	const char**		spawndefs;
} MissionSpawnLevels;

TokenizerParseInfo ParseMissionSpawnLevels[] = {
	{ "", TOK_STRUCTPARAM | TOK_INT(MissionSpawnLevels, minlevel,0) },
	{ "", TOK_STRUCTPARAM | TOK_INT(MissionSpawnLevels, maxlevel,0) },
	{ "SpawnDef",		TOK_STRINGARRAY(MissionSpawnLevels, spawndefs) },
	{ "{",				TOK_START,		0 },
	{ "}",				TOK_END,		0 },
	{ "", 0, 0 }
};

typedef struct MissionSpawnList
{
	const char*				filename;		// just for error reporting
	VillainGroupEnum	villaingroup;
	const MissionSpawnLevels** levels;
} MissionSpawnList;

TokenizerParseInfo ParseMissionSpawnList[] = {
	{ "",				TOK_CURRENTFILE(MissionSpawnList, filename) },
	{ "", TOK_STRUCTPARAM | TOK_INT(MissionSpawnList, villaingroup, 0), ParseVillainGroupEnum },
	{ "Levels",			TOK_STRUCT(MissionSpawnList, levels, ParseMissionSpawnLevels) },
	{ "{",				TOK_START,		0 },
	{ "}",				TOK_END,		0 },
	{ "", 0, 0 }
};

typedef struct MissionGenericSpawns
{
	const MissionSpawnList**	lists;
} MissionGenericSpawns;

TokenizerParseInfo ParseMissionGenericSpawns[] = {
	{ "MissionSpawns",	TOK_STRUCT(MissionGenericSpawns, lists, ParseMissionSpawnList) },
	{ "", 0, 0 }
};

SHARED_MEMORY MissionGenericSpawns g_genericspawns = {0};
SHARED_MEMORY MissionGenericSpawns g_genericHostageSpawns = {0};

void MissionSpawnPreload()
{
	int flags = PARSER_SERVERONLY;
	if (g_genericspawns.lists) {
		flags = PARSER_SERVERONLY | PARSER_FORCEREBUILD;
		ParserUnload(ParseMissionGenericSpawns, &g_genericspawns, sizeof(g_genericspawns));
	}
	if (!ParserLoadFilesShared("SM_MissionSpawns.bin", SCRIPTS_DIR, ".missionspawns", "missionspawns.bin", flags, ParseMissionGenericSpawns, &g_genericspawns, sizeof(g_genericspawns), NULL, NULL, NULL, NULL, NULL))
	{
		Errorf("Error loading mission spawn list");
	}
	if (g_genericHostageSpawns.lists) {
		flags = PARSER_SERVERONLY | PARSER_FORCEREBUILD;
		ParserUnload(ParseMissionGenericSpawns, &g_genericHostageSpawns, sizeof(g_genericHostageSpawns));
	}
	if (!ParserLoadFilesShared("SM_MissionHostageSpawns.bin", SCRIPTS_DIR, ".missionhostagespawns", "missionhostagespawns.bin", flags, ParseMissionGenericSpawns, &g_genericHostageSpawns, sizeof(g_genericHostageSpawns), NULL, NULL, NULL, NULL, NULL))
	{
		Errorf("Error loading hostage spawn list");
	}
}

static void MissionEncounterVerifySpawnLists(const MissionSpawnList** lists, char* type)
{
	int l, nlists, v, nlevels, d, ndefs;

	// verify all the spawndefs exist
	nlists = eaSize(&lists);
	for (l = 0; l < nlists; l++)
	{
		const MissionSpawnList* list = lists[l];
		nlevels = eaSize(&list->levels);
		for (v = 0; v < nlevels; v++)
		{
			const MissionSpawnLevels* level = list->levels[v];
			ndefs = eaSize(&level->spawndefs);
			for (d = 0; d < ndefs; d++)
			{
				if (!SpawnDefFromFilename(level->spawndefs[d]))
				{
					ErrorFilenamef(list->filename, "Invalid %s specified: %s", type, level->spawndefs[d]);
				}
			}
		}
	}
}

// called after encounters have loaded.  verify our spawndef references
void MissionEncounterVerify()
{
	MissionEncounterVerifySpawnLists(g_genericspawns.lists, "mission spawns");
	MissionEncounterVerifySpawnLists(g_genericHostageSpawns.lists, "mission hostage spawns");
}

#define MAX_MISSIONSPAWN_FIND 1000

static const SpawnDef* MissionSpawnGetFromList(const MissionSpawnList* list, int level)
{
	const char* spawndefs[MAX_MISSIONSPAWN_FIND];
	int numfound = 0;
	int v, numlevels, s, numspawns;

	// add up all the spawndefs I could have
	numlevels = eaSize(&list->levels);
	for (v = 0; v < numlevels; v++)
	{
		const MissionSpawnLevels* levels = list->levels[v];
		if (levels->minlevel > level) continue;
		if (levels->maxlevel < level) continue;
		numspawns = eaSize(&levels->spawndefs);
		for (s = 0; s < numspawns; s++)
		{
			if (numfound == MAX_MISSIONSPAWN_FIND) break;
			spawndefs[numfound++] = levels->spawndefs[s];
		}
	}

	if(!numfound)
	{
		//log_printf("bug", "\n(Server Error Msg)\nServer Ver:%s\nMissionSpawnGet: unable to find spawn defs in %s for VG %d for level %d\n@@END\n\n\n\n\n\n\n\n\n", getExecutablePatchVersion(), list->filename, list->villaingroup, level);
		dbLogBug("\n(Server Error Msg)\nServer Ver:%s\nMissionSpawnGet: unable to find spawn defs in %s for VG %d for level %d\n@@END\n\n\n\n\n\n\n\n\n", getExecutablePatchVersion("CoH Server"), list->filename, list->villaingroup, level);
		return NULL;
	}

	s = saSeedRoll(time(NULL), numfound);
	return SpawnDefFromFilename(spawndefs[s]);
}

static const SpawnDef* MissionSpawnGetFromLists(const MissionSpawnList** lists, VillainGroupEnum group, int level)
{
	const MissionSpawnList* generic = NULL;
	const MissionSpawnList* list = NULL;
	const SpawnDef *def;
	int i, n;

	// select the correct list, find generic at the same time
	n = eaSize(&lists);
	for (i = 0; i < n; i++)
	{
		const MissionSpawnList* cur = lists[i];
		if (cur->villaingroup == group)
		{
			list = cur;
		}
		else if (cur->villaingroup == VG_NONE)
		{
			generic = cur;
		}
	}

	// if we don't have the specific group use the generic one
	if (!list) list = generic;
	assert(list && "MissionSpawnGet: don't have a generic spawn list!");

	// If the list doesn't have any spawns for this level, fail back to
	// the generic list of spawns.
	if(!(def = MissionSpawnGetFromList(list, level)))
	{
		def = MissionSpawnGetFromList(generic, level);
	}

	assertmsg(def, "MissionSpawnGet: found empty list of spawns!");

	return def;
}

// return a spawndef for the given villain group, randomly selects from the appropriate list,
// returns a result from the generic list if the villain group doesn't have any specific
// spawndefs.
const SpawnDef* MissionSpawnGet(VillainGroupEnum group, int level)
{
	return MissionSpawnGetFromLists(g_genericspawns.lists, group, level);
}
const SpawnDef* MissionHostageSpawnGet(VillainGroupEnum group, int level)
{
	return MissionSpawnGetFromLists(g_genericHostageSpawns.lists, group, level);
}

// *********************************************************************************
//  Mission doors
// *********************************************************************************


// Any big change to how doors are assigned show also be applied to MissionDoorListFromTask as well
// for development only - give AssignMissionDoor failover modes, so you can always get assigned a door
#define MISSIONDOOR_FINDDEFAULT				0
#define MISSIONDOOR_WHEREISSUEDIMPOSSIBLE	1
#define MISSIONDOOR_IGNORESAMEDOOR			2
#define MISSIONDOOR_IGNOREMAP				3
#define MISSIONDOOR_IGNOREALL				4

#define CITYZONETYPE_ANY	0
#define CITYZONETYPE_WHEREISSUED 1
#define CITYZONETYPE_WHEREISSUEDIFPOSSIBLE 2
#define CITYZONETYPE_ANYPRIMALHERO 3
#define CITYZONETYPE_ANYPRIMALVILLAIN 4
#define CITYZONETYPE_MAPSPECIFIC 5

int AssignMissionDoorImpl(StoryInfo* info, StoryTaskInfo* task, int findtype, MissionsAllowed allowedByPlayerType)
{
	int numdoors, doorsel, i;
	DoorEntry* doorlist;
	const MissionDef* missionDef;
	DoorEntry* selectedDoor = NULL;
	static DoorEntry** candidates = NULL;
	const ContactDef* contactdef = ContactDefinition(TaskGetContactHandle(info, task));
	const char* tasktype = StaticDefineIntRevLookup(ParseMapSetEnum, task->mapSet);
	int level = CONTACT_IS_TASKFORCE(contactdef)?contactdef->minPlayerLevel:task->level;
	int baseMapId;
	int cityZoneType = CITYZONETYPE_ANY;

	// Parameter sanity check.
	if(!task)
		return 0;

	missionDef = task->def->missionref;
	if(!missionDef && task->def->missiondef)
		missionDef = task->def->missiondef[0];
	if(!missionDef || !missionDef->doortype)
		return 0;
	if (missionDef->doortype && strnicmp(missionDef->doortype, "ZoneTransfer", 12 /*strlen("ZoneTransfer")*/)==0)
		return 1; // don't need to assign a door number

	//task->def useRaidTeleporter would be a better flag, but right now this is easier
	//All SG missions currently go to the raid teleporter
	if ( TASK_IS_SGMISSION(task->def) )
		return 1;

	if (missionDef->cityzone)
	{
		cityZoneType = CITYZONETYPE_MAPSPECIFIC;
		if (!stricmp(missionDef->cityzone, "WhereIssued"))
			cityZoneType = CITYZONETYPE_WHEREISSUED;
		else if (!stricmp(missionDef->cityzone, "WhereIssuedIfPossible"))
			cityZoneType = CITYZONETYPE_WHEREISSUEDIFPOSSIBLE;
		else if (!stricmp(missionDef->cityzone, "AnyPrimalHero"))
			cityZoneType = CITYZONETYPE_ANYPRIMALHERO;
		else if (!stricmp(missionDef->cityzone, "AnyPrimalVillain"))
			cityZoneType = CITYZONETYPE_ANYPRIMALVILLAIN;
	}

	// otherwise, figure out which doors match our requirements.
	if (!selectedDoor)
	{
		eaSetSize(&candidates, 0);
		doorlist = dbGetTrackedDoors(&numdoors);
		for(i = 0; i < numdoors; i++)
		{
			StaticMapInfo* mapInfo;
			const MapSpec* mapSpec;

			// Is this a mission door?
			if (doorlist[i].type != DOORTYPE_MISSIONLOCATION)			
				continue;

			// is this a noMission mission door?
			if (doorlist[i].noMissions)			
				continue;

			// Does this player have this mission door assigned already?
			if (findtype < MISSIONDOOR_IGNORESAMEDOOR && info)
			{
				int t, numtasks = eaSize(&info->tasks);
				int collision = 0;
				for (t = 0; t < numtasks; t++)
				{
					StoryTaskInfo* other = info->tasks[t];
					if (other->doorId == doorlist[i].db_id)
						collision = 1;
				}
				if (collision) 
					continue;
			}

			// Can this door be used for the specified mission type?
			// NoFallbackMissions requires this always be true for special doors (like the Ski Chalet doors)
			if (findtype < MISSIONDOOR_IGNOREALL || doorlist[i].noFallbackMissions)
			{
				char buf[2000];
				char* types[200];
				int d, found = 0;

				if (!doorlist[i].name)
					continue;
				strcpy(buf, doorlist[i].name);
				tokenize_line(buf, types, NULL);
				for (d = 0; types[d]; d++)
				{
					const char* doorType = missionDef->doortype;
					if (doorType && !stricmp(doorType, "Random"))
					{
						if (MapSetAllowsRandomDoorType(task->mapSet, types[d]))
						{
							found = 1;
							break;
						}
						doorType = tasktype;
					}
					if (doorType && !stricmp(types[d], doorType))
					{
						found = 1;
						break;
					}
				}
				if (!found)
					continue;
			}

			// Get the static map info and map spec
			baseMapId = db_state.local_server?staticMapInfoMatchNames(saUtilCurrentMapFile()):doorlist[i].map_id;
			mapInfo = staticMapInfoFind(baseMapId);
			mapSpec = mapInfo?MapSpecFromMapId(mapInfo->mapID):NULL;
				
			// does the map not allow any missions?
			if (mapSpec && mapSpec->missionsAllowed == MA_NONE)
				continue;

			// Newspaper tasks need to be on the same map that the task was issued from
			if ((CONTACT_IS_NEWSPAPER(contactdef) && findtype < MISSIONDOOR_IGNOREALL)
				|| findtype >= MISSIONDOOR_IGNOREMAP)
			{
				if (doorlist[i].map_id != db_state.base_map_id)
					continue;
			}
			// do we need to get a mission on this map?
			else if (((cityZoneType == CITYZONETYPE_WHEREISSUED) // WhereIssued forces the mission door to be on this map, no matter what
							 || ((cityZoneType == CITYZONETYPE_WHEREISSUEDIFPOSSIBLE) && findtype < MISSIONDOOR_WHEREISSUEDIMPOSSIBLE)))
			{
				if (doorlist[i].map_id != db_state.base_map_id) 
					continue;
			}
			// or a specific set of maps?
			else if (cityZoneType == CITYZONETYPE_ANYPRIMALHERO)
			{
				if (!mapSpec || !MAP_ALLOWS_PRIMAL_ONLY(mapSpec) || !MAP_ALLOWS_HEROES_ONLY(mapSpec) || !MapSpecCanHaveMission(mapSpec, level))
				{
					continue;
				}
				if ((mapSpec->missionsAllowed != MA_ALL) && (mapSpec->missionsAllowed != MA_HERO))
				{
					continue;
				}
			}
			else if (cityZoneType == CITYZONETYPE_ANYPRIMALVILLAIN)
			{
				if (!mapSpec || !MAP_ALLOWS_PRIMAL_ONLY(mapSpec) || !MAP_ALLOWS_VILLAINS_ONLY(mapSpec) || !MapSpecCanHaveMission(mapSpec, level))
				{
					continue;
				}
				if ((mapSpec->missionsAllowed != MA_ALL) && (mapSpec->missionsAllowed != MA_VILLAIN))
				{
					continue;
				}

			}
			// or a specific map?
			else if (cityZoneType == CITYZONETYPE_MAPSPECIFIC)
			{
				if (mapInfo)
				{
					char mapname[MAX_PATH] = {0};
					saUtilFilePrefix(mapname, mapInfo->name);		//	stealing this function. strips extension and file path
					if (stricmp(mapname, missionDef->cityzone))
						continue;
				}
				else
					continue;
			}
			// otherwise, just use level requirements to choose any map
			else
			{
				if(mapSpec && !MapSpecCanHaveMission(mapSpec, level))
					continue;
			}

			if (	((cityZoneType == CITYZONETYPE_ANY)  || (cityZoneType == CITYZONETYPE_WHEREISSUEDIFPOSSIBLE) || 
					 (cityZoneType == CITYZONETYPE_ANYPRIMALHERO) || (cityZoneType == CITYZONETYPE_ANYPRIMALVILLAIN))
				&& mapSpec && mapSpec->mapfile && !strnicmp(mapSpec->mapfile, "P_", 2))
			{
				continue;
			}

			if (((cityZoneType == CITYZONETYPE_WHEREISSUEDIFPOSSIBLE) || 
				(cityZoneType == CITYZONETYPE_ANYPRIMALHERO) || (cityZoneType == CITYZONETYPE_ANYPRIMALVILLAIN))
				&& mapSpec && mapSpec->mapfile
				&& (!strnicmp(mapSpec->mapfile, "Trial_", 6) || !strnicmp(mapSpec->mapfile, "Hazard_", 7)
					|| !strnicmp(mapSpec->mapfile, "V_PvP_", 6) || !strnicmp(mapSpec->mapfile, "V_Trial_", 8)))
			{
				continue;
			}

			// Only allow the appropriate types of missions as specified per map
			if (findtype < MISSIONDOOR_IGNOREALL)
			{
				//	if designer specifies a zone, don't cull out the doors based on the player alignment
				//	it is now up to them to make sure the contacts don't allow the missions under bad circumstances
				if ((cityZoneType != CITYZONETYPE_MAPSPECIFIC) && (cityZoneType != CITYZONETYPE_ANYPRIMALHERO) &&
					(cityZoneType != CITYZONETYPE_ANYPRIMALVILLAIN))
				{
					if (!mapSpec || 
						(mapSpec->missionsAllowed != MA_ALL && mapSpec->missionsAllowed != allowedByPlayerType))
					{
						continue;
					}
				}
			}
			
			eaPush(&candidates, &doorlist[i]);
		}

		if (eaSize(&candidates))
		{
			// Pick a door.
			doorsel = saRoll(eaSize(&candidates));
			selectedDoor = candidates[doorsel];
		}
	}

	if (!selectedDoor)
	{
		// even on production servers, we just keep trying until we get a door
		if (findtype < MISSIONDOOR_IGNOREALL)
		{
			if (findtype == MISSIONDOOR_IGNORESAMEDOOR)
			{
				log_printf("storyerrs.log", "AssignMissionDoor: had to ignore requirements to get a door for task %s on map %i(base: %i), mapset: %s",
					task->def->logicalname, db_state.map_id, db_state.base_map_id, tasktype);
			}
			return AssignMissionDoorImpl(info, task, findtype + 1, allowedByPlayerType);
		}
		if (!db_state.local_server)
		{
			Errorf("mission.c: Couldn't match a mission door with the requirements for task %s", task->def->logicalname);
			log_printf("storyerrs.log", "AssignMissionDoor: couldn't allocate mission door at all for task %s!", 
				task->def->logicalname);
		}
		return 0;
	}
	else
	{
		// set up the player's info
		task->doorMapId = selectedDoor->map_id;
		task->doorId = selectedDoor->db_id;
		copyVec3(selectedDoor->mat[3], task->doorPos);
		return 1;
	}
}

int AssignMissionDoor(StoryInfo* info, StoryTaskInfo* task, int isVillain)
{
	return AssignMissionDoorImpl(info, task, MISSIONDOOR_FINDDEFAULT, (isVillain)?MA_VILLAIN:MA_HERO);
}

void MissionDoorListFromTask(ClientLink* client, Entity* player, char* taskname)
{
	int i, k, numdoors;
	static DoorEntry** candidates = NULL;
	const ContactDef** contacts = g_contactlist.contactdefs;
	const StoryTask* task = NULL;
	const MissionDef* missiondef = NULL;
	DoorEntry* doorlist;
	const char* doorType = NULL;

	// Find the task and and appropriate contact
	for (i = 0; i < eaSize(&contacts); i++)
	{
		for(k = 0; k < eaSize(&contacts[i]->tasks); k++)
		{
			if (contacts[i]->tasks[k]->logicalname && !stricmp(contacts[i]->tasks[k]->logicalname, taskname))
			{
				task = contacts[i]->tasks[k];
				missiondef = task->missionref;
				if (!missiondef && task->missiondef)
					missiondef = task->missiondef[0];
				doorType = missiondef?missiondef->doortype:0;
				break;
			}
		}
	}

	if (!doorType)
	{
		doorType = taskname;
		conPrintf(client, saUtilLocalize(0, "SearchDoorType", doorType));
	}
	else
	{
		conPrintf(client, saUtilLocalize(0, "SearchTaskDoor", taskname));
	}

	doorlist = dbGetLocalDoors(&numdoors);
	eaSetSize(&candidates, 0);
	for(i = 0; i < numdoors; i++)
	{
	
		char buf[2000];
		char* types[200];
		int d, found = 0;

		if (doorlist[i].type != DOORTYPE_MISSIONLOCATION || !doorlist[i].name || doorlist[i].noMissions)
			continue;

		strcpy(buf, doorlist[i].name);
		tokenize_line(buf, types, NULL);
		for (d = 0; types[d]; d++)
		{
			if (!stricmp(types[d], doorType))
			{
				found = 1;
				break;
			}
		}
		if (found)
			eaPush(&candidates, &doorlist[i]);
	}

	if (eaSize(&candidates))
	{
		START_PACKET(pak, player, SERVER_DEBUG_LOCATION);
		pktSendBits(pak, 1, 1); // Clear old doors
		pktSendBitsPack(pak, 1, eaSize(&candidates));
		for (i = 0; i < eaSize(&candidates); i++)
		{
			DoorEntry* door = candidates[i];
			pktSendF32(pak, door->mat[3][0]);
			pktSendF32(pak, door->mat[3][1]);
			pktSendF32(pak, door->mat[3][2]);
			pktSendBits(pak, 32, 0xaaaaaa33ff);
		}
		END_PACKET
	}
	else
	{
		conPrintf(client, saUtilLocalize(0, "BadDoor"));
		START_PACKET(pak, player, SERVER_DEBUG_LOCATION);
		pktSendBits(pak, 1, 1); // Clear old doors
		pktSendBitsPack(pak, 1, 0);
		END_PACKET
	}
}

// looks at spec files to see if we can get this door at this level
int VerifyCanAssignMissionDoor(const char* doortype, int level)
{
	int i, n;
	int found = 0;
	n = eaSize(&mapSpecList.mapSpecs);
	for (i = 0; i < n; i++)
	{
		const MapSpec* spec = mapSpecList.mapSpecs[i];
		if (!MapSpecCanHaveMission(spec, level)) 
			continue;
		found = StringArrayFind(spec->missionDoorTypes, doortype);
		if (found) 
			break;
	} // for each map
	return found;
}

// iterate through all maps until a match is found (if any)
int DebugMissionMapMeetsRequirements(char * mapname, int mapset, int maptime, char ** fullName)
{
	int i, nmaps;
	MissionMap** maps = MissionMapGetList( mapset, maptime );

	*fullName = 0;
	nmaps = eaSize(&maps);
	if (!mapname || !nmaps )
		return 0;

	for (i = 0; i < nmaps; i++)
	{
		// try full and partial matches
		char buf[1000];
		saUtilFilePrefix(buf, maps[i]->mapfile);
		if( !stricmp(mapname, maps[i]->mapfile)	|| !stricmp(mapname, buf) )
		{
			// found it!
			*fullName = maps[i]->mapfile;
			return 1;
		}
	}

	// didn't find one that matched
	return 0;
}


