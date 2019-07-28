// ZONE SCRIPT
//

// Used for testing new features of the script system

#include "scriptutil.h"


static void SpawnPowerSup()
{
	ENCOUNTERGROUP group = "first"; 
	int count, i;
	STRING loc, layout, def;

	count = VarGetArrayCount("PowerSupLocation");
	for (i = 0; i < count; i++)
	{
		loc = VarGetArrayElement("PowerSupLocation", i);
		layout = VarGetArrayElement("PowerSupSpawn", i);
		def = VarGetArrayElement("PowerSupDefinition", i);


		// spawn contact hero one
		group = FindEncounterGroupByRadius( 
			loc,
			0, 
			100, 
			layout, 
			1, 
			0 );  

		if (stricmp(group, "location:none") != 0) {
			Spawn( 1, "PowerSup", def, group, layout, 1, 0 );  
		}
	}
}

static void SpawnDebtProt()
{
	ENCOUNTERGROUP group = "first"; 
	int count, i;
	STRING loc, layout, def;

	count = VarGetArrayCount("DebtLocation");
	for (i = 0; i < count; i++)
	{
		loc = VarGetArrayElement("DebtLocation", i);
		layout = VarGetArrayElement("DebtSpawn", i);
		def = VarGetArrayElement("DebtDefinition", i);


		// spawn contact hero one
		group = FindEncounterGroupByRadius( 
			loc,
			0, 
			100, 
			layout, 
			1, 
			0 );  

		if (stricmp(group, "location:none") != 0) {
			Spawn( 1, "Debt", def, group, layout, 1, 0 );  
		}
	}
}


// called when the player leaves the map
int WeddingEventOnLeaveMap(ENTITY player)
{
	//Remove gang affiliation
	SetAIGang(player, 0);

	// give reward token
	EntityGrantRewardToken(player, "EventEntryToken", 1);

	return 0;
}

// called when the player enters the map
int WeddingEventOnEnterMap(ENTITY player)
{
	// Set ally correctly
	SetAIGang(player, GANG_OF_PLAYERS_WHEN_IN_COED_TEAMUP);

	// sending dialog message
	if (IsEntityHero(player))
		SendPlayerDialogWithIgnore(player, StringLocalize(VarGet("PopUpMessageHeroes")), 0);
	else
		SendPlayerDialogWithIgnore(player, StringLocalize(VarGet("PopUpMessageVillains")), 0);


	return 0;
}


// Just keeping everyone in line
void WeddingEventCheckPlayers(void)
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

// Switch everyone back to their regular teams
void WeddingEventResetPlayers(void)
{
	int j;
	int	groupNumber = NumEntitiesInTeam( ALL_PLAYERS );		

	for( j = 1 ; j <= groupNumber ; j++ )
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, j); 

		SetAIGang(player, 0);
	}
}


// Send everyone home
void WeddingEventClearPlayers(void)
{
	int j;
	int	groupNumber = NumEntitiesInTeam( ALL_PLAYERS );		

	for( j = 1 ; j <= groupNumber ; j++ )
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, j); 

		// clear any gang
		SetAIGang(player, 0);

		// give reward token
		EntityGrantRewardToken(player, "EventEntryToken", 1);

		if (GetAccessLevel(player) <= 0)
		{
			if (IsEntityHero(player))
			{
				// move player to 
				SetMap(player, 29, "NewPlayer");
			} else {
				// move player to 
				SetMap(player, 71, "NewPlayer");
			}
		}
	}
}

// Send everyone home
void WeddingEventClearRepeatPlayers(void)
{
	int j;
	int	groupNumber = NumEntitiesInTeam( ALL_PLAYERS );		

	for( j = 1 ; j <= groupNumber ; j++ )
	{
		ENTITY player = GetEntityFromTeam(ALL_PLAYERS, j); 

		if (EntityHasRewardToken(player, "EventEntryToken") &&
			EntityGetRewardTokenTime(player, "EventEntryToken") + (24 * 60 * 60) > SecondsSince2000())
		{
			// send player home

			if (GetAccessLevel(player) <= 0)
			{
				// give reward token
				EntityGrantRewardToken(player, "EventEntryToken", 1);

				// clear any gang
				SetAIGang(player, 0);

				if (IsEntityHero(player))
				{
					// move player to 
					SetMap(player, 29, "NewPlayer");
				} else {
					// move player to 
					SetMap(player, 71, "NewPlayer");
				}
			}
		}
	}
}


void WeddingEventEnd(void)
{
	OnPlayerEnteringMap( NULL );
	OnPlayerExitingMap( NULL );
	MapRemoveDataToken("WeddingEventResetToken");
	WeddingEventClearPlayers();
	EndScript();
}

