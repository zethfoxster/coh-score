// MISSION SCRIPT

// This is an encounter script that runs the Clockwork attempting to build a Paladin
//


#include "scriptutil.h"

#define FOREMAN_POSITION 1 //Foreman needs to be in the first postion in the encounter

static int g_OctopusRunning = 0;

static void showState( void )
{
	DebugString(StringAdd("In state: ", VarGet(STATE_VAR)));
}

//Pick a random selection of minions to say something
static void OctopusMinionSays( NUMBER count, STRING says )
{
	ENTITY minion;
	int i; 
	int numEnts = NumEntitiesInTeam("DockWorkers");

	for( i = 1 ; i <= numEnts ; i++ )
	{
		if( RandomFraction(0.0, 1.0) < (float)count/(float)numEnts )
		{
			minion = GetEntityFromTeam("DockWorkers", i);
			Say( GetEntityFromTeam("DockWorkers", i), says, 0  );
		}
	}
}


void OctopusAttack(void)
{  
	INITIAL_STATE     
//////////////////////////////////////////////////////////////////////////
	ON_ENTRY  //////////////////////////////////////////////////////////////////////////

	if (g_OctopusRunning > 0)
	{
		Kill( "DockWorkers", 0 );
		Kill( "Octopus", 0 );  
		CloseMyEncounterAndEndScript();
		return;
	}

	g_OctopusRunning++;

	//Spawn dock workers and set their behavior
	{
		ENCOUNTERGROUP group; 
		int numEnts, i;

		//Spawn striking dock workers in nearby dockworker spawn
		group = FindEncounterGroupByRadius( 
			EncounterPos( MyEncounter(), 1 ),
			0, 
			500, //Within 100 feet of this encounter
			VarGet("DockWorkerLayout"), 
			1, 
 			0 );  
		Spawn( 1, "DockWorkers", VarGet( "DockWorkerSpawn" ), group, VarGet("DockWorkerLayout"), 1, 0 );  
		DetachSpawn( "DockWorkers" ); 

		//Set the foreman and dockworkers to say something to the heroes
		numEnts = NumEntitiesInTeam("DockWorkers");
		for( i = 1 ; i <= numEnts ; i++ )
		{
			if( i == 1 ) //1 is the foreman position, he says something special
				SayOnClick( GetEntityFromTeam("DockWorkers", i), VarGet("DockForemanToHero") );
			else
				SayOnClick( GetEntityFromTeam("DockWorkers", i), VarGet("DockWorkerToHero") );
		}
	}
	////End spawn dock workers
	
	////Spawn the Octopus and set the spawns
	{
		ENCOUNTERGROUP group = 0; 
		ENTITY head = 0;

		//Returns a random layout in the group it finds (the octopus encounter group) 
		group = FindEncounterGroupByRadius( 
			EncounterPos( MyEncounter(), 1 ),
			0, 
			1000, //Within 100 feet of this encounter
			VarGet("OctopusLayout"), 
			1, 
			0 );  

	
		//Spawn Octopus Team at Random Octopus Spawn
		Spawn( 1, "Octopus", VarGet( "OctopusSpawn" ), group, VarGet("OctopusLayout"), 
				VarGetNumber( "OctopusSpawnLevel" ), 0 ); 

		VarSet( "OctopusSpawnGroup", group );
		
		head =  GetEntityFromTeam(group, 1);
		VarSet( "OctopusHead", head );
	
		VarSetNumber( "OldOctopusMemberCount", NumEntitiesInTeam( "Octopus" ) );
	}
	////End spawn Octopus

	// send alert
	SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
															"zonename", StringLocalizeMenuMessages(GetMapName())));

	SET_STATE("OctopusAttacking");

	END_ENTRY

