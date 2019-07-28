// LOCATION SCRIPT

// This is a location script that runs controls the sound effects for the monkey fight in Pocket D
//

#include "scriptutil.h"

static int gPocketDMonkeyFightRunning = 0;
 
int PocketDMonkeyFightCheckForMonkeys(void)
{
	return EncounterState( VarGet("group") );
}



static void SpawnCritter(STRING spawn, STRING layout, STRING team)
{
	ENCOUNTERGROUP group = "first"; 

	// spawn contact hero one
	group = FindEncounterGroupByRadius( 
		LOC_ORIGIN,
		0, 
		9999, 
		layout, 
		1, 
 		0 );  

	if (stricmp(group, "location:none") != 0) {
		Spawn( 1, team, spawn, group, layout, 1, 0 );  
	}
}



void PocketDMonkeyFight(void)
{  
	ENCOUNTERGROUP group = "first"; 

	INITIAL_STATE       
	//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// if there are too many running, don't spawn another
			if (ScriptCountRunningInstances("PocketDMonkeyFight") > 1)
			{
				EndScript();
				return;
			}
	 
			gPocketDMonkeyFightRunning++;

			group = FindEncounterGroupByRadius( 
							LOC_ORIGIN,
							0, 
							9999, 
							"RMKF", 
							1, 
 							0 );
			if (!StringEqual(group, "location:none")) {
				VarSet("group", group);
			} else {
				EndScript();
				return;
			}

			SET_STATE("NoMonkeys");

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////
	STATE("NoMonkeys") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Ummm... nothing

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// Check to see if there are monkeys
		if( DoEvery("MonkeyCheck", 0.05f, 0 ) ) // move to next state
		{ 
			int state = PocketDMonkeyFightCheckForMonkeys();
			if ( state > ENCOUNTER_ASLEEP  && state < ENCOUNTER_OFF )
				SET_STATE("Monkeys");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Monkeys") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// kill monkey no fight critter
			Kill("MonkeyNoFight", false);

			// spawn monkey fight critter
			SpawnCritter(VarGet("MonkeyFightSpawn"), VarGet("MonkeyFightLayout"), "MonkeyFight");

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// Check to see if there are no monkeys
		if( DoEvery("MonkeyCheck", 0.05f, 0 ) ) // move to next state
		{ 
			if (PocketDMonkeyFightCheckForMonkeys() >= ENCOUNTER_COMPLETED )
			{
				SET_STATE("Cooldown");
				// kill monkey fight critter
				Kill("MonkeyFight", false);

				// spawn monkey no fight critter
				SpawnCritter(VarGet("MonkeyNoFightSpawn"), VarGet("MonkeyNoFightLayout"), "MonkeyNoFight");
			}
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Cooldown") ////////////////////////////////////////////////////////

		// Give time for encounter to end

		if( DoEvery("CooldownCheck", 1.0f, 0 ) ) // move to next state
		{ 
				SET_STATE("NoMonkeys");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		gPocketDMonkeyFightRunning--;
		EndScript();

	END_STATES

	ON_SIGNAL("EndScript")
		SET_STATE( "EventShutdown" );
	END_SIGNAL

}


void PocketDMonkeyFightInit()
{
	SetScriptName( "PocketDMonkeyFight" );
	SetScriptFunc( PocketDMonkeyFight );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "MonkeyFightSpawn",							NULL,						0 );
	SetScriptVar( "MonkeyNoFightSpawn",							NULL,						0 );
	SetScriptVar( "MonkeyFightLayout",							NULL,						0 );
	SetScriptVar( "MonkeyNoFightLayout",						NULL,						0 );

	SetScriptSignal( "End Script", "EndScript" );		
}

