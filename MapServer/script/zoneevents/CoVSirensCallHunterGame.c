// ZONE SCRIPT
//
#include "scriptutil.h"
#include "storyarcutil.h"

// Used for testing new features of the script system

#define COVHUTNERGAME_CLEANUPPL		"PL_DestroyMe"

#define COVHUNTERGAME_REWARDCOUNT	2

enum {
	COV_SIRENSCALL_PAGE_HELLO = 1,
	COV_SIRENSCALL_PAGE_LEAVE = 3,
	COV_SIRENSCALL_PAGE_GETREWARD,
	COV_SIRENSCALL_PAGE_INSTRUCTIONS,
};

enum {
	COV_HUNTER_ALIVE,
	COV_HUNTER_KILLED,
	COV_HUNTER_NEEDNEWPLAYER,
	COV_HUNTER_DEFEATED,
};

enum {
	COV_HUNTER_HERO_LIST,
	COV_HUNTER_VILLAIN_LIST,
};

#define COV_SIRENSCALL_STATE_ID			"SirensCallState"				// who's currently winning the zone
#define COV_SIRENSCALL_STATE_HRANK_ID	"SirensCallStateHRank"			// how many times have heroes won it
#define COV_SIRENSCALL_STATE_VRANK_ID	"SirensCallStateVRank"			// how many times have villains won it
#define COV_SIRENSCALL_HSCORE_ID		"SirensCallScoreHero"			// what's the heroes current score
#define COV_SIRENSCALL_VSCORE_ID		"SirensCallScoreVillain"		// what's the villains current score

enum {
	COV_SIRENSCALL_STATE_HERO,
	COV_SIRENSCALL_STATE_VILLAIN,
};

// This function checks to see if the target is a valid bounty target:
//		Checks to see if a bounty has been claimed recently
NUMBER IsValidBountyTarget(ENTITY target)
{
	// a hidden player is never a valid target
	if (IsEntityHidden(target))
		return false;

	// GMs are never valid targets
	if (GetAccessLevel(target) > 0)
		return false;

	if (!EntityHasRewardToken(target, "HunterGameKilledTimer") ||
		(EntityHasRewardToken(target, "HunterGameKilledTimer") &&
			(EntityGetRewardToken(target, "HunterGameKilledTimer") + (60 * VarGetNumber("BountyTimer")) < CurrentTime())))
	{
		return true;
	} else {
		return false;
	}
}

STRING MostWantedString(STRING targetName)
{
	static char buf[100];
	sprintf(buf, "%s%s", saUtilScriptLocalize(VarGet("MostWantedText")), targetName);
	return buf;
}

// Put these on the variable list when I'm done with debugging
static StringArray villainList, heroList;

NUMBER CoVSirensCallHunterGameGetCurrentBounty(ENTITY player)
{
	NUMBER currentBounty = EntityGetRewardToken(player, VarGet("BountyPointsToken"));

	// checking to make sure player has a current bounty marker
	if (currentBounty == -1) 
	{
		EntityGrantRewardToken(player, VarGet("BountyPointsToken"), 0);
		currentBounty = 0;
	}
	return currentBounty;
}

NUMBER CoVSirensCallHunterGameGetBounty(ENTITY target)
{
	float reputation = GetReputation(target);

	if (reputation < 0.0f)
		reputation = -reputation;

	return (NUMBER) (((FRACTION) GetLevel(target) *10.0f)  + reputation);
}

NUMBER CoVSirensCallHunterGameGetID(ENTITY target)
{
	if (strnicmp(target, "player:", strlen("player:"))==0)
	{
		const char* id = target += strlen("player:");
		return atoi(id);
	} else {
		return -1;
	}
}

ENTITY CoVSirensCallHunterGameGetPlayerFromID(NUMBER id)
{
	if (id != 0)
		return StringAdd("player:", NumberToString(id));
	else 
		return "";
}


void CoVSirensCallHunterGameUpdateWaypoint(ENTITY player)
{
	NUMBER targetID = EntityGetRewardToken(player, VarGet("TargetToken"));
	NUMBER waypoint = EntityGetRewardToken(player, "WaypointToken");
	ENTITY target;

	if (targetID != -1)
	{
		target = CoVSirensCallHunterGameGetPlayerFromID(targetID);
	} else {
		// ERROR! - should always have a target in here...
		return;
	}

	if (!StringEqual(target, "") && waypoint >= 0) {
		UpdateWaypoint(waypoint, target, NULL, NULL, -1.0f);
	} else {
		DestroyWaypoint(waypoint);
		EntityRemoveRewardToken(player, "WaypointToken");
	}
	
}

void CoVSirensCallHunterGameUpdateStatus(ENTITY player)
{
	STRING /*header,*/ list;	
	int i;
	int teamCount = NumEntitiesInTeam(GetPlayerTeamFromPlayer(player));
//	NUMBER currentBounty;


	list = "";
	for (i = 1; i <= teamCount; i++)
	{
		ENTITY hunter = GetEntityFromTeam(GetPlayerTeamFromPlayer(player), i);
		STRING targetName;
		NUMBER targetID;
		ENTITY target;
//		STRING targetColor;

		if (VarGetNumber("ListMode") == 1)
			hunter = player;

		if (EntityHasRewardToken(hunter, VarGet("PlayingGameToken")))
		{

			targetID = EntityGetRewardToken(hunter, VarGet("TargetToken"));

			switch (EntityGetRewardToken(hunter, VarGet("TargetKilledToken")))
			{
			case COV_HUNTER_ALIVE:
				VarSetHidden(EntVar(hunter, "TargetColor"), "ffffffff");
				break;
			case COV_HUNTER_KILLED:
			case COV_HUNTER_NEEDNEWPLAYER:
			default:
				VarSetHidden(EntVar(hunter, "TargetColor"), "999999ff");
				break;
			}

			if (targetID != -1)
			{
				target = CoVSirensCallHunterGameGetPlayerFromID(targetID);
			} else {
				// No valid target assigned to player yet
				return;
			}

			// adding player name
			if (StringEqual(target, "")) {
				targetName = StringLocalize(VarGet("NoneText"));
			} else {
				targetName = GetDisplayName(target);
			}
			VarSetHidden(EntVar(hunter, "MostWantedEnemy"), MostWantedString(targetName));
			VarSetNumberHidden(EntVar(hunter, "MostWantedBounty"), CoVSirensCallHunterGameGetBounty(target)*3);
		}
		if (MapGetDataToken(COV_SIRENSCALL_STATE_ID) == COV_SIRENSCALL_STATE_HERO)
			VarSet("GroupInControl", VarGet("HeroesText"));
		else
			VarSet("GroupInControl", VarGet("VillainsText"));

		if (VarGetNumber("ListMode") == 1)
			break;
	}
}


void CoVSirensCallHunterGameUpdateStatusTeam(TEAM team)
{
	int i, count = NumEntitiesInTeam(team);

	for (i = 1; (i <= count); i++)
	{
		ENTITY player = GetEntityFromTeam(team, i);
		CoVSirensCallHunterGameUpdateStatus(player);
	}
}

NUMBER CoVSirensCallNumHotspotsThatShouldBeRunning()
{
	NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);

	if (VarGetNumber("UseTestCounts") == 1) {
		if (count > 2) {
			return 3;
		} else if (count > 1 ) {
			return 2;
		} else {
			return 1;
		}
	} else {
		if (count > 80) {
			return 3;
		} else if (count > 25 ) {
			return 2;
		} else {
			return 1;
		}
	}
}

