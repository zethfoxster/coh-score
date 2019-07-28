// ZONE EVENT SCRIPT
// This script runs the Nemesis Plot Event intended for the 8th Anniverary
// Draws heavily from the Rikti Invastion script

#include "scriptutil.h"

#define NEMESIS_PLOT_GANG	9

#define CLEANUP_BEHAVIOR		"DoNothing(AnimList(Praetorian_TeleportOut),Timer(2)),DestroyMe"
#define KILL_BEHAVIOR			"DestroyMe"
#define TROOP_SPAWN_BEHAVIOR	"Team(Villain2),Gang(9),ConsiderEnemyLevel(0),DoNotGoToSleep,DoNothing(Untargetable,DoNotDrawOnClient,Timer(Rand(1,10))),DoNothing(Untargetable,AnimList(Praetorian_TeleportIn),Timer(2)),Combat"
#define PET_SPAWN_BEHAVIOR		"Flag(FollowDistance(60)),ConsiderEnemyLevel(0),DoNotGoToSleep,DoNothing(Untargetable,AnimList(Praetorian_TeleportIn),Timer(2))"
#define I_AM_NEMESIS_TOKEN		"IAmNemesis"
#define SPAWN_NEMESIS_TOKEN		"SpawnNemesis"
#define NEMESIS_PLOT_BADGE		"NemesisPlot"

/////////////////////////////////////////////////////////////////////////
// Counting Ents
/////////////////////////////////////////////////////////////////////////

static void NemesisPlotCountEnts(LOCATION loc, NUMBER *players, NUMBER *nemesis)
{
	#define MAXENTS 100
	ENTITY entlist[MAXENTS];
	NUMBER i;
	NUMBER nemesisCount;
	FRACTION nemesisWeight = 0.0;

	// Look for nearby Nemesis
	if (nemesis != NULL) 
	{
		nemesisCount = GetAllEntities( entlist, 
									  0,								// villain group
									  0,								// villain def
									  0,								// villain tag
									  0,								// ent type - Not yet implemented
									  "Troops",							// script team
									  50.0f,							// radius to check 
									  loc,								// location to check
									  0,								// area
									  1,								// isAlive
									  0,								// isDead
									  MAXENTS,							// max ents
									  0									// only once
									);

		for (i = 0; i < nemesisCount; i++)
		{
			STRING class = GetVillainDefClassName(entlist[i]);
			if (StringEqual(class, "Class_Boss_Monster"))
				nemesisWeight += 9.0f;
			else if (StringEqual(class, "Class_Boss_Elite"))
				nemesisWeight += 3.0f;
			else if (StringEqual(class, "Class_Boss_Grunt"))
				nemesisWeight += 1.5f;
			else if (StringEqual(class, "Class_Lt_Grunt"))
				nemesisWeight += 0.75f;
			else 
				nemesisWeight += 0.33f;
		}
		*nemesis = (NUMBER) nemesisWeight;
	}

	// Look for nearby Players
	if (players != NULL)
		*players = GetAllEntities( entlist, 
									0,								// villain group
									0,								// villain def
									0,								// villain tag
									0,								// ent type - Not yet implemented
									ALL_PLAYERS,					// script team
									50.0f,							// radius to check 
									loc,							// location to check
									0,								// area
									1,								// isAlive
									0,								// isDead
									MAXENTS,						// max ents
									0								// only once
								);
}

static void NemesisPlotCountPets(ENTITY owner, NUMBER *nemesis)
{
	#define MAXENTS 100
	ENTITY entlist[MAXENTS];
	NUMBER i;
	NUMBER nemesisCount;
	FRACTION nemesisWeight = 0.0;

	if(nemesis != NULL)
	{
		nemesisCount = GetAllPets( owner,
									entlist, 
									0,								// villain group
									0,								// villain def
									"NemesisPlot",					// villain tag
									0,								// ent type - Not yet implemented
									0,								// script team - Not yet implemented
									0,								// radius to check 
									0,								// location to check
									0,								// area
									1,								// isAlive
									0,								// isDead
									MAXENTS							// max ents
									 );
		for (i = 0; i < nemesisCount; i++)
		{
			STRING class = GetVillainDefClassName(entlist[i]);
			if (StringEqual(class, "Class_Boss_Monster"))
				nemesisWeight += 9.0f;
			else if (StringEqual(class, "Class_Boss_Elite"))
				nemesisWeight += 3.0f;
			else if (StringEqual(class, "Class_Boss_Grunt"))
				nemesisWeight += 1.5f;
			else if (StringEqual(class, "Class_Lt_Grunt"))
				nemesisWeight += 0.75f;
			else 
				nemesisWeight += 0.33f;
		}
		*nemesis = (NUMBER) nemesisWeight;
	}
}

