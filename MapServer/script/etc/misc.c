// Miscellaneous small scripts

#include "scriptutil.h"


void EitherSideWins(void)
{
	int i, j, count = NumEntitiesInTeam(MyEncounter());
	ScriptControlsCompletion(MyEncounter());
	for (i = 1; i <= count; i++)
		for (j = i+1; j <= count; j++)
			if (GetAIAlly(GetEntityFromTeam(MyEncounter(), i)) !=
				GetAIAlly(GetEntityFromTeam(MyEncounter(), j)))
				return;
	SetEncounterComplete(MyEncounter(), 1);
	EndScript();
}

void MiscInit()
{
	SetScriptName( "EitherSideWins" );
	SetScriptFunc( EitherSideWins );
	SetScriptType( SCRIPT_ENCOUNTER );		
}

