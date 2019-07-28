
#include "scriptutil.h"


void GhostTrap()
{
	INITIAL_STATE   
 	{
		ENTITY ghostTrap;
		
		//Find the ghost trap, and Set Script to Ready
		ghostTrap = ActorFromEncounterPos( MyEncounter(), VarGetNumber("GhostTrapEncounterPosition")); 
		if( StringEqual( ghostTrap, TEAM_NONE ) )
		{
			EndScript(); //Error
		}
		else
		{
			VarSet( "GhostTrap", ghostTrap );
			VarSetNumber( "GhostsTrapped", 0 );
			SET_STATE("ReadyToTrap");
		}
	}

	////////////////////////////////////////////////////////////////////
	STATE("ReadyToTrap")
	{
		#define MAXENTS 100
		ENTITY entlist[MAXENTS];
		NUMBER kaboom = 0;
		NUMBER nearbyDeadGhostCount;
		NUMBER i;

		//Look for Nearby Dead Ghosts that you haven't checked yet
		nearbyDeadGhostCount = GetAllEntities( entlist, 
			VarGet( "GhostVillainGroup" ),	
			0,		//Not yet implemented
			0,		//Not yet implemented
			0,      //Not yet implemented
			0,
			VarGetNumber( "GhostTrapRange" ), 
			VarGet( "GhostTrap" ),
			0,
			0,
			1,
			MAXENTS,
			1
			);

		for( i = 0 ; i < nearbyDeadGhostCount ; i++ )
		{
			if( RandomNumber( 0, 100 ) < VarGetNumber( "ChanceTrapGetsGhost" ) )
			{
				int trappedCount;

				//When you Find one, Suck it into the trap
				SetAIUsePowerNow( VarGet( "GhostTrap" ), entlist[i], VarGet( "GhostTrappingPower" ) );
				trappedCount = VarGetNumber( "GhostsTrapped" ) + 1;

				VarSetNumber( "GhostsTrapped", trappedCount ); 
				
				CountTowardGhostTrapBadge( entlist[i] );

				//If the trap is overloaded, the trap goes boom
				if( trappedCount > VarGetNumber( "TrapCapacity" ) )
					kaboom = 1;
			}
		}

		//Make the trap go boom 
		if( kaboom )
		{
			TimerSet("UsePower", 0.03);
			SET_STATE( "BlowingUp" );
		}
	}

	////////////////////////////////////////////////////////////////////
	STATE("BlowingUp")
	{
		if( TimerElapsed("UsePower" ) )
		{
			VarSetNumber( "GhostsTrapped", 0 ); 
			SetAIUsePowerNow( VarGet( "GhostTrap" ), 0, VarGet( "TrapOverloadPower" ) );
			
			TimerSet("MakeSpawn", 0.05);
			SET_STATE( "BlowingUp2" );
		}
	}

	////////////////////////////////////////////////////////////////////
	STATE("BlowingUp2")
	{
		if( TimerElapsed("MakeSpawn" ) )
		{
			ENCOUNTERGROUP escapedGhostGroup;

			escapedGhostGroup = FindEncounter( VarGet( "GhostTrap" ), 0, 100, 0, 0, 0, VarGet( "EscapedGhostsEncounterGroupName" ), SEF_SORTED_BY_DISTANCE | SEF_UNOCCUPIED_ONLY );
			Spawn( 1, "EscapedGhosts", VarGet( "EscapedGhostsEncounter" ), escapedGhostGroup, NULL, 0, 0 ); //TO DO setting level and numHeroes needed?
			SetEncounterActive(escapedGhostGroup);
			DetachSpawn( escapedGhostGroup );

			SET_STATE( "ReadyToTrap" );
		}
	}

	END_STATES
}

void GhostTrapInit()
{
	SetScriptName( "GhostTrap" );
	SetScriptFunc( GhostTrap );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "GhostTrapEncounterPosition", "",			0 );
	SetScriptVar( "GhostVillainGroup",		"",				0 );  //VillainType ?? of Monster that is affected by the trap. 
	SetScriptVar( "ChanceTrapGetsGhost",	"",				0 );  //When ghost dies in radius, chance it is sucked into the ghost trap
	SetScriptVar( "TrapCapacity",			"",				0 );  //Number of ghosts a trap can hold before it explodes
	SetScriptVar( "EscapedGhostsEncounter",	"",				0 );  //.spawndef spawned when trap overloads
	SetScriptVar( "EscapedGhostsEncounterGroupName", "",	0 ); //Nearby encounter group to put escapes ghosts encounters on
	SetScriptVar( "TrapOverloadPower",		"",				0 );  //Power to use to show trap has exploded
	SetScriptVar( "GhostTrappingPower",		"",				0 );  //Power Trap uses on a dead ghost to show it has been trapped
	SetScriptVar( "GhostTrapRange",			"",				0 );

	//Ghost trap has 

	SetScriptSignal( "End Event", "EndScript" );
}