/////////////////////////////////////////////////////////////////////////
// Marker and Geometry Processing
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// Safezone
/////////////////////////////////////////////////////////////////////////

static int NemesisPlotIsEntityNearHospital(ENTITY player)
{
	if (LocationExists("marker:Hospital"))
	{
		return (DistanceBetweenLocations("marker:Hospital", PointFromEntity(player)) < VarGetFraction("SafeZoneRadius"));
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////
// Spawning
/////////////////////////////////////////////////////////////////////////


// Generic spawn function
static void GenericSpawn(STRING layout, STRING marker, STRING team)
{ 
	ENCOUNTERGROUP group = "first";

	// spawn 
	group = FindEncounterGroupByRadius( 
		marker,
		0, 
		500, 
		layout, 
		1, 
		0 );  

	if (stricmp(group, "location:none") != 0) 
	{
		Spawn( 1, team, "UseCanSpawns", group, layout, 54, 0 );
	}
}

static void SpawnNemesisHimself(ENTITY target, TEAM nemesisHimself)
{
	STRING villain = VarGetArrayElement("NemesisHimself", RandomNumber(0, VarGetArrayCount("NemesisHimself") - 1));

	if(NumEntitiesInTeam(target) > 0)
	{
		CreateVillainNearEntity(nemesisHimself, villain, VarGetNumber("TroopLevel"), NULL, target, 20.0f);
	
		SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("NemesisHimselfMessage"), 1,
															"zonename", StringLocalizeMenuMessages(GetMapName())));

		VarSetNumber("NemesisWaypoint", CreateWaypoint(EntityPos(nemesisHimself), VarGet("NemesisWaypointName"), VarGet("NemesisWaypointIcon"), NULL, 0, 0, 0.0f));
		SetWaypoint(ALL_PLAYERS, VarGetNumber("NemesisWaypoint"));

		VarSetNumber("NemesisHimselfSpawned", 1);
	}
}

static void NemesisPlotSpawnPets()
{
	#define MAXENTS 100
	NUMBER idx, i, numPlayers, nemToken, nemesisCount, loop;
	ENTITY target;
	char buf[2000];

	numPlayers = NumEntitiesInTeam(ALL_PLAYERS);

	for(idx = 1; idx <= numPlayers; idx++)
	{
		loop = 0;
		target = GetEntityFromTeam(ALL_PLAYERS, idx);

		// Spawn Nemesis Himself
		if(EntityGetRewardToken(target, SPAWN_NEMESIS_TOKEN) > 0)
		{
			SpawnNemesisHimself(target, "NewNemesisPet");
			
			SetAsControlledPets(target, "NewNemesisPet");
			
			Say("NewNemesisPet", VarGet("NemesisHimselfShout"), 2);

			AddAIBehavior("NewNemesisPet", PET_SPAWN_BEHAVIOR);
			
			SwitchScriptTeam("NewNemesisPet", "NewNemesisPet", "NemesisHimself");

			EntityRemoveRewardToken(target, SPAWN_NEMESIS_TOKEN);
		}

		nemToken = EntityGetRewardToken(target, I_AM_NEMESIS_TOKEN);
		if(nemToken <= 0)
			continue;

		do
		{
			STRING troopList = "TroopList";
			NUMBER nemesisWeight = 0;
			NUMBER targetTeamCount = 0;

			NemesisPlotCountPets(target, &nemesisCount);

			if (nemToken > (6 * nemesisCount))
			{	
				// spawn really big dudes
				targetTeamCount = MIN(8, nemToken - nemesisCount) / 3;
				troopList = "TroopListEB";
			} 
			else if (nemToken > (4 * nemesisCount))
			{	
				// spawn big dudes
				targetTeamCount = MIN(8, nemToken - nemesisCount) / 2;
				troopList = "TroopListBoss";
			}
			else if (nemToken > nemesisCount)
			{
				// spawn dudes
				targetTeamCount = MIN(8, nemToken - nemesisCount);
			}

			for (i = 0; i < targetTeamCount; i++)
			{
				STRING villain = VarGetArrayElement(troopList, RandomNumber(0, VarGetArrayCount(troopList) - 1));

				CreateVillainNearEntity("NewNemesisPet", villain, VarGetNumber("TroopLevel"), NULL, target, 20.0f);

				// set as Pet of the Nemesis stand-in
				SetAsControlledPets(target, "NewNemesisPet");

				// debug: give a shoutout when you spawn
				if(VarGetNumber("Debug") > 0)
				{
					sprintf(buf, "<shout t 0>T:%s - %s. P%d N%d TT%d/%d", GetDisplayName(target), villain, nemToken, nemesisCount, i+1, targetTeamCount);
					Say("NewNemesisPet", buf, 2);
				}

				// set AI Behavior
				AddAIBehavior("NewNemesisPet", PET_SPAWN_BEHAVIOR);

				// put on Pet Team
				SwitchScriptTeam("NewNemesisPet", "NewNemesisPet", "NemesisPets");
			}

			loop++;
		}
		while(nemesisCount < nemToken && loop < 10);
	}
}