////////////////////////////////////////////////////////////////////////////////////////
	STATE("OctopusAttacking") ////////////////////////////////////////////////////////

	//Dock workers grumble and the Foreman talks.  TO DO : Maybe generalize
	if( DoEvery("DockWorkerGrumble", 0.3f, 0 ) AND RandomFraction( 0.0f, 1.0f ) > 0.5f )  
	{
		Say( GetEntityFromTeam( "DockWorkers", FOREMAN_POSITION ), VarGet("DockForemanToWorkers"), 0 );
		OctopusMinionSays( 1, VarGet( "DockWorkerGrumble" ) );
	}
	//End grumbling

	//If the octopus is dead, change stuff, and go to success state
	if( !EntityExists( VarGet("OctopusHead") ) || 0 >= GetHealth( VarGet("OctopusHead") ) )
	{
		int numWorkers, i; 

		SET_STATE("HeroesSucceed");

		//TO DO : Hand out rewards
		Kill( "Octopus", 1 ); //Any remaining tentacles

		Say( GetEntityFromTeam("DockWorkers", FOREMAN_POSITION ), VarGet("DockForemanCheer"), 0 );
		OctopusMinionSays( 20, VarGet("DockWorkerCheer" ) );
		
		numWorkers = NumEntitiesInTeam( "DockWorkers" );
		for( i = 1 ; i <= numWorkers ; i++ )
		{
			if( i == 1 ) //1 is the foreman position, he says something special
				SayOnClick( GetEntityFromTeam("DockWorkers", i), VarGet("DockForemanToHeroThanks") );
			SayOnClick( GetEntityFromTeam("DockWorkers", i), VarGet("DockWorkerToHeroThanks") );
		}

		GiveRewardToAllInRadius( VarGet("Reward"), EncounterPos( MyEncounter(), 1 ) , VarGetFraction("RewardRadius") );
	}
	else
	//Otherwise, If a tentacle is destroyed go to sink and respawn
	{
		//If a tentacle has been slain, move the spawn
		if( NumEntitiesInTeam( "Octopus" ) < VarGetNumber( "OldOctopusMemberCount" ) )
		{
			SetAIPriorityList(VarGet("OctopusSpawnGroup"), "PL_OCTOPUS_SUBMERGE" );
			SET_STATE("OctopusMoving");
			TimerSet("OctopusSubmergeTime", 0.3f);
		}
	}

////////////////////////////////////////////////////////////////////////////////////////
	STATE("OctopusMoving") ///////////////////////////////////////////////////////////

		//Give it some time to sink the spawns, then move them somewhere else
	if( TimerElapsed("OctopusSubmergeTime") ) 
	{
		int i;
		ENCOUNTERGROUP newGroup;

		//Returns a random layout in the group it finds (the octopus encounter group) 
		newGroup = FindEncounterGroupByRadius( 
			EncounterPos( MyEncounter(), 1 ),
			0, 
			1000, //Within 100 feet of this encounter
			VarGet("OctopusLayout"), 
			1, 
			0 );  

		//Move the octopus spawn
		for( i = 1 ; i <= 9 ; i++ ) //TO DO define var
		{
			ENTITY ent;
			LOCATION loc;
			ent = ActorFromEncounterPos( VarGet("OctopusSpawnGroup"), i); 
			if( ent )
			{
				loc = EncounterPos(newGroup, i);
				SetPosition( ent, loc );
			}
		} 

		SetAIPriorityList(VarGet("OctopusSpawnGroup"), "PL_TENTACLESTILLRANGED" ); 
		VarSetNumber( "OldOctopusMemberCount", NumEntitiesInTeam( "Octopus" ) );

		SET_STATE("OctopusAttacking");
	}

	////////////////////////////////////////////////////////////////////////////////////////
	STATE("HeroesSucceed") ///////////////////////////////////////////////////////////
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  ////////////////////////////////////////////////////////////////
		{
			// send alert
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("ClearMessage"), 1,
			"zonename", StringLocalizeMenuMessages(GetMapName())));
		}
		END_ENTRY

		if( DoEvery("DockWorkerCheering", 0.3f, 0 ) AND RandomFraction( 0.0f, 1.0f ) > 0.5f )  
		{
			Say( GetEntityFromTeam( "DockWorkers", FOREMAN_POSITION ), VarGet("DockForemanCheer"), 0 );
			OctopusMinionSays( 1, VarGet( "DockWorkerCheer" ) );
		}

		//After 10 minutes of partay, kill the dock workers
		if( DoEvery("DockWorkerDance", (FRACTION)VarGetNumber("DockWorkersDanceTime"), 0 ) ) 
		{
			g_OctopusRunning--;
			Kill( "DockWorkers", 0 );  //Get rid of the docworkers
			CloseMyEncounterAndEndScript();
		}
	}
	END_STATES

}