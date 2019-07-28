
#include "scriptutil.h"

// Documentation at: http://code:8081/display/coh/Triggered+Behavior+Script

void ScriptTriggeredBehavior() 
{
	int i, count;
	ENTITY target;

	INITIAL_STATE    
 	{		
		// only valid on mission maps
		if (!OnMissionMap())
			EndScript(); //Error

		// validiate the target(s) of this script 
		count = VarGetArrayCount("EncounterPositionList");
		for (i = 0; i < count; i++)
		{
			STRING pos = VarGetArrayElement("EncounterPositionList", i);
			if (!StringEqual(pos, "all"))
			{
				target = ActorFromEncounterPos( MyEncounter(), StringToNumber(pos)); 
				if( StringEqual( target, TEAM_NONE ) )
				{
					EndScript(); //Error
					break;
				}
			}
		}

		// set script to waiting
		SET_STATE("Waiting");
	}

	////////////////////////////////////////////////////////////////////
	STATE("Waiting")
	{

		// check condition
		if (CheckMissionObjectiveExpression(VarGet("Condition")))
		{
			count = VarGetArrayCount("EncounterPositionList");
			for (i = 0; i < count; i++)
			{
				STRING pos = VarGetArrayElement("EncounterPositionList", i);
				if (!StringEqual(pos, "all"))
				{
					target = ActorFromEncounterPos( MyEncounter(), StringToNumber(pos)); 
					if( !StringEqual( target, TEAM_NONE ) )  // this can happen if the actor is killed before the condition is true
					{
						AddAIBehavior(target, VarGet("Behavior"));
						if (!(VarGetFraction("SetHealth") < 0.0))
							SetHealth(target, VarGetFraction("SetHealth"), 0);
					}
				} else {
					count = NumEntitiesInTeam( MyEncounter() );
					for (i = count; i > 0; i--)
					{
						ENTITY ent = GetEntityFromTeam(MyEncounter(), i);

						if( !StringEqual( ent, TEAM_NONE ) )  // this can happen if the actor is killed before the condition is true
						{
							AddAIBehavior(ent, VarGet("Behavior"));
							if (!(VarGetFraction("SetHealth") < 0.0))
								SetHealth(ent, VarGetFraction("SetHealth"), 0);
						}						
					}
					break;
				}
			}

			EndScript(); 
		}
	}


	END_STATES
}

void TriggeredBehaviorInit()
{
	SetScriptName( "TriggeredBehavior" );
	SetScriptFunc( ScriptTriggeredBehavior );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "EncounterPositionList",								NULL,		0 );
	SetScriptVar( "Condition",											NULL,		0 );
	SetScriptVar( "Behavior",											NULL,		0 );
	SetScriptVar( "SetHealth",											"-1.0",		SP_OPTIONAL );

	SetScriptSignal( "End Event", "EndScript" );
}