NUMBER CoVSirensCallIsHotspotRunning(NUMBER spot)
{
	char buf[50];

	sprintf(buf, "Hotspot%dRunning", spot);
	if (VarGetNumber(buf) == 1)
		return true;
	else
		return false;
}


void CoVSirensCallCheckFailsafe(NUMBER location, STRING side)
{
	int i;
	char lasttimer[50], timer[50], currentWave[50], loc[50], teamGroup[50];
	int spawnLocCount = 0;
	int lasttimerValue;

	// setting failsafe timer
	sprintf(lasttimer, "Hotspot%d%sWaveTimerLast", location, side);
	sprintf(currentWave, "Hotspot%d%sWave", location, side);

	lasttimerValue = VarGetNumber(lasttimer) - 1;
	if (lasttimerValue >= VarGetNumber(currentWave)) 
	{
		sprintf(timer, "Hotspot%d%sWave%dTimer", location, side, lasttimerValue);

		if (TimerElapsed(timer))
		{
			sprintf(loc, "%sSpawnLocationList%d", side, lasttimerValue);
			spawnLocCount = VarGetArrayCount(loc);
			for (i = 0; i < spawnLocCount; i++)
			{
				sprintf(teamGroup, "Hotspot%d%s%d%d", location, side, lasttimerValue, i);
				SetAIPriorityList( teamGroup, "PL_Follow_Route_Run" );  
			}
			VarSetNumber(lasttimer, lasttimerValue);
			TimerRemove(timer);
		}
	}
}



void CoVSirensCallSpawnWave(NUMBER location, NUMBER wave, STRING side)
{
	int i;
	char buf[50], team[50], teamGroup[50], marker[50], loc[50], spawn[512];
	ENCOUNTERGROUP group = "first"; 
	int spawnLocCount = 0;

	sprintf(marker, "marker:Hotspot%d", location);

	// spawn wave
	sprintf(team, "Hotspot%d%s", location, side);
	if (StringEqual(side, "Hero")) {
		sprintf(spawn, "%sSpawnDefList%d", side, wave);
	} else {
		sprintf(buf, "Hotspot%dVillainType", location);
		sprintf(spawn, "%sSpawnDefList%d%d", side, wave, VarGetNumber(buf));
	}
	sprintf(loc, "%sSpawnLocationList%d", side, wave);
	spawnLocCount = VarGetArrayCount(loc);
	for (i = 0; i < spawnLocCount; i++)
	{
		group = FindEncounterGroupByRadius( 
			marker,
			0, 
			400, //Within 400 feet of this encounter
			VarGetArrayElement(loc, i),
			1, 
 			0 );  

		sprintf(teamGroup, "Hotspot%d%s%d%d", location, side, wave, i);
		if (stricmp(group, "location:none") != 0) {
			Spawn( 1, teamGroup, VarGetArrayElement(spawn, i), group, VarGetArrayElement(loc, i), VarGetNumber("SpawnLevel"), 0 );  
			SetScriptTeam(teamGroup, team);
		} else {
			// error
		}
	}
//	DetachSpawn(team);

	// setting failsafe timer
	sprintf(team, "Hotspot%d%sWave%dTimer", location, side, wave);
	TimerSet(team, VarGetFraction("FailsafeTimer"));

	sprintf(team, "Hotspot%d%sWave", location, side);
	VarSetNumber(team, wave);
}


void CoVSirensCallStartHotspot()
{
	int waypointID;
	int firstSpot = 0;
	int spotToStart = 0;
	int foundSpot = false;
	int firstRun = true;
	char buf[50], marker[50], waypoint[50];
	ENCOUNTERGROUP group = "first"; 

	// find an empty spot
	firstSpot = spotToStart = RandomNumber(1, VarGetNumber("Hotspots"));
	while (!foundSpot && (firstRun || (firstSpot != spotToStart))) {
		firstRun = false;

		sprintf(marker, "marker:Hotspot%d", spotToStart);

		// no good if already running
		if (CoVSirensCallIsHotspotRunning(spotToStart)) {
			spotToStart++;
			if (spotToStart > VarGetNumber("Hotspots"))
				spotToStart = 1;
		} else {
			foundSpot = true;
		}
		/* Removed - Vince
		else 
		{
			// check to see if anyone's nearby
			int i, count = NumEntitiesInTeam(ALL_PLAYERS);
			int found = false;

			for (i = 1; (i <= count && !found); i++)
			{
				ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);

				if (DistanceBetweenLocations(marker, player) < VarGetFraction("StartupDistance")) 
				{
					found = true;
					foundSpot = true;
				}
			}

			if (!found)
				spotToStart++;;
		}
		*/
	}

	// check to see if we found a spot to start
	if (!foundSpot)
		return;

	// mark as used
	sprintf(buf, "Hotspot%dRunning", spotToStart);
	VarSetNumber(buf, 1);

	// pick random villain group for villain side
	sprintf(buf, "Hotspot%dVillainType", spotToStart);
	VarSetNumber(buf, 1);
//	VarSetNumber(buf, RandomNumber(1,3));

	// spawn waves
	CoVSirensCallSpawnWave(spotToStart, 3, "Hero");
	CoVSirensCallSpawnWave(spotToStart, 3, "Villain");

	// set failsafe timers
	sprintf(buf, "Hotspot%d%sWaveTimerLast", spotToStart, "Hero");
	VarSetNumber(buf, 4);
	sprintf(buf, "Hotspot%d%sWaveTimerLast", spotToStart, "Villain");
	VarSetNumber(buf, 4);

	// create icon
	sprintf(waypoint, "WaypointHotspot%d", spotToStart);
	waypointID = CreateWaypoint(marker, VarGet("HotspotIconName"), "map_enticon_mission_a", "map_enticon_mission_b", false, true, -1.0f);
	VarSetNumber( waypoint, waypointID);

	// add icon to all players on map
	SetWaypoint(ALL_PLAYERS, waypointID);

	// increment count of active hotspots
	VarSetNumber("HotspotsRunning", VarGetNumber("HotspotsRunning") + 1);
}


void CoVSirensCallCleanSurvivors(NUMBER spot, STRING side)
{
	int wave, group, spawnLocCount;
	char loc[50], teamGroup[50], marker[50], markerList[50], todo[100];

	// clean up survivors
	for (wave = 3; wave > 0; wave--)
	{
		sprintf(loc, "%sSpawnLocationList%d", side, wave);
		spawnLocCount = VarGetArrayCount(loc);
		sprintf(markerList, "%sSpawnRunToList%d", side, wave);
		sprintf(todo, "%sSpawnDoList%d", side, wave);
		for (group = 0; group < spawnLocCount; group++)
		{
			sprintf(teamGroup, "Hotspot%d%s%d%d", spot, side, wave, group);
			AddAIBehavior(teamGroup,  VarGetArrayElement(todo, group) );  // This sets the AI that they will use when they get to their destination
			if (!StringEqual(VarGetArrayElement(markerList, group), "None")) {
				sprintf(marker, "marker:%s%d", VarGetArrayElement(markerList, group), spot);
				SetAIMoveTarget( teamGroup, marker, MEDIUM_PRIORITY, 0, 1, 10.0f );
			}
		}
	}

	// reset wave numbers
	sprintf(teamGroup, "Hotspot%d%sWave", spot, side);
	VarSetNumber(teamGroup, 0);
}