static void NemesisPlotUpdateWaypoint()
{
	if (VarGetNumber("NemesisWaypoint") > 0)
	{
		UpdateWaypoint(VarGetNumber("NemesisWaypoint"), EntityPos("NemesisHimself"), NULL, NULL, 0.0f);
	}
}

static void NemesisPlotOnEnterMap(ENTITY player)
{
	if (VarGetNumber("NemesisWaypoint") > 0)
	{
		SetWaypoint(player, VarGetNumber("NemesisWaypoint"));
	}
}

static void NemesisPlotOnLeaveMap(ENTITY player)
{
	ClearAllWaypoints(player);
}

static void NemesisPlotOnEntityDefeat(ENTITY player, ENTITY victor)
{
	if(IsEntityOnScriptTeam(player, "Troops") || IsEntityOnScriptTeam(player, "NemesisPets"))
		VarSetNumber("NemesisDefeated", VarGetNumber("NemesisDefeated") + 1);

	if(IsEntityOnScriptTeam(player, "NemesisHimself"))
	{
		TeamGrantBadge(ALL_PLAYERS, NEMESIS_PLOT_BADGE, true);

		SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("NemesisHimselfDefeatedMessage"), 1,
															"zonename", StringLocalizeMenuMessages(GetMapName())));
		if(VarGetNumber("NemesisWaypoint") > 0)
		{
			DestroyWaypoint(VarGetNumber("NemesisWaypoint"));
			VarSetNumber("NemesisWaypoint", 0);
		}
	}
}

static NUMBER CheckForNemesisHimself(NUMBER playerCount, NUMBER nemesisCount)
{
	if(VarGetNumber("NemesisHimselfSpawned") > 0)
		return 0; // Nemesis Himself already spawned this go-round

	if(playerCount > 10 && VarGetNumber("NemesisDefeated") >= VarGetNumber("NemesisToDefeat"))
		return 1;

	return 0;
}

