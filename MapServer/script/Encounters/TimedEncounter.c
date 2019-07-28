
#include "scriptutil.h"

// Documentation at: http://code:8081/display/cs/Timed+Encounter+Script


void ScriptTimedEncounter()
{
	INITIAL_STATE    
 	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY 

			ScriptControlsCompletion( MyEncounter() ); 

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if( DoEvery("TimeCheck", VarGetFraction("Time"), 0 ) ) 
		{ 
			SET_STATE("Shutdown");
		}

	}

	STATE("Shutdown") ////////////////////////////////////////////////////////
	{
		CloseMyEncounterAndEndScript();
	}
	END_STATES

	ON_SIGNAL("End")
		SET_STATE("Shutdown");
	END_SIGNAL

}

void TimedEncounterInit()
{
	SetScriptName( "TimedEncounter" );
	SetScriptFunc( ScriptTimedEncounter );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptSignal( "End", "End" );		

	SetScriptVar( "Time",									NULL,		0 );
	SetScriptVar( "CleanupBehavior",						NULL,		SP_OPTIONAL );
}
