// ZONE SCRIPT
//

// Used for testing new features of the script system

#include "scriptutil.h"

NUMBER CoVZoneKillXCountVillainDefs(STRING def)
{
	int i, count = 0;
	for (i = 0; i < NumEntitiesInTeam(ALL_CRITTERS); i++)
	{	
		ENTITY targetEnt = GetEntityFromTeam(ALL_CRITTERS, i + 1 );

		if (StringEqual(def, GetVillainDefName(targetEnt)))
			count++;
	}
	return count;
}

void CoVZoneKillXCheckForSpawn()
{
	if (VarGetNumber("CurrentKillCount") >= VarGetNumber("KillCount") || VarGetNumber("ForceSpawn"))
	{
		// check to see if min target count exists and if it is, is it met
		if (VarGetNumber("MinKillTargetCount") > 0)
		{
			// count number of targets left in zone
			int i, numCritters;
			int count = 0;
			numCritters = NumEntitiesInTeam(ALL_CRITTERS);
			for (i = 1; i <= numCritters; i++)
			{
				ENTITY e = GetEntityFromTeam(ALL_CRITTERS, i);
				STRING name = GetVillainDefName(e);

				if (StringEqual(name, VarGet("KillTarget")))
				{
					count++;
				}
			}

			if (count > VarGetNumber("MinKillTargetCount"))
			{
				// reset the count
				VarSetNumber("CurrentKillCount", 0);
				return;
			}
		}

		// check to see if there is already a boss out
		if (!VarIsEmpty("BossDef"))
		{
			if (CoVZoneKillXCountVillainDefs(VarGet("BossDef")) > 0)
				return;
		}

		// check to see if timer has expired
		if (TimerElapsed("BossSpawnClock") || VarGetNumber("ForceSpawn")) {
			STRING group;

			// spawn nasty - Pick a random spawn for now
			group = FindEncounterGroupByRadius( 
				"coord:0.0,0.0,0.0",
				0, 
				9999, 
				VarGet("BossSpawnLoc"), 
				1, 
 				0 );  

			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Boss", VarGet("BossSpawn"), group, VarGet("BossSpawnLoc"), VarGetNumber("SpawnLevel"), 0 );  
				TimerSet("BossSpawnClock", (FRACTION) VarGetFraction("BossSpawnStart")); // the next one won't go off until...

				// send alert
				SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
																	"zonename", StringLocalizeMenuMessages(GetMapName())));

			}
		}
		// reset the count
		VarSetNumber("CurrentKillCount", 0);
		VarSetNumber("ForceSpawn", 0);
	}

}

void CoVZoneKillXOnEntityDefeat(ENTITY player, ENTITY victor)
{
	// check for desired type of critter
	if (IsEntityCritter(player))
	{
		STRING name = GetVillainDefName(player);

		if (StringEqual(name, VarGet("KillTarget")))
		{
			int count = VarGetNumber("CurrentKillCount");

			// checking for time limitations
			if (VarGetFraction("CountStartTime") >= 0 && VarGetFraction("CountEndTime")) {
				if (VarGetFraction("CountStartTime") < VarGetFraction("CountEndTime"))
				{
					if (GetDayNightTime() >= VarGetFraction("CountEndTime") || GetDayNightTime() <= VarGetFraction("CountStartTime"))
						return;
				} else {
					if (GetDayNightTime() >= VarGetFraction("CountStartTime") || GetDayNightTime() <= VarGetFraction("CountEndTime"))
						return;
				}
			}

			count++;

			VarSetNumber("CurrentKillCount", count);

			// check to see if the spawn check is only made as certain times of day
			if (VarGetFraction("MakeCountAt") < 0.0f)
			{
				CoVZoneKillXCheckForSpawn();
			}
		}
	}
}

void CoVZoneKillXScript(void)
{
	INITIAL_STATE

		ON_ENTRY 
			// put stuff that should run once on initialization here

			// Starting timer for boss
			TimerSet("BossSpawnClock", (FRACTION) 0.01f); // it will expire quickly

			//////////////////////////////////////////////////////////////////////////
			///Script Hooks
			// setup whatever to remove ore and stuff when player leaves map

			OnEntityDefeated ( CoVZoneKillXOnEntityDefeat );

			// setting up day counter
			VarSetNumber("DayCounter", 1);

		END_ENTRY

		if (VarGetFraction("LastTick") > GetDayNightTime()) 
		{
			VarSetNumber("DayCounter", 1); 
		}

		VarSetFraction("LastTick", GetDayNightTime());

		// check for daily check condition
		if (VarGetFraction("MakeCountAt") >= 0.0f)
		{
			if (VarGetNumber("DayCounter")) {			// has check been made 'today'
				if (GetDayNightTime() > VarGetFraction("MakeCountAt"))
				{
					CoVZoneKillXCheckForSpawn();

					VarSetNumber("DayCounter", 0);			
				}
			}
		}
	END_STATES

	ON_SIGNAL("Spawn")
		VarSetNumber("ForceSpawn", 1);
		CoVZoneKillXCheckForSpawn();
	END_SIGNAL

	ON_STOPSIGNAL
		EndScript();
	END_SIGNAL
}

void CoVZoneKillXInit()
{
	SetScriptName( "CoVZoneKillX" );
	SetScriptFunc( CoVZoneKillXScript );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "KillCount",								"100",		0 );
	SetScriptVar( "KillTarget",								"none",		0 );
	SetScriptVar( "BossSpawn",								"",			0 );
	SetScriptVar( "BossSpawnLoc",							"",			0 );
	SetScriptVar( "SpawnLevel",								"1",		0 );
	SetScriptVar( "BossSpawnStart",							"0.01",		0 );
	SetScriptVar( "CountStartTime",							"-1",		0 );
	SetScriptVar( "CountEndTime",							"-1",		0 );
	SetScriptVar( "MakeCountAt",							"-1",		0 );
	SetScriptVar( "MinKillTargetCount",						"0",		0 );
	SetScriptVar( "AlertMessage",							"0",		0 );
	SetScriptVar( "BossDef",								"",			SP_OPTIONAL );

	SetScriptSignal( "Spawn", "Spawn" );		

}