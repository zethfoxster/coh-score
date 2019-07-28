
#include "scriptutil.h"

// Documentation at: http://code:8081/display/cs/Store+NPC+Encounter+Script


int StoreNPCOnContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	int storeState = 0;
	
	if(!VarIsEmpty("StoreMapVar"))
	{
		// if a MapToken governs this store, that's all we check
		storeState = MapGetDataToken(VarGet("StoreMapVar"));
	}
	else
	{
		int i;
		int storeRequiresCount = VarGetArrayCount("StoreRequiresList");

		for (i = 0; i < storeRequiresCount; i++)
		{
			// check to see if player meets the condition for any of the stores in the list
			// if we find a store they are valid for, stop looking
			if (EvalEntityRequires(player, VarGetArrayElement("StoreRequiresList", i), Script_GetCurrentBlame()))
			{
				storeState = i + 1; // see storeState - 1 below due to MapToken value being 1 indexed
				break;
			}
		}
	}

	switch (link)
	{
	case 1:	// HELLO
		// check to see if this is a side-restricted store
		if ((VarGetNumber("OnlyHeroes") && IsEntityOnBlueSide(player)) ||
			(VarGetNumber("OnlyVillains") && IsEntityOnRedSide(player))) {
			// check to see if store should be open...
			if (storeState > 0) {
				if (storeState > VarGetArrayCount("StoreList"))
					storeState = VarGetArrayCount("StoreList");
				OpenStore(target, player, VarGetArrayElement("StoreList", storeState - 1));
				return 0;
			} else {
				sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));
				strcat(response->dialog, StringLocalizeWithVars( VarGet("ClosedText"), player));
				AddResponse(response, StringLocalizeWithVars(VarGet("Leave"), player), 3);
				return 1;
			}
		} else {
			sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));
			strcat(response->dialog, StringLocalizeWithVars( VarGet("RestrictedText"), player));
			AddResponse(response, StringLocalizeWithVars(VarGet("Leave"), player), 3);
			return 1;
		}
		break;
	default:
		return 0;
	}

}

void ScriptStoreNPC()
{
	INITIAL_STATE    
 	{
		ENTITY StoreNPC;
		
		//Find the ghost trap, and Set Script to Ready
		StoreNPC = ActorFromEncounterPos( MyEncounter(), VarGetNumber("StoreNPCEncounterPosition")); 
		if( StringEqual( StoreNPC, TEAM_NONE ) )
		{
			EndScript(); //Error
		}
		else
		{
			OnScriptedContactInteract(StoreNPCOnContactClick, StoreNPC);
			VarSet( "StoreNPC", StoreNPC );
			SET_STATE("Waiting");
		}
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
			
			if (IsEntityPlayer(player) && DistanceBetweenEntities(player, VarGet("StoreNPC")) < 20.0f)
			{
				// a player is close
				Say(VarGet("StoreNPC"), VarGet("Greeting"), 0);
				SET_STATE("PeopleNear");
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
			
			if (IsEntityPlayer(player) && DistanceBetweenEntities(player, VarGet("StoreNPC")) < 25.0f)
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

void StoreNPCInit()
{
	SetScriptName( "StoreNPC" );
	SetScriptFunc( ScriptStoreNPC );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "StoreNPCEncounterPosition",							NULL,		0 );
	SetScriptVar( "Greeting",											NULL,		SP_MULTIVALUE );
	SetScriptVar( "ClosedText",											NULL,		SP_MULTIVALUE );
	SetScriptVar( "RestrictedText",										NULL,		SP_MULTIVALUE );
	SetScriptVar( "StoreMapVar",										NULL,		SP_OPTIONAL );
	SetScriptVar( "StoreRequiresList",									NULL,		SP_OPTIONAL );
	SetScriptVar( "StoreList",											NULL,		0 );
	SetScriptVar( "Leave",												NULL,		SP_MULTIVALUE );
	SetScriptVar( "OnlyHeroes",											NULL,		0 );
	SetScriptVar( "OnlyVillains",										NULL,		0 );

	SetScriptSignal( "End Event", "EndScript" );
}
