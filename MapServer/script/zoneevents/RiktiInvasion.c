// ZONE EVENT SCRIPT

// This script runs the Rikti Invasion Event.
//

#include "scriptutil.h"

#define CLEANUP_PL		"DoNotGoToSleep, RiktiTroopLeave"
#define KILL_PL			"DestroyMe"
//#define PATROL_PL		"DoNotGoToSleep, OverridePerceptionRadius(300), PatrolRiktiDropship(MotionWire,FlyTurnRadius(0.4),CombatOverride(Passive),Fly)"
#define PATROL_PL		"DoNotGoToSleep, OverridePerceptionRadius(300), Patrol(MotionWire,FlyTurnRadius(0.4),CombatOverride(Passive),Fly)"
#define INVADER_PL		"ConsiderEnemyLevel(0), RiktiTroop"



/////////////////////////////////////////////////////////////////////////
// Counting Ents
/////////////////////////////////////////////////////////////////////////

void RiktiInvasionCountEnts(LOCATION loc, NUMBER *players, NUMBER *rikti)
{
	#define MAXENTS 100
	ENTITY entlist[MAXENTS];
	NUMBER i;
	NUMBER riktiCount;
	FRACTION riktiWieght = 0.0;

	// Look for nearby Rikti
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
// Marker and Geometry Processing
/////////////////////////////////////////////////////////////////////////

int RiktiInvasionMarkerProcessor(SCRIPTELEMENT elem)  
{
	NUMBER			geoID = 0;
	SCRIPTMARKER	marker = GetScriptMarkerFromScriptElement(elem);

	if (!StringEmpty(FindMarkerProperty(marker, "WarWall")))
	{
		geoID = DynamicGeometryFind(FindMarkerGeoName(marker), LOC_ORIGIN, -1);
		if (geoID) {
			DynamicGeometrySetHP(geoID, 100.0f, 0); // set it to full
			VarSetNumber("WarWallGeo", geoID);
		} else {
			// couldn't find this dyn geom group
		}
	}
	return true; 
}

/////////////////////////////////////////////////////////////////////////
// Safezone
/////////////////////////////////////////////////////////////////////////

int RiktiInvasionIsEntityNearHospital(ENTITY player)
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
void RiktiInvasionGenericSpawn(STRING layout, STRING marker, STRING team)
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

void RiktiInvasionDeSpawnDropship(NUMBER wave, NUMBER path)
{ 
	STRING layout = StringAdd("RiktiInvasion_", NumberToString(wave));
	STRING marker;
	STRING team = StringAdd(NumberToString(wave),NumberToString(path));
	int i, count;

	layout = StringAdd(layout, "_");
	layout = StringAdd(layout, NumberToString(path));
	marker = StringAdd("marker:", layout);
	marker = StringAdd(marker, "_end");

	count = NumEntitiesInTeam(team);
	for (i = 1; i <= count; i++)
	{
		ENTITY dropship = GetEntityFromTeam(team, i);

		if (DistanceBetweenLocations(PointFromEntity(dropship), marker) < VarGetFraction("DespawnDistance"))
			SetAIPriorityList(dropship, KILL_PL);
	}
}

void RiktiInvasionSpawnDropship(NUMBER wave, NUMBER path)
{ 
	ENCOUNTERGROUP group = "first";
	STRING layout = StringAdd("RiktiInvasion_", NumberToString(wave));
	STRING marker;
	STRING team = StringAdd(NumberToString(wave),NumberToString(path));

	layout = StringAdd(layout, "_");
	layout = StringAdd(layout, NumberToString(path));
	marker = StringAdd("marker:", layout);
 
	// spawn ships
	group = FindEncounterGroupByRadius( 
		marker,
		0, 
		30, 
		layout, 
		1, 
		0 );  

	if (stricmp(group, "location:none") != 0) 
	{
		Spawn( 1, "DS", "UseCanSpawns", group, layout, 54, 0 );
		SetAIPriorityList("DS", PATROL_PL);
		// set ally side
		SetAITeamVillain2("DS");

		DetachSpawn("DS");
		SetScriptTeam("DS", "Dropships");
		SwitchScriptTeam("DS", "DS", team);


	}
}

void RiktiInvasionSpawnBombs(NUMBER wave, NUMBER path, STRING type)
{ 
	LOCATION loc;
	STRING team = StringAdd(NumberToString(wave),NumberToString(path));
	NUMBER i, count;
	ENCOUNTERGROUP group = "first";

	if (NumEntitiesInTeam("Bombs") < VarGetNumber("MaxInvadersPerWave"))
	{

		count = NumEntitiesInTeam(team);
		for (i = 1; i <= count; i++)
		{
			ENTITY dropship = GetEntityFromTeam(team, i);

			loc = EntityPos( dropship );

			//Spawn invaders
			group = FindEncounterGroupByRadius( 
				loc,
				0, 
				VarGetNumber("InvaderSpawnDistance"), //Within a set distance of the drop ship
				type,  
				1, 
				0 );

			if( !StringEqual( LOCATION_NONE, group ) )
			{
				Spawn( 1, "Bombs", "UseCanSpawns", group, type, VarGetNumber( "InvaderLevel"), 0 );

				// set ally side
//				SetAITeamVillain2("Bombs"); 
			}
		}
	}
}



void RiktiInvasionSpawnInvaders()
{ 
//	LOCATION loc;
	NUMBER idx, i, numPlayers, count;
	ENTITY target;
	NUMBER loop = 0;
	NUMBER EBSpawned = false;
	NUMBER currentInvaders = NumEntitiesInTeam("Invaders");

	numPlayers = NumEntitiesInTeam(ALL_PLAYERS);

	if (currentInvaders < VarGetNumber("MaxInvadersPerWave") && currentInvaders < (numPlayers * 3))
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
					!RiktiInvasionIsEntityNearHospital(target))
				{
					STRING invaderList = "InvaderList";
					NUMBER playerCount, riktiCount;
					NUMBER ritkiWeight = 0;
					NUMBER targetTeamCount = 0;

					RiktiInvasionCountEnts(PointFromEntity(target), &playerCount, &riktiCount);

					if ((playerCount > (6 * riktiCount)) && (riktiCount > 0))
					{	
						// rikti are vastly out-numbered
						targetTeamCount = MIN(8, playerCount - riktiCount) / 3;
						invaderList = "InvaderListEB";
						EBSpawned = true;
					} 
					else if ((playerCount > (4 * riktiCount)) && (riktiCount > 0))
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

					/*
					TEAM targetTeam = GetPlayerTeamFromPlayer(target);
					int targetTeamCount = NumEntitiesInTeam(targetTeam);

					if (GetTeamSizeOverride() > 0)
						targetTeamCount = GetTeamSizeOverride();

					if (targetTeamCount > 3 || GetLevel(target) > VarGetNumber("InvaderLevel")) 
					{
						if (targetTeamCount > 6 && !EBSpawned && RandomNumber(0, 100) > 75)
						{
							invaderList = "InvaderListEB";
							targetTeamCount = 1;
							EBSpawned = true;
						} else {
							invaderList = "InvaderListBoss";
						}
					}
					*/

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

void RiktiInvasionCleanUpInvaders(void)
{

	NUMBER i, count;
	ENTITY target;

	count = NumEntitiesInTeam("Invaders");
	
	for (i = 1; i <= count; i++)
	{
		target = GetEntityFromTeam("Invaders", i);
		
		if (!StringEmpty(target))
		{
			if (!GetAIHasTarget(target))
				SetAIPriorityList(target, CLEANUP_PL); 
		}
	}
}


void RiktiInvasionResetTimers(void)
{
	TimerRemove("StateTimer");
	TimerRemove("InvaderTimer");
	TimerRemove("CleanupTimer");
	TimerRemove("WaveTimer");
}

void RiktiInvasionShutdown(void)
{
	// allow zone spawns
	// SetDisableMapSpawns(false);

	Kill("Invaders", false);
	Kill("Dropships", false);
	Kill("Bombs", false);
	Kill("Debt", false);
	EndScript();
}

void RiktiInvasion(void)
{  
	int i, count; 
	int subwave = VarGetNumber("Subwave");

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// if there are too many running, don't spawn another
			if (ScriptCountRunningInstances("RiktiInvasion") > 1)
			{
				EndScript();
				return;
			}
	 
			// set up war wall geometry tracking
			VarSetNumber("WarWallGeo", -1);

			ForEachScriptMarker(RiktiInvasionMarkerProcessor);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		SET_STATE("WaitingToStart");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("WaitingToStart") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// happy sunny skies
			SkyFileFade(1, 0, 1.0f); 

			// people are happy
			SetAreAllNPCsScared(false);

			// pick time for next invasion
			VarSetFraction("NextInvasion", RandomFraction(VarGetFraction("MinInvasion"), 
															VarGetFraction("MaxInvasion")));

			Kill("Dropships", false);
			Kill("Bombs", false);
			Kill("Debt", false); 
			SetAIPriorityList("Invaders", CLEANUP_PL); 


			// reset timers
			RiktiInvasionResetTimers();

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
			Kill("Bombs", false);

			// reset timers
			RiktiInvasionResetTimers();
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// if some other invasion is running, just wait
		if (InstanceVarIsEmpty("ZoneEventMutex"))
		{

			// if the world event is running, move immediately to the prelude
			if (MapGetDataToken(VarGet("RiktiEventInvasionToken")) > 0)
			{
				SET_STATE("Prelude");

				// grab the mutex
				VarSetNumber("IHaveMutex", 1);
				InstanceVarSet("ZoneEventMutex", "Rikti");
			} else {
				// if the world event isn't running, check for the one time invasions only every minute
				if (DoEvery("StateTimer", 1.0f, NULL))
				{			
					if (MapGetDataToken(VarGet("RiktiOneTimeInvasionToken")) > 0)
					{
						MapRemoveDataToken(VarGet("RiktiOneTimeInvasionToken"));
						SET_STATE("Prelude");

						// grab the mutex
						VarSetNumber("IHaveMutex", 1);
						InstanceVarSet("ZoneEventMutex", "Rikti");
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
			Kill("Bombs", false);

			// reset timers
			RiktiInvasionResetTimers();
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// if some other invasion is running, just wait
		if (InstanceVarIsEmpty("ZoneEventMutex"))
		{
			SET_STATE("Prelude");

			// grab the mutex
			VarSetNumber("IHaveMutex", 1);
			InstanceVarSet("ZoneEventMutex", "Rikti");
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
			SkyFileFade(0, 1, 1.0f);

			// Drop war walls
			if (VarGetNumber("WarWallGeo") >= 0)
				DynamicGeometrySetHP(VarGetNumber("WarWallGeo"), 0.0f, 0); 
			SendPlayerSound(ALL_PLAYERS, "WarWallsDown", SOUND_MUSIC, 1.0f); 

			// reset timers
			RiktiInvasionResetTimers();

			// send alert
			{
				STRING msg = StringAdd(StringLocalize(VarGet("AlertMessage")), " ");
				msg = StringAdd(msg, StringLocalizeMenuMessages(GetMapName()));
				msg = StringAdd(msg, ".");
				SendToAllMapsMessage(msg);
			}

			// disallow zone spawns
			// SetDisableMapSpawns(true);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if (DoEvery("StateTimer", 1.0f, NULL))
		{			
			SET_STATE("WaveOne");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("WaveOne") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			count = VarGetNumber("PatrolPathCount");
			for (i = 1; i <= count; i++)
			{
				// spawn initial wave of drop ships
				RiktiInvasionSpawnDropship(1, i);
			}

			// Set up subwave count
			VarSetNumber("Subwave", 1);
			subwave = 1;

			// reset timers
			RiktiInvasionResetTimers();

			// pick invader check time
			VarSetFraction("InvaderCheckTime", RandomFraction(VarGetFraction("MinInvaderCheckTime"), 
							VarGetFraction("MaxInvaderCheckTime")));

			// send alert
			{
				STRING msg = StringAdd(StringLocalize(VarGet("InvasionMessage")), " ");
				msg = StringAdd(msg, StringLocalizeMenuMessages(GetMapName()));
				msg = StringAdd(msg, ". ");
				msg = StringAdd(msg, StringLocalize(VarGet("InvasionMessagePost")));
				SendToAllMapsMessage(msg);
			}

			SendPlayerSound(ALL_PLAYERS, VarGet("AirRaidSound"), SOUND_MUSIC, 1.0f); 
			
			SetAreAllNPCsScared(true);

			Kill("Dropships", false);

			// spawn in dept protector
			RiktiInvasionGenericSpawn("Hospital", "marker:Hospital", "Debt");

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if (DoEvery("StateTimer", VarGetFraction("TimeBetweenSubwaves"), NULL))
		{	
			count = subwave + 1;

			if (count <= VarGetNumber("SubWaveCount"))
			{
				VarSetNumber("Subwave", count);
				subwave = count;

				count = VarGetNumber("PatrolPathCount");
				for (i = 1; i <= count; i++)
				{
					// spawn initial wave of drop ships
					RiktiInvasionSpawnDropship(1, i);
				}
			}
		}

		// when all wave one ships leave the zone
		if (subwave >= VarGetNumber("SubWaveCount") && NumEntitiesInTeam("Dropships") <= 0)
			SET_STATE("WaveTwo");

		// check for ships at end of paths
		count = VarGetNumber("PatrolPathCount");
		for (i = 1; i <= count; i++)
		{
			RiktiInvasionDeSpawnDropship(1, i);
		}

		if (DoEvery("InvaderTimer", VarGetFraction("InvaderCheckTime"), NULL))
		{	
			count = VarGetNumber("PatrolPathCount");
			for (i = 1; i <= count; i++)
			{
				RiktiInvasionSpawnBombs(1, i, "RiktiInvasion_UXB");
			}
		}

		// failsafe timer
		if (DoEvery("WaveTimer", VarGetFraction("MaxInvaderWaveTime"), NULL))
		{			
			SET_STATE("WaveTwo");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("WaveTwo") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			count = VarGetNumber("PatrolPathCount");
			for (i = 1; i <= count; i++)
			{
				// spawn initial wave of drop ships
				RiktiInvasionSpawnDropship(2, i);
			}

			// Set up subwave count
			VarSetNumber("Subwave", 1);
			subwave = 1;

			// reset timers
			RiktiInvasionResetTimers();

			Kill("Dropships", false);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if (DoEvery("StateTimer", VarGetFraction("TimeBetweenSubwaves"), NULL))
		{	
			count = subwave + 1;

			if (count <= VarGetNumber("SubWaveCount"))
			{
				VarSetNumber("Subwave", count);
				subwave = count;

				count = VarGetNumber("PatrolPathCount");
				for (i = 1; i <= count; i++)
				{
					// spawn initial wave of drop ships
					RiktiInvasionSpawnDropship(2, i);
				}
			}
		}

		// when all wave one ships leave the zone
		if (subwave >= VarGetNumber("SubWaveCount") && NumEntitiesInTeam("Dropships") <= 0)
			SET_STATE("Epilogue");

		// check for ships at end of paths
		count = VarGetNumber("PatrolPathCount");
		for (i = 1; i <= count; i++)
		{
			RiktiInvasionDeSpawnDropship(2, i);
		}

		// spawn invaders check
		if (DoEvery("InvaderTimer", 0.1, NULL))
		{	
			RiktiInvasionSpawnInvaders();
		}

		// cleanup inactive invaders check
		if (DoEvery("CleanupTimer", 1.0, NULL))
		{	
			RiktiInvasionCleanUpInvaders();
		}

		VarSetNumber("CurrentInvaderCount", NumEntitiesInTeam("Invaders"));

		// failsafe timer
		if (DoEvery("WaveTimer", VarGetFraction("MaxInvaderWaveTime"), NULL))
		{			
			SET_STATE("Epilogue");
		}


	//////////////////////////////////////////////////////////////////////////////////
	STATE("Epilogue") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// reset timers
			RiktiInvasionResetTimers();

			// raise war walls
			if (VarGetNumber("WarWallGeo") >= 0)
				DynamicGeometrySetHP(VarGetNumber("WarWallGeo"), 100.0f, 0); 

			VarSetNumber("TimeToCleanup", SecondsSince2000() + (60*VarGetNumber("CleanupTime")));

			Kill("Dropships", false);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// end state timer
		if (DoEvery("StateTimer", 1.0f, NULL))
		{			
			if (NumEntitiesInTeam("Invaders") < 10 || VarGetNumber("TimeToCleanup") < SecondsSince2000())
			{
				{
					STRING msg = StringAdd(StringLocalize(VarGet("DoneMessage")), " ");
					msg = StringAdd(msg, StringLocalizeMenuMessages(GetMapName()));
					msg = StringAdd(msg, ".");
					SendToAllMapsMessage(msg);
				}
				SET_STATE("WaitingToStart");
			}
		}

		// cleanup inactive invaders check
		if (DoEvery("CleanupTimer", 1.0, NULL))
		{	
			RiktiInvasionCleanUpInvaders();
		}

		VarSetNumber("CurrentInvaderCount", NumEntitiesInTeam("Invaders"));


	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		RiktiInvasionShutdown();

	END_STATES

	ON_STOPSIGNAL
		RiktiInvasionShutdown();
	END_SIGNAL

	ON_SIGNAL("Prelude")
		MapSetDataToken(VarGet("RiktiOneTimeInvasionToken"), 1);
		SET_STATE( "OneTimePause" );
	END_SIGNAL

	ON_SIGNAL("RunMe")
		SET_STATE( "RunMe" );
	END_SIGNAL

	ON_SIGNAL("WaveOne")
		SET_STATE( "WaveOne" );
	END_SIGNAL

	ON_SIGNAL("WaveTwo")
		SET_STATE( "WaveTwo" );
	END_SIGNAL

	ON_SIGNAL("Epilogue")
		SET_STATE( "Epilogue" );
	END_SIGNAL

		
	ON_SIGNAL("Restart")
		SET_STATE( "WaitingToStart" );
	END_SIGNAL

}


void RiktiInvasionInit()
{
	SetScriptName( "RiktiInvasion" );
	SetScriptFunc( RiktiInvasion );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "MinInvasion",							"1.0",						SP_OPTIONAL );
	SetScriptVar( "MaxInvasion",							"2.0",						SP_OPTIONAL );
	SetScriptVar( "PatrolPathCount",						"2",						SP_OPTIONAL );
	SetScriptVar( "SubWaveCount",							"3",						SP_OPTIONAL );
	SetScriptVar( "TimeBetweenSubwaves",					"2.5",						SP_OPTIONAL );

	SetScriptVar( "InvaderLoopCount",						"5",						SP_OPTIONAL );

	SetScriptVar( "DespawnDistance",						"20.0",						SP_OPTIONAL );

	SetScriptVar( "InvaderLevel",							"50",						SP_OPTIONAL );
	SetScriptVar( "InvaderSpawnDistance",					"500.0",					SP_OPTIONAL );

	SetScriptVar( "SafeZoneRadius",							"300.0",					SP_OPTIONAL );

	SetScriptVar( "MinInvaderCheckTime",					"0.5",						SP_OPTIONAL );
	SetScriptVar( "MaxInvaderCheckTime",					"0.75",						SP_OPTIONAL );
	SetScriptVar( "MaxInvaderWaveTime",						"15.0",						SP_OPTIONAL );

	SetScriptVar( "MaxInvadersPerWave",						"100",						SP_OPTIONAL );
	SetScriptVar( "CleanupTime",							"30",						SP_OPTIONAL );

	SetScriptVar( "RiktiEventInvasionToken",				"RiktiEventInvasionToken",	SP_OPTIONAL );
	SetScriptVar( "RiktiOneTimeInvasionToken",				"RiktiOneTimeInvasionToken",SP_OPTIONAL );

	SetScriptVar( "AlertMessage",							"",							0 );
	SetScriptVar( "InvasionMessage",						"",							0 );
	SetScriptVar( "InvasionMessagePost",					"",							0 );
	SetScriptVar( "DoneMessage",							"",							0 );
	SetScriptVar( "AirRaidSound",							"",							0 );

	SetScriptVar( "InvaderList",							"",							0 );
	SetScriptVar( "InvaderListBoss",						"",							0 );
	SetScriptVar( "InvaderListEB",							"",							0 );

	SetScriptSignal( "Prelude", "Prelude" );		
	SetScriptSignal( "RunMe", "RunMe" );		
	SetScriptSignal( "One", "WaveOne" );		
	SetScriptSignal( "Two", "WaveTwo" );		
	SetScriptSignal( "Epilogue", "Epilogue" );		
	SetScriptSignal( "Restart", "Restart" );		
}

