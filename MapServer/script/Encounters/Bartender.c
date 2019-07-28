
#include "scriptutil.h"


int BartenderCountClose(FRACTION dist)
{
	int i;
	int closeCount = 0;
	NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);

	for (i = 1; i <= count; i++)
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);
		
		if (IsEntityPlayer(player) && DistanceBetweenEntities(player, VarGet("Bartender")) < dist)
		{
			// a player is close
			closeCount++;
		}
	}

	return closeCount;
}


ENTITY BartenderCountPlayer(FRACTION dist)
{
	int i;
	NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);

	for (i = 1; i <= count; i++)
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);
		
		if (IsEntityPlayer(player) && DistanceBetweenEntities(player, VarGet("Bartender")) < dist)
		{
			// a player is close
			return player;
		}
	}
	return NULL;
}


int BartenderOnContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	if (!StringEqual(GET_STATE, "PeopleNear"))
		SET_STATE("PeopleNear");			

	switch (link)
	{
	case 1:	// HELLO
		OpenStore(target, player, "TaskForceStore");
		return 0;
		break;
	default:
		return 0;
	}

}

void ScriptBartender() 
{
	INITIAL_STATE    
 	{
		ENTITY Bartender;
		
		//Find the ghost trap, and Set Script to Ready
		Bartender = ActorFromEncounterPos( MyEncounter(), VarGetNumber("BartenderEncounterPosition")); 
		if( StringEqual( Bartender, TEAM_NONE ) )
		{
			EndScript(); //Error
		}
		else
		{
			OnScriptedContactInteract(BartenderOnContactClick, Bartender);
			VarSet( "Bartender", Bartender );
			SET_STATE("Waiting");
		}
	}

	////////////////////////////////////////////////////////////////////
	STATE("Waiting")
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
			SetAIPriorityList(VarGet("Bartender"), "PL_StandStill");
		}
		END_ENTRY

		{
			ENTITY player = BartenderCountPlayer(20.0f);
			// maybe someone's around?
			if (player != NULL)
			{
				// a player is close 
				Say(VarGet("Bartender"), StringLocalizeWithVars(VarGet("Greeting"), player), 0);
				SET_STATE("PeopleNear");
			}
		}
	}

	////////////////////////////////////////////////////////////////////
	STATE("PeopleNear")
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
			SetAIPriorityList(VarGet("Bartender"), "PL_StandStill");
			VarSetFraction("DanceTime", RandomFraction(0.7f, 2.0f));
		}
		END_ENTRY

		{

			if (DoEvery("CloseTimer", 0.1f, NULL) )
			{
				if (BartenderCountClose(20.0f) < 1)
				{
					SET_STATE("Waiting");
				}
			}
 
			if (DoEvery("DanceTimer", VarGetFraction("DanceTime"), NULL))
			{
				if (BartenderCountClose(20.0f) > 0)
				{
					SET_STATE("Dancing");
				} else {
					SET_STATE("Waiting");
				}
			}
		}
	}
	////////////////////////////////////////////////////////////////////
	STATE("Dancing")
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
			SetAIPriorityList(VarGet("Bartender"), "PL_DanceRave");
			VarSetFraction("DanceTime", RandomFraction(0.2f, 2.0f));

		}
		END_ENTRY

		if (DoEvery("CloseTimer", 0.1f, NULL) )
		{
			if (BartenderCountClose(20.0f) < 1)
			{
				SET_STATE("Waiting");
			}
		}

		if (DoEvery("DanceTimer", VarGetFraction("DanceTime"), NULL))
		{
			SET_STATE("PeopleNear");			
		}
	}
	END_STATES
}

void BartenderInit()
{
	SetScriptName( "Bartender" );
	SetScriptFunc( ScriptBartender );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "BartenderEncounterPosition",							NULL,		0 );
	SetScriptVar( "Greeting",											NULL,		SP_MULTIVALUE );

	SetScriptSignal( "End Event", "EndScript" );
}
