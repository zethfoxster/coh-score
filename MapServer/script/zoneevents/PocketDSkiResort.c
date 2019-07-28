// LOCATION SCRIPT

// This is a location script that runs controls the opening and closing of the Pocket D ski resort.
//

#include "scriptutil.h"
 

// called when the player enters the map
int PocketDSkiResortOnEnterMap(ENTITY player)
{
	if (StringEqual(GET_STATE, "ResortClosed"))
	{
		// check to see if player is in the forbidden zone!
		if (EntityInArea(player, VarGet("VolumeName")))
			SetPosition(player, VarGet("VolumeTeleportMarker"));
	}
	return 0;
}

void PocketDSkiResortheckPlayers(void)
 {
	int j;
	int	groupNumber = NumEntitiesInTeam( ALL_PLAYERS );		

	for( j = 1 ; j <= groupNumber ; j++ )
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, j); 

		PocketDSkiResortOnEnterMap(player);
	}
}




void PocketDSkiResort(void)
{  

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// if there are too many running, don't spawn another
			if (ScriptCountRunningInstances("PocketDSkiResort") > 1)
			{
				EndScript();
				return;
			}
	 
			VarSetNumber("GeometryID", DynamicGeometryFind(VarGet("GeometryName"), LOC_ORIGIN, -1));

			OnPlayerEnteringMap( PocketDSkiResortOnEnterMap );

			SET_STATE("ResortClosed");

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////
	STATE("ResortClosed") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Check to make sure the geometry is invisible
			if (VarGetNumber("GeometryID") > 0)
				DynamicGeometrySetHP(VarGetNumber("GeometryID"), 100, 0);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// Check to see if the place should be opened every minute.
		if( DoEvery("VariableCheck", 1.0f, 0 ) ) // move to next state
		{ 
			if (MapGetDataToken(VarGet("OpenToken")) > 0)
				SET_STATE("ResortOpen");
		}

		DoEvery("TeleportCheck", 0.25f, PocketDSkiResortheckPlayers );

	//////////////////////////////////////////////////////////////////////////////////
	STATE("ResortOpen") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Check to make sure the geometry is visible
			if (VarGetNumber("GeometryID") > 0)
				DynamicGeometrySetHP(VarGetNumber("GeometryID"), 0, 0);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// Check to see if the place should be opened every minute.
		if( DoEvery("VariableCheck", 1.0f, 0 ) ) // move to next state
		{ 
			if (MapGetDataToken(VarGet("OpenToken")) < 1)
			{
				SET_STATE("ResortClosed");
			} else {
				if (VarGetNumber("GeometryID") > 0)
					DynamicGeometrySetHP(VarGetNumber("GeometryID"), 0, 0);
			}
		}
	

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		EndScript();

	END_STATES

	ON_SIGNAL("EndScript")
		SET_STATE( "EventShutdown" );
	END_SIGNAL

	ON_SIGNAL("Open")
		MapSetDataToken(VarGet("OpenToken"), 1);
	END_SIGNAL

	ON_SIGNAL("Close")
		MapSetDataToken(VarGet("OpenToken"), 0);
	END_SIGNAL

}


void PocketDSkiResortInit()
{
	SetScriptName( "PocketDSkiResort" );
	SetScriptFunc( PocketDSkiResort );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "OpenToken",									NULL,						0 );
	SetScriptVar( "GeometryName",								NULL,						0 );
	SetScriptVar( "VolumeName",									NULL,						0 );
	SetScriptVar( "VolumeTeleportMarker",						NULL,						0 );

	SetScriptSignal( "Open Resort", "Open" );		
	SetScriptSignal( "Close Resort", "Close" );		
	SetScriptSignal( "End Script", "EndScript" );		
}

