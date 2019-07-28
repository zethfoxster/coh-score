
#include "scriptutil.h"

// Documentation at: http://code:8081/display/coh/Multi-Triggered+Behavior+Script

void ScriptMultiTriggeredBehaviorHandleCondition(ENTITY target, NUMBER idx)
{
	if (VarGetArrayCount("BehaviorList") > 0)
	{
		STRING behavior = VarGetArrayElement("BehaviorList", idx);
		if (!StringEmpty(behavior) && !StringEqual(behavior, "none"))
			AddAIBehavior(target, behavior);
	}

	if (VarGetArrayCount("SetHealthList") > 0)
	{
		STRING healthString = VarGetArrayElement("SetHealthList", idx);
		FRACTION health = VarGetArrayElementFraction("SetHealthList", idx);
		if (!StringEmpty(healthString) && !(health < 0.0) && !StringEqual("none", healthString))
			SetHealth(target, health, 0);
	}

	if (VarGetArrayCount("SaysList") > 0)
	{
		STRING says = VarGetArrayElement("SaysList", idx);
		if (!StringEmpty(says) && !StringEqual("none", says))
			Say(target, says, 2);
	}
}

void ScriptMultiTriggeredBehavior() 
{
	int i, j, conditionCount, entCount, count;
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
			STRING entHandle = StringAdd("Ent", pos);

			if (!StringEqual(pos, "all"))
			{
				target = ActorFromEncounterPos( MyEncounter(), StringToNumber(pos));
				VarSet(entHandle, target);
			}
		}

		// set script to waiting
		SET_STATE("Waiting");
	}

	////////////////////////////////////////////////////////////////////
	STATE("Waiting")
	{

		conditionCount = VarGetArrayCount("ConditionList");
		for (i = 0; i < conditionCount; i++)
		{
			STRING alreadyDone = StringAdd("Done", NumberToString(i));

			// check condition
			if (VarIsEmpty(alreadyDone) && CheckMissionObjectiveExpression(VarGetArrayElement("ConditionList", i)))
			{
				STRING pos = VarGetArrayElement("EncounterPositionList", i);

				if (!StringEqual(pos, "all"))
				{
					STRING entHandle = StringAdd("Ent", pos);
					target = VarGet(entHandle);
					if( !StringEqual( target, TEAM_NONE ) )  // this can happen if the actor is killed before the condition is true
					{
						ScriptMultiTriggeredBehaviorHandleCondition(target, i);
					}
				} else {
					entCount = NumEntitiesInTeam( MyEncounter() );
					for (j = entCount; j > 0; j--)
					{
						ENTITY ent = GetEntityFromTeam(MyEncounter(), j);

						if( !StringEqual( ent, TEAM_NONE ) )  // this can happen if the actor is killed before the condition is true
						{
							ScriptMultiTriggeredBehaviorHandleCondition(ent, i);
						}						
					}
				}
				VarSet(alreadyDone, "Done");
			}
		}
	}


	END_STATES
}

void MultiTriggeredBehaviorInit()
{
	SetScriptName( "MultiTriggeredBehavior" );
	SetScriptFunc( ScriptMultiTriggeredBehavior );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "EncounterPositionList",									NULL,		0 );
	SetScriptVar( "ConditionList",											NULL,		0 );
	SetScriptVar( "BehaviorList",											NULL,		SP_OPTIONAL );
	SetScriptVar( "SetHealthList",											"",			SP_OPTIONAL );
	SetScriptVar( "SaysList",												"",			SP_OPTIONAL );

	SetScriptSignal( "End Event", "EndScript" );
}
