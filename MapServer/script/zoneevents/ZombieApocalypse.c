// ZONE EVENT SCRIPT

// This script runs the Zombie Apocalypse Event.
//

#include "scriptutil.h"

#define CLEANUP_PL		"DoNotGoToSleep, DoNothing(Untargetable,AnimList(Dismiss),Timer(2)), DestroyMe"
#define KILL_PL			"DestroyMe"
#define INVADER_PL		"ConsiderEnemyLevel(0), DoNothing(Untargetable,AnimList(SpawnAsPet),Timer(2)), Combat"


/////////////////////////////////////////////////////////////////////////
// Counting Ents
/////////////////////////////////////////////////////////////////////////

void ZombieApocalypseCountEnts(LOCATION loc, NUMBER *players, NUMBER *rikti)
{
	#define MAXENTS 100
	ENTITY entlist[MAXENTS];
	NUMBER i;
	NUMBER riktiCount;
	FRACTION riktiWieght = 0.0;

	// Look for nearby zombies
	if (rikti != NULL) 
	{
		riktiCount = GetAllEntities( entlist, 
									  0,								// villain group
									  0,								// villain def - Not yet implemented	
									  0,								// villain tag - Not yet implemented
									  0,								// ent type - Not yet implemented
									  "Invaders",						// script team
									  30.0f,							// radius to check 
									  loc,								// location to check
									  0,								// area
									  1,								// isAlive
									  0,								// isDead
									  MAXENTS,							// max ents
									  0									// only once
									);

		for (i = 0; i < riktiCount; i++)
		{
			STRING class = GetVillainDefClassName(entlist[i]);
			if (StringEqual(class, "Class_Boss_Elite"))
				riktiWieght += 3.0f;
			if (StringEqual(class, "Class_Boss_Grunt"))
				riktiWieght += 1.5f;
			else if (StringEqual(class, "Class_Lt_Grunt"))
				riktiWieght += 0.75f;
			else 
				riktiWieght += 0.33f;
		}
		*rikti = (NUMBER) riktiWieght;
	}

	// Look for nearby Players
	if (players != NULL)
		*players = GetAllEntities( entlist, 
									0,								// villain group
									0,								// villain def - Not yet implemented	
									0,								// villain tag - Not yet implemented
									0,								// ent type - Not yet implemented
									ALL_PLAYERS,						// script team
									30.0f,							// radius to check 
									loc,							// location to check
									0,								// area
									1,								// isAlive
									0,								// isDead
									MAXENTS,						// max ents
									0								// only once
								);

}

/////////////////////////////////////////////////////////////////////////
// Safezone
/////////////////////////////////////////////////////////////////////////

int ZombieApocalypseIsEntityNearHospital(ENTITY player)
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
void ZombieApocalypseGenericSpawn(STRING layout, STRING marker, STRING team)
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


