// ZONE SCRIPT
//

// Script for the zone capture game in ReclusesVictory

#include "scriptutil.h"


#define COV_RECLUSES_VICTORY_BUNKER_H_WPICON		"map_button_HeroPillbox"
#define COV_RECLUSES_VICTORY_BUNKER_V_WPICON		"map_button_VillainPillbox"
#define COV_RECLUSES_VICTORY_BUNKER_N_WPICON		"map_button_neutralPillbox"

#define COV_RECLUSES_VICTORY_BUNKER_N_FLAGICON		"sp_icon_pillbox_neutral"
#define COV_RECLUSES_VICTORY_BUNKER_H_FLAGICON		"sp_icon_pillbox_hero"
#define COV_RECLUSES_VICTORY_BUNKER_V_FLAGICON		"sp_icon_pillbox_Villain"

#define COV_RECLUSES_VICTORY_HERO_POINTS			"HeroPoints"
#define COV_RECLUSES_VICTORY_VILLAIN_POINTS			"VillainPoints"

#define COV_RECLUSES_VICTORY_BOUNTY_TOKEN			"ReclusesVictoryBounty"
#define COV_RECLUSES_VICTORY_HEAVY_TOKEN			"ReclusesVictoryHeavy"

#define COV_RECLUSES_VICTORY_NUM_FORCEFIELD_ENTS	1

enum {
	COV_RECLUSES_VICTORY_PAGE_HELLO = 1,
	COV_RECLUSES_VICTORY_PAGE_LEAVE = 3,
	COV_RECLUSES_VICTORY_PAGE_ACCEPTMISSION,		// not used
	COV_RECLUSES_VICTORY_PAGE_INSTRUCTIONS,			// not used
};

enum {
	COV_RECLUSES_VICTORY_NEUTRAL = 0,
	COV_RECLUSES_VICTORY_HERO,
	COV_RECLUSES_VICTORY_VILLAIN,
};

typedef struct {
	NUMBER ID;
	NUMBER BunkerID;
	FRACTION health;
	FRACTION target;
} CoVReculsesVictoryGeometryState;
CoVReculsesVictoryGeometryState **CoVReculsesVictoryGeometryList = NULL;


/////////////////////////////////////////////////////////////////////////
// Rewards
/////////////////////////////////////////////////////////////////////////

NUMBER CoVReclusesVictoryGetBounty(ENTITY player)
{
	NUMBER currentBounty = EntityGetRewardToken(player, COV_RECLUSES_VICTORY_BOUNTY_TOKEN);

	// checking to make sure player has a current bounty marker
	if (currentBounty == -1) 
	{
		EntityGrantRewardToken(player, COV_RECLUSES_VICTORY_BOUNTY_TOKEN, 0);
		currentBounty = 0;
	}
	return currentBounty;
}

void CoVReclusesVictorySetBounty(ENTITY player, NUMBER value)
{
	EntityGrantRewardToken(player, COV_RECLUSES_VICTORY_BOUNTY_TOKEN, value);
}

void CoVReclusesVictoryAddBounty(ENTITY player, NUMBER value)
{
	EntityGrantRewardToken(player, COV_RECLUSES_VICTORY_BOUNTY_TOKEN, value + CoVReclusesVictoryGetBounty(player));
}

void CoVReclusesVictoryGiveReward(NUMBER side, NUMBER reward)
{
	int count, i;

	count = NumEntitiesInTeam(ALL_PLAYERS);

	for (i = 1; i <= count; i++)
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);

		if (IsEntityHero(player) == side)
		{
			CoVReclusesVictoryAddBounty(player, reward);
		}
	}
}

void CoVReclusesVictoryCheckScores(void)
{
	int count, i;

	count = NumEntitiesInTeam(ALL_PLAYERS);

	for (i = 1; i <= count; i++)
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);
		
		if (CoVReclusesVictoryGetBounty(player) >= VarGetNumber("GoalScore"))
		{
			if (IsEntityHero(player)) 
			{
				EntityGrantReward(player, VarGet("GoalRewardHero"));
			} else {
				EntityGrantReward(player, VarGet("GoalRewardVillain"));
			}
		}
		CoVReclusesVictorySetBounty(player, 0);
	}
}


/////////////////////////////////////////////////////////////////////////
// Bunker Location Functions
/////////////////////////////////////////////////////////////////////////

// given the bunker name, return the index of that bunker in the bunker list array
int CoVReclusesVictoryFindBunkerIndex(STRING name)
{
	int i, count;
	if (!StringEmpty(name)) {
		count = VarGetArrayCount("BunkerList");
		for (i = 0; i < count; i++)
		{
			if (StringEqual(VarGetArrayElement("BunkerList", i), name))
				return i;
		}
	}
	return -1;
}


int CoVReclusesVictoryFindBunkerLocationFromLocation(LOCATION spot, NUMBER winning)
{
	int i, count;
	FRACTION bestDist = 9999.0f, dist;
	char loc[50], owner[50];
	int location = -1;
	
	// Find location
	count = VarGetArrayCount("BunkerList");
	for (i = 0; i < count; i++)
	{
		sprintf(owner, "Bunker%dOwner", i);
		if (winning < 0 || VarGetNumber(owner) == winning)
		{
			sprintf(loc, "marker:%s", VarGetArrayElement("BunkerList", i));

			dist = DistanceBetweenLocations(spot, loc);
			if (dist < bestDist)
			{
				bestDist = dist;
				location = i;
			}
		}
	}
	return location;
}

int CoVReclusesVictoryFindBunkerLocation(ENTITY control)
{
	return CoVReclusesVictoryFindBunkerLocationFromLocation(PointFromEntity(control), -1);
}

/////////////////////////////////////////////////////////////////////////
// Reinforcements
/////////////////////////////////////////////////////////////////////////
void CoVReclusesVictoryCleanupReinforcements()
{
	// kill off any support that are still around
	DetachSpawn("Support");

//	Kill("Support", false);
	AddAIBehavior("Support", "DoNothing(AnimBit(SIGNATUREEXIT), Untargetable, Timer(5)),DestroyMe");
	SwitchScriptTeam("Support", "Support", "OldSupport"); // this prevents any new support that spawns in from overriding the AI Behavior set above
	VarSetNumber("SupportIssued", -1);
	VarSetNumber("SupportTarget", 0);
	VarSetNumber("SupportCount", 0);
}