void CoVSirensCallEndHotspot(NUMBER spot, STRING winner)
{
	char buf[50], wins[50], waypoint[50];
	int score, i;
	NUMBER count; 
	STRING winMessage, winFloat;

	// add to winning total
	sprintf(wins, "SirensCallScore%s", winner);
	score = MapGetDataToken(wins);
	if (score >= 0)
	{
		MapSetDataToken(wins, score + 1);
	} else {
		MapSetDataToken(wins, 1);
	}
	
	// check for total win condition
	if (MapGetDataToken(COV_SIRENSCALL_HSCORE_ID) - MapGetDataToken(COV_SIRENSCALL_VSCORE_ID) > VarGetNumber("VictoryCondition"))
	{
		// heroes win
		MapSetDataToken(COV_SIRENSCALL_STATE_HRANK_ID, MapGetDataToken(COV_SIRENSCALL_STATE_HRANK_ID) + 1);
		MapSetDataToken(COV_SIRENSCALL_STATE_VRANK_ID, 0);
		MapSetDataToken(COV_SIRENSCALL_STATE_ID, COV_SIRENSCALL_STATE_HERO);
		MapSetDataToken(COV_SIRENSCALL_HSCORE_ID, 0);
		MapSetDataToken(COV_SIRENSCALL_VSCORE_ID, 0);

		// update UI for everyone
		CoVSirensCallHunterGameUpdateStatusTeam(ALL_PLAYERS);
	} else if (MapGetDataToken(COV_SIRENSCALL_VSCORE_ID) - MapGetDataToken(COV_SIRENSCALL_HSCORE_ID) > VarGetNumber("VictoryCondition"))
	{
		// villains win
		MapSetDataToken(COV_SIRENSCALL_STATE_VRANK_ID, MapGetDataToken(COV_SIRENSCALL_STATE_VRANK_ID) + 1);
		MapSetDataToken(COV_SIRENSCALL_STATE_HRANK_ID, 0);
		MapSetDataToken(COV_SIRENSCALL_STATE_ID, COV_SIRENSCALL_STATE_VILLAIN);
		MapSetDataToken(COV_SIRENSCALL_HSCORE_ID, 0);
		MapSetDataToken(COV_SIRENSCALL_VSCORE_ID, 0);

		// update UI for everyone
		CoVSirensCallHunterGameUpdateStatusTeam(ALL_PLAYERS);
	}

	// let everyone know about the win
	if (StringEqual(winner, "Hero")) {
		winMessage = StringAdd(StringLocalize(VarGet("HeroesText")), " ");
		winMessage = StringAdd(winMessage, StringLocalize(VarGet("NeighborhoodWon")));
		winFloat = StringAdd(StringLocalize(VarGet("HeroesText")), " ");
		winFloat = StringAdd(winFloat, StringLocalize(VarGet("NeighborhoodWonFloat")));
	} else {
		winMessage = StringAdd(StringLocalize(VarGet("VillainsText")), " ");
		winMessage = StringAdd(winMessage, StringLocalize(VarGet("NeighborhoodWon")));
		winFloat = StringAdd(StringLocalize(VarGet("VillainsText")), " ");
		winFloat = StringAdd(winFloat, StringLocalize(VarGet("NeighborhoodWonFloat")));
	}

	count = NumEntitiesInTeam(ALL_PLAYERS);

	for (i = 1; i <= count; i++)
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);

		// let everyone know about the win
		SendPlayerAttentionMessage(player, winFloat);
		SendPlayerSystemMessage(player, winMessage);

		// give credit to winners
		if ((StringEqual(winner, "Hero") && IsEntityHero(player)) ||
			(StringEqual(winner, "Villain") && IsEntityVillain(player)))
		{
			char marker[50];

			// check for radius 
			sprintf(marker, "marker:Hotspot%d", spot);

			if (DistanceBetweenLocations(marker, player) < 300.0f) {
				EntityGrantRewardToken(player, VarGet("BountyPointsToken"), 
					CoVSirensCallHunterGameGetCurrentBounty(player) + VarGetNumber("BattleRewardNumber"));
				CoVSirensCallHunterGameUpdateStatus(player);
			}
		}
	} 

	// clean up survivors
	CoVSirensCallCleanSurvivors(spot, "Hero");
	CoVSirensCallCleanSurvivors(spot, "Villain");

	// remove icon
	sprintf(waypoint, "WaypointHotspot%d", spot);
	DestroyWaypoint(VarGetNumber(waypoint));

	// setup cooldown timer
	sprintf(buf, "Hotspot%dCoolDown", spot);
	VarSetNumber(buf, CurrentTime());

	// decrement count of active hotspots
	VarSetNumber("HotspotsRunning", VarGetNumber("HotspotsRunning") - 1);
}


void CoVSirensCallUpdateHotspots()
{
	int i, entCount;
	char buf[50], wave[50], team[50];
	int waveCount = 0;

	for (i = 1; i <= VarGetNumber("Hotspots"); i++)
	{
		if (CoVSirensCallIsHotspotRunning(i))
		{
			// spot is active
 
			// check hero count for new wave
			sprintf(wave, "Hotspot%dHeroWave", i);
			sprintf(team, "Hotspot%dHero", i);
			entCount = NumEntitiesInTeam(team);
			waveCount = VarGetNumber(wave);
			if (waveCount > 0 && entCount < atoi(VarGetArrayElement("NextSpawnAt", waveCount-1)))
			{
				waveCount--;
				// check for villain victory
				if (waveCount > 0) {
					CoVSirensCallSpawnWave(i, waveCount, "Hero");
				} else {
					// villains win
					CoVSirensCallEndHotspot(i, "Villain");
				}
			} else {
				CoVSirensCallCheckFailsafe(i, "Hero");
			}


			// check villain count for new wave
			sprintf(wave, "Hotspot%dVillainWave", i);
			sprintf(team, "Hotspot%dVillain", i);
			entCount = NumEntitiesInTeam(team);
			waveCount = VarGetNumber(wave);
			if (waveCount > 0 && entCount < atoi(VarGetArrayElement("NextSpawnAt", waveCount-1)))
			{
				waveCount--;
				// check for hero victory
				if (waveCount > 0) {
					CoVSirensCallSpawnWave(i, waveCount, "Villain");
				} else {
					// heroes win
					CoVSirensCallEndHotspot(i, "Hero");
				}
			} else {
				CoVSirensCallCheckFailsafe(i, "Villain");
			}
		}

		// check to see if it's a dead hotspot
		sprintf(wave, "Hotspot%dCoolDown", i);
		if (VarGetNumber(wave) > 0) {

			// clean up after 2 minutes
			if (VarGetNumber(wave) + (60 * 2) < CurrentTime())
			{
				// clean up any leftover entities that haven't nicely cleaned up yet
				sprintf(buf, "Hotspot%dHero", i); 
				Kill( buf , false);  
				sprintf(buf, "Hotspot%dVillain", i);
				Kill( buf , false);  
			}

			// reset after 5 minutes
			if (VarGetNumber(wave) + (60 * 5) < CurrentTime())
			{
				// mark as unused
				sprintf(buf, "Hotspot%dRunning", i);
				VarSetNumber(buf, 0);

				// reset cooldown timer
				VarSetNumber(wave, 0);
			}
		}
	}
}



