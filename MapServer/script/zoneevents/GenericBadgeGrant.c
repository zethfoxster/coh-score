// ZONE SCRIPT
//
// The generic badge grant script is pretty simple.  It grants a badge to all players who are entering the zone.

#include "scriptutil.h"

// don't run on city zones that don't have the marker we expect
static int CorrectCityZone()
{
	return LocationExists(VarGetArrayElement("Markers", 0));
}

//////////////////////////////////////////////////////////////////////////////////////////
// MAP CALLBACKS
//////////////////////////////////////////////////////////////////////////////////////////

// called when the player enters the map
int GenericBadgeGrantOnEnterMap(ENTITY player)
{
	EntityGrantReward(player, VarGet("Badge"));

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// MAIN 
//////////////////////////////////////////////////////////////////////////////////////////

void GenericBadgeGrant(void)
{

	INITIAL_STATE

		ON_ENTRY 
			OnPlayerEnteringMap( GenericBadgeGrantOnEnterMap );
		END_ENTRY
	END_STATES


	ON_SIGNAL("End")
		EndScript();
	END_SIGNAL

//	ON_STOPSIGNAL
//	END_SIGNAL
}

void GenericBadgeGrantInit()
{
	SetScriptName( "GenericBadgeGrant" );
	SetScriptFunc( GenericBadgeGrant );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptSignal( "End", "End" );		

	SetScriptVar( "Badge",									"",			0 );

}
