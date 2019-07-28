
#include "scriptutil.h"

// Documentation at: http://code:8081/display/cs/Info+NPC+Encounter+Script


int InfoNPCOnRightSide(ENTITY player)
{
	return ((VarGetNumber("OnlyHeroes") && IsEntityOnBlueSide(player)) ||
			(VarGetNumber("OnlyVillains") && IsEntityOnRedSide(player)));
}


// InfoNPCIsLevelRestricted
//		returns true if this contact has a level restriction and the player specified should be restricted
//		returns false otherwise
int InfoNPCIsLevelRestricted(ENTITY player)
{
	if (!VarIsEmpty("MinLevelRestricted"))
	{
		if (IsEntityPlayer(player) && GetLevel(player) < VarGetNumber("MinLevelRestricted"))
			return true;
	}

	if (!VarIsEmpty("MaxLevelRestricted"))
	{
		if (IsEntityPlayer(player) && GetLevel(player) > VarGetNumber("MaxLevelRestricted"))
			return true;
	}

	return false;
}

int InfoNPCOnContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	if (!VarIsEmpty("Dialog") && !VarIsEmpty("DialogStartPage"))
	{
		return ContactHandleDialogDef(player, target, VarGet("Dialog"), VarGet("DialogStartPage"), VarGet("DialogFile"), link, response);
	} else {
		switch (link)
		{
		case 1:	// HELLO
			sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));

			// check to see if this is a restricted contact
			if (InfoNPCIsLevelRestricted(player) && !VarIsEmpty("LevelRestrictedText"))	{
				strcat(response->dialog, StringLocalizeWithVars( VarGet("LevelRestrictedText"), player));
			} else if (InfoNPCOnRightSide(player)) {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("InfoText"), player));
			} else {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("RestrictedText"), player));
			}

			AddResponse(response, StringLocalizeWithVars(VarGet("Leave"), player), 3);
			if (TimerElapsed("ContactTalkTimer"))
			{
				Say(VarGet("InfoNPC"), StringLocalizeWithVars(VarGet("OutLoudInfoText"), player), 0);
				TimerSet("ContactTalkTimer", 0.25f);
			}

			if (!StringEmpty(VarGet("InfoReward")))
				EntityGrantReward(player, VarGet("InfoReward"));

			return 1;
			break;
		default:
			return 0;
		}
	}
}

void ScriptInfoNPC()
{
	INITIAL_STATE    
 	{
		ENTITY InfoNPC;
		ScriptType scriptType = GetScriptType();

		if(scriptType == SCRIPT_ENCOUNTER)
		{
			// take control of the encounter
			if (VarGetNumber("ScriptControlsCompletion"))
				ScriptControlsCompletion( MyEncounter() ); 

			if (VarGetNumber("CheckForInvention"))
			{
				if (!IsInventionEnabled())
				{
					Kill( MyEncounter(), false );
					EndScript(); //Error
				}
			} 

			//Find the NPC and Set Script to Ready
			InfoNPC = ActorFromEncounterPos( MyEncounter(), VarGetNumber("InfoNPCEncounterPosition")); 
		}
		else if(scriptType == SCRIPT_ENTITY)
		{
			InfoNPC = MyEntity();
		}
		else
		{
			EndScript(); // Unsupported script type
		}

		if( StringEqual( InfoNPC, TEAM_NONE ) )
		{
			EndScript(); //Error
		}
		else
		{
			OnScriptedContactInteract(InfoNPCOnContactClick, InfoNPC);
			VarSet( "InfoNPC", InfoNPC );
			SET_STATE("Waiting");
		}
		TimerSet("ContactTalkTimer", 0.25f);
	}

	////////////////////////////////////////////////////////////////////
	STATE("Waiting")
	{
		// maybe someone's around?
		int i;
		NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);

		for (i = 1; i <= count; i++)
		{
			ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);
			
			if (IsEntityPlayer(player) && InfoNPCOnRightSide(player) && 
				DistanceBetweenEntities(player, VarGet("InfoNPC")) < VarGetFraction("CloseDistance"))
			{
				if (!InfoNPCIsLevelRestricted(player)) 
				{
					// a player is close
					if (!VarIsEmpty("GreetingRequires") && !EvalEntityRequires(player, VarGet("GreetingRequires"), Script_GetCurrentBlame())) {
						if (!VarIsEmpty("GreetingFailedRequiresText"))
							Say(VarGet("InfoNPC"), StringLocalizeWithVars(VarGet("GreetingFailedRequiresText"), player), 0);
					} else if (!VarIsEmpty("Greeting"))
					{
						Say(VarGet("InfoNPC"), StringLocalizeWithVars(VarGet("Greeting"), player), 0);
						SET_STATE("PeopleNear");
					}
				} else if (!VarIsEmpty("LevelRestrictedGreeting")) {
					Say(VarGet("InfoNPC"), StringLocalizeWithVars(VarGet("LevelRestrictedGreeting"), player), 0);
					SET_STATE("PeopleNear");
				} 
			}
		}
	}

	////////////////////////////////////////////////////////////////////
	STATE("PeopleNear")
	{
		int i;
		int closeCount = 0;
		NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);

		for (i = 1; i <= count; i++)
		{
			ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);
			
			if (IsEntityPlayer(player) && DistanceBetweenEntities(player, VarGet("InfoNPC")) < VarGetFraction("CloseDistance") + 5.0f)
			{
				// a player is close
				closeCount++;
			}
		}

		if (closeCount == 0)
		{
			SET_STATE("Waiting");
		}

	}

	END_STATES
}

static void InfoInitCommon()
{
	SetScriptFunc( ScriptInfoNPC );

	SetScriptVar( "Greeting",											NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "RestrictedText",										NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "GreetingRequires",									NULL,		SP_OPTIONAL );
	SetScriptVar( "GreetingFailedRequiresText",							NULL,		SP_MULTIVALUE | SP_OPTIONAL);
	SetScriptVar( "InfoText",											NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "OutLoudInfoText",									NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "InfoReward",											NULL,		SP_OPTIONAL );
	SetScriptVar( "Leave",												NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "OnlyHeroes",											NULL,		SP_OPTIONAL );
	SetScriptVar( "OnlyVillains",										NULL,		SP_OPTIONAL );

	SetScriptVar( "MinLevelRestricted",									NULL,		SP_OPTIONAL );
	SetScriptVar( "MaxLevelRestricted",									NULL,		SP_OPTIONAL );
	SetScriptVar( "LevelRestrictedText",								NULL,		SP_MULTIVALUE | SP_OPTIONAL);
	SetScriptVar( "LevelRestrictedGreeting",							NULL,		SP_MULTIVALUE | SP_OPTIONAL );

	SetScriptVar( "Dialog",												NULL,		SP_OPTIONAL );
	SetScriptVar( "DialogStartPage",									NULL,		SP_OPTIONAL );
	SetScriptVar( "DialogFile",											NULL,		SP_OPTIONAL );

	SetScriptVar( "CloseDistance",										"25.0",		SP_OPTIONAL );

	SetScriptSignal( "End", "EndScript" );
}

void InfoNPCInit()
{
	SetScriptName( "InfoNPC" );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "ScriptControlsCompletion",							"1",		SP_OPTIONAL );
	SetScriptVar( "InfoNPCEncounterPosition",							NULL,		0 );

	SetScriptVar( "CheckForInvention",									NULL,		SP_OPTIONAL );

	InfoInitCommon();
}

void InfoNPCEntityInit()
{
	SetScriptName( "InfoNPCEntity" );
	SetScriptType( SCRIPT_ENTITY );

	InfoInitCommon();
}