void CoVSirensCallHunterGameRemoveFromGame(ENTITY player, STRING reason)
{
	int i;

	// remove this player from everyone's target list that might still have him
	NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);

	for (i = 1; i <= count; i++)
	{
		ENTITY killer = GetEntityFromTeam(ALL_PLAYERS, i);

		// player is playing game, has this player as a target and hasn't already killed this target as a hit
		if (EntityHasRewardToken(killer, VarGet("TargetToken")) &&
			EntityGetRewardToken(killer, VarGet("TargetToken")) == CoVSirensCallHunterGameGetID(player))
		{
			// prep to give new name
			EntityGrantRewardToken(killer, VarGet("TargetKilledToken"), COV_HUNTER_NEEDNEWPLAYER);

			// grey out name
			CoVSirensCallHunterGameUpdateStatus(killer);

			// remove waypoint
			DestroyWaypoint(EntityGetRewardToken(killer, "WaypointToken"));
			EntityRemoveRewardToken(killer, "WaypointToken");

			// send reason
			if (!StringEmpty(reason))
				SendPlayerSystemMessage(killer, reason);
		}
	}
}


void CoVSirensCallHunterGameSetTarget(ENTITY player, ENTITY target)
{
	int waypointID = -1;

	// set player to playing game
	EntityGrantRewardToken(player, VarGet("PlayingGameToken"), 1);

	// set target
	if (!StringEqual(target, "")) {
		// set up target
		waypointID = CreateWaypoint("location:none", GetDisplayName(target), NULL, NULL, false, false, -1.0f);
		EntityGrantRewardToken(player, VarGet("TargetToken"), CoVSirensCallHunterGameGetID(target));
		EntityGrantRewardToken(player, "WaypointToken", waypointID);

		// remove any target killed status
		EntityGrantRewardToken(player, VarGet("TargetKilledToken"), COV_HUNTER_ALIVE);
	} else {
		// doesn't currently have a target, needs a new one
		EntityGrantRewardToken(player, VarGet("TargetToken"), 0);
		EntityGrantRewardToken(player, VarGet("TargetKilledToken"), COV_HUNTER_NEEDNEWPLAYER);
	}

	// update UI
	CoVSirensCallHunterGameUpdateStatus(player);

	// set radar waypoint
	if (waypointID >= 0) {
		SetWaypoint(player, waypointID);
		CoVSirensCallHunterGameUpdateWaypoint(player);
	}

}


void CoVSirensCallHunterGameAddToTopOfList(StringArray *list, ENTITY player)
{
	char *pEntString = NULL;
	int idx = StringArrayFind(*list, player);
		
	// check to see if they're already on the list
	if (idx == -1)
	{	
		pEntString = (char *) malloc(strlen(player) + 1);
		strcpy(pEntString, player);
		eaInsert(list, pEntString, 0);
	}
}

void CoVSirensCallHunterGameAddToList(StringArray *list, ENTITY player)
{
	char *pEntString = NULL;
	int idx = StringArrayFind(*list, player);
		
	// check to see if it is a player 
	if (IsEntityPlayer(player)) {
		// check to see if they're already on the list
		if (idx == -1)
		{	
			pEntString = (char *) malloc(strlen(player) + 1);
			strcpy(pEntString, player);
			eaPush(list, pEntString);
		}
	}
}

ENTITY CoVSirensCallHunterGameGetAndMoveToBottomOfList(NUMBER listType)
{
	StringArray *list; 
	STRING retval;
	char *pEntString;
	int size;
	int i = 0;

	switch (listType) {
		case COV_HUNTER_HERO_LIST:
			list = &heroList;
			break;
		case COV_HUNTER_VILLAIN_LIST:
			list = &villainList;
			break;
	}

	size = eaSize(list);
	for (i = 0; i < size; i++)
	{
		pEntString = (*list)[i];
		if (pEntString)
		{
			// this is a good target if they haven't been killed since last being assigned or 
			//		they haven't been killed in the last 5 mintues
			if (IsValidBountyTarget(pEntString))
			{
				// check to make sure this guy is on the right list
				if ((listType == COV_HUNTER_HERO_LIST && IsEntityHero(pEntString)) ||
					(listType == COV_HUNTER_VILLAIN_LIST && IsEntityVillain(pEntString)))
				{
					retval = StringAdd(pEntString, "");

					EntityRemoveRewardToken(pEntString, "HunterGameKilledTimer");
					eaRemove(list, i);
					eaPush(list, pEntString);
					return retval;
				} else {
					// on the wrong list!
					EntityRemoveRewardToken(pEntString, "HunterGameKilledTimer");
					eaRemove(list, i);
					if (IsEntityHero(pEntString)) {
						CoVSirensCallHunterGameAddToList(&heroList, pEntString);
					} else {
						CoVSirensCallHunterGameAddToList(&villainList, pEntString);
					}
				}
			}
		}
	}
	return "";
}

void CoVSirensCallHunterGamePrintList(char *buf, StringArray list)
{
	int count = eaSize(&list);
	int i;

	for (i = 0; i < count; i++)
	{
		STRING name = GetDisplayName(list[i]);

		strcat(buf, "<br>");
		if (!StringEqual(name, "")) {
			strcat(buf, name);
		} else {
			strcat(buf, "[Player Not Found]");
		}
	}
}

ENTITY CoVSirensCallHunterGameRemoveFromList(StringArray *list, ENTITY player)
{
	int idx = StringArrayFind(*list, player);
	char *pEntString = NULL;
	STRING retval;

	if (idx != -1)
	{	
		pEntString = eaRemove(list, idx);
		if (pEntString) 
		{
			retval = StringAdd(pEntString, "");
			free(pEntString);
		}
	}
	return retval;
}


int CoVSirensCallHunterGameOnContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	ENTITY prey = "";
	NUMBER currentBounty = CoVSirensCallHunterGameGetCurrentBounty(player);
	NUMBER i;
	NUMBER getReward;
 
	switch (link)
	{
	case COV_SIRENSCALL_PAGE_HELLO:	// HELLO
		// verify and clean up lists
		if (IsEntityHero(player)) {
			CoVSirensCallHunterGameRemoveFromList(&villainList, player);
			CoVSirensCallHunterGameAddToList(&heroList, player);
		} else {
			CoVSirensCallHunterGameRemoveFromList(&heroList, player);
			CoVSirensCallHunterGameAddToList(&villainList, player);
		}
/*
		// Debug information to check lists
		CoVSirensCallHunterGamePrintList(response->dialog, heroList);
		strcat(response->dialog, "<br><br>");
		CoVSirensCallHunterGamePrintList(response->dialog, villainList);
		strcat(response->dialog, "<br><br>");
*/
		// headshot
		sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));

		// check to see if player has any bounty
		getReward = CoVSirensCallHunterGameGetCurrentBounty(player) >= VarGetNumber(StringAdd("RewardLimit", NumberToString(1)));
		if (getReward)
		{
			// player has gotten enough reward for a prize
			if (IsEntityHero(player)) {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("HeroContactReturnDoneSpeech"), player));
				AddResponse(response, StringLocalizeWithVars(VarGet("HeroContactInstructionsResponse"), player), COV_SIRENSCALL_PAGE_INSTRUCTIONS);
				AddResponse(response, StringLocalizeWithVars(VarGet("HeroContactRewardResponse"), player), COV_SIRENSCALL_PAGE_GETREWARD);
			} else {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("VillainContactReturnDoneSpeech"), player));
				AddResponse(response, StringLocalizeWithVars(VarGet("VillainContactInstructionsResponse"), player), COV_SIRENSCALL_PAGE_INSTRUCTIONS);
				AddResponse(response, StringLocalizeWithVars(VarGet("VillainContactRewardResponse"), player), COV_SIRENSCALL_PAGE_GETREWARD);
			}
		} else {
			// No reward collected yet
			if (IsEntityHero(player)) {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("HeroContactStartSpeech"), player));
			} else {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("VillainContactStartSpeech"), player));
			}
		}
		AddResponse(response, StringLocalizeWithVars(VarGet("ContactExitText"), player), COV_SIRENSCALL_PAGE_LEAVE);
		return 1;
	case COV_SIRENSCALL_PAGE_INSTRUCTIONS:
		if (IsEntityHero(player)) {
			strcat(response->dialog, StringLocalizeWithVars( VarGet("HeroContactStartSpeech"), player));
		} else {
			strcat(response->dialog, StringLocalizeWithVars( VarGet("VillainContactStartSpeech"), player));
		}
		AddResponse(response, StringLocalizeWithVars(VarGet("ContactExitText"), player), COV_SIRENSCALL_PAGE_LEAVE);
		return 1;
	case COV_SIRENSCALL_PAGE_GETREWARD:
		// find correct reward state
		for (i = COVHUNTERGAME_REWARDCOUNT; i > 0; i--)
		{
			NUMBER Limit = VarGetNumber(StringAdd("RewardLimit", NumberToString(i)));

			if (currentBounty >= Limit)
			{
				// award it 
				EntityGrantReward(player, VarGet(StringAdd("RewardTable", NumberToString(i))));

				// remove cost from bounty
				EntityGrantRewardToken(player, VarGet("BountyPointsToken"), currentBounty - Limit);

				break;
			}
		}
		return 0;
	case COV_SIRENSCALL_PAGE_LEAVE:
	default:
		return 0;
	}
}

int CoVSirensCallHunterGameOnVillainContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	if (IsEntityVillain(player)) {
		return CoVSirensCallHunterGameOnContactClick( player, target, link, response);
	} else {
		if (TimerElapsed("VillainContactTalk")) {
			Say(target, VarGet("VillainGoAway"), 0);
			TimerSet("VillainContactTalk", 0.25f);
		}
		return 0;
	}
}

int CoVSirensCallHunterGameOnHeroContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	if (IsEntityHero(player)) {  
		return CoVSirensCallHunterGameOnContactClick( player, target, link, response);
	} else {
		if (TimerElapsed("HeroContactTalk")) {
			Say(target, VarGet("HeroGoAway"), 0);
			TimerSet("HeroContactTalk", 0.25f);
		}
		return 0;
	}
}

// Sets up the player to be playing the hunter game
void CoVSirensCallHunterGameStartGame(ENTITY player)
{
	ENTITY prey = "";

	if (IsEntityHero(player)) {
		CoVSirensCallHunterGameAddToList(&heroList, player);
		prey = CoVSirensCallHunterGameGetAndMoveToBottomOfList(COV_HUNTER_VILLAIN_LIST);

		ScriptUIShow("Collection:HeroUI", player);
	} else {
		CoVSirensCallHunterGameAddToList(&villainList, player);
		prey = CoVSirensCallHunterGameGetAndMoveToBottomOfList(COV_HUNTER_HERO_LIST);

		ScriptUIShow("Collection:VillainUI", player);
	}
	CoVSirensCallHunterGameSetTarget(player, prey);
	EntityGrantRewardToken(player, VarGet("BountyPointsToken"), 0);
}

// called when the player leaves the map
int CoVSirensCallHunterGameOnLeaveMap(ENTITY player)
{
	int i;
	char wave[50], waypointHS[50];
	NUMBER waypoint = EntityGetRewardToken(player, "WaypointToken");

	if (IsEntityHero(player)) {
		CoVSirensCallHunterGameRemoveFromList(&heroList, player);
	} else {
		CoVSirensCallHunterGameRemoveFromList(&villainList, player);
	}

	// clean up tokens
	EntityRemoveRewardToken(player, VarGet("PlayingGameToken"));
	EntityRemoveRewardToken(player, VarGet("TargetToken"));
	EntityRemoveRewardToken(player, VarGet("TargetKilledToken"));
	EntityRemoveRewardToken(player, VarGet("BountyPointsToken"));

	// need to remove player from everyone who has him
	CoVSirensCallHunterGameRemoveFromGame(player, VarGet("PlayerFledText"));

	// updates UI for player and rest of team
	CoVSirensCallHunterGameUpdateStatus(player);

	// remove waypoint
	DestroyWaypoint(waypoint);
	EntityRemoveRewardToken(player, "WaypointToken");

	// remove waypoints
	ClearWaypoint(player, VarGetNumber("HeroContact"));
	ClearWaypoint(player, VarGetNumber("VillainContact"));

	// remove all hotspot icons from player
	for (i = 1; i <= VarGetNumber("Hotspots"); i++)
	{
		sprintf(wave, "Hotspot%dHeroWave", i);
		if (VarGetNumber(wave) > 0)
		{
			sprintf(waypointHS, "WaypointHotspot%d", i);
			ClearWaypoint(player, VarGetNumber(waypointHS));
		}
	}

	ClearAllWaypoints(player); // clean up anything that might have been missed

	return 0;
}

// called when the player enters the map
int CoVSirensCallHunterGameOnEnterMap(ENTITY player)
{
	int i;
	char wave[50], waypoint[50];

	// clean up tokens - just in case something strange has happened
	EntityRemoveRewardToken(player, VarGet("PlayingGameToken"));
	EntityRemoveRewardToken(player, VarGet("TargetToken"));
	EntityRemoveRewardToken(player, VarGet("TargetKilledToken"));
	EntityRemoveRewardToken(player, VarGet("BountyPointsToken"));
	EntityRemoveRewardToken(player, "WaypointToken");

	// register player with game
	if (IsEntityHero(player)) {
		CoVSirensCallHunterGameAddToTopOfList(&heroList, player);
		SetWaypoint(player, VarGetNumber("HeroContact"));
	} else {
		CoVSirensCallHunterGameAddToTopOfList(&villainList, player);
		SetWaypoint(player, VarGetNumber("VillainContact"));
	}

	// player is always playing game now
	CoVSirensCallHunterGameStartGame(player);

	// removing old waypoints
	ClearAllWaypoints(player);

	// add all hotspot icons to player
	for (i = 1; i <= VarGetNumber("Hotspots"); i++)
	{
		sprintf(wave, "Hotspot%dHeroWave", i);
		if (VarGetNumber(wave) > 0)
		{
			sprintf(waypoint, "WaypointHotspot%d", i);
			SetWaypoint(player, VarGetNumber(waypoint));
		}
	}

	return 0;
}


// called when the player joins a team
int CoVSirensCallHunterGameOnJoinTeam(ENTITY player)
{
	//Set gang affiliation

	// set up ui
	CoVSirensCallHunterGameUpdateStatus(player);

	return 0;
}

// called when the player leaves a team
int CoVSirensCallHunterGameOnLeaveTeam(ENTITY player)
{
	//Set gang affiliation

	// set up ui
	CoVSirensCallHunterGameUpdateStatus(player);

	return 0;
}

