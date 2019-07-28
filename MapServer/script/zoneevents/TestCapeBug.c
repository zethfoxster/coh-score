// LOCATION SCRIPT

// This is a location script that runs controls the opening and closing of the Pocket D ski resort.
//

#include "scriptutil.h"

void TestCapeBugSpawn(NUMBER loc)
{
	ENCOUNTERGROUP group = "first"; 

	// spawn heavy
	group = FindEncounterGroupByRadius( 
		LOC_ORIGIN,
		0, 
		9999, 
		"CapeSpawn", 
		1, 
		0 );  

	if (stricmp(group, "location:none") != 0) {
		STRING team = StringAdd("Cape", NumberToString(loc));

		Spawn( 1, team, VarGet("Spawn"), group, "CapeSpawn", 1, 0 );  
	}
}


void TestCapeBug(void)
{  

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////

		// Set Timer
		VarSetFraction("TimerTime", RandomFraction(0.1, 0.25));
		VarSetNumber("Counter", 0);

		SET_STATE("Running");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Running") ////////////////////////////////////////////////////////
		// Kill everything and start it up again
		if (DoEvery("VariableCheck", VarGetFraction("TimerTime"), NULL ))
		{
			NUMBER loc = VarGetNumber("Counter");
			STRING team = StringAdd("Cape", NumberToString(loc));

			// Kill some guys
			Kill(team, false);

			// Spawn some guys
			if (loc > 0)
				TestCapeBugSpawn(loc - 1);
			else
				TestCapeBugSpawn(VarGetNumber("SpawnCount"));

			// Set Timer
			VarSetFraction("TimerTime", RandomFraction(0.05, 0.1));

			// increment counter
			if (loc < VarGetNumber("SpawnCount"))
				VarSetNumber("Counter", loc + 1);
			else 
				VarSetNumber("Counter", 0);

		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		EndScript();

	END_STATES

	ON_SIGNAL("EndScript")
		SET_STATE( "EventShutdown" );
	END_SIGNAL


}


void TestCapeBugInit()
{
	SetScriptName( "TestCapeBug" );
	SetScriptFunc( TestCapeBug );
	SetScriptType( SCRIPT_ZONE );		

/*
	SetScriptVar( "VolumeNameList",					NULL,										SP_OPTIONAL );
	SetScriptVar( "VolumeRequiresList",				NULL,										SP_OPTIONAL );
	SetScriptVar( "VolumeTeleportMarkerList",		NULL,										SP_OPTIONAL );
	SetScriptVar( "TeleportMessageList",			NULL,										SP_OPTIONAL );

	SetScriptVar( "MapMoveNumber",					NULL,										SP_OPTIONAL );
	SetScriptVar( "MapMoveTarget",					NULL,										SP_OPTIONAL );

	SetScriptVar( "CheckTime",						"0.1",										SP_OPTIONAL );
*/
	SetScriptVar( "SpawnCount",						"7",										SP_OPTIONAL );
	SetScriptVar( "Spawn",							"Test\\Spawndefs\\CapeTest_D0_V0.spawndef",	SP_OPTIONAL );

	SetScriptSignal( "End Script", "EndScript" );		
}

