// ZONE SCRIPT
//

// Used for testing new features of the script system

#include "scriptutil.h"




// called when the player leaves the map
int PlayTogetherOnLeaveMap(ENTITY player)
{
	//Remove gang affiliation
	SetAIGang(player, 0);

	return 0;
}

// called when the player enters the map
int PlayTogetherOnEnterMap(ENTITY player)
{
	// Set ally correctly
	SetAIGang(player, GANG_OF_PLAYERS_WHEN_IN_COED_TEAMUP);

	// Send greeting dialog if we're supposed to
	if( atoi( VarGet( "ShowPopUp" ) ) )
	{
		if (IsEntityHero(player))
			SendPlayerDialogWithIgnore(player, StringLocalize(VarGet("PopUpMessageHeroes")), 0);
		else
			SendPlayerDialogWithIgnore(player, StringLocalize(VarGet("PopUpMessageVillains")), 0);
	}

	return 0;
}


// called when the player enters the map
void PlayTogetherCheckPlayers(void)
{
	int j;
	int	groupNumber = NumEntitiesInTeam( ALL_PLAYERS );		

	for( j = 1 ; j <= groupNumber ; j++ )
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, j); 

		if (GetAIGang(player) != GANG_OF_PLAYERS_WHEN_IN_COED_TEAMUP)
			SetAIGang(player, GANG_OF_PLAYERS_WHEN_IN_COED_TEAMUP);
	}
}

void PlayTogetherRunOnInitialization(void)
{
	if( stricmp( VarGet( "WhoCanTeam" ), "Factions" ) == 0 )
	{
		AllowHeroAndVillainPlayerTypesToTeamUpOnThisMap( true );
	}
	else if( stricmp( VarGet( "WhoCanTeam" ), "Universes" ) == 0 )
	{
		AllowPrimalAndPraetorianPlayersToTeamUpOnThisMap( true );
	}
	else if( stricmp( VarGet( "WhoCanTeam" ), "Everyone" ) == 0 )
	{
		AllowHeroAndVillainPlayerTypesToTeamUpOnThisMap( true );
		AllowPrimalAndPraetorianPlayersToTeamUpOnThisMap( true );
	}

	OnPlayerExitingMap( PlayTogetherOnLeaveMap );
	OnPlayerEnteringMap( PlayTogetherOnEnterMap );
}

void PlayTogether(void)
{
	INITIAL_STATE

		ON_ENTRY 
				
		END_ENTRY

		// Check to see if anyone has gotten on the wrong team
		PlayTogetherCheckPlayers();

	END_STATES

	ON_SIGNAL("End")
		EndScript();
	END_SIGNAL

	ON_STOPSIGNAL
		EndScript();
	END_SIGNAL
}

void PlayTogetherInit()
{
	SetScriptName( "PlayTogether" );
	SetScriptFunc( PlayTogether );
	SetScriptFuncToRunOnInitialization( PlayTogetherRunOnInitialization );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "WhoCanTeam",							"Everyone",	SP_OPTIONAL | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "PopUpMessageHeroes",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "PopUpMessageVillains",				NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ShowPopUp",							"1",		SP_OPTIONAL | SP_DONTDISPLAYDEBUG );

	SetScriptSignal( "End", "End" );		

}