static void NemesisPlotSpawnTroops()
{
	NUMBER idx, i, numPlayers, count;
	ENTITY target;
	NUMBER loop = 0;
	NUMBER currentTroops = NumEntitiesInTeam("Troops");
	char buf[2000];

	numPlayers = NumEntitiesInTeam(ALL_PLAYERS);

	if (currentTroops < VarGetNumber("MaxTroopsPerWave") && currentTroops < (numPlayers * 3))
	{

		if (numPlayers > 0)
		{
			count = VarGetNumber("TroopLoopCount");

			// prevents players from being overwhelmed if there are few people in the zone
			if (count > numPlayers)
				count = numPlayers;

			idx = RandomNumber(1, numPlayers);

			while (loop < count)
			{
				// find a player and spawn stuff near them
				target = GetEntityFromTeam(ALL_PLAYERS, idx);

				if (GetCombatLevel(target) > 25 &&
					!IsEntityMoving(target) && 
					!IsEntityUntargetable(target) && 
					GetHealth(target) > 0.2f && 
					IsEntityOutside(target) &&
					IsEntityNearGround(target, 10.0f) &&
					!NemesisPlotIsEntityNearHospital(target))
				{
					STRING troopList = "TroopList";
					NUMBER playerCount, nemesisCount;
					NUMBER nemesisWeight = 0;
					NUMBER targetTeamCount = 0;

					NemesisPlotCountEnts(PointFromEntity(target), &playerCount, &nemesisCount);

					// Add Fudge factors for testing
					playerCount += VarGetNumber("FudgePlayerCount");
					nemesisCount += VarGetNumber("FudgeNemesisCount");

					if(CheckForNemesisHimself(playerCount, nemesisCount))
					{
						// Spawn Nemesis Himself
						SpawnNemesisHimself(target, "NewTroop");
						
						Say("NewTroop", VarGet("NemesisHimselfShout"), 2);
			
						AddAIBehavior("NewTroop", TROOP_SPAWN_BEHAVIOR);
			
						SwitchScriptTeam("NewTroop", "NewTroop", "NemesisHimself");
					}
					else if ((playerCount > (6 * nemesisCount)) && (nemesisCount > 0))
					{	
						// nemesis are vastly out-numbered
						targetTeamCount = MIN(8, playerCount - nemesisCount) / 3;
						troopList = "TroopListEB";
					} 
					else if ((playerCount > (4 * nemesisCount)) && (nemesisCount > 0))
					{	
						// nemesis are out-numbered
						targetTeamCount = MIN(8, playerCount - nemesisCount) / 2;
						troopList = "TroopListBoss";
					}
					else if (playerCount > nemesisCount)
					{
						// nemesis are out-gunned
						targetTeamCount = MIN(8, playerCount - nemesisCount);
					}

					// spawn based on team size
					for (i = 0; i < targetTeamCount; i++)
					{
						STRING villain = VarGetArrayElement(troopList, RandomNumber(0, VarGetArrayCount(troopList) - 1));

						CreateVillainNearEntity("NewTroop", villain, VarGetNumber("TroopLevel"), 
													NULL, target, 20.0f);

						// set AI Behavior
						AddAIBehavior("NewTroop", TROOP_SPAWN_BEHAVIOR);

						// debug: give a shoutout when you spawn
						if(VarGetNumber("Debug") > 0)
						{
							sprintf(buf, "<shout t 0>T:%s - %s. P%d N%d TT%d/%d", GetDisplayName(target), villain, playerCount, nemesisCount, i+1, targetTeamCount);
							Say("NewTroop", buf, 2);
						}

						// put on Troop Team
						SwitchScriptTeam("NewTroop", "NewTroop", "Troops");
					}
				}
				loop++;

				idx++;
				if (idx > numPlayers)
					idx = 1;
			}
		}
	}
}

static void NemesisPlotCleanUpTroops(void)
{
	NUMBER i, count;
	ENTITY target;

	count = NumEntitiesInTeam("Troops");
	
	for (i = 1; i <= count; i++)
	{
		target = GetEntityFromTeam("Troops", i);
		
		if (!StringEmpty(target))
		{
			if (!GetAIHasTarget(target))
				AddAIBehavior(target, CLEANUP_BEHAVIOR); 
		}
	}
}

static void NemesisPlotResetTimers(void)
{
	TimerRemove("StateTimer");
	TimerRemove("TroopTimer");
	TimerRemove("CleanupTimer");
	TimerRemove("WaveTimer");

	VarSetNumber("NemesisHimselfSpawned", 0);
	VarSetNumber("NemesisDefeated", 0);

	if(VarGetNumber("NemesisWaypoint") > 0)
	{
		DestroyWaypoint(VarGetNumber("NemesisWaypoint"));
		VarSetNumber("NemesisWaypoint", 0);
	}
}

static void NemesisPlotCleanSpawns(void)
{
	AddAIBehavior("Troops", CLEANUP_BEHAVIOR);
	AddAIBehavior("NemesisPets", CLEANUP_BEHAVIOR);
	AddAIBehavior("NemesisHimself", CLEANUP_BEHAVIOR);
}

static void NemesisPlotKillSpawns(void)
{
	Kill("Troops", false);
	Kill("NemesisPets", false);
	AddAIBehavior("NemesisHimself", CLEANUP_BEHAVIOR);
}

static void NemesisPlotShutdown(void)
{
	SetAreAllNPCsScared(false);
	// SetDisableMapSpawns(false);

	Kill("Troops", false);
	Kill("NemesisPets", false);
	AddAIBehavior("NemesisHimself", CLEANUP_BEHAVIOR);
	Kill("Debt", false);
	EndScript();
}