void ZombieApocalypseSpawnInvaders()
{ 
//	LOCATION loc;
	NUMBER idx, i, numPlayers, count;
	ENTITY target;
	NUMBER loop = 0;
	NUMBER EBSpawned = false;
	NUMBER currentInvaders = NumEntitiesInTeam("Invaders");

	numPlayers = NumEntitiesInTeam(ALL_PLAYERS);

	if (currentInvaders < VarGetNumber("MaxZombies") && currentInvaders < (numPlayers * 3))
	{

		if (numPlayers > 0)
		{
			count = VarGetNumber("InvaderLoopCount");

			// prevents players from being overwhelmed if there are few people in the zone
			if (count > numPlayers)
				count = numPlayers;

			idx = RandomNumber(1, numPlayers );

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
					!ZombieApocalypseIsEntityNearHospital(target))
				{
					STRING invaderList = "InvaderList";
					NUMBER playerCount, riktiCount;
					NUMBER ritkiWeight = 0;
					NUMBER targetTeamCount = 0;

					ZombieApocalypseCountEnts(PointFromEntity(target), &playerCount, &riktiCount);

					if ((playerCount > (6 * riktiCount)) && (riktiCount > 0))
					{	
						// rikti are vastly out-numbered
						targetTeamCount = MIN(8, playerCount - riktiCount) / 3;
						invaderList = "InvaderListEB";
						EBSpawned = true;
					} 
					else if ((playerCount > (3 * riktiCount)) && (riktiCount > 0))
					{	
						// rikti are out-numbered
						targetTeamCount = MIN(8, playerCount - riktiCount) / 2;
						invaderList = "InvaderListBoss";
					}
					else if (playerCount > riktiCount)
					{
						// rikti are out-gunned
						targetTeamCount = MIN(8, playerCount - riktiCount);
						
					}

					// spawn based on team size
					for (i = 0; i < targetTeamCount; i++)
					{
						STRING villain = VarGetArrayElement(invaderList, RandomNumber(0, VarGetArrayCount(invaderList) - 1));

						CreateVillainNearEntity("NewInvader", villain, VarGetNumber("InvaderLevel"), 
													NULL, target, 20.0f);

						// set AI
						SetAIPriorityList( "NewInvader", INVADER_PL );

						// set ally side
						SetAITeamVillain2("NewInvader");

						// put on Invader Team
						SwitchScriptTeam("NewInvader", "NewInvader", "Invaders");
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

void ZombieApocalypseCleanUpInvaders(void)
{

	NUMBER i, count;
	ENTITY target;

	count = NumEntitiesInTeam("Invaders");
	
	for (i = 1; i <= count; i++)
	{
		target = GetEntityFromTeam("Invaders", i);
		
		if (!StringEmpty(target))
		{
			if (!GetAIHasTarget(target) && SecondsSince2000() - GetMonsterCreateTime(target) > 10)
				SetAIPriorityList(target, CLEANUP_PL); 
//				Kill(target, false);
		}
	}
}


void ZombieApocalypseResetTimers(void)
{
	TimerRemove("StateTimer");
	TimerRemove("InvaderTimer");
	TimerRemove("CleanupTimer");
	TimerRemove("WaveTimer");
}

void ZombieApocalypseShutdown(void)
{
	// allow zone spawns
	// SetDisableMapSpawns(false);

	Kill("Invaders", false);
	Kill("Debt", false);
	EndScript();
}

void ZombieApocalypse(void)
{  
	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// if there are too many running, don't spawn another
			if (ScriptCountRunningInstances("ZombieApocalypse") > 1)
			{
				EndScript();
				return;
			}
	 
			VarSetNumber("ZombieSky", SkyFileGetByName("Sun_Halloween_Zombie.txt"));

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		VarSetNumber("FirstTime", 1);
		SET_STATE("WaitingToStart");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("WaitingToStart") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// happy sunny skies
			if (VarGetNumber("ZombieSky") >= 0)
				SkyFileFade(VarGetNumber("ZombieSky"), 0, 1.0f); 

			// people are happy
			SetAreAllNPCsScared(false);

			// pick time for next invasion
			VarSetFraction("NextInvasion", RandomFraction(VarGetNumber("FirstTime") ? 15.0f : VarGetFraction("MinInvasion"), 
															VarGetFraction("MaxInvasion")));
			VarSetNumber("FirstTime", 0);

			Kill("Debt", false); 
//			Kill("Invaders", false); 
			SetAIPriorityList("Invaders", CLEANUP_PL); 

			// reset timers
			ZombieApocalypseResetTimers();

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
		if (DoEvery("StateTimer", VarGetFraction("NextInvasion"), NULL))
		{			
			Kill("Invaders", false);
			SET_STATE("OneTimePause");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("OneTimePause") ////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Be Certain all invaders are dead!
			Kill("Invaders", false);

			// reset timers
			ZombieApocalypseResetTimers();
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// if some other invasion is running, just wait
		if (InstanceVarIsEmpty("ZoneEventMutex"))
		{
			// if the world event is running, move immediately to the prelude
			if (MapGetDataToken(VarGet("ZombieEventToken")) > 0)
			{
				SET_STATE("Prelude");

				// grab the mutex
				InstanceVarSet("ZoneEventMutex", "Zombies");
				VarSetNumber("IHaveMutex", 1);

			} else {
				// if the world event isn't running, check for the one time invasions only every minute
				if (DoEvery("StateTimer", 1.0f, NULL))
				{			
					if (MapGetDataToken(VarGet("ZombieOneTimeToken")) > 0)
					{
						MapRemoveDataToken(VarGet("ZombieOneTimeToken"));
						SET_STATE("Prelude");

						// grab the mutex
						InstanceVarSet("ZoneEventMutex", "Zombies");
						VarSetNumber("IHaveMutex", 1);
					}
				}
			}
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("RunMe") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Be Certain all invaders are dead!
			Kill("Invaders", false);

			// reset timers
			ZombieApocalypseResetTimers();
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// if some other invasion is running, just wait
		if (InstanceVarIsEmpty("ZoneEventMutex"))
		{
			SET_STATE("Prelude");

			// grab the mutex
			VarSetNumber("IHaveMutex", 1);
			InstanceVarSet("ZoneEventMutex", "Zombies");
		}
		else
		{
			SET_STATE("WaitingToStart");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Prelude") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// darken sky
			if (VarGetNumber("ZombieSky") >= 0)
				SkyFileFade(0, VarGetNumber("ZombieSky"), 1.0f); 

			// reset timers
			ZombieApocalypseResetTimers();

			// send alert
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
																"zonename", StringLocalizeMenuMessages(GetMapName())));

			SendPlayerSound(ALL_PLAYERS, VarGet("AlertSound"), SOUND_MUSIC, 1.0f); 

			SetAreAllNPCsScared(true);

			// spawn in dept protector
			ZombieApocalypseGenericSpawn("Hospital", "marker:Hospital", "Debt");

			// disallow zone spawns
			// SetDisableMapSpawns(true);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if (DoEvery("StateTimer", 1.0f, NULL))
		{			
			SET_STATE("Invasion");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Invasion") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// reset timers
			ZombieApocalypseResetTimers();

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


		// spawn invaders check
		if (DoEvery("InvaderTimer", 0.1, NULL))
		{	
			ZombieApocalypseSpawnInvaders();
		}

		// cleanup inactive invaders check
		if (DoEvery("CleanupTimer", 1.0, NULL))
		{	
			ZombieApocalypseCleanUpInvaders();
		}

		VarSetNumber("CurrentZombieCount", NumEntitiesInTeam("Invaders"));

		// failsafe timer
		if (DoEvery("WaveTimer", VarGetFraction("InvasionTime"), NULL))
		{			
			SET_STATE("Epilogue");
		}


	//////////////////////////////////////////////////////////////////////////////////
	STATE("Epilogue") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// reset timers
			ZombieApocalypseResetTimers();

			VarSetNumber("TimeToCleanup", SecondsSince2000() + (60*VarGetNumber("CleanupTime")));

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// end state timer
		if (DoEvery("StateTimer", 1.0f, NULL))
		{			
			if (NumEntitiesInTeam("Invaders") < 10 || VarGetNumber("TimeToCleanup") < SecondsSince2000())
			{
				SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("DoneMessage"), 1,
																	"zonename", StringLocalizeMenuMessages(GetMapName())));
				SET_STATE("WaitingToStart");
			}
		}

		// cleanup inactive invaders check
		if (DoEvery("CleanupTimer", 1.0, NULL))
		{	
			ZombieApocalypseCleanUpInvaders();
		}

		VarSetNumber("CurrentZombieCount", NumEntitiesInTeam("Invaders"));


	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		ZombieApocalypseShutdown();

	END_STATES

	ON_STOPSIGNAL
		ZombieApocalypseShutdown();
	END_SIGNAL

	ON_SIGNAL("OneTime")
		SET_STATE( "OneTimePause" );
	END_SIGNAL

	ON_SIGNAL("Prelude")
		MapSetDataToken(VarGet("ZombieOneTimeToken"), 1);
		SET_STATE( "OneTimePause" );
	END_SIGNAL

	ON_SIGNAL("RunMe")
		SET_STATE("RunMe");
	END_SIGNAL

	ON_SIGNAL("Invasion")
		SET_STATE( "Invasion" );
	END_SIGNAL
		
	ON_SIGNAL("Epilogue")
		SET_STATE( "Epilogue" );
	END_SIGNAL

	ON_SIGNAL("Restart")
		SET_STATE( "WaitingToStart" );
	END_SIGNAL

}


void ZombieApocalypseInit()
{
	SetScriptName( "ZombieApocalypse" );
	SetScriptFunc( ZombieApocalypse );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "MinInvasion",							"1.0",						SP_OPTIONAL );
	SetScriptVar( "MaxInvasion",							"2.0",						SP_OPTIONAL );

	SetScriptVar( "InvaderLoopCount",						"5",						SP_OPTIONAL );

	SetScriptVar( "ZombieLevel",							"50",						SP_OPTIONAL );
	SetScriptVar( "InvaderLevel",							"30",						SP_OPTIONAL );

	SetScriptVar( "SafeZoneRadius",							"300.0",					SP_OPTIONAL );

	SetScriptVar( "InvasionTime",							"15.0",						SP_OPTIONAL );

	SetScriptVar( "MaxZombies",								"100",						SP_OPTIONAL );
	SetScriptVar( "CleanupTime",							"30",						SP_OPTIONAL );

	SetScriptVar( "ZombieEventToken",						"ZombieEventToken",			SP_OPTIONAL );
	SetScriptVar( "ZombieOneTimeToken",						"ZombieOneTimeToken",		SP_OPTIONAL );

	SetScriptVar( "AlertMessage",							"",							0 );
	SetScriptVar( "DoneMessage",							"",							0 );
	SetScriptVar( "AlertSound",								"",							0 );

	SetScriptVar( "InvaderList",							"",							0 );
	SetScriptVar( "InvaderListBoss",						"",							0 );
	SetScriptVar( "InvaderListEB",							"",							0 );

	SetScriptSignal( "OneTime", "OneTime" );		
	SetScriptSignal( "Prelude", "Prelude" );		
	SetScriptSignal( "RunMe", "RunMe" );		
	SetScriptSignal( "Invasion", "Invasion" );		
	SetScriptSignal( "Epilogue", "Epilogue" );		
	SetScriptSignal( "Restart", "Restart" );		
}