void CoVReclusesVictoryCallReinforcements(NUMBER winning)
{
	STRING loc = "marker:Support";
	STRING spawn = "scripts.loc/SpawnDefs/V_PvP_05_01/";
	STRING message;
	NUMBER closest;
	ENCOUNTERGROUP group;
	NUMBER team = RandomNumber(1, VarGetNumber("NumberOfSupportLocations"));

	// kill off any support that are still around
	CoVReclusesVictoryCleanupReinforcements();

	// find location to spawn at
	if (winning == COV_RECLUSES_VICTORY_HERO)
		loc = StringAdd(loc, "V");
	else
		loc = StringAdd(loc, "H");
	loc = StringAdd(loc, NumberToString(RandomNumber(1, VarGetNumber("NumberOfSupportLocations"))));
	
	// pick group to spawn
	if (winning == COV_RECLUSES_VICTORY_HERO)
		spawn = StringAdd(spawn, "V_Support_D0_V");
	else
		spawn = StringAdd(spawn, "H_Support_D0_V");
	spawn = StringAdd(spawn, NumberToString(team-1));
	spawn = StringAdd(spawn, ".spawndef");

	// find closest enemy occupied bunker
	closest = CoVReclusesVictoryFindBunkerLocationFromLocation(loc, winning);
	
	if (closest < 0 || closest > VarGetArrayCount("BunkerList"))  // check for case where it couldn't find any enemy locations... shouldn't be spawning in that case
		return;

	// spawn
	group = FindEncounterGroupByRadius( 
		loc,
		0, 
		100, 
		"Support", 
		1, 
 		0 );  

	if (stricmp(group, "location:none") != 0) {
		char buf[100];

		Spawn( 1, "Support", spawn, group, "Support", 50, 0 );  

		// reset AI
		RemoveAllBehaviors( "Support" );

		// set to wander after getting to location
		AddAIBehavior("Support", "PriorityList(PL_Wander,CombatOverride(Aggressive),OverridePerceptionRadius(300),ProtectSpawnRadius(300))");

		// set to move to closest bunker
		sprintf(buf, "marker:%s", VarGetArrayElement("BunkerList", closest));
		SetAIMoveTarget( "Support", buf, MEDIUM_PRIORITY, 0, true, 20.0f );
		AddAIBehaviorPermFlag("Support", "StandoffDistance(30),DoNotGoToSleep(1)");
//		SetAIStandoffDistance("Support", 30.0f);

		// saving new state
		VarSetNumber("SupportIssued", winning);
		VarSetNumber("SupportTarget", closest);
		DetachSpawn("Support");

		// send message to players
		if (winning == COV_RECLUSES_VICTORY_HERO) 
		{
			message = StringAdd(StringLocalize(VarGet("VillainLocationTell")), StringLocalize(VarGetArrayElement("BunkerNameList", closest)));
			BroadcastAttentionMessage(VarGetArrayElement("VillainSupportMessages", team-1), 0, 0);
			SendPlayerSystemMessage(ALL_PLAYERS, message);
		} else {
			message = StringAdd(StringLocalize(VarGet("HeroLocationTell")), StringLocalize(VarGetArrayElement("BunkerNameList", closest)));
			BroadcastAttentionMessage(VarGetArrayElement("HeroSupportMessages", team-1), 0, 0);
			SendPlayerSystemMessage(ALL_PLAYERS, message);
		}

	}
}

void CoVReclusesVictoryUpdateReinforcements()
{ 
	char owner[50], loc[50];
	int closest;
	STRING message;

	if (VarGetNumber("SupportIssued")>= 0) {

		// check to see if closest bunker has been taken
		sprintf(owner, "Bunker%dOwner", VarGetNumber("SupportTarget"));
		if (VarGetNumber("SupportIssued") != VarGetNumber(owner)) {
			// if it has, move to next closest enemy bunker
			sprintf(loc, "marker:%s", VarGetArrayElement("BunkerList", VarGetNumber("SupportTarget")));
			closest = CoVReclusesVictoryFindBunkerLocationFromLocation(loc, VarGetNumber("SupportIssued"));

			// check for case where it couldn't find any enemy locations... shouldn't be spawning in that case
			if (closest < 0 || closest > VarGetArrayCount("BunkerList"))  {
				CoVReclusesVictoryCleanupReinforcements();
				return;
			}

			// reset AI
			RemoveAllBehaviors( "Support" );

			// set to wander after getting to location
			AddAIBehavior("Support", "PriorityList(PL_Wander,CombatOverride(Aggressive))");

			// set to move to closest bunker
			sprintf(loc, "marker:%s", VarGetArrayElement("BunkerList", closest));
			SetAIMoveTarget( "Support", loc, MEDIUM_PRIORITY, 0, true, 20.0f );
			SetAIStandoffDistance("Support", 20.0f);

			VarSetNumber("SupportTarget", closest);
			
			if (VarGetNumber("SupportIssued") == COV_RECLUSES_VICTORY_HERO) {
				message = StringAdd(StringLocalize(VarGet("VillainLocationTell")), StringLocalize(VarGetArrayElement("BunkerNameList", closest)));
			} else {
				message = StringAdd(StringLocalize(VarGet("HeroLocationTell")), StringLocalize(VarGetArrayElement("BunkerNameList", closest)));
			}
			SendPlayerSystemMessage(ALL_PLAYERS, message);

		}

		// check to see if support has died
		if (NumEntitiesInTeam("Support") <= 0)
		{
			CoVReclusesVictoryCleanupReinforcements();
		}
	}
}



/////////////////////////////////////////////////////////////////////////
// UI
/////////////////////////////////////////////////////////////////////////

void CoVReclusesVictorySetupPlayerUI(ENTITY player)
{
	if (IsEntityHero(player)) {
		if (StringEqual(GET_STATE, "BaseCapture") || StringEqual(GET_STATE, STATE_INITIAL) )
		{
			ScriptUIHide("Collection:WinnerUI", player);
			ScriptUIHide("Collection:LoserUI", player);
			ScriptUIShow("Collection:HeroUI", player);
		} else {
			ScriptUIHide("Collection:HeroUI", player);
			if (VarGetNumber("RoundWinner")) 
			{
				ScriptUIShow("Collection:WinnerUI", player);
			} else {
				ScriptUIShow("Collection:LoserUI", player);
			}
		}
	} else {
		if (StringEqual(GET_STATE, "BaseCapture") || StringEqual(GET_STATE, STATE_INITIAL) )
		{
			ScriptUIHide("Collection:WinnerUI", player);
			ScriptUIHide("Collection:LoserUI", player);
			ScriptUIShow("Collection:VillainUI", player);
		} else {
			ScriptUIHide("Collection:VillainUI", player);
			if (!VarGetNumber("RoundWinner")) 
			{
				ScriptUIShow("Collection:WinnerUI", player);
			} else {
				ScriptUIShow("Collection:LoserUI", player);
			}
		}
	}
}

void CoVReclusesVictorySetupUI()
{
	int count, i;

	count = NumEntitiesInTeam(ALL_PLAYERS);

	for (i = 1; i <= count; i++)
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);

		CoVReclusesVictorySetupPlayerUI(player);
	}
}

/////////////////////////////////////////////////////////////////////////
// Contacts
/////////////////////////////////////////////////////////////////////////


int CoVReclusesVictoryOnHeroContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	if (IsEntityHero(player)) {  
		switch (link)
		{
			case COV_RECLUSES_VICTORY_PAGE_HELLO:	// HELLO
				// headshot
				sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));
				// instructions
				strcat(response->dialog, StringLocalizeWithVars( VarGet("HeroContactInstructions"), player));
				break;
			case COV_RECLUSES_VICTORY_PAGE_LEAVE:
			default:
				return 0;
		}
		AddResponse(response, StringLocalizeWithVars(VarGet("ContactExitText"), player), COV_RECLUSES_VICTORY_PAGE_LEAVE);
	} else {
		if (TimerElapsed("HeroContactTalk")) {
			Say(target, VarGet("HeroGoAway"), 0);
			TimerSet("HeroContactTalk", 0.25f);
		}
		return 0;
	}
	return 1;
}


int CoVReclusesVictoryOnVillainContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	if (IsEntityVillain(player)) {  
		switch (link)
		{
			case COV_RECLUSES_VICTORY_PAGE_HELLO:	// HELLO
				// headshot
				sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));
				// instructions
				strcat(response->dialog, StringLocalizeWithVars( VarGet("VillainContactInstructions"), player));
				break;
			case COV_RECLUSES_VICTORY_PAGE_LEAVE:
			default:
				return 0;
		}
		AddResponse(response, StringLocalizeWithVars(VarGet("ContactExitText"), player), COV_RECLUSES_VICTORY_PAGE_LEAVE);
	} else {
		if (TimerElapsed("VillainContactTalk")) {
			Say(target, VarGet("VillainGoAway"), 0);
			TimerSet("VillainContactTalk", 0.25f);
		}
		return 0;
	}
	return 1;
}