void NemesisPlot(void)
{
	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// if there are too many running, don't spawn another
			if (ScriptCountRunningInstances("NemesisPlot") > 1)
			{
				EndScript();
				return;
			}

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		SET_STATE("WaitingToStart");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("WaitingToStart") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// people are happy
			SetAreAllNPCsScared(false);

			// pick time for next plot
			VarSetFraction("NextPlot", RandomFraction(VarGetFraction("MinPlot"), 
															VarGetFraction("MaxPlot")));

			Kill("Debt", false);
			NemesisPlotCleanSpawns();

			// reset timers
			NemesisPlotResetTimers();

			// allow zone spawns
			// SetDisableMapSpawns(false);

			if (VarGetNumber("IHaveMutex"))
			{
				// release the mutex
				InstanceVarRemove("ZoneEventMutex");
				VarSetNumber("IHaveMutex", 0);
			}

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// Check 
		if (DoEvery("StateTimer", VarGetFraction("NextPlot"), NULL))
		{			
			NemesisPlotKillSpawns();
			SET_STATE("OneTimePause");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("OneTimePause") ////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Be Certain all troops are dead!
			NemesisPlotKillSpawns();

			// reset timers
			NemesisPlotResetTimers();
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// if some other invasion is running, just wait
		if (InstanceVarIsEmpty("ZoneEventMutex"))
		{
			// if the world event is running, move immediately to the prelude
			if (MapGetDataToken(VarGet("NemesisPlotShardToken")) > 0)
			{
				SET_STATE("Prelude");

				// grab the mutex
				VarSetNumber("IHaveMutex", 1);
				InstanceVarSet("ZoneEventMutex", "Nemesis");
			}
			else
			{
				SET_STATE("WaitingToStart");
			}
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("RunMe") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Be Certain all troops are dead!
			NemesisPlotKillSpawns();

			// reset timers
			NemesisPlotResetTimers();
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// if some other invasion is running, just wait
		if (InstanceVarIsEmpty("ZoneEventMutex"))
		{
			SET_STATE("Prelude");

			// grab the mutex
			VarSetNumber("IHaveMutex", 1);
			InstanceVarSet("ZoneEventMutex", "Nemesis");
		}
		else
		{
			SET_STATE("WaitingToStart");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Prelude") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY

			// reset timers
			NemesisPlotResetTimers();

			// send alert
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
															"zonename", StringLocalizeMenuMessages(GetMapName())));

			// disallow zone spawns
			// SetDisableMapSpawns(true);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if (DoEvery("StateTimer", 1.0f, NULL))
		{			
			SET_STATE("Waves");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Waves") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY

			// reset timers
			NemesisPlotResetTimers();

			// pick troop check time
			VarSetFraction("TroopCheckTime", RandomFraction(VarGetFraction("MinTroopCheckTime"), 
							VarGetFraction("MaxTroopCheckTime")));

			// send alert
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("TroopLandingMessage"), 1,
															"zonename", StringLocalizeMenuMessages(GetMapName())));
			
			SetAreAllNPCsScared(true);

			// spawn in dept protector
			GenericSpawn("Hospital", "marker:Hospital", "Debt");

			VarSetNumber("NemesisToDefeat", RandomNumber(VarGetNumber("MinNemesisToDefeat"), 
							VarGetNumber("MaxNemesisToDefeat")));
			
			OnEntityDefeated(NemesisPlotOnEntityDefeat);
			OnPlayerEnteringMap(NemesisPlotOnEnterMap);
			OnPlayerExitingMap(NemesisPlotOnLeaveMap);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// spawn troops check
		if (DoEvery("TroopTimer", 0.1, NULL))
		{	
			NemesisPlotSpawnTroops();
			NemesisPlotSpawnPets();
			NemesisPlotUpdateWaypoint();
		}

		// cleanup inactive troops check
		if (DoEvery("CleanupTimer", 1.0, NULL))
		{	
			NemesisPlotCleanUpTroops();
		}

		VarSetNumber("CurrentTroopCount", NumEntitiesInTeam("Troops"));

		// failsafe timer
		if (DoEvery("WaveTimer", VarGetFraction("MaxTroopWaveTime"), NULL))
		{			
			VarSetNumber("WaveTimerElapsed", 1);
		}

		if(VarGetNumber("WaveTimerElapsed") && !VarGetNumber("ToggleLock"))
		{
			VarSetNumber("WaveTimerElapsed", 0);
			SET_STATE("Epilogue");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Epilogue") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY
			// reset timers
			NemesisPlotResetTimers();

			VarSetNumber("TimeToCleanup", SecondsSince2000() + (60*VarGetNumber("CleanupTime")));

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// end state timer
		if (DoEvery("StateTimer", 1.0f, NULL))
		{			
			if (NumEntitiesInTeam("Troops") < 10 || VarGetNumber("TimeToCleanup") < SecondsSince2000())
			{
				SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("DoneMessage"), 1,
															"zonename", StringLocalizeMenuMessages(GetMapName())));

				SET_STATE("WaitingToStart");
			}
		}

		// cleanup inactive invaders check
		if (DoEvery("CleanupTimer", 1.0, NULL))
		{	
			NemesisPlotCleanUpTroops();
		}

		VarSetNumber("CurrentTroopCount", NumEntitiesInTeam("Troops"));

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////
		NemesisPlotShutdown();
	END_STATES

	ON_STOPSIGNAL
		NemesisPlotShutdown();
	END_SIGNAL

	ON_SIGNAL("RunMe")
		SET_STATE( "RunMe" );
	END_SIGNAL

	ON_SIGNAL("Waves")
		SET_STATE( "Waves" );
	END_SIGNAL

	ON_SIGNAL("Epilogue")
		SET_STATE( "Epilogue" );
	END_SIGNAL
		
	ON_SIGNAL("Restart")
		SET_STATE( "WaitingToStart" );
	END_SIGNAL

	ON_SIGNAL("GiveBadge")
		TeamGrantBadge(ALL_PLAYERS, NEMESIS_PLOT_BADGE, true);
	END_SIGNAL

	ON_SIGNAL("SkipKillCount")
		VarSetNumber("NemesisDefeated", VarGetNumber("NemesisToDefeat"));
	END_SIGNAL

	ON_SIGNAL("ToggleLock")
		VarSetNumber("ToggleLock", VarGetNumber("ToggleLock") ? 0 : 1);
	END_SIGNAL

}