void WeddingEvent(void) 
{
	INITIAL_STATE
	{
		ON_ENTRY 
		{
			OnPlayerExitingMap( WeddingEventOnLeaveMap );
			SpawnDebtProt();
			AllowHeroAndVillainPlayerTypesToTeamUpOnThisMap( false );
		}
		END_ENTRY

		// wait to start event
		SET_STATE("Startup");

	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Startup
	//////////////////////////////////////////////////////////////////////////////////////
	STATE("Startup") ////////////////////////////////////////////////////////
	{ 

		// no one should be here yet

		if( DoEvery("CleanupCheck", 0.1f, 0 ) ) // move to next stat
		{
			WeddingEventClearPlayers();
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// WeddingMode
	//////////////////////////////////////////////////////////////////////////////////////
	STATE("WeddingMode") ////////////////////////////////////////////////////////
	{ 
		ON_ENTRY 
		{
			// open event
			MapSetDataToken("WeddingEventResetToken", 2);

			// setup coop
			OnPlayerEnteringMap( WeddingEventOnEnterMap );

			// spawn power suppression
			SpawnPowerSup();

		}
		END_ENTRY

		// Check to see if anyone has gotten on the wrong team
		WeddingEventCheckPlayers();

		// Check for players who have already attended
		if( DoEvery("CleanupCheck", 0.1f, 0 ) )
		{
			WeddingEventClearRepeatPlayers();
		}

	}

	//////////////////////////////////////////////////////////////////////////////////////
	// FreeForAll
	//////////////////////////////////////////////////////////////////////////////////////
	STATE("FreeForAll") ////////////////////////////////////////////////////////
	{
		ON_ENTRY 
		{
			// close event
			MapSetDataToken("WeddingEventResetToken", 1);

			// remove coop
			OnPlayerEnteringMap( NULL );
			WeddingEventResetPlayers();

			// kill power suppression
			Kill("PowerSup", false);

		}
		END_ENTRY
	
		// Check for players who have already attended
		if( DoEvery("CleanupCheck", 0.1f, 0 ) )
		{
			WeddingEventClearRepeatPlayers();
		}

	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Cleanup
	//////////////////////////////////////////////////////////////////////////////////////
	STATE("Cleanup") ////////////////////////////////////////////////////////
	{
		ON_ENTRY 
		{
			WeddingEventClearPlayers(); 
		}
		END_ENTRY

		if( DoEvery("CleanupCheck", 0.1f, 0 ) ) // move to next stat
		{
			WeddingEventClearPlayers();
		}

	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Reset
	//////////////////////////////////////////////////////////////////////////////////////
	STATE("Reset") ////////////////////////////////////////////////////////
	{
		ON_ENTRY 
		{
		}
		END_ENTRY

		// Remove all players - Shouldn't be any, but...
		if( DoEvery("CleanupCheck", 0.1f, 0 ) )
		{
			WeddingEventClearPlayers();
		}


		if( DoEvery("TokenCheck", 2.0f, 0 ) ) // move to next state
		{
			MapRemoveDataToken("WeddingEventResetToken");
			Kill("PowerSup", false);
			SET_STATE("Startup");
		}
	}

	END_STATES

	ON_SIGNAL("Wedding")
		SET_STATE("WeddingMode");
	END_SIGNAL

	ON_SIGNAL("Fight")
		SET_STATE("FreeForAll");
	END_SIGNAL

	ON_SIGNAL("Cleanup")
		SET_STATE("Cleanup");
	END_SIGNAL

	ON_SIGNAL("Reset")
		SET_STATE("Reset");
	END_SIGNAL

	ON_SIGNAL("End")
		WeddingEventEnd();
	END_SIGNAL

	ON_STOPSIGNAL
		WeddingEventEnd();
	END_SIGNAL
}

void WeddingEventInit()
{
	SetScriptName( "WeddingEvent" );
	SetScriptFunc( WeddingEvent );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "PopUpMessageHeroes",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "PopUpMessageVillains",				NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "PowerSupLocation",					NULL,		0 );
	SetScriptVar( "PowerSupSpawn",						NULL,		0 );
	SetScriptVar( "PowerSupDefinition",					NULL,		0 );
	SetScriptVar( "DebtLocation",						NULL,		0 );
	SetScriptVar( "DebtSpawn",							NULL,		0 );
	SetScriptVar( "DebtDefinition",						NULL,		0 );

	SetScriptSignal( "Wedding", "Wedding" );		
	SetScriptSignal( "Fight", "Fight" );		
	SetScriptSignal( "Cleanup", "Cleanup" );		
	SetScriptSignal( "Reset", "Reset" );		
	SetScriptSignal( "End", "End" );		

}