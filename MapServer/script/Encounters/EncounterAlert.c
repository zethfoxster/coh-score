
#include "scriptutil.h"

// Documentation at: http://ncncsvn01:8081/display/coh/Encounter+Alert+Script


void ScriptEncounterAlert()
{
	INITIAL_STATE    
	{
		// send alert
		SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
																"zonename", StringLocalizeMenuMessages(GetMapName())));
		EndScript(); //Error
	}

	END_STATES
}

void EncounterAlertInit()
{
	SetScriptName( "EncounterAlert" );
	SetScriptFunc( ScriptEncounterAlert );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "AlertMessage",						NULL,		0 );
}
