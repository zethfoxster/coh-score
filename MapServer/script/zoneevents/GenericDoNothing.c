// ZONE SCRIPT
//
// The generic do nothing script is pretty simple.  It does nothing.

#include "scriptutil.h"


//////////////////////////////////////////////////////////////////////////////////////////
// MAIN 
//////////////////////////////////////////////////////////////////////////////////////////

void GenericDoNothing(void)
{

	INITIAL_STATE
		// nothing
	END_STATES


	ON_SIGNAL("End")
		EndScript();
	END_SIGNAL

//	ON_STOPSIGNAL
//	END_SIGNAL
}

void GenericDoNothingInit()
{
	SetScriptName( "GenericDoNothing" );
	SetScriptFunc( GenericDoNothing );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptSignal( "End", "End" );		

//	SetScriptVar( "Badge",									"",			0 );

}