void NemesisPlotInit()
{
	SetScriptName( "NemesisPlot" );
	SetScriptFunc( NemesisPlot );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "MinPlot",								"1.0",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "MaxPlot",								"2.0",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );

	SetScriptVar( "SafeZoneRadius",							"300.0",					SP_OPTIONAL | SP_DONTDISPLAYDEBUG );

	SetScriptVar( "TroopLevel",								"50",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TroopLoopCount",							"5",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "MinTroopCheckTime",						"0.1",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "MaxTroopCheckTime",						"0.2",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "MaxTroopWaveTime",						"15.0",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "MaxTroopsPerWave",						"100",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "CleanupTime",							"5",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );

	SetScriptVar( "FudgePlayerCount",						"0",						SP_OPTIONAL );
	SetScriptVar( "FudgeNemesisCount",						"0",						SP_OPTIONAL );
	SetScriptVar( "Debug",									"0",						SP_OPTIONAL );

	SetScriptVar( "NemesisPlotShardToken",					"NemesisPlotShard",			SP_OPTIONAL | SP_DONTDISPLAYDEBUG );

	SetScriptVar( "AlertMessage",							"",							SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TroopLandingMessage",					"",							SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "DoneMessage",							"",							SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NemesisHimselfMessage",					"",							SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NemesisHimselfShout",					"",							SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NemesisHimselfDefeatedMessage",			"",							SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );

	SetScriptVar( "NemesisWaypointName",					"",							SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NemesisWaypointIcon",					"map_enticon_mission_c",	SP_DONTDISPLAYDEBUG );

	SetScriptVar( "MinNemesisToDefeat",						"10",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "MaxNemesisToDefeat",						"20",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );

	SetScriptVar( "TroopList",								"Event_Nemesis_Lieutenant;Event_Nemesis_Elite;Event_Nemesis_Jaeger;Event_Nemesis_Jaeger_Slicer;Event_Nemesis_Jaeger_Bomber",							SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TroopListBoss",							"Event_Nemesis_Warhulk;Event_Fake_Nemesis",								SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TroopListEB",							"Event_Nemesis_Warhulk_EB;Event_Fake_Nemesis_EB",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NemesisHimself",							"Event_Nemesis",							SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
		
	SetScriptSignal( "RunMe", "RunMe" );
	SetScriptSignal( "Waves", "Waves" );
	SetScriptSignal( "Epilogue", "Epilogue" );
	SetScriptSignal( "Restart", "Restart" );
	SetScriptSignal( "GiveBadge", "GiveBadge" );
	SetScriptSignal( "SkipKillCount", "SkipKillCount" );
	SetScriptSignal( "ToggleLock", "ToggleLock" );
}

