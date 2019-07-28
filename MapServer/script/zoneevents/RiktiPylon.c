// ZONE EVENT SCRIPT

// This script runs the Rikti Warzone Pylon Game.
//

#include "scriptutil.h"
#include "NwWrapper.h"	// nwSetRagdollCount

int *eaRiktiPylonMarkers = NULL;
int *eaRiktiPylonRespawn = NULL;
char **eaRiktiPylonGlowies = NULL;
int *eaRiktiPylonGlowiesBoom = NULL;

#define DONOTGOTOSLEEP_PL	"DoNotGoToSleep(1)"

#define CLEANUP_PL			"DoNotGoToSleep(1), RiktiShipTroopLeave"
 
/////////////////////////////////////////////////////////////////////////
// Enter and Exit
/////////////////////////////////////////////////////////////////////////

// called when the player leaves the map
int RiktiPylonOnLeaveMap(ENTITY player)
{
	// remove waypoints
	ClearAllWaypoints(player);

	return 0;
}

// called when the player enters the map
int RiktiPylonOnEnterMap(ENTITY player)
{
	int i;
	int count = VarGetNumber("PylonCount");

	// Set waypoints correctly
	for (i = 0; i < count; i++)
	{
		if (eaRiktiPylonMarkers[i] != 0)
			SetWaypoint(player, eaRiktiPylonMarkers[i]);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////
// Master-At-Arms 
/////////////////////////////////////////////////////////////////////////

// Generic spawn function
void RiktiPylonSpawnBoss(STRING layout, STRING marker, STRING team)
{ 
	ENCOUNTERGROUP group = "first";

	// spawn 
	group = FindEncounterGroupByRadius( 
		marker,
		0, 
		100, 
		layout, 
		1, 
		0 );  

	if (stricmp(group, "location:none") != 0) 
	{
		Spawn( 1, team, "UseCanSpawns", group, layout, 54, 0 );
		VarSet("BossEnt", ActorFromEncounterPos(group, 1));
	}
}


void RiktiPylonMasterAtArmsSpawn()
{
	// spawn boss and friends
	RiktiPylonSpawnBoss("RiktiShipBoss", "marker:RiktiShield", "Invaders");

	VarSetNumber("BossSpeech", 0);

	// add to timer
	VarSetFraction("CurrentShieldDownTime", VarGetFraction("CurrentShieldDownTime") + VarGetFraction("MasterAtArmsTime"));
	TimerSet("RespawnTimer", TimerRemain("RespawnTimer") + VarGetFraction("MasterAtArmsTime"));

	// set Master-at-Arms flag
	VarSetNumber("MasterSpawned", 1);

}

void RiktiPylonSpawnPatrol()
{
	if (!VarGetNumber("PatrolSpawned"))
	{
		if (RandomNumber(0, 100) < VarGetNumber("PatrolChance"))
		{
			STRING group = FindEncounterGroupByRadius("marker:RiktiPylonPatrol", 0, 100, VarGet("PatrolSpawnLoc"), 1, 0);
			if (stricmp(group, "location:none") != 0)
			{
				Spawn(1, "Patrol", VarGet("PatrolSpawnDef"), group, VarGet("PatrolSpawnLoc"), 54, 0);
				VarSetNumber("PatrolSpawned", 1);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////
// Shield Up Critter Kill
/////////////////////////////////////////////////////////////////////////
void RiktiPylonShieldUpCritterKill(void)
{ 
	int j;
	int	groupNumber = NumEntitiesInTeam( ALL_CRITTERS );	
	int numKilled = 0;

	for( j = 1 ; j <= groupNumber && numKilled < 10; j++ )
	{
		ENTITY player = GetEntityFromTeam(ALL_CRITTERS, j); 

		if (DistanceBetweenLocations("marker:RiktiShield", PointFromEntity(player)) < 1000.0f)  // optimization since EntityInArea is more expensive
		{
			if (EntityInArea(player, VarGet("TeleportVolume")))
			{
				STRING name = GetVillainDefClassName(player);

				if (StringEqual(name, "Class_Boss_Shield"))
				{
					continue;
				}
				SetAIPriorityList(player, CLEANUP_PL);  
				numKilled++;
			}
		}
	} 
}

/////////////////////////////////////////////////////////////////////////
// Defeat Tracking
/////////////////////////////////////////////////////////////////////////

void RiktiPylonOnEntityDefeat(ENTITY player, ENTITY victor) 
{
	int i, count;
	int foundLive = false;

	// check for desired type of critter
	if (IsEntityCritter(player))
	{
		STRING name = GetVillainDefClassName(player);

		if (StringEqual(name, VarGet("MonsterClass")))
		{
			count = VarGetNumber("PylonCount");
			for (i = 0; i < count; i++)
			{
				STRING team = StringAdd("RiktiPylon", NumberToString(i + 1));

				if (eaRiktiPylonRespawn[i] == 0 && NumEntitiesInTeam(team) == 0)
				{
					// set respawn time
					eaRiktiPylonRespawn[i] = SecondsSince2000() + VarGetNumber("PylonRespawnTime") * 60;

					// remove marker
					if (eaRiktiPylonMarkers[i] != 0)
					{
						DestroyWaypoint(eaRiktiPylonMarkers[i]);
						eaRiktiPylonMarkers[i] = 0;
					}
				}

				if (eaRiktiPylonRespawn[i] == 0)
					foundLive = true;
			}

			// check to see if all of the pylons are dead
			if (!foundLive && !StringEqual(GET_STATE, "ShieldDown"))
				SET_STATE("ShieldDown");

			// Attempt to spawn the the Patrol if at least one Pylon is alive.
			if (foundLive) RiktiPylonSpawnPatrol();

		} 
		else if (VarGetNumber("MasterSpawned") > 0)
		{
			// check for defeat of Master-at-Arms
			if (StringEqual(player, VarGet("BossEnt")))
			{
				// add to timer
				VarSetFraction("CurrentShieldDownTime", VarGetFraction("CurrentShieldDownTime") + VarGetFraction("MasterAtArmsDownTime"));
				TimerSet("RespawnTimer", TimerRemain("RespawnTimer") + VarGetFraction("MasterAtArmsDownTime"));
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////
// Spawning
/////////////////////////////////////////////////////////////////////////

// Generic spawn function
void RiktiPylonSpawn(STRING layout, STRING marker, STRING team)
{ 
	ENCOUNTERGROUP group = "first";

	// spawn 
	group = FindEncounterGroupByRadius( 
		marker,
		0, 
		1000, 
		layout, 
		1, 
		0 );  

	if (stricmp(group, "location:none") != 0) 
	{
		Spawn( 1, team, "UseCanSpawns", group, layout, 54, 0 );
	}
}


// Invader wave spawns
void RiktiPylonSpawnWaves(void)
{ 
	STRING marker = "marker:RiktiShield";
	ENCOUNTERGROUP group = "first";
	NUMBER groups = 0;

	while (stricmp(group, "location:none") != 0 && groups < VarGetNumber("WaveSpawnMax"))
	{
		// spawn 
		group = FindEncounterGroupByRadius( 
			marker,
			0, 
			VarGetFraction("EncounterClearRadius"), 
			"RiktiShipWave", 
			1, 
			0 );  

		if (stricmp(group, "location:none") != 0) 
		{
			Spawn( 1, "InvadersTemp", "UseCanSpawns", group, "RiktiShipWave", 54, 0 );
			AddAIBehaviorPermFlag("InvadersTemp", DONOTGOTOSLEEP_PL);
			SwitchScriptTeam("InvadersTemp", "InvadersTemp", "Invaders");
		}
		groups++;
	}
}


// Pylon spawn (singular)
void RiktiPylonSpawnPylon(NUMBER location)
{ 
	STRING layout = StringAdd("RiktiPylon", NumberToString(location + 1));
	STRING marker = StringAdd("marker:RiktiPylon", NumberToString(location + 1));
	STRING pylonName = StringAdd(StringLocalize(VarGet("PylonPinName")), " ");
	ENCOUNTERGROUP group = "first";

	pylonName = StringAdd(pylonName, NumberToString(location + 1));
	RiktiPylonSpawn(layout, marker, layout);
	SetScriptTeam(layout, "Pylons");
	eaRiktiPylonRespawn[location] = 0;

	eaRiktiPylonMarkers[location] = CreateWaypoint(marker, pylonName, 
													"map_enticon_code_fullB", NULL, false, 
													false, -1.0f);

	if (eaRiktiPylonMarkers[location] > 0)
		SetWaypoint(ALL_PLAYERS, eaRiktiPylonMarkers[location]);

}

// Pylon spawn (all)
void RiktiPylonSpawnAllPylons(void)
{ 
	int i, count = VarGetNumber("PylonCount");

	for (i = 0; i < count; i++)
	{
		RiktiPylonSpawnPylon(i);
	}
	AddAIBehaviorPermFlag("Pylons", DONOTGOTOSLEEP_PL);
}

void RiktiPylonCheckRespawn(void)
{
	int i, count = VarGetNumber("PylonCount");

	for (i = 0; i < count; i++)
	{
		if (eaRiktiPylonRespawn[i] > 0 && eaRiktiPylonRespawn[i] < SecondsSince2000())
			RiktiPylonSpawnPylon(i);
	}
}

/////////////////////////////////////////////////////////////////////////
// Bomb Glowies
/////////////////////////////////////////////////////////////////////////

int RiktiPylonAllowedToPlantBomb(ENTITY player)
{
	STRING token = VarGet("BombToken");
	return (!EntityHasRewardToken(player, token) || 
				EntityGetRewardTokenTime(player, token) + VarGetNumber("BombRewardTimer") < SecondsSince2000());
}

// This function is called when the player initiates the bomb placement.  It checks to see if the player has 
//		recently placed a bomb
int RiktiPylonBombInteract(ENTITY player, ENTITY target)
{
	if (RiktiPylonAllowedToPlantBomb(player))
	{
		return false;
	} else {
		SendPlayerDialog(player, StringLocalize(VarGet("BombGottenTooRecently")));
		return true;
	}
}

// This function is called when the player has successfully completed the bomb placement timer.
int RiktiPylonBombComplete(ENTITY player, ENTITY target)
{
	NUMBER i;
	STRING runawayName;
	STRING token = VarGet("BombToken");

	// this only matters if the shield is down
	if (!StringEqual(GET_STATE, "ShieldDown"))
		return true;

	// Check to see if player does not have token.  Another member of the their team might have completed a
	//		bomb placement while this one was in progress.
	if (RiktiPylonAllowedToPlantBomb(player))
	{
		// give reward and token to entire team
		TEAM victors = GetPlayerTeamFromPlayer(player);
		int countKillers = NumEntitiesInTeam(victors);
		NUMBER bombsPlaced = VarGetNumber("BombPlacedCount") + 1;

		// add bounty to victors
		for (i = 1; i <= countKillers; i++)
		{
			ENTITY killer = GetEntityFromTeam(victors, i);

			if (RiktiPylonAllowedToPlantBomb(killer))
			{
				// give credit
				GiveBadgeCredit(killer, kScriptBadgeTypeRWZBombPlace);

				// give reward token
				EntityGrantRewardToken(killer, token, 1);

				// give phat loot
				EntityGrantSalvage(killer, VarGet("BombReward"), VarGetNumber("BombRewardAmount"));

				// display message
				SendPlayerAttentionMessageWithColor(killer, VarGet("BombRewardFloat"), 0xffff00ff, kFloaterStyle_PriorityAlert);
			}

		}

		// add to the count
		VarSetNumber("BombPlacedCount", bombsPlaced);

		// if MaA has not spawned 
		if (VarGetNumber("MasterSpawned") == 0)
		{
			NUMBER playerBand = (NumEntitiesInTeam( ALL_PLAYERS ) / 25);	
			
			if (playerBand >= VarGetArrayCount("BombCountList"))
				playerBand = VarGetArrayCount("BombCountList") - 1;

			// check to see if we should spawn MaA
			if (bombsPlaced >= StringToNumber(VarGetArrayElement("BombCountList", playerBand)))
			{
				RiktiPylonMasterAtArmsSpawn();
			} else {
				// add to timer
				VarSetFraction("CurrentShieldDownTime", VarGetFraction("CurrentShieldDownTime") + VarGetFraction("BombPlacedTime"));
				TimerSet("RespawnTimer", TimerRemain("RespawnTimer") + VarGetFraction("BombPlacedTime"));
			}
		}

		// setup respawn timer
		for (i = 0; i < VarGetNumber("GlowieCount"); i++)
		{
			if (StringEqual(eaRiktiPylonGlowies[i], target))
			{
				// figure respawn time
				eaRiktiPylonGlowiesBoom[i] = SecondsSince2000() + VarGetNumber("BombExplodeTime");

				// spawn runaway critter
				runawayName = StringAdd("RiktiRunAway", NumberToString(i + 1));
				RiktiPylonSpawn(runawayName, "marker:RiktiShield", runawayName);

				break;
			}
		}


	}
	return true;
}

void RiktiPylonCreateGlowie(NUMBER location)
{
	GLOWIEDEF bombDef = NULL; 
	ENTITY glowie;
	STRING loc = StringAdd("marker:RiktiClick", NumberToString(location + 1));

	if (eaRiktiPylonGlowies[location] == NULL)
	{
		// create glowie 
		bombDef = GlowieCreateDef(StringLocalize(VarGet("BombControlName")), 
			VarGet("BombControlModel"), 
			VarGet("BombControlInteract"), VarGet("BombControlInterrupt"), 
			VarGet("BombControlComplete"), VarGet("BombControlInteractBar"), 
			VarGetNumber("BombControlInteractTime"));

		glowie = GlowiePlace(bombDef, loc, false, RiktiPylonBombInteract, RiktiPylonBombComplete);
		SetScriptTeam(glowie, "glowies");

		eaRiktiPylonGlowies[location] = strdup(glowie);
	}
}

void RiktiPylonBombExplosionCheck(void)
{
	int i, count = VarGetNumber("GlowieCount");
	STRING explosionName;
	STRING runawayName;

	for (i = 0; i < count; i++)
	{
		if (eaRiktiPylonGlowies[i] != NULL && 
			eaRiktiPylonGlowiesBoom[i] > 0 && 
			eaRiktiPylonGlowiesBoom[i] < SecondsSince2000())
		{
			GlowieRemove(eaRiktiPylonGlowies[i]);
			SAFE_FREE(eaRiktiPylonGlowies[i]);
			eaRiktiPylonGlowiesBoom[i] = 0;

			// spawn bomb
			explosionName = StringAdd("RiktiGrateExplosion", NumberToString(i + 1));
			RiktiPylonSpawn(explosionName, "marker:RiktiShield", "grate");

			// clear up runaway critter
			runawayName = StringAdd("RiktiRunAway", NumberToString(i + 1));
			Kill(runawayName, false);

		}
	}
}

void RiktiPylonBombCleanup(void)
{
	int i, count = VarGetNumber("GlowieCount");

	for (i = 0; i < count; i++)
	{
		if (eaRiktiPylonGlowies[i] != NULL)
		{
			GlowieRemove(eaRiktiPylonGlowies[i]);
			SAFE_FREE(eaRiktiPylonGlowies[i]);
			eaRiktiPylonGlowiesBoom[i] = 0;
		}
	}
}


/////////////////////////////////////////////////////////////////////////
// Util
/////////////////////////////////////////////////////////////////////////

void RiktiPylonResetTimers(void)
{
	TimerRemove("TeleportTimer");
	TimerRemove("RespawnTimer");
	TimerRemove("KillTimer");
	TimerRemove("StateTimer");
	TimerRemove("WaveTimer");
	TimerRemove("BossTimer");
}

void RiktiPylonShutdown(void)
{
	Kill("Pylons", false);
	Kill("Invaders", false);
	Kill("grate", false);
	Kill("shield", false);

	OnPlayerExitingMap( NULL );
	OnPlayerEnteringMap( NULL );

	EndScript();
}

/////////////////////////////////////////////////////////////////////////
// M A I N
/////////////////////////////////////////////////////////////////////////

void RiktiPylon(void)
{  
	int i, count;

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////
 
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// if there are too many running, don't spawn another
			if (ScriptCountRunningInstances("RiktiPylon") > 1)
			{
				EndScript();
				return;
			}

			// setup kill callback
			OnEntityDefeated ( RiktiPylonOnEntityDefeat );

			// setup Pylon tracking
			count = VarGetNumber("PylonCount");
			eaiCreate(&eaRiktiPylonRespawn);
			eaiSetSize(&eaRiktiPylonRespawn, count);
			for (i = 0; i < count; i++)
			{
				eaRiktiPylonRespawn[i] = 0;
			}

			eaiCreate(&eaRiktiPylonMarkers);
			eaiSetSize(&eaRiktiPylonMarkers, count);
			for (i = 0; i < count; i++)
			{
				eaRiktiPylonMarkers[i] = 0;
			}
			
			// setup glowie tracking
			count = VarGetNumber("GlowieCount");
			eaiCreate(&eaRiktiPylonGlowiesBoom);
			eaiSetSize(&eaRiktiPylonGlowiesBoom, count);
			eaCreate(&eaRiktiPylonGlowies);
			eaSetSize(&eaRiktiPylonGlowies, count);
			for (i = 0; i < count; i++)
			{
				eaRiktiPylonGlowiesBoom[i] = 0;
				eaRiktiPylonGlowies[i] = NULL;
			}

			// set up map callbacks
			OnPlayerExitingMap( RiktiPylonOnLeaveMap );
			OnPlayerEnteringMap( RiktiPylonOnEnterMap );

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		SET_STATE("ShieldUp");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("ShieldUp") ////////////////////////////////////////////////////////
 
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// spawn shield
			RiktiPylonSpawn("MotherShipShield", "marker:RiktiShield", "shield");

			// Kill invaders
			SetAIPriorityList("Invaders", CLEANUP_PL); 

			// spawn pylons
			RiktiPylonSpawnAllPylons();

			// clear grates
			Kill("grate", false);

			// cleanup bombs
			RiktiPylonBombCleanup();

			// reset timers
			RiktiPylonResetTimers();
			VarSetNumber("PatrolSpawned", 0);

#if !BEACONIZER
			// enable ragdoll
			nwSetRagdollCount(MAX_NX_SCENES / 2);
#endif

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// teleport check - done with powers now
//		DoEvery("TeleportTimer", VarGetFraction("TeleportCheckTime"), RiktiPylonTeleportCheckPlayers);  // every second

		// Check for respawns of Pylons
		if (DoEvery("RespawnTimer", VarGetFraction("RespawnCheckTime"), NULL))
		{			
			RiktiPylonCheckRespawn();
		}

		// Check for critters still in shield
		if (DoEvery("KillTimer", 0.25f, NULL))
		{			
			RiktiPylonShieldUpCritterKill();
		}

		VarSetNumber("CurrentPylonCount", NumEntitiesInTeam("Pylons"));

	//////////////////////////////////////////////////////////////////////////////////
	STATE("ShieldDown") ////////////////////////////////////////////////////////
 

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

#if !BEACONIZER
			// set ragdolls to minimum to save on CPU usage
			nwSetRagdollCount(1);
#endif

			// despawn shield
			Kill("shield", false);

			// clean up any invaders that are still around
			Kill("Invaders", false);

			// clean up any pylons
			Kill("Pylons", false);

			// clear grate explosions if any are still around
			Kill("grate", false);

			// spawn grates
			RiktiPylonSpawn("RiktiGrate", "marker:RiktiShield", "grate");

			// create glowies
			count = VarGetNumber("GlowieCount");
			for (i = 0; i < count; i++)
			{
				RiktiPylonCreateGlowie(i);
			}

			// spawn invaders
			RiktiPylonSpawnWaves();

			// let them know the shield is down
			SendPlayerSystemMessage(ALL_PLAYERS, StringLocalize(VarGet("ShieldDownMessage")));
			BroadcastAttentionMessageWithColor(VarGet("ShieldDownFloat"), 0, 0, 0xffff00ff, kFloaterStyle_PriorityAlert);

			// reset timers
			RiktiPylonResetTimers();
		
			// reset bomb count
			VarSetNumber("BombPlacedCount", 0);

			// reset Master-at-Arms flags
			VarSetNumber("MasterSpawned", 0);
			VarSetNumber("BossSpeech", 0);

			// set initial timer count
			VarSetFraction("CurrentShieldDownTime", VarGetFraction("ShieldDownTime"));


		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// check for new wave
		if (DoEvery("WaveTimer", VarGetFraction("WaveTime"), NULL))
		{
			RiktiPylonSpawnWaves();
		}

		// timer to end shield down
		if (DoEvery("RespawnTimer", VarGetFraction("CurrentShieldDownTime"), NULL))
		{
			SET_STATE("ShieldRecovery"); 
		}

		// check for bomb explosions
		DoEvery("KillTimer", 0.05f, RiktiPylonBombExplosionCheck);

		// update invader count
		VarSetNumber("CurrentInvaderCount", NumEntitiesInTeam("Invaders"));

		// check for boss dialog
		if (VarGetNumber("MasterSpawned") > 0 && VarGetNumber("BossSpeech") < 1)
		{
			if (DoEvery("BossTimer", 0.35f, NULL))
			{
				Say(VarGet("BossEnt"), VarGet("BossMessage"), 2);
				VarSetNumber("BossSpeech", 1);
			}
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("ShieldRecovery") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// give players the boot - should be done with powers now

			// let them know the shield is coming back up
			SendPlayerSystemMessage(ALL_PLAYERS, StringLocalize(VarGet("ShieldUpMessage")));
			BroadcastAttentionMessageWithColor(VarGet("ShieldUpFloat"), 0, 0, 0xffff00ff, kFloaterStyle_PriorityAlert);

			// reset timers
			RiktiPylonResetTimers();

			SetAIPriorityList("Invaders", CLEANUP_PL); 

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


		if (DoEvery("StateTimer", 0.25f, NULL))
		{			
			SET_STATE("ShieldUp");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		RiktiPylonShutdown();

	END_STATES

	ON_STOPSIGNAL
		RiktiPylonShutdown();
	END_SIGNAL

	ON_SIGNAL("Up")
		SET_STATE( "ShieldRecovery" );
	END_SIGNAL

	ON_SIGNAL("Down")
		SET_STATE( "ShieldDown" );
	END_SIGNAL


	ON_SIGNAL("EndScript")
		RiktiPylonShutdown();
	END_SIGNAL

}


void RiktiPylonInit()
{
	SetScriptName( "RiktiPylon" );
	SetScriptFunc( RiktiPylon );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "TeleportCheckTime",						"0.0167",					SP_OPTIONAL );
	SetScriptVar( "MasterTime",								"15.0",						SP_OPTIONAL );

	// Shield Down Timers
	SetScriptVar( "ShieldDownTime",							"10.0",						SP_OPTIONAL );		// initial time on ship
	SetScriptVar( "BombPlacedTime",							"3.0",						SP_OPTIONAL );		// time granted for each bomb placed - doesn't count bomb that spawns MaA
	SetScriptVar( "MasterAtArmsTime",						"10.0",						SP_OPTIONAL );		// time granted for spawning MaA
	SetScriptVar( "MasterAtArmsDownTime",					"5.0",						SP_OPTIONAL );		// time granted for defeating MaA

	SetScriptVar( "PylonRespawnTime",						"30.0",						SP_OPTIONAL );
	SetScriptVar( "WaveTime",								"1.0",						SP_OPTIONAL );
	SetScriptVar( "WaveSpawnMax",							"30",						SP_DONTDISPLAYDEBUG|SP_OPTIONAL );

	SetScriptVar( "EncounterClearRadius",					"750.0",					SP_OPTIONAL );
	SetScriptVar( "TeleportVolume",							"trigger:ShieldVolume",		SP_OPTIONAL );
	SetScriptVar( "TeleportCount",							"4",						SP_OPTIONAL );
	SetScriptVar( "PylonCount",								"18",						SP_OPTIONAL );
	SetScriptVar( "GlowieCount",							"18",						SP_OPTIONAL );
	SetScriptVar( "GlowieCountRequired",					"3",						SP_OPTIONAL );
	SetScriptVar( "MonsterClass",							"Class_Boss_Archvillain",	SP_OPTIONAL );
	SetScriptVar( "ShieldDownMessage",						"",							SP_OPTIONAL );
	SetScriptVar( "ShieldUpMessage",						"",							SP_OPTIONAL );
	SetScriptVar( "ShieldDownFloat",						"",							SP_OPTIONAL );
	SetScriptVar( "ShieldUpFloat",							"",							SP_OPTIONAL );

	SetScriptVar( "BossMessage",							"",							SP_DONTDISPLAYDEBUG );

	SetScriptVar( "PylonPinName",							"",							SP_DONTDISPLAYDEBUG );

	SetScriptVar( "BombControlName",						NULL,						SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BombControlModel",						NULL,						SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BombControlInteract",					NULL,						SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BombControlInterrupt",					NULL,						SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BombControlComplete",					NULL,						SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BombControlInteractBar",					NULL,						SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BombControlInteractTime",				NULL,						SP_DONTDISPLAYDEBUG );

	SetScriptVar( "BombCountList",							"",							0 );
	SetScriptVar( "BombToken",								"RiktiWarzoneBomb",			SP_OPTIONAL );
	SetScriptVar( "BombReward",								"",							0 );
	SetScriptVar( "BombRewardAmount",						"",							0 );
	SetScriptVar( "BombRewardFloat",						"",							SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BombRewardTimer",						"1800",						SP_OPTIONAL );
	SetScriptVar( "BombExplodeTime",						"10",						SP_OPTIONAL );
	SetScriptVar( "BombGottenTooRecently",					NULL,						SP_DONTDISPLAYDEBUG );

	SetScriptVar( "PatrolChance",							"10",						SP_OPTIONAL );
	SetScriptVar( "PatrolSpawnLoc",							"",							SP_DONTDISPLAYDEBUG );
	SetScriptVar( "PatrolSpawnDef",							"",							SP_DONTDISPLAYDEBUG );

	SetScriptSignal( "Up", "Up" );		
	SetScriptSignal( "Down", "Down" );		
	SetScriptSignal( "EndScript", "EndScript" );		
}

