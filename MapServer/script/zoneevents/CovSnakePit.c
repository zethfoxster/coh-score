// LOCATION SCRIPT

// This is a location script that runs the Snake Pit event
//

#include "scriptutil.h"

#define CLEANUP_PL							"PL_FleeToNearestDoor"
#define COV_SNAKEPIT_MOVETO_VARIANCE		10.0f
#define COV_SNAKEPIT_MOVETO_VARIANCE_CHECK	20.0f

static int gCoVSnakePitRunning = 0;
 
void CoVSnakePit(void)
{  
	ENCOUNTERGROUP group = "first"; 
	int i;
	int groupNumber;

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

		// if there are too many running, don't spawn another
		if (gCoVSnakePitRunning >= 1)
		{
			EndScript();
			return;
		}
 
		gCoVSnakePitRunning++;

		// Find  police
		group = FindEncounterGroupByRadius( 
			MyLocation(),
			0, 
			300,  // Nearby
			"Script_Flyer", 
			1, 
  			0 );    

		if (stricmp(group, "location:none") != 0) {
			Spawn( 1, "Flyer", VarGet("FlyerSpawn"), group, "Script_Flyer", VarGetNumber("FlyerSpawnLevel"), 0 );  
			groupNumber = NumEntitiesInTeam( "Flyer" );			// there should be only one of these... but...
			if ( groupNumber == 0 ) {
				gCoVSnakePitRunning--;
				EndScript();
			} else {	
/*				for( i = 1 ; i <= groupNumber ; i++ )
				{
					ENTITY e = GetEntityFromTeam("Flyer", i);

					SetPosition(e, VarGet("FlyerSpawnLoc"));
				}
*/				SetAIMoveTarget("Flyer", MyLocation(), MEDIUM_PRIORITY, false, true, COV_SNAKEPIT_MOVETO_VARIANCE);
			}
		} else {
			gCoVSnakePitRunning--;
			EndScript();
		}

		VarSetNumber( "CurrentSnakes", 0);

		SET_STATE("FlyerMoving");

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////
	STATE("FlyerMoving") ////////////////////////////////////////////////////////

	// checking for the arrival of the flyer
	if( DoEvery("ArrivalCheck", 0.10f, 0 ) ) 
	{ 
		groupNumber = NumEntitiesInTeam( "Flyer" );			// there should be only one of these... but...
		if ( groupNumber == 0 ) {
			gCoVSnakePitRunning--;
			EndScript();
		} else {
			for( i = 1 ; i <= groupNumber ; i++ )
			{
				ENTITY e = GetEntityFromTeam("Flyer", i);
				FRACTION dist = DistanceBetweenLocations(e, MyLocation());

				if (dist < COV_SNAKEPIT_MOVETO_VARIANCE_CHECK)
				{
					SetAIPriorityList("Flyer", "PL_FrozenInPlace");
					SET_STATE("FlyerArrived");
					dist = 0;
				}
				dist = 0;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("FlyerArrived") ////////////////////////////////////////////////////////

	if( DoEvery("ArrivalCheck", 1.50f, 0 ) ) // after a minute and a half, check if there are any players around...
	{ 
		int found = false;

		groupNumber = NumEntitiesInTeam( ALL_PLAYERS );		
		for( i = 1 ; i <= groupNumber ; i++ )
		{
			ENTITY e = GetEntityFromTeam(ALL_PLAYERS, i); 
			if (DistanceBetweenLocations(e, MyLocation()) < VarGetFraction("ClosePlayerDist"))
			{
				SET_STATE("SnakesComeOut");
				found = true;
				break;
			}
		}

		if (!found)
			SET_STATE("FlyerMovingOff");

	}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("SnakesComeOut") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Find snake spawn
			group = FindEncounterGroupByRadius( 
				MyLocation(),
				0, 
				300, // Nearby
				"Script_Snakes", 
				1, 
 				0 );  

			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Snakes", VarGet("SnakeSpawn"), group, "Script_Snakes", VarGetNumber("SnakeSpawnLevel"), 0 );  
				VarSetNumber("SnakeWave", 1);
			}
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if( DoEvery("FlyerArrivalCheck", 1.50f, 0 ) ) // move to next state
		{ 
				SET_STATE("Snakes");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Snakes") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Find  spiders
			group = FindEncounterGroupByRadius( 
				MyLocation(),
				0, 
				300, // Anywhere on level
				"Script_Assault", 
				1, 
 				0 );  

			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Spiders", VarGet("SpiderSpawn"), group, "Script_Assault", VarGetNumber("SpiderSpawnLevel"), 0 );  
			}
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////
 
		if( DoEvery("ArrivalCheck", 0.5f, 0 ) ) 
		{ 
			// check if we need more snakes or are out of snakes
			if ( NumEntitiesInTeam( "Snakes" ) < VarGetNumber("SnakeWaveMin"))
			{
				if ( VarGetNumber("SnakeWave") > VarGetNumber("SnakeMaxWaves")) {
					SET_STATE("FlyerMovingOff");
				} else {
					// release any old snakes
					DetachSpawn("Snakes");

					// make sure they're in a fighting mood
					SetAIPriorityList("Snakes", "PL_UseAIConfig");

					// Find snake spawn
					group = FindEncounterGroupByRadius( 
						MyLocation(),
						0, 
						300, // Nearby
						"Script_Snakes", 
						1, 
 						0 );  

					if (stricmp(group, "location:none") != 0) {
						Spawn( 1, "Snakes", VarGet("SnakeSpawn"), group, "Script_Snakes", VarGetNumber("SnakeSpawnLevel"), 0 );  
						VarSetNumber("SnakeWave", VarGetNumber("SnakeWave") + 1);
					}
				}
			}
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("FlyerMovingOff") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			SetAIMoveTarget("Flyer", VarGet("FlyerExitLocation"), MEDIUM_PRIORITY, false, true, COV_SNAKEPIT_MOVETO_VARIANCE);

			Kill("Snakes", false);
			SetAIPriorityList("Spiders", CLEANUP_PL);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if( DoEvery("ArrivalCheck", 3.0f, 0 ) ) 
		{ 
			SET_STATE("EventShutdown");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		Kill("Flyer", false);
		Kill("Spiders", false);
		Kill("Snakes", false);

		gCoVSnakePitRunning--;

		EndScript();

	END_STATES

	ON_SIGNAL("EndScript")
		SET_STATE( "EventShutdown" );
	END_SIGNAL

	ON_SIGNAL("FlyAway")
		SET_STATE( "FlyerMovingOff" );
	END_SIGNAL

}


void CoVSnakePitInit()
{
	SetScriptName( "CoVSnakePit" );
	SetScriptFunc( CoVSnakePit );
	SetScriptType( SCRIPT_LOCATION );		

	SetScriptVar( "ClosePlayerDist",						"500.0",					0 );
	SetScriptVar( "FlyerExitLocation",						"marker:ScriptFlyerExit",	0 );
	SetScriptVar( "FlyerSpawn",								NULL,						0 );
	SetScriptVar( "FlyerSpawnLoc",							NULL,						0 );
	SetScriptVar( "FlyerSpawnLevel",						"15",						0 );
	SetScriptVar( "SnakeSpawn",								NULL,						0 );
	SetScriptVar( "SnakeSpawnLevel",						"2",						0 );
	SetScriptVar( "SpiderSpawn",							NULL,						0 );
	SetScriptVar( "SpiderSpawnLevel",						"10",						0 );

	SetScriptVar( "SnakeMaxWaves",							"4",						0 );
	SetScriptVar( "SnakeWaveMin",							"3",						0 );


	SetScriptSignal( "End Event", "EndScript" );		
	SetScriptSignal( "Fly Away", "FlyAway" );		
}

