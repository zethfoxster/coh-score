// LOCATION SCRIPT

// This is a location script that runs controls the opening and closing of the Pocket D ski resort.
//

#include "scriptutil.h"
 

// called when the player enters the map
void ForbiddenZonesCheckPlayers(void)
 {
	int i, j;
	int	groupNumber = NumEntitiesInTeam( ALL_PLAYERS );		
	int requires = true;

	for( j = 1 ; j <= groupNumber ; j++ )
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, j); 

		for (i = 0; i < VarGetArrayCount("VolumeNameList"); i++)
		{
			// check to see if player is in the forbidden zone and fails the condition
			if (EntityInArea(player, VarGetArrayElement("VolumeNameList", i)) &&
				!EvalEntityRequires(player, VarGetArrayElement("VolumeRequiresList", i), Script_GetCurrentBlame()))
			{
				SetPosition(player, VarGetArrayElement("VolumeTeleportMarkerList", i));
				SendPlayerDialog(player, VarGetArrayElement("TeleportMessageList", i));
			}
		}
	
		if (!VarIsEmpty("MapMoveRequires"))
		{
			requires = EvalEntityRequires(player, VarGet("MapMoveRequires"), Script_GetCurrentBlame());
		}

		if (!VarIsEmpty("MapMoveNumber") && !VarIsEmpty("MapMoveTarget") && requires)
		{
			SetMap (player, VarGetNumber("MapMoveNumber"), VarGet("MapMoveTarget"));
		}
	}
}



void ForbiddenZones(void)
{  

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////


		// Check to see if the place should be opened every minute.
		DoEvery("VariablCheck", VarGetFraction("CheckTime"), ForbiddenZonesCheckPlayers );
	

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		EndScript();

	END_STATES

	ON_SIGNAL("EndScript")
		SET_STATE( "EventShutdown" );
	END_SIGNAL


}


void ForbiddenZonesInit()
{
	SetScriptName( "ForbiddenZones" );
	SetScriptFunc( ForbiddenZones );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "VolumeNameList",									NULL,						SP_OPTIONAL );
	SetScriptVar( "VolumeRequiresList",								NULL,						SP_OPTIONAL );
	SetScriptVar( "VolumeTeleportMarkerList",						NULL,						SP_OPTIONAL );
	SetScriptVar( "TeleportMessageList",							NULL,						SP_OPTIONAL );

	SetScriptVar( "MapMoveNumber",									NULL,						SP_OPTIONAL );
	SetScriptVar( "MapMoveTarget",									NULL,						SP_OPTIONAL );
	SetScriptVar( "MapMoveRequires",								NULL,						SP_OPTIONAL );

	SetScriptVar( "CheckTime",										"0.1",						SP_OPTIONAL );

	SetScriptSignal( "End Script", "EndScript" );		
}

