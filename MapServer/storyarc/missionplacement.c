/*\
 *
 *	missionplacement.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	handles rules for placing encounters and objectives on a mission map
 *	deals mainly with logical rooms, and then random placement within those
 *	rooms
 *
 */

#include "mission.h"
#include "storyarcprivate.h"
#include "NpcServer.h"
#include "entai.h"
#include "StringCache.h"
#include "groupnetsend.h"
#include "dbcomm.h"
#include "entity.h"
#include "skillobj.h"
#include "logcomm.h"

// *********************************************************************************
//  General placement functions
// *********************************************************************************

int g_objectiveroomrotor = 1;

static int PlacementSelectInner(int* possiblerooms, MissionPlaceEnum place)
{
	int* rooms;
	int p, nump = eaiSize(&possiblerooms);
	int sel = -1;

	if (place == MISSION_OBJECTIVE) // uses a global rotor to select different rooms in the mission
	{
		// have to check each objective number starting from the global rotor
		int rotor, firsttime;
		for (rotor = g_objectiveroomrotor, firsttime = 1;
			 firsttime || rotor != g_objectiveroomrotor;
			 rotor = rotor % MAX_OBJECTIVE_MARKER + 1)
		{
			firsttime = 0; // completed loop at least once
			eaiCreate(&rooms);
			for (p = 0; p < nump; p++)
			{
				RoomInfo* testroom = g_rooms[possiblerooms[p]];
				int o, numo = eaiSize(&testroom->objectiveMarkers);
				for (o = 0; o < numo; o++)
				{
					if (testroom->objectiveMarkers[o] == rotor)
					{
						eaiPush(&rooms, possiblerooms[p]);
						break;
					}
				}
			} // each room in possiblerooms;

			// ok, we have some rooms at this objective number
			if (eaiSize(&rooms))
			{
				sel = rand() % eaiSize(&rooms);
				sel = rooms[sel];
				eaiDestroy(&rooms);
				g_objectiveroomrotor = rotor % MAX_OBJECTIVE_MARKER + 1;
				return sel;
			}
			eaiDestroy(&rooms);
		}
		return -1;
	}
	else if (place == MISSION_FRONT || place == MISSION_MIDDLE || place == MISSION_BACK || place == MISSION_LOBBY)
	{
		eaiCreate(&rooms);
		for (p = 0; p < nump; p++)
		{
			RoomInfo* testroom = g_rooms[possiblerooms[p]];
			if (place == MISSION_FRONT && !testroom->startroom) continue;
			if (place == MISSION_BACK && !testroom->endroom) continue;
			if (place == MISSION_MIDDLE && (testroom->startroom || testroom->endroom)) continue;
			if ((place == MISSION_LOBBY) ^ (testroom->lobbyroom)) continue;
			eaiPush(&rooms, possiblerooms[p]);
		}
		if (eaiSize(&rooms))
		{
			sel = rand() % eaiSize(&rooms);
			sel = rooms[sel];
		}
		eaiDestroy(&rooms);
		return sel;
	}
	else // any room
	{
		if (eaiSize(&possiblerooms))
		{
			sel = rand() % eaiSize(&possiblerooms);
			sel = possiblerooms[sel];
		}
		return sel;
	}
}

static int MissionPlacementRoomSelect(int* possiblerooms, MissionPlaceEnum place)
{
	int room = PlacementSelectInner(possiblerooms, place);
	if (room == -1)
	{
		log_printf("storyerrs.log", "I had to fail over to placing at MISSION_ANY");
		room = PlacementSelectInner(possiblerooms, MISSION_ANY);
	}
	return room;
}

