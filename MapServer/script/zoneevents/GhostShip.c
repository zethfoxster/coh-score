// MISSION SCRIPT

// This is an encounter script that runs the Ghostship through Independence Port
//

#include "scriptutil.h"
#include "stdio.h"

static void showState( void )
{
	DebugString(StringAdd("In state: ", VarGet(STATE_VAR)));
}

void GhostShip(void)
{  
	INITIAL_STATE     
//////////////////////////////////////////////////////////////////////////
	ON_ENTRY  //////////////////////////////////////////////////////////////////////////
	{
		ENCOUNTERGROUP group; 
		ENTITY ghostShip;
		//Spawn Ghost Ship

		group = FindEncounterGroupByRadius( 
			EncounterPos( MyEncounter(), 1 ),
			0, 
			100, //Within 100 feet of this encounter
			"GhostShip", 
			1, 
			0 );
		EncounterPos( MyEncounter(), 1 );
		Spawn( 1, "GhostShip", VarGet( "GhostShipSpawn" ), group, "GhostShip", VarGetNumber( "GhostShipSpawnLevel" ) , 0 );

		ghostShip =  ActorFromEncounterPos(group, 1);  //Ghost Ship spawns at position 1
		VarSet( "GhostShip", ghostShip );
		

		// send alert
		SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
																"zonename", StringLocalizeMenuMessages(GetMapName())));

		//Set ghost ship on it's track (automatic if the layout has a patrol track?
	}
	SET_STATE("GhostShipMoving");

	END_ENTRY

////////////////////////////////////////////////////////////////////////////////////////
	STATE("GhostShipMoving") ////////////////////////////////////////////////////////

	//Spawn dudes wherever the Ghost ship goes
	if( DoEvery( "IntervalBetweenSpawns", 0.5, 0 ) )
	{
		ENCOUNTERGROUP group; 
		LOCATION loc;
		NUMBER numToKill, i;

		numToKill =  NumEntitiesInTeam("Ghosts") - VarGetNumber( "MaxGhosts" );
		for( i = 0 ; i < numToKill ; i++ )
		{
			Kill( GetEntityFromTeam("Ghosts", i+1), 0 );
		}
		
		loc = EntityPos( VarGet( "GhostShip" ) );

		//Spawn Ghosts
		group = FindEncounterGroupByRadius( 
			loc,
			0, 
			400, //Within 400 feet of this encounter
			"Around",  //"GhostSpawn is an around"
			1, 
			0 );

		if( !StringEqual( LOCATION_NONE, group ) )
			Spawn( 1, "Ghosts", VarGet( "GhostSpawn" ), group, 0, VarGetNumber( "GhostSpawnLevel") , 0 );
	}
 
	if( DoEvery( "IntervalBetweenChatter", 0.03f, 0 ) )  
	{ 
		NUMBER i; 
		NUMBER numEnts = NumEntitiesInTeam("Ghosts");

		for( i = 1 ; i <= numEnts ; i++ )
		{
			if( RandomFraction(0.0, 1.0) < 0.01f )
			{
				Say( GetEntityFromTeam("Ghosts", i), VarGet("GhostSpeak"), 0  );
			}
		}
	}
	
	//if the ghost ship reaches it's destination, end the event
	if( EntityInArea( VarGet( "GhostShip" ), "marker:GhostShipPathEnd" ) || NumEntitiesInTeam("GhostShip") <= 0) //If the ghost ship reaches it's destination
	{
		Kill( "GhostShip", 0 ); 

		// send alert
		SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("ClearMessage"), 1,
																"zonename", StringLocalizeMenuMessages(GetMapName())));

		SET_STATE( "WaitingToCleanGhosts" );
		TimerSet( "GhostKillTimer", VarGetFraction( "TimeToKillGhosts" ) );
	}

	////////////////////////////////////////////////////////////////////////////////////////
	STATE("WaitingToCleanGhosts") ///////////////////////////////////////////////////////

	if( TimerElapsed( "GhostKillTimer" ) )
	{
		Kill( "Ghosts", 0 );  //Get rid of the ghosts
		CloseMyEncounterAndEndScript();
	}

	END_STATES

}