
#include "scriptutil.h"


int BetaNPCOnClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	int storeState = MapGetDataToken(VarGet("StoreMapVar"));

	switch (link)
	{
	case 1:	// HELLO
		// check to see if the player has already been here
		sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));
		if (!VarIsEmpty("Level"))
		{
			NUMBER level = VarGetNumber("Level");

			if ((!EntityHasRewardToken(player, VarGet("Token")) || 	EntityGetRewardToken(player, VarGet("Token")) < level) &&
				GetLevel(player) < level)
			{
				strcat(response->dialog, StringLocalizeWithVars( VarGet("LevelUpText"), player));
				EntityGrantRewardToken(player, VarGet("Token"), level);
				EntitySetXPByLevel(player, level);
				EntityAddInfluence(player, VarGetNumber("Influence"));
			} else{
				strcat(response->dialog, StringLocalizeWithVars( VarGet("ReturnText"), player));
			}
		} else {	
			if (EntityHasRewardToken(player, VarGet("Token")))
			{
				strcat(response->dialog, StringLocalizeWithVars( VarGet("ReturnText"), player));
				AddResponse(response, StringLocalizeWithVars(VarGet("Teleport"), player), 5);
			} else {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("LevelUpText"), player));
				EntityGrantRewardToken(player, VarGet("Token"), 1);
				EntityGrantReward(player, VarGet("Reward"));
				EntitySetXP(player, VarGetNumber("XP"));
				EntityAddInfluence(player, VarGetNumber("Influence"));
			}
		}
		AddResponse(response, StringLocalizeWithVars(VarGet("Leave"), player), 3);
		return 1;
		break;
	case 5: // teleport
		// add teleport code here
		SetMap(player, VarGetNumber("MapMove"), VarGet("SpawnTarget"));
		return 0;
	case 3: // exit
	default:
		return 0;
	}


	return 0;
}

void ScriptBetaNPC()
{
	ENCOUNTERGROUP group;

	////////////////////////////////////////////////////////////////////////////////////////
	INITIAL_STATE    
 	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
			// spawn NPCs
			group = FindEncounterGroupByRadius( 
				MyLocation(),
				0, 
				100, 
				"NPC_Leveler", 
				1, 
 				0 );  
 
			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Leveler", VarGet("SpawnDef"), group, "NPC_Leveler", 1, 0 );  

				// add on click handler
				OnScriptedContactInteract(BetaNPCOnClick, ActorFromEncounterPos(group,1));

				// playing with map markers
	//			VarSetNumber("Leveler", CreateWaypoint(EncounterPos(group, 1), VarGet("ScientistVillainMapName"), "map_enticon_contact", false, -1.0f));
			} else {
				EndScript();
			}
		}
		END_ENTRY

	}

	////////////////////////////////////////////////////////////////////////////////////////
	STATE("EndScript") ////////////////////////////////////////////////////////
	{
		Kill("Leveler", false);
		EndScript();
	}
	END_STATES


	ON_SIGNAL("Level15")
	{
		VarSetNumber("Level", 15);
	}
	END_SIGNAL

	ON_SIGNAL("Level20")
	{
		VarSetNumber("Level", 20);
	}
	END_SIGNAL

	ON_SIGNAL("Level25")
	{
		VarSetNumber("Level", 25);
	}
	END_SIGNAL

	ON_SIGNAL("Level30")
	{
		VarSetNumber("Level", 30);
	}
	END_SIGNAL


	ON_SIGNAL("Level35")
	{
		VarSetNumber("Level", 35);
	}
	END_SIGNAL

	ON_SIGNAL("Level40")
	{
		VarSetNumber("Level", 40);
	}
	END_SIGNAL

	ON_SIGNAL("Level45")
	{
		VarSetNumber("Level", 45);
	}
	END_SIGNAL

	ON_SIGNAL("Level50")
	{
		VarSetNumber("Level", 50);
	}
	END_SIGNAL

	ON_SIGNAL("EndScript")
	{
		Kill("Leveler", false);
		EndScript();
	}
	END_SIGNAL

	ON_STOPSIGNAL
	{
		Kill("Leveler", false);
		EndScript();
	}
	END_SIGNAL
}

void BetaNPCInit()
{
	SetScriptName( "BetaNPC" );     
	SetScriptFunc( ScriptBetaNPC );
	SetScriptType( SCRIPT_LOCATION );		

	SetScriptVar( "LevelUpText",	NULL,		0 );
	SetScriptVar( "Token",			NULL,		0 );
	SetScriptVar( "Level",			NULL,		SP_OPTIONAL );
	SetScriptVar( "ReturnText",		NULL,		SP_OPTIONAL );
	SetScriptVar( "Teleport",		NULL,		SP_OPTIONAL );
	SetScriptVar( "Leave",			NULL,		SP_OPTIONAL );
	SetScriptVar( "MapMove",		NULL,		SP_OPTIONAL );
	SetScriptVar( "SpawnTarget",	NULL,		SP_OPTIONAL );
	SetScriptVar( "Reward",			NULL,		SP_OPTIONAL );
	SetScriptVar( "XP",				NULL,		SP_OPTIONAL );
	SetScriptVar( "Influence",		NULL,		SP_OPTIONAL );
	SetScriptVar( "SpawnDef",		"scripts.loc/SpawnDefs/V_BetaTest/NPC_Leveler_D0_V0.spawndef",		SP_OPTIONAL );


	SetScriptSignal( "15", "Level15" );
	SetScriptSignal( "20", "Level20" );
	SetScriptSignal( "25", "Level25" );
	SetScriptSignal( "30", "Level30" );
	SetScriptSignal( "35", "Level35" );
	SetScriptSignal( "40", "Level40" );
	SetScriptSignal( "45", "Level45" );
	SetScriptSignal( "50", "Level50" );

	SetScriptSignal( "Remove Leveler", "EndScript" );
}