void CoVSirensCallHunterGameOnTeamReward(ENTITY player, ENTITY victor)
{
	int j, k;

	if (IsEntityPlayer(player) && IsEntityPlayer(victor))
	{
		int countKillers = NumEntitiesInTeam(victor);
 
		int teamCredited = 1;
		NUMBER bounty = CoVSirensCallHunterGameGetBounty(player) / countKillers;
		char buf[256];

		for (j = 1; j <= countKillers; j++)
		{
			ENTITY killer = GetEntityFromTeam(victor, j);

			// does the killer have the player as a target or does the player have the victim as a target?
			if ((EntityHasRewardToken(killer, VarGet("TargetToken")) &&
				 EntityGetRewardToken(killer, VarGet("TargetToken")) == CoVSirensCallHunterGameGetID(player) &&
	 			 EntityGetRewardToken(killer, VarGet("TargetKilledToken")) != COV_HUNTER_NEEDNEWPLAYER) ||
				(EntityHasRewardToken(player, VarGet("TargetToken")) &&
				 EntityGetRewardToken(player, VarGet("TargetToken")) == CoVSirensCallHunterGameGetID(killer)) &&
	 			 EntityGetRewardToken(player, VarGet("TargetKilledToken")) != COV_HUNTER_NEEDNEWPLAYER)
			{
				// check for defense case
				if ((EntityHasRewardToken(player, VarGet("TargetToken")) &&
					EntityGetRewardToken(player, VarGet("TargetToken")) == CoVSirensCallHunterGameGetID(killer)) &&
	 				EntityGetRewardToken(player, VarGet("TargetKilledToken")) != COV_HUNTER_NEEDNEWPLAYER)
				{
					EntityGrantRewardToken(player, VarGet("TargetKilledToken"), COV_HUNTER_NEEDNEWPLAYER);
					CoVSirensCallHunterGameUpdateStatus(player);
				} else {
					// flag the killer as successful
					EntityGrantRewardToken(killer, VarGet("TargetKilledToken"), COV_HUNTER_NEEDNEWPLAYER);
					CoVSirensCallHunterGameUpdateStatus(killer);

					// remove waypoint
					DestroyWaypoint(EntityGetRewardToken(killer, "WaypointToken"));
					EntityRemoveRewardToken(killer, "WaypointToken");
				}
	
				teamCredited = 3;
			}
		}

		// check to see if this was a random kill & player should not award bounty
		if (teamCredited <= 1 && !IsValidBountyTarget(player))
		{
			bounty = 0;
		} else {
			bounty = bounty * teamCredited;
		}

		sprintf(buf, StringLocalize(VarGet("TargetDefeatedText")), bounty);

		// give credit for kill to this team
		for (k = 1; k <= countKillers; k++)
		{
			ENTITY killerReward = GetEntityFromTeam(victor, k);
			NUMBER currentBounty = CoVSirensCallHunterGameGetCurrentBounty(killerReward);

			// only gets reward if playing game
			if (EntityHasRewardToken(killerReward, VarGet("PlayingGameToken")))
			{
				if (bounty > 0) {
					EntityGrantRewardToken(killerReward, VarGet("BountyPointsToken"), currentBounty + bounty);
					CoVSirensCallHunterGameUpdateStatus(killerReward);
					SendPlayerSystemMessage(killerReward, buf);
				} else {
					SendPlayerSystemMessage(killerReward, StringLocalize(VarGet("TargetClaimedRecentlyText")));
				}
			}
		}

		// set timer on dead guy
		EntityGrantRewardToken(player, "HunterGameKilledTimer", CurrentTime());

		// checking if bounty has been claimed - then reset the victim and everyone who's after them
		if (teamCredited > 1)
		{
			// penalize victim and his team
/*			{
				int teamCount = NumEntitiesInTeam(GetPlayerTeamFromPlayer(player));
				NUMBER bounty = (NUMBER) (((FRACTION) CoVSirensCallHunterGameGetBounty(player) * 0.5f) / (FRACTION) teamCount);
				char buf[256];

				sprintf(buf, StringLocalize(VarGet("SlainText")), bounty);

				for (j = 1; j <= teamCount; j++)
				{
					ENTITY killer = GetEntityFromTeam(GetPlayerTeamFromPlayer(player), j);
					NUMBER currentBounty = CoVSirensCallHunterGameGetCurrentBounty(killer);



					// only gets penalty if playing game
					if (EntityHasRewardToken(killer, VarGet("PlayingGameToken")))
					{
						currentBounty -= bounty;
						if (currentBounty < 0)	// Can't have negative bounty
							currentBounty = 0;

						EntityGrantRewardToken(killer, VarGet("BountyPointsToken"), currentBounty);
						CoVSirensCallHunterGameUpdateStatus(killer);
						SendPlayerSystemMessage(killer, buf);
					}
				}
			}
*/			
			CoVSirensCallHunterGameRemoveFromGame(player, VarGet("MissedTarget"));		

			// re-add player to the bottom of the list
			if (IsEntityHero(player)) {
				CoVSirensCallHunterGameRemoveFromList(&heroList, player);
				CoVSirensCallHunterGameAddToList(&heroList, player);
			} else {
				CoVSirensCallHunterGameRemoveFromList(&villainList, player);
				CoVSirensCallHunterGameAddToList(&villainList, player);
			}
		}
	}
}

/*
int GlowieInteract(ENTITY player, ENTITY target)
{
	int num = VarGetNumber("GlowieClicks");

	num ++;

	if (num > 2) {
		GlowieRemove(target);
	} else {
		Say(target, "You clicked me!", 0);

		GlowieSetState(target);
	}
 
	VarSetNumber("GlowieClicks", num);
	return true;
}
*/