// Weird little specialty function:  Used to place objectives, encounters, hostages in rooms.
// Assumes that each room has n locations that may be suitable for placing this something, and
// must be picked randomly
static int PlaceSomethingInRoom(void* something, MissionPlaceEnum place,
		int (*numPositionsInRoom)(RoomInfo* room),
		int (*thisPositionOk)(RoomInfo* room, int pos, void* something),
		int (*placeIt)(RoomInfo* room, int pos, void* something))
{
	int* possiblerooms;
	int r, numr;
	int s, nums, sels;
	int room, placed = 0;

	if (place == MISSION_REPLACEGENERIC)
		return 0;

	// look first at every room and see which ones can be placed in
	eaiCreate(&possiblerooms);
	numr = eaSize(&g_rooms);
	for (r = 1; r < numr; r++)			// don't allow placement in NULL room
	{
		int found = 0;
		nums = numPositionsInRoom(g_rooms[r]);
		for (s = 0; s < nums; s++)
		{
			if (thisPositionOk(g_rooms[r], s, something))
			{
				found = 1;
				break;
			}
		}
		if (found)
		{
			//	don't add a lobby room as possible unless it is called for.
			if ((place == MISSION_LOBBY) ^ !g_rooms[r]->lobbyroom)
				eaiPush(&possiblerooms, r);
		}
	}

	// then narrow according to our placement type
	if (!eaiSize(&possiblerooms))
	{
		eaiDestroy(&possiblerooms);
		return 0;
	}
	room = MissionPlacementRoomSelect(possiblerooms, place);
	if (room == -1)
	{
		eaiDestroy(&possiblerooms);
		return 0;
	}

	// now make an array of correct locations in this room
	nums = numPositionsInRoom(g_rooms[room]);
	eaiSetSize(&possiblerooms, 0);
	for (s = 0; s < nums; s++)
	{
		if (thisPositionOk(g_rooms[room], s, something))
			eaiPush(&possiblerooms, s);
	}
	nums = eaiSize(&possiblerooms);
	if (!nums)
	{
		LOG_OLD_ERR("Internal error in PlaceSomethingInRoom");
		return 0;
	}

	// finally, do the placement
	sels = rand() % nums;
	placed = placeIt(g_rooms[room], possiblerooms[sels], something);
	eaiDestroy(&possiblerooms);
	return placed;
}

// *********************************************************************************
//  Objective Placement
// *********************************************************************************

static int ObjectivePlaceHostage(const MissionObjective* objective, EncounterGroup* group)
{
	const SpawnDef* spawnDef;

	spawnDef = MissionHostageSpawnGet(g_activemission->missionGroup, g_activemission->missionLevel);
	if(!spawnDef)
		return 0;

	group->missionspawn = spawnDef;
	group->conquered = 0;
	MissionObjectiveAttachEncounter(objective, group, spawnDef);
	return 1;
}

static int ObjectivePlaceItem(const MissionObjective* objective, ItemPosition* pos)
{
	Entity* e = NULL;
	MissionObjectiveInfo* info;

	// Create an entity.
	if (objective->model)
	{
		if(objective->skillCheck)
		{
			e = villainCreateByName(objective->model, g_activemission->missionLevel, NULL, false, NULL, 0, NULL);
		}
		else
		{
			e = npcCreate(ScriptVarsTableLookup(&g_activemission->vars, objective->model), ENTTYPE_MISSION_ITEM);
			// Next line is redundant, I think. -- poz
			if(e) 
				SET_ENTTYPE(e) = ENTTYPE_MISSION_ITEM;

			//If the model isn't an NPC, then it must be a villainDef
			if( !e )
			{
				e = villainCreateByName(objective->model, g_activemission->missionLevel, NULL, false, NULL, 0, NULL);
				if( e )
					e->notAttackedByJustAnyone = 1;
			}
		}
	}

	if(!e)
	{
		if( objective->skillCheck  )
			ErrorFilenamef(g_activemission->def->filename, "Invalid villainDef specified %s specified for objective %s, recovering.  Looking for villaindef, not costume because this objective is a skillcheck", objective->model, objective->name);
		else
			ErrorFilenamef(g_activemission->def->filename, "Invalid costume or villainDef specified %s specified for objective %s, recovering", objective->model, objective->name);
		return 1;  // need to pretend we placed it since we can't recover
	}

	// set the position
	entUpdateMat4Instantaneous(e, pos->pos);
	pos->used = 1;

	// Attach mission objective information.
	info = MissionObjectiveInfoCreate();
	info->def = objective;
	info->name = strdup(objective->name);
	info->optional = MissionObjectiveIsSideMission(objective);

	e->missionObjective = info;
	info->objectiveEntity = erGetRef(e);

	// Assign the item a name
	if (objective->modelDisplayName)
	{
		const char* name = saUtilTableLocalize(objective->modelDisplayName, &g_activemission->vars);
		strncpy(e->name, name, MAX_NAME_LEN);
		e->name[MAX_NAME_LEN-1] = 0;
	}

	eaPush(&objectiveInfoList, info);
	MissionWaypointCreate(info);

	//If the object has a requires, (and we don't meet the requirement) it doesn't start out glowing.
	if (info->def && info->def->activateRequires && !(MissionObjectiveExpressionEval(info->def->activateRequires)))
	{
		MissionItemSetState(info, MOS_REQUIRES);	//Initialized, but not glowing (yet)
	} else {
		MissionItemSetState(info, MOS_INITIALIZED);
	}


	aiInit(e, NULL);

	return 1;
}

