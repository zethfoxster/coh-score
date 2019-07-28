// ZONE SCRIPT
//
// This script will set a map token when it starts and then remove it when it shuts down.

#include "scriptutil.h"


//////////////////////////////////////////////////////////////////////////////////////////
// MAIN 
//////////////////////////////////////////////////////////////////////////////////////////

void SetMapToken(void)
{

	INITIAL_STATE
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			if (!MapHasDataToken(VarGet("TokenName")))
				MapSetDataToken(VarGet("TokenName"), VarGetNumber("TokenValue"));
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////
	END_STATES


	ON_SIGNAL("End")
		if (MapHasDataToken(VarGet("TokenName")))
			MapRemoveDataToken(VarGet("TokenName"));
		EndScript();
	END_SIGNAL

	ON_STOPSIGNAL
		if (MapHasDataToken(VarGet("TokenName")))
			MapRemoveDataToken(VarGet("TokenName"));
		EndScript();
	END_SIGNAL
}

void SetMapTokenInit()
{
	SetScriptName( "SetMapToken" );
	SetScriptFunc( SetMapToken );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptSignal( "End", "End" );		

	SetScriptVar( "TokenName",								"",			0 );
	SetScriptVar( "TokenValue",								"",			0 );
//	SetScriptVar( "Badge",									"",			0 );

}