void CoVSirensCallHunterGame(void) 
{
	ENCOUNTERGROUP group = "first"; 
	int scriptID = 0;

	INITIAL_STATE

		ON_ENTRY 
			// put stuff that should run once on initialization here

			//////////////////////////////////////////////////////////////////////////
			// setup timers for contact
			TimerSet("HeroContactTalk", 0.25f);
			TimerSet("VillainContactTalk", 0.25f);

			VarSetNumber("MaxScore", VarGetNumber("VictoryCondition")*2);

			//////////////////////////////////////////////////////////////////////////
			// Check Siren's Call winning tokens
			//////////////////////////////////////////////////////////////////////////

			if (!MapHasDataToken(COV_SIRENSCALL_STATE_ID))
				MapSetDataToken(COV_SIRENSCALL_STATE_ID, COV_SIRENSCALL_STATE_HERO);

			if (!MapHasDataToken(COV_SIRENSCALL_STATE_HRANK_ID))
				MapSetDataToken(COV_SIRENSCALL_STATE_HRANK_ID, 1);

			if (!MapHasDataToken(COV_SIRENSCALL_STATE_VRANK_ID))
				MapSetDataToken(COV_SIRENSCALL_STATE_VRANK_ID, 0);

			if (!MapHasDataToken(COV_SIRENSCALL_HSCORE_ID))
				MapSetDataToken(COV_SIRENSCALL_HSCORE_ID, 0);

			if (!MapHasDataToken(COV_SIRENSCALL_VSCORE_ID))
				MapSetDataToken(COV_SIRENSCALL_VSCORE_ID, 0);

			//////////////////////////////////////////////////////////////////////////
			// spawn NPCs
			//////////////////////////////////////////////////////////////////////////
			group = FindEncounterGroupByRadius( 
				"coord:0.0,0.0,0.0",
				0, 
				9999, 
				"NPC_Villain_Contact", 
				1, 
 				0 );  
			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Contacts", "scripts.loc/SpawnDefs/V_PvP_03_01/NPC_Contacts_D0_V1.spawndef", group, "NPC_Villain_Contact", 1, 0 );  

				// add on click handler
				OnScriptedContactInteract(CoVSirensCallHunterGameOnVillainContactClick, ActorFromEncounterPos(group, 1));

				// map markers
				VarSetNumber("VillainContact", CreateWaypoint(EncounterPos(group, 1), VarGet("VillainContactName"), "map_enticon_contact", NULL, false, false, -1.0f));

			}

			group = FindEncounterGroupByRadius( 
				"coord:0.0,0.0,0.0",
				0, 
				9999, 
				"NPC_Hero_Contact", 
				1, 
 				0 );  
			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Contacts", "scripts.loc/SpawnDefs/V_PvP_03_01/NPC_Contacts_D0_V0.spawndef", group, "NPC_Hero_Contact", 1, 0 );  

				// add on click handler
				OnScriptedContactInteract(CoVSirensCallHunterGameOnHeroContactClick, ActorFromEncounterPos(group, 1));

				// map markers
				VarSetNumber("HeroContact", CreateWaypoint(EncounterPos(group, 1), VarGet("HeroContactName"), "map_enticon_contact", NULL, false, false, -1.0f));
			}


			//////////////////////////////////////////////////////////////////////////
			// find count of the fight locations
			//////////////////////////////////////////////////////////////////////////
			{
				int count = 0,
					finished = false;
				char buf[25];

				while (!finished)
				{
					sprintf(buf, "marker:Hotspot%d", count+1);
 
					if (!LocationExists(buf)) 
					{
						finished = true;
					} else {
						count++;
					}
				}

				if (count > 0) {
					VarSetNumber("Hotspots", count);				
				} else {
					EndScript();
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// Setting up the player lists
			//////////////////////////////////////////////////////////////////////////
			eaCreate(&villainList);
			eaCreate(&heroList);

			//////////////////////////////////////////////////////////////////////////
			// Doing the UI thing
			//////////////////////////////////////////////////////////////////////////
			// UI Widgets
			ScriptUITitle("HeroTitleUI", "BountyHunterGameName", "BountyHunterGameInfoHero");
			ScriptUITitle("VillainTitleUI", "BountyHunterGameName", "BountyHunterGameInfoVillain");
			ScriptUIList("ControlListUI", "BountyHunterGameName", "ControlText", "ffffffff", "GroupInControl", "ffffffff");
			ScriptUIList("BountyListUI", "BountyHunterGameName", "CurrentBountyPointsText", "ffffffff", "Token:HunterGameBountyPoints", "ffffffff");
			ScriptUIList("WantedListUI", "BountyHunterGameName", PERPLAYER("MostWantedEnemy"), PERPLAYER("TargetColor"), PERPLAYER("MostWantedBounty"), PERPLAYER("TargetColor"));
			ScriptUIMeter("ScoreMeterUI", "GameScore", "Score", "0", "0", "MaxScore", "2255ffff", "dd0000ff", "ScoreMeterTooltip");

			// Hero and Villain UI
			ScriptUICreateCollection("HeroUI", 5, "HeroTitleUI", "ControlListUI", "BountyListUI", "WantedListUI", "ScoreMeterUI");
			ScriptUICreateCollection("VillainUI", 5, "VillainTitleUI", "ControlListUI", "BountyListUI", "WantedListUI", "ScoreMeterUI");

			//////////////////////////////////////////////////////////////////////////
			///Script Hooks
			// setup whatever to remove ore and stuff when player leaves map
			OnPlayerExitingMap( CoVSirensCallHunterGameOnLeaveMap );

			OnPlayerEnteringMap( CoVSirensCallHunterGameOnEnterMap );
				
			OnPlayerJoinsTeam( CoVSirensCallHunterGameOnJoinTeam );

			OnPlayerLeavesTeam(	CoVSirensCallHunterGameOnLeaveTeam );

			OnTeamRewarded ( CoVSirensCallHunterGameOnTeamReward );
//			OnEntityDefeated ( CoVSirensCallHunterGameOnEntityDefeat );

			//////////////////////////////////////////////////////////////////////////
			/// Check for Players Already On Map
			// TODO!

		END_ENTRY

		// put stuff that should run every tick here
		
		//////////////////////////////////////////////////////////////////////////////////////
		// updating the score
		{
			int score = MapGetDataToken(COV_SIRENSCALL_HSCORE_ID) - MapGetDataToken(COV_SIRENSCALL_VSCORE_ID) + VarGetNumber("VictoryCondition");
			VarSetNumber("GameScore", score);
		}

		//////////////////////////////////////////////////////////////////////////////////////
		// updating location (if map mode correct)
		if (VarGetNumber("MapMode") == 3) {
			if ( DoEvery("RadarUpdateTimer", VarGetFraction("MapUpdateTime"), 0)) 
			{
				int i;
				NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);

				for (i = 1; i <= count; i++)
				{
					ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);
					if (EntityHasRewardToken(player, VarGet("PlayingGameToken")))
					{
						CoVSirensCallHunterGameUpdateWaypoint(player);
					}
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////
		// Updating people who need new bounty targets
		if( DoEvery("RefreshTargets", 1.0f, 0 ) )  
		{
			int i;

			NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);

			for (i = 1; i <= count; i++)
			{
				ENTITY killer = GetEntityFromTeam(ALL_PLAYERS, i);

				// is this player in the right list?
				if (IsEntityHero(killer)) {
					CoVSirensCallHunterGameAddToTopOfList(&heroList, killer);
				} else {
					CoVSirensCallHunterGameAddToTopOfList(&villainList, killer);
				}

				// does this guy need a new target?
				if (EntityHasRewardToken(killer, VarGet("TargetKilledToken")) &&
					EntityGetRewardToken(killer, VarGet("TargetKilledToken")) == COV_HUNTER_NEEDNEWPLAYER)
				{
					// give a new name
					ENTITY target;
					
					if (IsEntityHero(killer)) {
						target = CoVSirensCallHunterGameGetAndMoveToBottomOfList(COV_HUNTER_VILLAIN_LIST);
					} else {
						target = CoVSirensCallHunterGameGetAndMoveToBottomOfList(COV_HUNTER_HERO_LIST);
					}

					// set name
					if (!StringEqual(target, "")) {
						CoVSirensCallHunterGameSetTarget(killer, target);
						SendPlayerSystemMessage(killer, VarGet("NewTargetTransmittedText"));
						SendPlayerAttentionMessage(killer, VarGet("NewTargetTransmittedFloat"));
					}
				}
			}		
		}

		//////////////////////////////////////////////////////////////////////////////////////
		// check the state of any hotspots
		if( DoEvery("RefreshHotspots", 0.25f, 0 ) )  
		{
			if (VarGetNumber("DontRunHotspots") != 1) {
				// check to see if we need to start more hotspots
				if (VarGetNumber("HotspotsRunning") < CoVSirensCallNumHotspotsThatShouldBeRunning())
				{
					CoVSirensCallStartHotspot();
				}

				// update any hotspots that are running
				CoVSirensCallUpdateHotspots();
				
			}
		}

		//////////////////////////////////////////////////////////////////////////////////
		STATE("EventEnd") ////////////////////////////////////////////////////////

		{	// clean up all actors left over
			int i;
			char team[50];

			for (i = 1; i <= VarGetNumber("Hotspots"); i++)
			{
				// check hero count for new wave
				sprintf(team, "Hotspot%dHero", i);
				Kill(team, false);
				sprintf(team, "Hotspot%dVillain", i);
				Kill(team, false);
			}
		}

		// destroy waypoints
		DestroyWaypoint(VarGetNumber("HeroContact"));
		DestroyWaypoint(VarGetNumber("VillainContact"));

		// clean up lists
		eaDestroyEx(&villainList, NULL);
		eaDestroyEx(&heroList, NULL);

		// clean up handlers
		OnPlayerExitingMap( NULL );
		OnPlayerEnteringMap( NULL );
		OnPlayerJoinsTeam( NULL );
		OnPlayerLeavesTeam(	NULL );
		OnTeamRewarded ( NULL );

		EndScript();

	END_STATES

	ON_SIGNAL("End")
		SET_STATE("EventEnd");
	END_SIGNAL

		/*
	ON_SIGNAL("SwitchList")
	{
		NUMBER listMode = VarGetNumber("ListMode");
		if (listMode == 1)
		{
			listMode = 2;
		} else {
			listMode = 1;
		}
		VarSetNumber("ListMode", listMode);
	}
	END_SIGNAL

	ON_SIGNAL("Map1")
		VarSetNumber("MapMode", 1);
	END_SIGNAL

	ON_SIGNAL("Map2")
		VarSetNumber("MapMode", 2);
	END_SIGNAL

	ON_SIGNAL("Map3")
		VarSetNumber("MapMode", 3);
	END_SIGNAL
	*/

	ON_SIGNAL("HeroesWin")
		// heroes win
		MapSetDataToken(COV_SIRENSCALL_STATE_HRANK_ID, MapGetDataToken(COV_SIRENSCALL_STATE_HRANK_ID) + 1);
		MapSetDataToken(COV_SIRENSCALL_STATE_VRANK_ID, 0);
		MapSetDataToken(COV_SIRENSCALL_STATE_ID, COV_SIRENSCALL_STATE_HERO);
		MapSetDataToken(COV_SIRENSCALL_HSCORE_ID, 0);
		MapSetDataToken(COV_SIRENSCALL_VSCORE_ID, 0);

		// update UI for everyone
		CoVSirensCallHunterGameUpdateStatusTeam(ALL_PLAYERS);
	END_SIGNAL

	ON_SIGNAL("VillainsWin")
		// villains win
		MapSetDataToken(COV_SIRENSCALL_STATE_VRANK_ID, MapGetDataToken(COV_SIRENSCALL_STATE_VRANK_ID) + 1);
		MapSetDataToken(COV_SIRENSCALL_STATE_HRANK_ID, 0);
		MapSetDataToken(COV_SIRENSCALL_STATE_ID, COV_SIRENSCALL_STATE_VILLAIN);
		MapSetDataToken(COV_SIRENSCALL_HSCORE_ID, 0);
		MapSetDataToken(COV_SIRENSCALL_VSCORE_ID, 0);

		// update UI for everyone
		CoVSirensCallHunterGameUpdateStatusTeam(ALL_PLAYERS);
	END_SIGNAL

	ON_SIGNAL("Counts")
		VarSetNumber("UseTestCounts", 1);
	END_SIGNAL

	ON_SIGNAL("Spawn")
	{
		char team[50];
		VarSetNumber("DontRunHotspots", 1);
		VarSetNumber("Hotspot1VillainType", 1);

		sprintf(team, "Hotspot%d%s", 1, "Hero");
		Kill(team, false); 
		CoVSirensCallSpawnWave(1, 3, "Hero");

		sprintf(team, "Hotspot%d%s", 1, "Villain");
		Kill(team, false); 
		CoVSirensCallSpawnWave(1, 3, "Villain");
	}
	END_SIGNAL


	ON_STOPSIGNAL
		SET_STATE("EventEnd");
	END_SIGNAL
}

void CoVSirensCallHunterGameInit()
{
	SetScriptName( "CoVSirensCallHunterGame" );
	SetScriptFunc( CoVSirensCallHunterGame );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "HeroContactStartSpeech",						NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG);
	SetScriptVar( "VillainContactStartSpeech",					NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroContactInstructionsResponse",			NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactInstructionsResponse",			NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroContactReturnDoneSpeech",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactReturnDoneSpeech",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroContactRewardResponse",					NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactRewardResponse",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "PlayerFledText",								NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NewTargetTransmittedText",					NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NewTargetTransmittedFloat",					NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TargetDefeatedText",							NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TargetClaimedRecentlyText",					NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "MissedTargetText",							NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "SlainText",									NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BountyHunterGameName",						NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BountyHunterGameInfoHero",					NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BountyHunterGameInfoVillain",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "SlainText",									NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "PlayingGameToken",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TargetToken",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TargetKilledToken",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BountyPointsToken",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ContactExitText",							NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NoneText",									NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroesText",									NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainsText",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainGoAway",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroGoAway",									NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ControlText",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "CurrentBountyPointsText",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "MostWantedText",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NeighborhoodWon",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NeighborhoodWonFloat",						NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "RewardLimit1",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "RewardTable1",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "RewardLimit2",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "RewardTable2",								NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "BattleRewardNumber",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VictoryCondition",							NULL,		0 );

	SetScriptVar( "MapUpdateTime",								"1",		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "MapMode",									"3",		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ListMode",									"1",		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactName",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroContactName",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HotspotIconName",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ScoreMeterText",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ScoreMeterTooltip",							NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "SpawnLevel",									"30",		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "StartupDistance",							"500.0",	SP_DONTDISPLAYDEBUG );

	SetScriptVar( "HeroSpawnDefList1",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnLocationList1",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnRunToList1",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnDoList1",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnDefList2",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnLocationList2",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnRunToList2",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnDoList2",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnDefList3",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnLocationList3",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnRunToList3",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroSpawnDoList3",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDefList11",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDefList12",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDefList13",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnLocationList1",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnRunToList1",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDoList1",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDefList21",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDefList22",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDefList23",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnLocationList2",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnRunToList2",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDoList2",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDefList31",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDefList32",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDefList33",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnLocationList3",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnRunToList3",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSpawnDoList3",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NextSpawnAt",								NULL,		0 );
	SetScriptVar( "FailsafeTimer",								"5",		0 );
	SetScriptVar( "BountyTimer",								"5",		0 );

	SetScriptSignal( "End Event", "EndScript" );		
	SetScriptSignal( "Spawn", "Spawn" );		
	SetScriptSignal( "Test Counts", "Counts" );
	SetScriptSignal( "Heroes Win", "HeroesWin" );
	SetScriptSignal( "Villains Win", "VillainsWin" );
//	SetScriptSignal( "Switch List Mode", "SwitchList" );		
//	SetScriptSignal( "Map Mode 1", "Map1" );		
//	SetScriptSignal( "Map Mode 2", "Map2" );		
//	SetScriptSignal( "Map Mode 3", "Map3" );		
}