// hooks for PlaceSomethingInRoom - mission objectives
static int numItemsInRoom(RoomInfo* room)
{
	return eaSize(&room->items);
}
static int itemPositionOk(RoomInfo* room, int pos, const MissionObjective* objective)
{
	if (room->items[pos]->used) 
		return 0;
	if (room->items[pos]->loctype != objective->locationtype) 
		return 0;
	if (objective->locationName || (room->items[pos]->customName && !g_ArchitectTaskForce) )
	{
		if( !objective->locationName || !room->items[pos]->customName || 0 != stricmp( objective->locationName, room->items[pos]->customName ) )
			return 0;
	}
	return 1;
}
static int placeItem(RoomInfo* room, int pos, const MissionObjective* objective)
{
	return ObjectivePlaceItem(objective, room->items[pos]);
}

// hooks for PlaceSomethingInRoom - hostages
static int numHostagesInRoom(RoomInfo* room)
{
	return eaSize(&room->hostageEncounters);
}
static int hostagePositionOk(RoomInfo* room, int pos, const MissionObjective* objective)
{
	if (room->hostageEncounters[pos]->missionspawn) 
		return 0;

	if (objective->locationName || room->hostageEncounters[pos]->groupname)
	{
		if( !objective->locationName || !room->hostageEncounters[pos]->groupname || 0 != stricmp( objective->locationName, room->hostageEncounters[pos]->groupname ) )
			return 0;
	}

	return 1;
}
static int placeHostage(RoomInfo* room, int pos, const MissionObjective* objective)
{
	return ObjectivePlaceHostage(objective, room->hostageEncounters[pos]);
}

void ObjectiveDoPlacement()
{
	int i;
	int j;
	int objectiveCount = eaSize(&g_activemission->def->objectives);

	// For each objective to be placed in the mission
	for(i = 0; i < objectiveCount; i++)
	{
		const MissionObjective* objective = g_activemission->def->objectives[i];

		if (!MissionObjectiveIsEnabled(objective))
			continue;

		for(j = 0; j < objective->count; j++)
		{
			if (objective->locationtype != LOCATION_PERSON)
			{
				if (!PlaceSomethingInRoom((void*)objective, objective->room, numItemsInRoom, itemPositionOk, placeItem))
				{
					ErrorFilenamef(g_activemission->def->filename, "Unable to pick a location to place mission objective");					
				}
			}
			else if (!PlaceSomethingInRoom((void*)objective, objective->room, numHostagesInRoom, hostagePositionOk, placeHostage))
			{
				ErrorFilenamef(g_activemission->def->filename, "Unable to pick an encounter to place hostage mission objective");
			}
		}
	}
}

