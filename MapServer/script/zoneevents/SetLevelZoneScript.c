// ZONE SCRIPT
//

// Sets level of all players in zone

#include "scriptutil.h"

void SetLevelZoneScriptShutdown(void)
{
	SetScriptCombatLevel(0);
	SetScriptMinCombatLevel(0);
	SetScriptMaxCombatLevel(0);
	EndScript();
}

void SetLevelZoneScript(void)
{
	INITIAL_STATE
		ON_ENTRY
			SetScriptCombatLevel(VarGetNumber("Level"));	
			SetScriptMinCombatLevel(VarGetNumber("MinLevel"));
			SetScriptMaxCombatLevel(VarGetNumber("MaxLevel"));
		END_ENTRY
	END_STATES

	ON_SIGNAL("End")
		SetLevelZoneScriptShutdown();
	END_SIGNAL

	ON_STOPSIGNAL
		SetLevelZoneScriptShutdown();
	END_SIGNAL
}

void SetLevelZoneScriptInit()
{
	SetScriptName( "SetLevelZoneScript" );
	SetScriptFunc( SetLevelZoneScript );
	SetScriptType( SCRIPT_ZONE );	

	SetScriptSignal( "End", "End" );		

	SetScriptVar( "Level",					"",		0 );
	SetScriptVar( "MinLevel",				"",		0 );
	SetScriptVar( "MaxLevel",				"",		0 );

}