/////////////////////////////////////////////////////////////////////////
// Marker and Geometry Processing
/////////////////////////////////////////////////////////////////////////

int CoVReculsesVictoryMarkerProcessor(SCRIPTELEMENT elem)
{
	SCRIPTMARKER marker = GetScriptMarkerFromScriptElement(elem);

	if (!StringEmpty(FindMarkerProperty(marker, "Bunker")))
	{
		CoVReculsesVictoryGeometryState *state = malloc(sizeof(CoVReculsesVictoryGeometryState));

		state->ID = DynamicGeometryFind(FindMarkerGeoName(marker), LOC_ORIGIN, -1);
		if (state->ID) {
			state->BunkerID = CoVReclusesVictoryFindBunkerIndex(FindMarkerProperty(marker, "Bunker"));
			state->health = state->target = 50.0f;
			DynamicGeometrySetHP(state->ID, state->health, 0);

			eaPush(&CoVReculsesVictoryGeometryList, state);
		} else {
			// couldn't find this dyn geom group
			free (state);
		}
	}
	return true; 
}

void CoVReclusesVictorySetAllTargets(NUMBER location, FRACTION target)
{
	int i, count;
	count = eaSize(&CoVReculsesVictoryGeometryList);
	for (i = 0; i < count; i++)
	{
		if (CoVReculsesVictoryGeometryList[i]) {
			if (location >= 0)
			{ 
				if (CoVReculsesVictoryGeometryList[i]->BunkerID == location)
					CoVReculsesVictoryGeometryList[i]->target = target;
			} else {
				CoVReculsesVictoryGeometryList[i]->target = target;
			}
		}
	}
}

