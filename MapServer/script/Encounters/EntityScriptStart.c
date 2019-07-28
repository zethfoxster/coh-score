
#include "scriptutil.h"
extern void ScriptError(char* s, ...);

void EntityScriptStart()
{
	INITIAL_STATE    
	{
		if(VarIsEmpty("EntityScriptStartScriptDef") && VarIsEmpty("EntityScriptStartScriptFunction"))
		{
			ScriptError("EntityScriptStart needs either a ScriptDef or Function specified\n");
		}
		else
		{
			ENTITY entity;
			STRING scriptDef = VarGet("EntityScriptStartScriptDef");
			STRING function = VarGet("EntityScriptStartScriptFunction");
			ScriptType scriptType = GetScriptType();

			if(scriptType == SCRIPT_ENCOUNTER)
			{
				STRING pos = VarGet("EntityScriptStartEncounterPosition");
				entity = ActorFromEncounterPos(MyEncounter(), StringToNumber(pos));
				if(StringEqual(entity, TEAM_NONE))
				{
					ScriptError("EntityScriptStart couldn't find entity to start script on\n");
				}
				else
				{
					if(!StringEmpty(scriptDef))
						EntityScriptBeginScriptDefFromScript(entity, scriptDef);
					
					if(!StringEmpty(function))
						EntityScriptBeginFunctionFromScript(entity, function);
				}
			}
			else
			{
				ScriptError("EntityScriptStart called from unsupported script type\n");
			}
		}	
		EndScript();
	}

	END_STATES
}

void EntityScriptStartInit()
{
	SetScriptName( "EntityScriptStart" );
	SetScriptFunc( EntityScriptStart );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "EntityScriptStartEncounterPosition",							NULL,		0 );
	SetScriptVar( "EntityScriptStartScriptDef",									NULL,		SP_OPTIONAL );
	SetScriptVar( "EntityScriptStartScriptFunction",							NULL,		SP_OPTIONAL );
}