// *********************************************************************************
//  Skill Placement
// *********************************************************************************

Entity **g_SkillObjects;

static int itemPositionOkForSkill(RoomInfo* room, int pos, void* o)
{
	if (room->items[pos]->used) return 0;
	if (room->items[pos]->loctype != LOCATION_ITEMFLOOR) return 0;
	return 1;
}

static int placeSkill(RoomInfo* room, int pos, const char *pchDef)
{
	Entity *e;

	if(!g_SkillObjects)
	{
		eaCreate(&g_SkillObjects);
	}

	// verbose_printf("Creating skill object: '%s' at level %d\n", pchDef, g_activemission->skillLevel);
	e = villainCreateByName(pchDef, g_activemission->skillLevel, NULL, false, NULL, 0, NULL);
	if(!e)
	{
		ErrorFilenamef(g_activemission->def->filename, "Invalid villainDef specified %s specified for skill.", pchDef);
	}
	else
	{
		// set the position
		entUpdateMat4Instantaneous(e, room->items[pos]->pos);
		room->items[pos]->used = 1;

		eaPush(&g_SkillObjects, e);

		aiInit(e, NULL);
	}

	return 1;
}

void SkillDoPlacement(void)
{
	const char * const **pppchSkills;
	int iNumSkills;

	// TODO: HACK: Read this info from a config file.
	static int aLocations[] = { MISSION_MIDDLE, MISSION_FRONT, MISSION_BACK, MISSION_LOBBY };
	static int aMaxCount[] = { 1, 2, 3 };

	// There are no skills in missions under level 15.
	if(g_activemission->missionLevel<15)
		return;

	// Determine the max number of checks in the mission based on the 
	// map length.
	if(g_activemission->def->maplength>0 && g_activemission->def->maplength<=30)
		iNumSkills = aMaxCount[0];
	else if(g_activemission->def->maplength<=45)
		iNumSkills = aMaxCount[1];
	else
		iNumSkills = aMaxCount[2];

	// There can be one less than the max count, but there's always at least 
	// one.
	saSeed(g_activemission->seed); // just seed it
	iNumSkills += saRoll(2);

	if(iNumSkills>0)
	{
		pppchSkills = skillobj_GetPossibleSkillObjectives(db_state.map_name, villainGroupGetName(g_activemission->missionGroup));
		if(pppchSkills && eaSize(pppchSkills)>0)
		{
			// Choose a starting point in the list and then use them in order.
			// This is a cheesy but effective way to avoid duplicate objects.
			int nCnt = eaSize(pppchSkills);
			int idx = saRoll(nCnt);

			// Go through the list of objects three times before giving up. 
			//   I go through the list more than once in case there's only a
			//   single match. It'll find that match again the next time
			//   through. This is actually an uncommon case and an edge
			//   condition.
			int n = nCnt*3;

			while(iNumSkills>0 && n>0)
			{
				const VillainDef* def;

				idx %= nCnt;

				def = villainFindByName((*pppchSkills)[idx]);
				if(def)
				{
					// Can the chosen villain def support this skill level?
					if(def->levels[0]->level <= g_activemission->skillLevel
						&& def->levels[eaSize(&def->levels)-1]->level >= g_activemission->skillLevel)
					{
						if(!PlaceSomethingInRoom((void*)(*pppchSkills)[idx], 
							aLocations[(sizeof(aLocations)-iNumSkills)%sizeof(aLocations)], 
							numItemsInRoom, itemPositionOkForSkill, 
							placeSkill))
						{
							Errorf("Unable to place skill object in room.");
						}

						iNumSkills--;
					}
					else
					{
						// verbose_printf("'%s' has levels %d to %d, looking for %d. Skipping.\n",
						// 	(*pppchSkills)[idx],
						// 	def->levels[0]->level,
						// 	def->levels[eaSize(&def->levels)-1]->level,
						// 	g_activemission->skillLevel);
					}
				}

				idx++;
				n--;
			}

			if(iNumSkills>0)
			{
				verbose_printf("Unable to find enough skill objects matching the skill level (%d). Giving up.\n", g_activemission->skillLevel);
			}
		}
		else
		{
			// ErrorFilenamef("defs/mapobjectives.def", "No Skill Objectives found for map %s and/or group %s\n", db_state.map_name, villainGroupGetName(g_activemission->missionGroup)?villainGroupGetName(g_activemission->missionGroup):"(null)");
		}
	}
}

