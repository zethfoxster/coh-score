
#include "scriptutil.h"

// Documentation at: http://code:8081/display/cs/Trade+NPC+Encounter+Script


int TradeNPCOnRightSide(ENTITY player)
{
	return ((VarGetNumber("OnlyHeroes") && IsEntityOnBlueSide(player)) ||
			(VarGetNumber("OnlyVillains") && IsEntityOnRedSide(player)));
}

int TradeNPCHaveTheGoods(ENTITY player)
{
	int number = VarGetArrayCount("SalvageList");
	int i;

	for (i = 0; i < number; i++)
	{
		if (EntityCountSalvage(player, VarGetArrayElement("SalvageList", i)) < 
			StringToNumber(VarGetArrayElement("SalvageQuantityList", i)))
		{
			return false;
		}
	}

	if (!StringEmpty(VarGet("Influence")))
	{
		if (EntityGetInfluence(player) < VarGetNumber("Influence"))
			return false;
	}

	return true;
}


void TradeNPCTakeTheGoods(ENTITY player)
{
	if (TradeNPCHaveTheGoods(player))
	{
		int number = VarGetArrayCount("SalvageList");
		int i;

		// remove salvage
		for (i = 0; i < number; i++)
		{
			EntityGrantSalvage(player, VarGetArrayElement("SalvageList", i), -StringToNumber(VarGetArrayElement("SalvageQuantityList", i)));
		}

		// remove any inf
		if (!StringEmpty(VarGet("Influence")))
		{
			EntityAddInfluence(player, -(VarGetNumber("Influence")));
		}
	}
}

// TradeNPCIsLevelRestricted
//		returns true if this contact has a level restriction and the player specified should be restricted
//		returns false otherwise
int TradeNPCIsLevelRestricted(ENTITY player)
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

int TradeNPCOnContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	int i, count;

	switch (link)
	{
	case 1:	// HELLO
		sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));

		// check to see if this is a restricted contact
		if (TradeNPCIsLevelRestricted(player) && !VarIsEmpty("LevelRestrictedText"))	{
			// Not correct level
			strcat(response->dialog, StringLocalizeWithVars( VarGet("LevelRestrictedText"), player));
		} else if (!TradeNPCOnRightSide(player)) {
			// wrong hero/villain side
			strcat(response->dialog, StringLocalizeWithVars( VarGet("RestrictedText"), player));
		} else if (!EvalEntityRequires(player, VarGet("Requires"), Script_GetCurrentBlame())) {
			// failed evail
			strcat(response->dialog, StringLocalizeWithVars( VarGet("FailedRequiresText"), player));
		} else {
			// correct side
			strcat(response->dialog, StringLocalizeWithVars( VarGet("OfferText"), player));
			if (TimerElapsed("ContactTalkTimer") && !VarIsEmpty("OutLoudOfferText"))
			{
				Say(VarGet("TradeNPC"), StringLocalizeWithVars(VarGet("OutLoudOfferText"), player), 0);
				TimerSet("ContactTalkTimer", 0.25f);
			}
			if (TradeNPCHaveTheGoods(player))
			{
				AddResponse(response, StringLocalizeWithVars(VarGet("Accept"), player), 4);
			}
		}

		AddResponse(response, StringLocalizeWithVars(VarGet("Leave"), player), 3);
		return 1;
		break;
	case 4:	// ACCEPTED
		sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));
		
		if (TradeNPCIsLevelRestricted(player) && !VarIsEmpty("LevelRestrictedText"))	{
			// Not correct level
			strcat(response->dialog, StringLocalizeWithVars( VarGet("LevelRestrictedText"), player));
		} else if (!TradeNPCOnRightSide(player)) {
			// wrong hero/villain side
			strcat(response->dialog, StringLocalizeWithVars( VarGet("RestrictedText"), player));
		} else if (!EvalEntityRequires(player, VarGet("Requires"), Script_GetCurrentBlame())) {
			// failed evail
			strcat(response->dialog, StringLocalizeWithVars( VarGet("FailedRequiresText"), player));
		} else {
			strcat(response->dialog, StringLocalizeWithVars( VarGet("AcceptText"), player));
			
			if (TradeNPCHaveTheGoods(player))
			{
				// take the salvage
				TradeNPCTakeTheGoods(player);

				// give the reward
				EntityGrantReward(player, VarGet("Reward"));		
				if (!VarIsEmpty("RewardFloat"))
					SendPlayerAttentionMessage(player, VarGet("RewardFloat"));

				// check for map tokens
				if (!VarIsEmpty("RewardMapTokenList") && TimerElapsed("TokenAwardTimer") && 
					(VarIsEmpty("RewardMapTokenRequires") || EvalEntityRequires(player, VarGet("RewardMapTokenRequires"), Script_GetCurrentBlame())))
				{
					int chance = RandomNumber(1, 100);

					if (chance <= VarGetNumber("RewardMapChance"))
					{
						count = VarGetArrayCount("RewardMapTokenList");
						for (i = 0; i < count; i++)
						{
							MapSetDataToken(VarGetArrayElement("RewardMapTokenList", i), 1);
						}
						TimerSet("TokenAwardTimer", VarGetFraction("RewardMapTokenMinTime"));
					}
				}

			}
		}
		AddResponse(response, StringLocalizeWithVars(VarGet("Leave"), player), 3);
		return 1;
		break;
	default:
		return 0;
	}

}