void CoVReclusesVictoryUpdateGeometryState(void)
{
	int count, i;

	// update bunker geometry state
	count = eaSize(&CoVReculsesVictoryGeometryList);
	for (i = 0; i < count; i++)
	{
		CoVReculsesVictoryGeometryState *state = CoVReculsesVictoryGeometryList[i];
		if (state && state->health != state->target) {
			NUMBER oldHealth = (NUMBER) state->health;
			NUMBER newHealth = 0;
			FRACTION delta = VarGetFraction("BunkerChangeRate");

			// move towards target
			if (fabs(state->health - state->target) < delta) {
				state->health = state->target;
			} else {
				if (state->health > state->target)
				{
					delta = -delta;
				}
				state->health += delta;
			}

			// update health ingame
			newHealth = (NUMBER) state->health;
			if (oldHealth != newHealth)
			{
				DynamicGeometrySetHP(state->ID, newHealth, 0);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// BUNKER CAPTURE
//////////////////////////////////////////////////////////////////////////////////////////

// CoVReclusesVictoryRecordState - This function updates a global map variable to allow other maps
//										to see what the current state of RV is.  (See DataNPC.c)
void CoVReclusesVictoryRecordState()
{
	int currentState = 0;
	int i, bunker;
	int count = VarGetArrayCount("BunkerList");
	char owner[50];

	// only update on primary instance of RV
	if (GetBaseMapID() != 81)
		return;

	for (i = 0; i < count; i++)
	{
		sprintf(owner, "Bunker%dOwner", i);
		bunker = (VarGetNumber(owner) << (i*2));
		currentState |= bunker;
	}

	MapSetDataToken("RVBunkerState", currentState);

	if (MapGetDataToken("RVHeroScore") < 0)
		MapSetDataToken("RVHeroScore", 0);

	if (MapGetDataToken("RVVillainScore") < 0)
		MapSetDataToken("RVVillainScore", 0);

}

int CoVReclusesVictoryBunkerAreSameSide(ENTITY player, NUMBER location)
{
	char owner[50];

	sprintf(owner, "Bunker%dOwner", location);

	if ((IsEntityVillain(player) && VarGetNumber(owner) == COV_RECLUSES_VICTORY_VILLAIN) ||
		(IsEntityHero(player) && VarGetNumber(owner) == COV_RECLUSES_VICTORY_HERO))
		return true;
	else
		return false;
}

// This function is called when the player initiates the bunker capture.  It checks to see if the bunker 
//		defenses are down and can be captured.
int CoVReclusesVictoryBunkerInteract(ENTITY player, ENTITY target)
{
	int location = CoVReclusesVictoryFindBunkerLocation(target); 
	char guards[50], owner[50], playerID[50];

	STRING ownerPlayer;
	sprintf(playerID, "Bunker%dPlayer", location);
	ownerPlayer = VarGet(playerID);

	sprintf(owner, "Bunker%dOwner", location);
	sprintf(guards, "Bunker%dGuards", location);
	
	if (StringEqual(GET_STATE, "BaseCapture")) 
	{
		// sanity check to make certain we have a valid location
		if (location >= 0)
		{
			// Check to see if player has defeated all the guards or if the bunker is unowned by a player and
			//		the clicking player is of the same team as the bunker

			if (CoVReclusesVictoryBunkerAreSameSide(player, location))
			{
				if (NumEntitiesInTeam(guards) <= COV_RECLUSES_VICTORY_NUM_FORCEFIELD_ENTS) {
					SendPlayerDialog(player, StringLocalize(VarGet("BunkerNothingLeft")));
				} else if (!StringEmpty(ownerPlayer))  {
					SendPlayerDialog(player, StringLocalize(VarGet("BunkerAlreadyControlled")));
				} else {
					return false;
				}
			} else {
				if ((NumEntitiesInTeam(guards) > COV_RECLUSES_VICTORY_NUM_FORCEFIELD_ENTS))
				{
					SendPlayerDialog(player, StringLocalize(VarGet("BunkerGuardsNotCleared")));
				} else {
					return false;
				}
			}
		}
	} else {
		SendPlayerDialog(player, StringLocalize(VarGet("BunkerCantBeCaptured")));
	}
	return true;
}


// This function is called when the player has successfully completed the bunker capture timer.  This
//		function switches control of the bunker to the capturing player's side and respawns defenses.

int CoVReclusesVictoryBunkerComplete(ENTITY player, ENTITY target)
{
	int location = CoVReclusesVictoryFindBunkerLocation(target); 
	char loc[50], waypoint[50], guards[50], owner[50], glowieID[50];
	ENCOUNTERGROUP group;
	ENTITY forcefield;
	int points;
	STRING icon;
	int dynGeomID;

	// sanity check to make certain we have a valid location
	if (location >= 0)
	{

		// check to make sure player is still alive and close to bunker
		sprintf(loc, "marker:%s", VarGetArrayElement("BunkerList", location));

//		if (DistanceBetweenLocations(player, loc) <= VarGetFraction("BunkerReleaseDist") && 
//			GetHealth(player) > 0.0f)
//		{

			// remove glowie
			sprintf(glowieID, "Bunker%dGlowie", location);
			GlowieRemove((ENTITY) VarGet(glowieID));
			VarSet(glowieID, "0");

			// guards
			sprintf(guards, "Bunker%dGuards", location);

			if (CoVReclusesVictoryBunkerAreSameSide(player, location)) 
			{

				// reset AI
				RemoveAllBehaviors( guards );

				// setting the player as the master of these pets
				SetAsPets(player, guards);
				AddAIBehaviorFlag(guards, "NoTeleport(1)");
				sprintf(owner, "Bunker%dPlayer", location);
				VarSet(owner, player);

				sprintf(owner, "Bunker%dFF", location);
				SetAsPets(TEAM_NONE, VarGet(owner));
				RemoveAllBehaviors( VarGet(owner) );
				SetAIPriorityList( VarGet(owner), "PL_UseAIConfig" );

				
			} else {

	//			sprintf(loc, "marker:%s", VarGetArrayElement("BunkerList", location));

				dynGeomID = DynamicGeometryFind(FindMarkerPropertyByMarkerName(VarGetArrayElement("BunkerList", location), 
																				"BunkerGeometry"), 
												LOC_ORIGIN, -1);

				// kill anything that's left
				DetachSpawn(guards);
				Kill(guards, false);

				// respawn turrets
				group = FindEncounterGroupByRadius( 
					loc,
					0, 
					100, 
					"Bunker", 
					1, 
 					0 );  

				if (stricmp(group, "location:none") != 0) {
					if (IsEntityHero(player)) {
						Spawn( 1, guards, "scripts.loc/SpawnDefs/V_PvP_05_01/H_Bunker_D0_V0.spawndef", group, "Bunker", 50, 0 );  
					} else {
						Spawn( 1, guards, "scripts.loc/SpawnDefs/V_PvP_05_01/V_Bunker_D0_V0.spawndef", group, "Bunker", 50, 0 );  
					}

					// finding forcefield
					sprintf(owner, "Bunker%dFF", location);
					forcefield = ActorFromEncounterPos(group, VarGetNumber("ForceFieldPosition"));
					VarSet(owner, forcefield);

					// reset AI
					RemoveAllBehaviors( guards );

					// setting the player as the master of these pets
					SetAsPets(player, guards);
					AddAIBehaviorFlag(guards, "NoTeleport(1)");
					sprintf(owner, "Bunker%dPlayer", location);
					VarSet(owner, player);

					// clearing forcefield ent
					sprintf(owner, "Bunker%dFF", location);
					SetAsPets(TEAM_NONE, VarGet(owner));
					RemoveAllBehaviors( forcefield );
					AddAIBehavior(forcefield, "PriorityList(PL_UseAIConfig)");

				}

				// check to see if this is a switch
				sprintf(owner, "Bunker%dOwner", location);
				if ((VarGetNumber(owner) != COV_RECLUSES_VICTORY_VILLAIN && IsEntityVillain(player)) ||
					(VarGetNumber(owner) != COV_RECLUSES_VICTORY_HERO && IsEntityHero(player)))
				{

					/////////////////////////////////////////////////////
					// Remove old points 
					points = 0;
					if (VarGetNumber(owner) == COV_RECLUSES_VICTORY_VILLAIN)	{
						points = VarGetNumber(COV_RECLUSES_VICTORY_VILLAIN_POINTS);
					} else if (VarGetNumber(owner) == COV_RECLUSES_VICTORY_HERO ) {
						points = VarGetNumber(COV_RECLUSES_VICTORY_HERO_POINTS);
					}

					// sanity check
					if (points > 0) {
						points--;

						if (VarGetNumber(owner) == COV_RECLUSES_VICTORY_VILLAIN)	{
							VarSetNumber(COV_RECLUSES_VICTORY_VILLAIN_POINTS, points);
						} else if (VarGetNumber(owner) == COV_RECLUSES_VICTORY_HERO ) {
							VarSetNumber(COV_RECLUSES_VICTORY_HERO_POINTS, points);
						}
					}

					/////////////////////////////////////////////////////
					// update waypoint
					sprintf(waypoint, "Bunker%dWaypoint", location);
					if (IsEntityHero(player)) {
						UpdateWaypoint(VarGetNumber(waypoint), 0, 0, COV_RECLUSES_VICTORY_BUNKER_H_WPICON, 0);
					} else {
						UpdateWaypoint(VarGetNumber(waypoint), 0, 0, COV_RECLUSES_VICTORY_BUNKER_V_WPICON, 0);
					}

					/////////////////////////////////////////////////////
					// switch ownership 
					icon = StringAdd("BunkerIcon", NumberToString(location));
					if (IsEntityVillain(player)) 
					{
						VarSetNumber(owner, COV_RECLUSES_VICTORY_VILLAIN);
						points = VarGetNumber(COV_RECLUSES_VICTORY_VILLAIN_POINTS) + 1;
						VarSetNumber(COV_RECLUSES_VICTORY_VILLAIN_POINTS, points);
						VarSet(icon, COV_RECLUSES_VICTORY_BUNKER_V_FLAGICON);
						CoVReclusesVictorySetAllTargets(location, 0.0f); // geometry changes
						if (dynGeomID)
							DynamicGeometrySetHP(dynGeomID, 0, 0);

					} else {
						VarSetNumber(owner, COV_RECLUSES_VICTORY_HERO);
						points = VarGetNumber(COV_RECLUSES_VICTORY_HERO_POINTS) + 1;
						VarSetNumber(COV_RECLUSES_VICTORY_HERO_POINTS, points);
						VarSet(icon, COV_RECLUSES_VICTORY_BUNKER_H_FLAGICON);
						CoVReclusesVictorySetAllTargets(location, 100.0f); // geometry changes
						if (dynGeomID)
							DynamicGeometrySetHP(dynGeomID, 100, 0);
					}
					CoVReclusesVictoryRecordState();

					// add bounty - all members of team
					{
						TEAM victors = GetPlayerTeamFromPlayer(player);
						int j, countKillers = NumEntitiesInTeam(victors);
						// add bounty to victors
						for (j = 1; j <= countKillers; j++)
						{
							ENTITY killer = GetEntityFromTeam(victors, j);

							CoVReclusesVictoryAddBounty(killer, VarGetNumber("CaptureScore"));

							// give credit
							GiveBadgeCredit(killer, kScriptBadgeTypeRVPillbox);
						}
					}

					/////////////////////////////////////////////////////
					// check for victory conditions
					if (points >= VarGetNumber("NumberRequiredForWin"))
					{
						// winner!
						SET_STATE("BonusOvertime");
						VarSetNumber("RoundWinner", IsEntityHero(player));

					} else {
						// send message to players saying point has been taken
						BroadcastAttentionMessage(IsEntityHero(player)?VarGet("BunkerCapturedMessageHero"):VarGet("BunkerCapturedMessageVillain"), 0, 0);

						if (points >= VarGetNumber("NumberRequiredForSupport") && VarGetNumber("SupportIssued") < 0)
						{
							// call in reinforcements 
							CoVReclusesVictoryCallReinforcements(IsEntityHero(player)?COV_RECLUSES_VICTORY_HERO:COV_RECLUSES_VICTORY_VILLAIN);
						} else if (VarGetNumber("SupportIssued") == (IsEntityHero(player)?COV_RECLUSES_VICTORY_VILLAIN:COV_RECLUSES_VICTORY_HERO)) {
							int count = VarGetNumber("SupportCount") + 1;

							if (count > 1)
								CoVReclusesVictoryCleanupReinforcements();
							else
								VarSetNumber("SupportCount", count);
						}
					}
				}
			}
//		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// BUNKER RELEASE
//////////////////////////////////////////////////////////////////////////////////////////

void CoVReclusesVictoryCheckBunkerDistance(void)
{
	int i;
	int count = VarGetArrayCount("BunkerList");
	char loc[50], guards[50], playerID[50];

	for (i = 0; i < count; i++)
	{
		STRING player;
		sprintf(playerID, "Bunker%dPlayer", i);

		player = VarGet(playerID);
		if (!StringEmpty(player))
		{
			sprintf(loc, "marker:%s", VarGetArrayElement("BunkerList", i));

			if (DistanceBetweenLocations(player, loc) > VarGetFraction("BunkerReleaseDist"))
			{
				sprintf(guards, "Bunker%dGuards", i);

				// free bunker from player
				VarSet(playerID, "");
				SetAsPets(TEAM_NONE, guards ); //Don't be anyone's follower anymore 

				// reset AI
				RemoveAllBehaviors( guards );
				SetAIPriorityList( guards, "PL_UseAIConfig" );
			}
		}
	}
}		


void CoVReclusesVictoryCheckBunkerLogout(ENTITY player)
{
	int i;
	int count = VarGetArrayCount("BunkerList");
	char guards[50], ownerID[50];

	for (i = 0; i < count; i++)
	{
		STRING owner;
		sprintf(ownerID, "Bunker%dPlayer", i);

		owner = VarGet(ownerID);
		if (StringEqual(owner, player) )
		{
			sprintf(guards, "Bunker%dGuards", i);

			// free bunker from player
			VarSet(ownerID, "");
			SetAsPets(TEAM_NONE, guards ); //Don't be anyone's follower anymore 

			// reset AI
			RemoveAllBehaviors( guards );
			SetAIPriorityList( guards, "PL_UseAIConfig" );
		}
	}
}		

//////////////////////////////////////////////////////////////////////////////////////////
// BUNKER GLOWIE RESPAWN
//////////////////////////////////////////////////////////////////////////////////////////

void CoVReclusesVictoryCheckGlowieRespawn(void)
{

	ENTITY glowie;
	GLOWIEDEF bunkerControl = NULL; 
	int i;
	int count = VarGetArrayCount("BunkerList");
	char loc[50], guards[50], owner[50], glowieID[50], playerID[50];

	for (i = 0; i < count; i++)
	{
		sprintf(owner, "Bunker%dOwner", i);
		sprintf(glowieID, "Bunker%dGlowie", i);

		if (VarGetNumber(owner) != COV_RECLUSES_VICTORY_NEUTRAL && StringEqual(VarGet(glowieID), "0"))
		{
			// Check to see if player has defeated all the guards or if the controlling player has unlinked
			STRING player;
			sprintf(playerID, "Bunker%dPlayer", i);
			player = VarGet(playerID);

			sprintf(guards, "Bunker%dGuards", i);
			if (NumEntitiesInTeam(guards) <= COV_RECLUSES_VICTORY_NUM_FORCEFIELD_ENTS || 
				StringEmpty(player))
			{
				// all defenders have been defeated or the bunker is unowned
				// setup location 
				sprintf(loc, "marker:%s", VarGetArrayElement("BunkerList", i));
				
				// create glowie 
				bunkerControl = GlowieCreateDef(StringLocalize(VarGet("BunkerControlName")), 
												VarGet("BunkerControlModel"), 
												VarGet("BunkerControlInteract"), VarGet("BunkerControlInterrupt"), 
												VarGet("BunkerControlComplete"), VarGet("BunkerControlInteractBar"), 
												VarGetNumber("BunkerControlInteractTime"));

				glowie = GlowiePlace(bunkerControl, loc, false, CoVReclusesVictoryBunkerInteract, CoVReclusesVictoryBunkerComplete);
				SetScriptTeam(glowie, "glowies");

				VarSet(glowieID, glowie);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// BUNKER SETUP & RESET
//////////////////////////////////////////////////////////////////////////////////////////

void CoVReclusesVictoryResetBunker(int location)
{
	char loc[100];
	char guards[50], waypoint[50], owner[50];
	ENCOUNTERGROUP group;
	int points;
	int dynGeomID;
	STRING icon;
	char glowieID[50];
	ENTITY glowie;
	GLOWIEDEF bunkerControl = NULL; 


	// setup location 
	sprintf(loc, "marker:%s", VarGetArrayElement("BunkerList", location));

	// update geometry
	dynGeomID = DynamicGeometryFind(FindMarkerPropertyByMarkerName(VarGetArrayElement("BunkerList", location), 
																	"BunkerGeometry"), 
									LOC_ORIGIN, -1);
	if (dynGeomID)
		DynamicGeometrySetHP(dynGeomID, 50, 0);

	// update waypoint
	sprintf(waypoint, "Bunker%dWaypoint", location);
	UpdateWaypoint(VarGetNumber(waypoint), 0, 0, COV_RECLUSES_VICTORY_BUNKER_N_WPICON, 0);

	// fix UI flags
	icon = StringAdd("BunkerIcon", NumberToString(location));
	VarSet(icon, COV_RECLUSES_VICTORY_BUNKER_N_FLAGICON);
	CoVReclusesVictorySetAllTargets(location, 50.0f);


	/////////////////////////////////////////////////////
	// Remove old points 
	sprintf(owner, "Bunker%dOwner", location);
	if (VarGetNumber(owner) == COV_RECLUSES_VICTORY_VILLAIN)	{
		points = VarGetNumber(COV_RECLUSES_VICTORY_VILLAIN_POINTS);
	} else if (VarGetNumber(owner) == COV_RECLUSES_VICTORY_HERO ) {
		points = VarGetNumber(COV_RECLUSES_VICTORY_HERO_POINTS);
	}

	// sanity check
	if (points > 0)
		points--;

	if (VarGetNumber(owner) == COV_RECLUSES_VICTORY_VILLAIN)	{
		VarSetNumber(COV_RECLUSES_VICTORY_VILLAIN_POINTS, points);
	} else if (VarGetNumber(owner) == COV_RECLUSES_VICTORY_HERO ) {
		VarSetNumber(COV_RECLUSES_VICTORY_HERO_POINTS, points);
	}

	VarSetNumber(owner, COV_RECLUSES_VICTORY_NEUTRAL);
	CoVReclusesVictoryRecordState();

	// kill anything that is currently existing
	sprintf(guards, "Bunker%dGuards", location);
	DetachSpawn(guards);
	Kill(guards, false);

	// spawn neutral critters
	group = FindEncounterGroupByRadius( 
		loc,
		0, 
		100, 
		"Bunker", 
		1, 
 		0 );  

	if (stricmp(group, "location:none") != 0) {
		Spawn( 1, guards, "scripts.loc/SpawnDefs/V_PvP_05_01/N_Bunker_D0_V0.spawndef", group, "Bunker", 50, 0 );  
	}


	// remove glowie
	sprintf(glowieID, "Bunker%dGlowie", location);
	GlowieRemove((ENTITY) VarGet(glowieID));

	// create glowie 
	bunkerControl = GlowieCreateDef(StringLocalize(VarGet("BunkerControlName")), 
									VarGet("BunkerControlModel"), 
									VarGet("BunkerControlInteract"), VarGet("BunkerControlInterrupt"), 
									VarGet("BunkerControlComplete"), VarGet("BunkerControlInteractBar"), 
									VarGetNumber("BunkerControlInteractTime"));

	glowie = GlowiePlace(bunkerControl, loc, false, CoVReclusesVictoryBunkerInteract, CoVReclusesVictoryBunkerComplete);
	SetScriptTeam(glowie, "glowies");
	VarSet(glowieID, glowie);


}

// Set up for when the bunker has been reset
void CoVReclusesVictorySetupBunker(int location)
{
	char loc[100];
	char waypoint[50];

	// setup location 
	sprintf(loc, "marker:%s", VarGetArrayElement("BunkerList", location));
	
	// setup waypoint
	sprintf(waypoint, "Bunker%dWaypoint", location);
	VarSetNumber(waypoint, CreateWaypoint(loc, VarGetArrayElement("BunkerNameList", location), COV_RECLUSES_VICTORY_BUNKER_N_WPICON, NULL, false, false, -1.0f));

	// spawn neutral turrets
	CoVReclusesVictoryResetBunker(location);
}


void CoVReclusesVictorySetupAllBunkers(int reset)
{
	int i;
	int count = VarGetArrayCount("BunkerList");

	for (i = 0; i < count; i++)
	{
		STRING point = VarGetArrayElement("BunkerList", i);
		STRING bunker = StringAdd("marker:", point);

		// check and see if location exists
		if (LocationExists(bunker))
		{
			if (reset)
				CoVReclusesVictoryResetBunker(i);
			else
				CoVReclusesVictorySetupBunker(i);
		}
	}
}		

//////////////////////////////////////////////////////////////////////////////////////////
// MAP CALLBACKS
//////////////////////////////////////////////////////////////////////////////////////////

// called when the player enters the map
int CoVReclusesVictoryOnEnterMap(ENTITY player)
{
	int i, count;
	char waypoint[50];

	// add ui
	CoVReclusesVictorySetupPlayerUI(player);

	// add waypoints
	if (IsEntityHero(player)) {
		SetWaypoint(player, VarGetNumber("HeroContact"));
	} else {
		SetWaypoint(player, VarGetNumber("VillainContact"));
	}
	count = VarGetArrayCount("BunkerList");
	for (i = 0; i < count; i++)
	{
		sprintf(waypoint, "Bunker%dWaypoint", i);
		SetWaypoint(player, VarGetNumber(waypoint));
	}

	// reset bounty
	CoVReclusesVictorySetBounty(player, 0);

	// reset pet owner, just in case
	EntityRemoveRewardToken(player, COV_RECLUSES_VICTORY_HEAVY_TOKEN);

	return 0;
}

// called when the player leaves the map
int CoVReclusesVictoryOnLeaveMap(ENTITY player)
{
	int count, i;
	char waypoint[50];

	// remove any turrets the might control
	CoVReclusesVictoryCheckBunkerLogout(player);

	// remove ui
	if (IsEntityHero(player)) {
		ScriptUIHide("Collection:HeroUI", player);
		ClearWaypoint(player, VarGetNumber("HeroContact"));
	} else {
		ScriptUIHide("Collection:VillainUI", player);
		ClearWaypoint(player, VarGetNumber("VillainContact"));
	}

	// remove waypoints
	count = VarGetArrayCount("BunkerList");
	for (i = 0; i < count; i++)
	{
		sprintf(waypoint, "Bunker%dWaypoint", i);
		ClearWaypoint(player, VarGetNumber(waypoint));
	}

	// reset bounty
	CoVReclusesVictorySetBounty(player, 0);

	// reset pet owner, just in case
	EntityRemoveRewardToken(player, COV_RECLUSES_VICTORY_HEAVY_TOKEN);

	return 0;
}


// PvP rewards
void CoVReclusesVictoryOnTeamReward(ENTITY player, ENTITY victor)
{
	int j;
	int i, targetCount;
	int countKillers;

	FRACTION mult = 1.0f;

	// PvP kill?
	if (IsEntityPlayer(player) && IsEntityPlayer(victor) && ValidForPvPReward(victor, player))
	{
		countKillers = NumEntitiesInTeam(victor);
		// are we in overtime and was the killer on the winning side?
		if (!StringEqual(GET_STATE, "BaseCapture") && VarGetNumber("RoundWinner") == IsEntityHero(victor)) 
		{	 
			for (j = 1; j <= countKillers; j++)
			{
				ENTITY killer = GetEntityFromTeam(victor, j);

				// give reward to each member of team here!
				EntityGrantReward(killer, VarGet("PvPReward"));

			}

			mult = VarGetFraction("VictoryScoreMult");
		}

		// add bounty to victors
		for (j = 1; j <= countKillers; j++)
		{
			ENTITY killer = GetEntityFromTeam(victor, j);

			// give reward to each member of team here!
			CoVReclusesVictoryAddBounty(killer, (NUMBER) (VarGetNumber("PvPKillScore") * mult));
		}

	}
	else if (IsEntityCritter(player) && IsEntityPlayer(victor) )
	{
		STRING name = GetVillainDefName(player);
		countKillers = NumEntitiesInTeam(victor);

		// check for sig heroes
		targetCount = VarGetArrayCount("SigEntitiesList");
		for (i = 0; i < targetCount; i++)
		{
			if (StringEqual(VarGetArrayElement("SigEntitiesList", i), name))
			{
				// found it!
				for (j = 1; j <= countKillers; j++)
				{
					ENTITY killer = GetEntityFromTeam(victor, j);

					// give reward to each member of team here!
					CoVReclusesVictoryAddBounty(killer, VarGetNumber("SigKillScore"));
				}
				break;
			}
		}

		// check for turrets
		targetCount = VarGetArrayCount("TurretEntitiesList");
		for (i = 0; i < targetCount; i++)
		{
			if (StringEqual(VarGetArrayElement("TurretEntitiesList", i), name))
			{
				// found it!
				for (j = 1; j <= countKillers; j++)
				{
					ENTITY killer = GetEntityFromTeam(victor, j);
					mult = 1.0f;

					// is this overtime and was the killer on the winning side?
					if (!StringEqual(GET_STATE, "BaseCapture") && VarGetNumber("RoundWinner") == IsEntityHero(killer)) 
						mult = VarGetFraction("VictoryScoreMult");

					// give reward to each member of team here!
					CoVReclusesVictoryAddBounty(killer, (NUMBER) (VarGetNumber("TurretScore") * mult));
				}
				break;
			}
		}
	}
}

void CoVReclusesVictoryOnEntityDefeat(ENTITY player, ENTITY victor)
{
	int hero = IsEntityHero(victor);
	int count = VarGetNumber("LoserCount");

	//
	if (IsEntityPlayer(player))
	{
		// remove bounty from victim
		CoVReclusesVictoryAddBounty(player, VarGetNumber("DeathScore"));

		// remove any bunkers that they might have had
		CoVReclusesVictoryCheckBunkerLogout(player);
	}

	// don't need to bother with this if it isn't a player
	if (IsEntityPlayer(victor))
	{

		// if we're in overtime and the side of the player is the type needed
		//		decrement the count left to kill. 
		if (!StringEqual(GET_STATE, "BaseCapture")) 
		{
			// overtime
			if (VarGetNumber("RoundWinner") != hero) 
			{
				// victor is on the losing side
				if ((hero && IsEntityVillain(player)) || (!hero && IsEntityHero(player)))
				{
					// player is on winning side
					count--;

					if (count <= 0)
					{
						// If we've killed enough, end the overtime
						// give reward
						CoVReclusesVictoryGiveReward(VarGetNumber("RoundWinner")?0:1, VarGetNumber("VictoryScore"));

						// reset the states
						SET_STATE("Reset");
					} else {
						VarSetNumber("LoserCount", count);
					}
				}
			}			
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// SCRIPT SHUTDOWN FUNCTION
//////////////////////////////////////////////////////////////////////////////////////////

void CoVReclusesVictoryShutdown()
{
	int i, count;

	// cleaning up the bunker geometry list
	count = eaSize(&CoVReculsesVictoryGeometryList);
	for (i = 0; i < count; i++)
	{
		if (CoVReculsesVictoryGeometryList[i])
			free(CoVReculsesVictoryGeometryList[i]);
		CoVReculsesVictoryGeometryList[i] = NULL;
	}
	eaDestroy(&CoVReculsesVictoryGeometryList);
	CoVReculsesVictoryGeometryList = NULL;
	
	Kill("glowies", false);

	// cleaning up any reinforcements
	CoVReclusesVictoryCleanupReinforcements();

	EndScript();
}

//////////////////////////////////////////////////////////////////////////////////////////
// SCRIPT MAIN FUNCTION
//////////////////////////////////////////////////////////////////////////////////////////

void CoVReclusesVictory(void)
{
	ENCOUNTERGROUP group;
	int i;
	STRING icon;

	INITIAL_STATE
		ON_ENTRY

			/////////////////////////////////////////////////////////////////////////
			// Setup Bunkers
			/////////////////////////////////////////////////////////////////////////

			// Geometry Swap Setup
			eaCreate(&CoVReculsesVictoryGeometryList);
			eaSetSize(&CoVReculsesVictoryGeometryList, 0);
 			ForEachScriptMarker(CoVReculsesVictoryMarkerProcessor);

			// find all of the capturable hotspots and set them up
			CoVReclusesVictorySetupAllBunkers(false);


			//////////////////////////////////////////////////////////////////////////////////
			// create UI 
			//////////////////////////////////////////////////////////////////////////////////

			// Always show
			ScriptUIMeter("ScoreMeterUI", "Token:"COV_RECLUSES_VICTORY_BOUNTY_TOKEN, "ScoreBarName", "1", "0", "GoalScore", "2255ffff", "dd0000ff", "{VAL}/{MAX}");

			// Base Capture 
			ScriptUITitle("HeroTitleUI", "EventName", "EventInfoHero");
			ScriptUITitle("VillainTitleUI", "EventName", "EventInfoVillain");

			if (VarGetArrayCount("BunkerList") == 7) {
				ScriptUICollectItems("BasesUI", 7,	"AlwaysOn", VarGetArrayElement("BunkerNameList", 0), "BunkerIcon0",
													"AlwaysOn", VarGetArrayElement("BunkerNameList", 1), "BunkerIcon1",
													"AlwaysOn", VarGetArrayElement("BunkerNameList", 2), "BunkerIcon2",
													"AlwaysOn", VarGetArrayElement("BunkerNameList", 3), "BunkerIcon3",
													"AlwaysOn", VarGetArrayElement("BunkerNameList", 4), "BunkerIcon4",
													"AlwaysOn", VarGetArrayElement("BunkerNameList", 5), "BunkerIcon5",
													"AlwaysOn", VarGetArrayElement("BunkerNameList", 6), "BunkerIcon6");
				for (i = 0; i < VarGetArrayCount("BunkerList"); i++)
				{
					icon = StringAdd("BunkerIcon", NumberToString(i));
					VarSet(icon, COV_RECLUSES_VICTORY_BUNKER_N_FLAGICON);
				}
			} else {
				ScriptUITitle("BasesUI", "Wrong Number of Bunkers", "Wrong Number of Bunkers");
			}

			ScriptUICreateCollection("HeroUI", 3, "HeroTitleUI", "ScoreMeterUI", "BasesUI");
			ScriptUICreateCollection("VillainUI", 3, "VillainTitleUI", "ScoreMeterUI", "BasesUI");

			// Winner Overtime
			ScriptUITimer("OvertimerUI", "OvertimeRemaining", "OvertimeTimeText", "OvertimeTimeTooltip");
			ScriptUITitle("WinnerTitleUI", "WinnerTitle", "WinnerInfo");

			ScriptUICreateCollection("WinnerUI", 3, "WinnerTitleUI", "ScoreMeterUI", "OvertimerUI" );

			// Loser Overtime
			ScriptUITitle("LoserTitleUI", "LoserTitle", "LoserInfo");
			ScriptUIList("LoserCountUI", "LoserTitle", "LoserCountText", "ffffffff", "LoserCount", "ffffffff");
			ScriptUICreateCollection("LoserUI", 4, "LoserTitleUI", "ScoreMeterUI", "OvertimerUI", "LoserCountUI" );


			//////////////////////////////////////////////////////////////////////////////////
			// create script NPCs
			// villain contact
			group = FindEncounterGroupByRadius( 
				"coord:0.0,0.0,0.0",
				0, 
				9999, 
				"NPC_Villain_Contact", 
				1, 
 				0 );  
			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Contacts", "scripts.loc/SpawnDefs/V_PvP_05_01/NPC_Contacts_D0_V1.spawndef", group, "NPC_Villain_Contact", 1, 0 );  

				// add on click handler
				OnScriptedContactInteract(CoVReclusesVictoryOnVillainContactClick, ActorFromEncounterPos(group, 1));

				// map markers
				VarSetNumber("VillainContact", CreateWaypoint(EncounterPos(group, 1), VarGet("VillainContactName"), "map_enticon_contact", NULL, false, false, -1.0f));
			}

			// hero contact
			group = FindEncounterGroupByRadius( 
				"coord:0.0,0.0,0.0",
				0, 
				9999, 
				"NPC_Hero_Contact", 
				1, 
 				0 );  
			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Contacts", "scripts.loc/SpawnDefs/V_PvP_04_01/NPC_Contacts_D0_V0.spawndef", group, "NPC_Hero_Contact", 1, 0 );  

				// add on click handler
				OnScriptedContactInteract(CoVReclusesVictoryOnHeroContactClick, ActorFromEncounterPos(group, 1));

				// map markers
				VarSetNumber("HeroContact", CreateWaypoint(EncounterPos(group, 1), VarGet("HeroContactName"), "map_enticon_contact", NULL, false, false, -1.0f));
			}

			// setup timers for contact
			TimerSet("HeroContactTalk", 0.25f);
			TimerSet("VillainContactTalk", 0.25f);

			//////////////////////////////////////////////////////////////////////////////////
			// register callbacks
			OnPlayerExitingMap( CoVReclusesVictoryOnLeaveMap );
			OnPlayerEnteringMap( CoVReclusesVictoryOnEnterMap );
			OnTeamRewarded ( CoVReclusesVictoryOnTeamReward );			// for PvP kills
			OnEntityDefeated ( CoVReclusesVictoryOnEntityDefeat );		// for overtime team kills


			// support setup
			VarSetNumber("SupportIssued", -1);

			SET_STATE("BaseCapture");

		END_ENTRY


	//////////////////////////////////////////////////////////////////////////////////
	STATE("BaseCapture") ////////////////////////////////////////////////////////

		ON_ENTRY
			VarSetNumber(COV_RECLUSES_VICTORY_VILLAIN_POINTS, 0);
			VarSetNumber(COV_RECLUSES_VICTORY_HERO_POINTS, 0);
		END_ENTRY

 
		// check for base owners too far from base & for defeat of all defenders
		if (DoEvery("BaseDistanceTimer", VarGetFraction("BaseDistanceTime"), NULL))
		{
			CoVReclusesVictoryCheckBunkerDistance();
			CoVReclusesVictoryCheckGlowieRespawn();
		}

		// update geometry
		CoVReclusesVictoryUpdateGeometryState();

		// update reinforcements
		CoVReclusesVictoryUpdateReinforcements();

	//////////////////////////////////////////////////////////////////////////////////
	STATE("BonusOvertime") ////////////////////////////////////////////////////////
		ON_ENTRY
			// give winners award
			CoVReclusesVictoryGiveReward(VarGetNumber("RoundWinner"), VarGetNumber("VictoryScore"));

			// broadcast message
			BroadcastAttentionMessage(VarGetNumber("RoundWinner")?VarGet("RoundWonMessageHero"):VarGet("RoundWonMessageVillain"), 0, 0);

			// change UIs
			if (VarGetNumber("RoundWinner")) 
			{	// heroes win
				VarSet("WinnerTitle", VarGet("OvertimeWinnerTextHeroes"));
				VarSet("WinnerInfo", VarGet("OvertimeWinnerInfoHeroes"));
				VarSet("LoserTitle", VarGet("OvertimeLoserTextVillains"));
				VarSet("LoserInfo", VarGet("OvertimeLoserInfoVillains"));
				VarSet("LoserCountText", VarGet("OvertimeLoserCountVillains"));
				CoVReclusesVictorySetAllTargets(-1, 100.0f);
				SkyFileFade(0, 2, 1.0f);
				MapSetDataToken("RVHeroScore", MapGetDataToken("RVHeroScore") + 1);
			} else {
				// villains win
				VarSet("WinnerTitle", VarGet("OvertimeWinnerTextVillains"));
				VarSet("WinnerInfo", VarGet("OvertimeWinnerInfoVillains"));
				VarSet("LoserTitle", VarGet("OvertimeLoserTextHeroes"));
				VarSet("LoserInfo", VarGet("OvertimeLoserInfoHeroes"));
				VarSet("LoserCountText", VarGet("OvertimeLoserCountHeroes"));
				CoVReclusesVictorySetAllTargets(-1, 0.0f);
				SkyFileFade(0, 1, 1.0f);
				MapSetDataToken("RVVillainScore", MapGetDataToken("RVVillainScore") + 1);
			}
			CoVReclusesVictorySetupUI();

			// reset kill count for losing team
			VarSetNumber("LoserCount", VarGetNumber("OvertimeDefeatCount"));

			// start timer
			TimerSet("OvertimeTimer", VarGetFraction("Overtime"));
			VarSetNumber("OvertimeRemaining", (VarGetFraction("Overtime") * 60) + SecondsSince2000());

		END_ENTRY

		// update geometry
		CoVReclusesVictoryUpdateGeometryState();

		if (TimerElapsed("OvertimeTimer"))
		{ 
			// winners win
			SET_STATE("Reset");
		}

		// check for base owners too far from base
		if (DoEvery("BaseDistanceTimer", VarGetFraction("BaseDistanceTime"), NULL))
		{
			CoVReclusesVictoryCheckBunkerDistance();
		}

	STATE("Reset") ////////////////////////////////////////////////////////

		// clean up reinforcements
		CoVReclusesVictoryCleanupReinforcements();

		// count scores and give rewards then reset
		CoVReclusesVictoryCheckScores();

		// set message to all players
		BroadcastAttentionMessage(VarGet("RoundResetMessage"), 0, 0);

		// find all of the capturable hotspots and set them up
		CoVReclusesVictorySetupAllBunkers(true);

		// reset sky
		SkyFileFade(0, 1, 0.0f);

		SET_STATE("BaseCapture");

		// change UIs back - must be after set state since it checks if the state is "BaseCapture"
		CoVReclusesVictorySetupUI();
	END_STATES

	ON_SIGNAL("ResetBunkers")
	{
		int i;
		int count = VarGetArrayCount("BunkerList");

		for (i = 0; i < count; i++)
		{
			STRING point = VarGetArrayElement("BunkerList", i);
			STRING bunker = StringAdd("marker:", point);

			// check and see if location exists
			if (LocationExists(bunker))
			{
				CoVReclusesVictoryResetBunker(i);
			}
		}
	}		
	END_SIGNAL

	ON_SIGNAL("EndScript")
		CoVReclusesVictoryShutdown();
	END_SIGNAL

	ON_SIGNAL("ResetGame")
	{
		SET_STATE("Reset");
	}
	END_SIGNAL

	ON_SIGNAL("Set100")
	{ 
		CoVReclusesVictorySetAllTargets(-1, 100.0f);
	}
	END_SIGNAL

	ON_SIGNAL("Set50")
	{ 
		CoVReclusesVictorySetAllTargets(-1, 50.0f);
	}
	END_SIGNAL

	ON_SIGNAL("Set0")
	{ 
		CoVReclusesVictorySetAllTargets(-1, 0.0f);
	}
	END_SIGNAL

	ON_SIGNAL("HeroesWin")
	{ 
		SET_STATE("BonusOvertime");
		VarSetNumber("RoundWinner", 1);
	}
	END_SIGNAL

	ON_SIGNAL("VillainsWin")
	{ 
		SET_STATE("BonusOvertime");
		VarSetNumber("RoundWinner", 0);
	}
	END_SIGNAL


	ON_STOPSIGNAL
		CoVReclusesVictoryShutdown();
	END_SIGNAL
}

void CoVReclusesVictoryInit()
{
	SetScriptName( "CoVReclusesVictory" );
	SetScriptFunc( CoVReclusesVictory );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptSignal( "End Event", "EndScript" );		
	SetScriptSignal( "Reset", "ResetGame" );		
	SetScriptSignal( "100", "Set100" );		
	SetScriptSignal( "50", "Set50" );		
	SetScriptSignal( "0", "Set0" );		
	SetScriptSignal( "HeroesWin", "HeroesWin" );		
	SetScriptSignal( "VillainsWin", "VillainsWin" );		



	SetScriptVar( "EventName",									NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "EventInfoHero",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "EventInfoVillain",							NULL,		SP_DONTDISPLAYDEBUG );
	
	SetScriptVar( "BunkerList",									NULL,		0 );
	SetScriptVar( "BunkerNameList",								NULL,		0 );
	SetScriptVar( "BunkerControlName",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerControlModel",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerControlInteract",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerControlInterrupt",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerControlComplete",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerControlInteractBar",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerControlInteractTime",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerGuardsNotCleared",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerCantBeCaptured",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerAlreadyControlled",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerNothingLeft",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerCapturedMessageHero",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerCapturedMessageVillain",				NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerReleaseDist",							"100",		0 );

//	SetScriptVar( "BunkerGeometryList",							NULL,		SP_DONTDISPLAYDEBUG );
//	SetScriptVar( "BunkerLinkList",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BunkerChangeRate",							NULL,		0 );

	SetScriptVar( "OvertimeTimeText",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeTimeTooltip",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeWinnerTextHeroes",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeWinnerInfoHeroes",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeWinnerTextVillains",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeWinnerInfoVillains",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeLoserTextHeroes",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeLoserInfoHeroes",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeLoserCountHeroes",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeLoserTextVillains",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeLoserInfoVillains",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "OvertimeLoserCountVillains",					NULL,		SP_DONTDISPLAYDEBUG );


	SetScriptVar( "RoundWonMessageHero",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "RoundWonMessageVillain",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "RoundResetMessage",							NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "HeroContactName",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroContactInstructions",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroGoAway",									NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactName",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactInstructions",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainGoAway",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ContactExitText",							NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "NumberRequiredForSupport",					NULL,		0 );
	SetScriptVar( "NumberOfSupportLocations",					NULL,		0 );
	SetScriptVar( "HeroSupportMessages",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroLocationTell",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainSupportMessages",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainLocationTell",						NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "GoalRewardHero",								NULL,		0 );
	SetScriptVar( "GoalRewardVillain",							NULL,		0 );
	SetScriptVar( "GoalScore",									NULL,		0 );
	SetScriptVar( "PvPReward",									NULL,		0 );

	SetScriptVar( "TurretEntitiesList",							NULL,		0 ); // done
	SetScriptVar( "SigEntitiesList",							NULL,		0 ); // done
	SetScriptVar( "TurretScore",								NULL,		0 ); // done mult
	SetScriptVar( "CaptureScore",								NULL,		0 ); // done
	SetScriptVar( "PvPKillScore",								NULL,		0 ); // done mult
	SetScriptVar( "DeathScore",									NULL,		0 ); // done 
	SetScriptVar( "SigKillScore",								NULL,		0 ); // done
	SetScriptVar( "VictoryScore",								NULL,		0 ); // done
	SetScriptVar( "VictoryScoreMult",							NULL,		0 ); // 

	SetScriptVar( "ScoreBarName",								NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "NumberRequiredForWin",						NULL,		0 );
	SetScriptVar( "Overtime",									NULL,		0 );
	SetScriptVar( "OvertimeDefeatCount",						NULL,		0 );
	SetScriptVar( "AlwaysOn",									"1",		SP_OPTIONAL );
	SetScriptVar( "BaseDistanceTime",							"0.05",		SP_OPTIONAL );
	SetScriptVar( "ForceFieldPosition",							"5",		SP_OPTIONAL );

}