// *********************************************************************************
//  Encounter Placement
// *********************************************************************************

// hooks for PlaceSomethingInRoom - spawns
typedef struct SpawnPlaceInfo
{
	const SpawnDef* spawndef;
	const char* spawntype;
} SpawnPlaceInfo;

static const char* GetSpawnType(const SpawnDef* spawndef, const MissionDef* mission)
{
	if (spawndef->spawntype)
	{
		ScriptVarsTable vars = {0};
		ScriptVarsTablePushScope(&vars, &mission->vs);
		ScriptVarsTablePushScope(&vars, &spawndef->vs);
		return ScriptVarsTableLookup(&vars, spawndef->spawntype);
	}
	else
		return "Mission";
}
static int numEncountersInRoom(RoomInfo* room)
{
	return eaSize(&room->standardEncounters);
}
static int thisEncounterOk(RoomInfo* room, int pos, void* s)
{
	SpawnPlaceInfo* info = (SpawnPlaceInfo*)s;
	if (room->standardEncounters[pos]->missionspawn) return 0;
//	if (info && info->spawndef && (room->standardEncounters[pos]->spawnOccupiedBy & (1 << info->spawndef->playerCreatedDetailType)) )	return 0;
	//Skip this test if not a player created map.
	if (info && info->spawndef && info->spawndef->playerCreatedDetailType && (room->standardEncounters[pos]->spawnOccupiedBy & (1 << info->spawndef->playerCreatedDetailType)) )	return 0;
	if (EncounterGetMatchingPoint(room->standardEncounters[pos], info->spawntype) == -1) return 0;
	return 1;
}
static int placeSpawn(RoomInfo* room, int pos, void* s)
{
	SpawnPlaceInfo* info = (SpawnPlaceInfo*)s;
	EncounterGroup* encounter = room->standardEncounters[pos];
	int point = EncounterGetMatchingPoint(encounter, info->spawntype);
	if (point == -1) 
		return 0;

	if (info->spawndef->playerCreatedDetailType)
	{
		//	usually 0, except when on player created map.
		//	on player created missions, the spawn needs to be marked for its type so
		//	that we don't double up on a room's encounter
		encounter->spawnOccupiedBy |= 1 << info->spawndef->playerCreatedDetailType;
	}
	// check if we need to clone this encounter group
	if (info->spawndef->flags & SPAWNDEF_CLONEGROUP)
		encounter = EncounterGroupClone(encounter);

	// attach
	encounter->missionspawn = info->spawndef;

	// make sure the group is set to go off exactly once
	if( encounter->missionspawn && (encounter->missionspawn->flags & SPAWNDEF_MISSIONRESPAWN ) )
	{ 
		encounter->singleuse = 0;
	} 
	else 
	{
		encounter->singleuse = 1;
	}
	encounter->conquered = 0;
	encounter->spawnprob = 100;
	encounter->time_start = 0.0;
	encounter->time_duration = 24.0;
	if( encounter->missionspawn && (encounter->missionspawn->flags & SPAWNDEF_AUTOSTART ) )
		encounter->autostart_time = 1;
	if (info->spawndef->objective)
		MissionObjectiveAttachEncounter(info->spawndef->objective[0], encounter, info->spawndef);
	return 1;
}