void ScriptTradeNPC()
{
	INITIAL_STATE    
 	{
		ENTITY TradeNPC;
		int numberSal = VarGetArrayCount("SalvageList");
		int numberSalAmt = VarGetArrayCount("SalvageQuantityList");
		
		//Find the NPC, and Set Script to Ready
		TradeNPC = ActorFromEncounterPos( MyEncounter(), VarGetNumber("TradeNPCEncounterPosition")); 
		if( StringEqual( TradeNPC, TEAM_NONE ) || numberSal != numberSalAmt )
		{
			EndScript(); //Error
		}
		else
		{
			OnScriptedContactInteract(TradeNPCOnContactClick, TradeNPC);
			VarSet( "TradeNPC", TradeNPC );
			SET_STATE("Waiting");
		}
		TimerSet("ContactTalkTimer", 0.25f);
		TimerSet("TokenAwardTimer", VarGetFraction("RewardMapTokenMinTime"));
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
			
			if (IsEntityPlayer(player) && TradeNPCOnRightSide(player) && 
				DistanceBetweenEntities(player, VarGet("TradeNPC")) < VarGetFraction("CloseDistance"))
			{
				if (!TradeNPCIsLevelRestricted(player)) {
					// a player is close
					if (!VarIsEmpty("Greeting"))
						Say(VarGet("TradeNPC"), StringLocalizeWithVars(VarGet("Greeting"), player), 0);
					SET_STATE("PeopleNear");
				} else if (!VarIsEmpty("LevelRestrictedGreeting")) {
					Say(VarGet("TradeNPC"), StringLocalizeWithVars(VarGet("LevelRestrictedGreeting"), player), 0);
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
			
			if (IsEntityPlayer(player) && DistanceBetweenEntities(player, VarGet("TradeNPC")) < VarGetFraction("CloseDistance") + 5.0f)
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

void TradeNPCInit()
{
	SetScriptName( "TradeNPC" );
	SetScriptFunc( ScriptTradeNPC );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "TradeNPCEncounterPosition",							NULL,		0 );
	SetScriptVar( "Greeting",											NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "RestrictedText",										NULL,		SP_MULTIVALUE );
	SetScriptVar( "Requires",											NULL,		SP_MULTIVALUE );
	SetScriptVar( "FailedRequiresText",									NULL,		SP_MULTIVALUE );
	SetScriptVar( "OfferText",											NULL,		SP_MULTIVALUE );
	SetScriptVar( "OutLoudOfferText",									NULL,		SP_MULTIVALUE | SP_OPTIONAL );
	SetScriptVar( "Accept",												NULL,		SP_MULTIVALUE );
	SetScriptVar( "AcceptText",											NULL,		SP_MULTIVALUE );
	SetScriptVar( "Leave",												NULL,		SP_MULTIVALUE );
	SetScriptVar( "OnlyHeroes",											NULL,		0 );
	SetScriptVar( "OnlyVillains",										NULL,		0 );


	SetScriptVar( "Influence",											NULL,		SP_OPTIONAL );
	SetScriptVar( "SalvageList",										NULL,		SP_OPTIONAL );
	SetScriptVar( "SalvageQuantityList",								NULL,		SP_OPTIONAL );

	SetScriptVar( "Reward",												NULL,		0 );
	SetScriptVar( "RewardFloat",										NULL,		SP_OPTIONAL );

	SetScriptVar( "RewardMapTokenList",									NULL,		SP_OPTIONAL );
	SetScriptVar( "RewardMapChance",									"100",		SP_OPTIONAL );
	SetScriptVar( "RewardMapTokenRequires",								NULL,		SP_OPTIONAL );
	SetScriptVar( "RewardMapTokenMinTime",								"60",		SP_OPTIONAL );

	SetScriptVar( "MinLevelRestricted",									NULL,		SP_OPTIONAL );
	SetScriptVar( "MaxLevelRestricted",									NULL,		SP_OPTIONAL );
	SetScriptVar( "LevelRestrictedText",								NULL,		SP_MULTIVALUE | SP_OPTIONAL);
	SetScriptVar( "LevelRestrictedGreeting",							NULL,		SP_MULTIVALUE | SP_OPTIONAL );

	SetScriptVar( "CloseDistance",										"25.0",		SP_OPTIONAL );

	SetScriptSignal( "End Event", "EndScript" );
}
