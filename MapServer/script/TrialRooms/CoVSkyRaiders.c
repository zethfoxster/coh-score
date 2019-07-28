// MISSION SCRIPT
// Encounter Script
//
#include "scriptutil.h"

void CoVSkyRaidersOnEntityDefeat(ENTITY player, ENTITY victor)
{
	// check for desired type of critter
	if (IsEntityCritter(player))
	{
		STRING name = GetVillainDefName(player);
		int i, count = VarGetArrayCount("TurretList");
		ENCOUNTERGROUP group = "first"; 

		for (i = 0; i < count; i++) 
		{
			if (StringEqual(name, VarGetArrayElement("TurretList", i)))
			{
				int killCount = VarGetNumber("CurrentKillCount") + 1;
				int totalCount = VarGetNumber("TotalKillCount") + 1;
								
				if (killCount > VarGetNumber("Count"))
				{
					// Spawn More Trolls using distant arounds
					group = FindEncounter( 
											LOCATION_NONE,					//Relative to this spot
											0,								//No Nearer than this
											0,								//No Farther than this
											0,								//Inside this AREA
											0,								//Assigned this MissionEncounter (the LogicalName)
											VarGet("SpawnLocation"),		//Layout (Many Named Demon. Called "EncounterSpawn" in Spawndef files and in Maps, "Layouts" in mission specs and scripts and often throughout the code, "Spawntype" in SpawnDef struct, "EncounterLayouts" in mapspec)
											0,								//Group Name
											SEF_UNOCCUPIED_ONLY | SEF_SORTED_BY_DISTANCE //Flags
										);

					if (stricmp(group, "location:none") != 0) {
						Spawn( 1, "Team", "UseCanSpawns", group, VarGet("SpawnLocation"), VarGetNumber("SpawnLevel"), 0 );  			
					}
					killCount -= VarGetNumber("Count");
				}

				if (totalCount > VarGetNumber("ObjectiveCount"))
				{
					SetMissionObjectiveComplete(OBJECTIVE_SUCCESS, VarGet("Objective"));
				}
				VarSetNumber("CurrentKillCount", killCount);
				VarSetNumber("TotalKillCount", totalCount);
			}
		}
	}
}


void CoVSkyRaiders(void)
{  
	INITIAL_STATE 
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
				// count number of deaths
				OnEntityDefeated ( CoVSkyRaidersOnEntityDefeat );
		}
		END_ENTRY

		// spawn stuff
		if( MissionIsComplete() )
		{
			SET_STATE( "MissionOver" );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	STATE( "MissionOver" ) ////////////////////////////////////////////////////////
	{
		EndScript();
	}

	END_STATES 

	ON_SIGNAL("CompleteObjective")
	{ 
		SetMissionObjectiveComplete(OBJECTIVE_SUCCESS, VarGet("Objective"));
	}
	END_SIGNAL


	ON_STOPSIGNAL
		// nothing really to do...
	END_SIGNAL

}


void CoVSkyRaidersInit()
{
	SetScriptName( "CoVSkyRaiders" );
	SetScriptFunc( CoVSkyRaiders );
	SetScriptType( SCRIPT_MISSION );		

	SetScriptSignal( "Objective", "CompleteObjective" );		

	SetScriptVar( "TurretList",			"",							0 );
	SetScriptVar( "Count",				"",							0 );
	SetScriptVar( "SpawnLocation",		"",							0 );
	SetScriptVar( "SpawnLevel",			"",							0 );
	SetScriptVar( "ObjectiveCount",		"",							0 );
	SetScriptVar( "Objective",			"",							0 );

}