// place all encounters specified in the mission into rooms and
// specific encounter groups
void EncounterDoPlacement(void)
{
	int i, n, c;
	SpawnPlaceInfo info;

	// iterate through encounters specified in mission
	// First do all the non "Mission" spawntypes
	n = eaSize(&g_activemission->def->spawndefs);
	for (i = 0; i < n; i++)
	{
		info.spawndef = g_activemission->def->spawndefs[i];
		info.spawntype = GetSpawnType(info.spawndef, g_activemission->def);
		if (info.spawndef->missionplace == MISSION_REPLACEGENERIC)
			continue;
		if (info.spawndef->missionplace == MISSION_SCRIPTCONTROLLED)
			continue;
		if (!stricmp(info.spawntype, "Mission"))
			continue;
		if (info.spawndef->objective && !MissionObjectiveIsEnabled(info.spawndef->objective[0]))
			continue;

		for (c = 0; c < info.spawndef->missioncount; c++)
		{
			if (!PlaceSomethingInRoom(&info, info.spawndef->missionplace, numEncountersInRoom, thisEncounterOk, placeSpawn))
			{
				Errorf("EncounterDoPlacement: couldn't place encounter (%s, %d) for mission %s\n", info.spawntype, c, g_activemission->def->filename);
			}
		}
	}

	// Now do the all the "Mission" spawntypes
	for (i = 0; i < n; i++)
	{
		info.spawndef = g_activemission->def->spawndefs[i];
		info.spawntype = GetSpawnType(info.spawndef, g_activemission->def);
		if (info.spawndef->missionplace == MISSION_REPLACEGENERIC)
			continue;
		if (info.spawndef->missionplace == MISSION_SCRIPTCONTROLLED)
			continue;
		if (stricmp(info.spawntype, "Mission"))
			continue;
		if (info.spawndef->objective && !MissionObjectiveIsEnabled(info.spawndef->objective[0]))
			continue;

		for (c = 0; c < info.spawndef->missioncount; c++)
		{
			if (!PlaceSomethingInRoom(&info, info.spawndef->missionplace, numEncountersInRoom, thisEncounterOk, placeSpawn))
			{
				Errorf("EncounterDoPlacement: couldn't place encounter (%s, %d) for mission %s\n", info.spawntype, c, g_activemission->def->filename);
			}
		}
	}
}

// *********************************************************************************
//  Script Placement hooks
// *********************************************************************************

// looking for an encounter within a mission room - very similar to EncounterGroupByName
//   layout is optional
EncounterGroup* MissionFindEncounter(RoomInfo* room, const char* layout, int inactiveonly)
{
	int* grouplist;
	int sel, i;
	EncounterGroup* result = NULL;

	eaiCreate(&grouplist);
	for (i = 0; i < eaSize(&room->standardEncounters); i++)
	{
		EncounterGroup* group = room->standardEncounters[i];
		if (inactiveonly &&
			(group->state == EG_WORKING || group->state == EG_INACTIVE ||
			group->state == EG_ACTIVE || group->state == EG_COMPLETED)) continue;
		if (layout &&
			EncounterGetMatchingPoint(group, layout) == -1) continue;
		eaiPush(&grouplist, i);
	}
	if (eaiSize(&grouplist))
	{
		sel = rule30Rand() % eaiSize(&grouplist);
		result = room->standardEncounters[grouplist[sel]];
	}
	eaiDestroy(&grouplist);
	return result;
}

// *********************************************************************************
//  Replacement for generic spawns
// *********************************************************************************

const SpawnDef* MissionSpawnReplaceGeneric(const MissionDef* def)
{
	static const SpawnDef** list = NULL;
	int i, n;

	// construct a list of possible replacements
	if (!list) 
		eaCreateConst(&list);
	eaSetSizeConst(&list, 0);
	n = eaSize(&g_activemission->def->spawndefs);
	for (i = 0; i < n; i++)
	{
		if (g_activemission->def->spawndefs[i]->missionplace == MISSION_REPLACEGENERIC)
			eaPushConst(&list, g_activemission->def->spawndefs[i]);
	}

	// and make selection
	n = eaSize(&list);
	if (!n) 
		return NULL;
	i = saRoll(n);
	return list[i];
}



