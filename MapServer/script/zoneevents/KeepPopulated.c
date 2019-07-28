// LOCATION SCRIPT

// This is a location script that keeps a specified population in a zone
//

#include "scriptutil.h"


#define CLEANUP_PL			"PL_FleeToNearestDoor"

 
void KeepPopulated(void)
{  
	ENCOUNTERGROUP group = "first"; 
	int currentCritters;

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			Kill("Team", false);
			SET_STATE("Populate");

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////
	STATE("Populate") ////////////////////////////////////////////////////////

	currentCritters = NumEntitiesInTeam( "Team" );
	VarSetNumber("CurrentCritters", currentCritters);

	if( DoEvery("RespawnCheckTime", VarGetFraction("RespawnCheckTime"), 0 ) ) 
	{ 
		int numSpawns = VarGetArrayCount("SpawnLocation") - 1;
		STRING layout;

		// Spawn more trolls
		while (	currentCritters < VarGetNumber( "PopulationMin") &&
				stricmp(group, "location:none") != 0)
		{
			layout = VarGetArrayElement("SpawnLocation", RandomNumber(0, numSpawns));
			// Spawn More Trolls using distant arounds
			group = FindEncounterGroupByRadius( 
				MyLocation(),
				0, 
				600, //Between 0 and 600 feet of this encounter
				layout, 
				1, 
 				0 );  

			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Team", "UseCanSpawns", group, layout, VarGetNumber("SpawnLevel"), 0 );  			
				currentCritters = NumEntitiesInTeam( "Team" );
				VarSetNumber("CurrentCritters", currentCritters);

			}
		}
	} // End respawn check time

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventCleanup") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

		SetAIPriorityList( "Team", CLEANUP_PL ); 

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


		if( DoEvery("TimeTillEnd", VarGetFraction("TimeToCleanupWin"), 0 ) ) 
		{
			SET_STATE("EventShutdown");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		Kill("Team", false);

		EndScript();

	END_STATES

	ON_SIGNAL("EndScript")
		SET_STATE( "EventShutdown" );
	END_SIGNAL

}

void KeepPopulatedInit()
{
	SetScriptName( "KeepPopulated" );
	SetScriptFunc( KeepPopulated );
	SetScriptType( SCRIPT_LOCATION );		

//	SetScriptVar( "Spawns",									NULL,					0 );
	SetScriptVar( "SpawnLevel",								"1",					0 );
	SetScriptVar( "SpawnLocation",							NULL,					0 );
	SetScriptVar( "PopulationMin",							"5",					0 );

	SetScriptVar( "TimeToCleanupWin",						"2.0",					0 );

	SetScriptSignal( "End Event", "EndScript" );		
}