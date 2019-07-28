// MISSION SCRIPT

// This is an encounter script that runs the Clockwork attempting to build a Paladin
//


#include "scriptutil.h"

//Very temp debugging 
#include "scriptengine.h"
#include "assert.h"
//End very temp debugging 

static void showState( void )
{
	DebugString(StringAdd("In state: ", VarGet(STATE_VAR)));
}


//Pick a random selection of minions to say something
static void ClockworkMinionSays( NUMBER count, STRING says )
{
	int i; 
	int numEnts = NumEntitiesInTeam("ClockworkAttackers");

	for( i = 1 ; i <= numEnts ; i++ )
	{
		if( RandomFraction(0.0, 1.0) < (float)count/(float)numEnts )
		{
			Say( GetEntityFromTeam("ClockworkAttackers", i), says, 0  );
		}
	}
}

static void HealWalkers(void)
{
	int i; 
	int numEnts = NumEntitiesInTeam("ClockworkAttackers");

	for( i = 1 ; i <= numEnts ; i++ )
	{
		SetHealth( GetEntityFromTeam("ClockworkAttackers", i), 1.0, 1 );
	}
}

//Spawn
static void SpawnFarClockworkAttackSquad(void)
{
	if ( NumEntitiesInTeam("ClockworkAttackers") < VarGetNumber( "MinClockwork" ) ) 
	{
		ENCOUNTERGROUP group; 
		group = FindEncounterGroupByRadius( 
			EncounterPos( MyEncounter(), 1 ),
			VarGetNumber( "ATTACK_DOUGHNUT_INNER"), 
			VarGetNumber( "ATTACK_DOUGHNUT_OUTER"), 
			VarGet("FarSpawnLayout"),//"AROUND",  "AROUNDVANDALISM", 
			1, 
			0 );  

		if( StringEqual( group, LOCATION_NONE ) )
		{  //debug
			VarSetNumber("TimesIFailedToGetFarSpawn", VarGetNumber( "TimesIFailedToGetFarSpawn" ) + 1 );
		}
		else
		{
			Spawn( 1, "ClockworkAttackers", VarGet( "StartingFarSpawn" ), group, VarGet("FarSpawnLayout"), VarGetNumber("StartingFarSpawnLevel"), 0 );   //TO DO make a var 

			AddAIBehavior(group, "Combat");
			SetAIMoveTarget( group, EncounterPos(MyEncounter(), 1 ), MEDIUM_PRIORITY, 0, 1, 10.0f );
			DetachSpawn( "ClockworkAttackers" );
		}
	}
}

//Create some
static void SpawnNearbyHeapAttackSquad(void)
{
	if ( NumEntitiesInTeam("ClockworkAttackers") < VarGetNumber( "MinClockwork" ) )  
	{
		NUMBER lvl = VarGetNumber( "CurrentHeapSpawnLevel");
		STRING heapSpawn = VarGet( "CurrentHeapSpawn" );
 
		while( NumEntitiesInTeam("ClockworkAttackers") < VarGetNumber( "MinClockwork"  ) )
		{
			ENCOUNTERGROUP heapLoc   = FindEncounterGroupByRadius( EncounterPos(MyEncounter(), 1 ), 0, 100, "Defenders", 1, 0 );
			ENCOUNTERGROUP heapGroup = Spawn( 1, "ClockworkAttackers", heapSpawn, heapLoc, "Defenders", lvl , 0 );
			
			//if you didn't spawn any new guys, then stop trying
			if( StringEqual( heapGroup, LOCATION_NONE ) )
				break;
		}
		DetachSpawn("ClockworkAttackers");
		
		ClockworkMinionSays( 1, VarGet( "MinionNeedsMore1" ));
		ClockworkMinionSays( 1, VarGet( "MinionNeedsMore2" ));
	}
}

static void SpawnUnfinishedPaladin(void)
{
	ENTITY ent;
	LOCATION loc;
	loc	= EncounterPos(MyEncounter(), 1 );                                       
	ent = CreateVillain( "UnfinishedPaladinTeam", VarGet( "UnfinishedPaladinType" ), VarGetNumber( "UnfinishedPaladinLevel" ), VarGet( "UnfinishedPaladinDisplayName" ), loc );
	AddAIBehavior(ent, "DoNothing");
	SetAIAttackTarget(ent, "PowerTower", HIGH_PRIORITY);
	AddAIBehavior(ent, "DoNotFaceTarget,GriefHPRatio(0),FeelingFear(0)");
	SetHealth( ent, 0.1f, 0 ); 
	VarSet( "UnfinishedPaladin", ent );
}

ClockworkSucceed()
{	
	ENTITY paladin;
	LOCATION loc;

	//Minions cheer, as Unfinished Paladin is replaced by the Finished Paladin.
	ClockworkMinionSays( 3, VarGet( "MinionSuccess" ) );
	Kill(VarGet( "UnfinishedPaladin" ), 0);

	//Create Paladin, he fronts, collects his minions and wanders off
	loc	= EncounterPos(MyEncounter(), 1 );
	paladin = CreateVillain( "Paladin", VarGet( "PaladinType" ), VarGetNumber( "PaladinLevel" ), VarGet( "PaladinDisplayName" ), loc );
	VarSet( "Paladin", paladin );

	Say( paladin, VarGet( "PaladinFront" ), 2  );
	SetFollowers( paladin, "ClockworkAttackers" );

	AddAIBehavior(paladin, "Combat");
	SetAIMoveTarget( paladin, "marker:ShadowShardInvasion_01", MEDIUM_PRIORITY, 0, 1, 10.0f );
	//TO DO: what does Paladin do when it arrives?  Will it revert to wander off?

	//Get rid of the Heaps
	Kill( "Heaps", 0 );  //Get rid of the heaps

	// send alert
	SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("PaladinMessage"), 1,
															"zonename", StringLocalizeMenuMessages(GetMapName())));

	//Wait for paladin to die to reset script
	SET_STATE( "ClockworkSucceeded"); 
}

void ClockworkPaladin(void)
{
	int i;
	examineScript();    
  
	//Debug only vars
	VarSetNumber( "SizeOfClockworkTeam", NumEntitiesInTeam("ClockworkAttackers") );  
	VarSetNumber("TimesIFailedToGetFarSpawn", 0 );
	//End debug only Vars

	INITIAL_STATE    
//////////////////////////////////////////////////////////////////////////
	ON_ENTRY  //////////////////////////////////////////////////////////////////////////

	VarSet( "CurrentHeapSpawnLevel", VarGet( "HeapSpawnLevelStage1" ) );  
	VarSet( "CurrentHeapSpawn", VarGet( "HeapSpawnStage1" ) );

	//Create power tower
	CreateVillain( "PowerTower", VarGet( "PowerTower" ), 0, 0, EncounterPos(MyEncounter(), 2 ) );
	
	//Create UnfinishedPaladin (and have the UnfinishedPaladin attack the PowerTower)
	SpawnUnfinishedPaladin();

	//For every spawn in 100 feet with the name "Heap", throw down a heap ent that does nothing 
	for(;;) 
	{
		ENCOUNTERGROUP heapLoc;
		ENTITY heap;
		heapLoc = FindEncounterGroupByRadius( EncounterPos(MyEncounter(), 1 ), 0, 100, "Heap", 1, 0 );
		if( StringEqual( LOCATION_NONE, heapLoc ) ) //If you fail to find a spawn, quit
			break;

		heap = Spawn( 1, "Heaps", VarGet( "HeapSpawn" ), heapLoc, "Heap", 1 , 0 );
		if( StringEqual( LOCATION_NONE, heap ) )  //If you fail to spawn something, quit
			break;
	} 
	DetachSpawn( "Heaps" ); //Free spawn location for use by the Script to create dudes

	//Create some clockwork to march across the city
	{
		int StartingFarSpawns = VarGetNumber( "StartingFarSpawnCount" );
		for (i = 1; i <= StartingFarSpawns; i++) 
			SpawnFarClockworkAttackSquad();
	}

	//Create guards up to the maximum clockwork
	SpawnNearbyHeapAttackSquad(); 

	// send alert
	SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
														"zonename", StringLocalizeMenuMessages(GetMapName())));

	SET_STATE("ClockworkBuildingPaladin");
	END_ENTRY

////////////////////////////////////////////////////////////////////////////////////////
	STATE("ClockworkBuildingPaladin") ////////////////////////////////////////////////////////

	DoEvery( "HealWalkers", 1.0f, HealWalkers);

	if( STATE_MINS > VarGetNumber( "TimeToHeroSuccess" ) ) //If the heores have won, yay!
	{
		ClockworkMinionSays( 3, VarGet( "MinionFailure" ) );
		SET_STATE("HeroesAlmostSucceed");
	}
	else //If the heores haven't won, manage the battle, and keep building 
	{
		//Set the Level and Spawn of the Defenders based on elapsed time
		if( STATE_MINS > VarGetNumber( "HeapSpawnTimeStage4" ) )
		{
			if( !VarEqual(VarGet( "HeapSpawnLevelStage4" ), VarGet("CurrentHeapSpawnLevel") ) ) 
			{
				VarSet( "CurrentHeapSpawnLevel", VarGet( "HeapSpawnLevelStage4" ) );
				VarSet( "CurrentHeapSpawn", VarGet( "HeapSpawnStage4" ) );
				ClockworkMinionSays( 2, VarGet( "MinionNextPhase" ));
			}
		}
		else if( STATE_MINS > VarGetNumber( "HeapSpawnTimeStage3" ) )
		{
			if( !VarEqual(VarGet( "HeapSpawnLevelStage3" ), VarGet("CurrentHeapSpawnLevel" ) ) )
			{
				VarSet( "CurrentHeapSpawnLevel", VarGet( "HeapSpawnLevelStage3" ) );
				VarSet( "CurrentHeapSpawn", VarGet( "HeapSpawnStage3" ) );
				ClockworkMinionSays( 2, VarGet( "MinionNextPhase" ));
			}
		}
		else if( STATE_MINS > VarGetNumber( "HeapSpawnTimeStage2" ) )
		{
			if( !VarEqual(VarGet( "HeapSpawnLevelStage2" ), VarGet("CurrentHeapSpawnLevel" ) ) )
			{
				VarSet( "CurrentHeapSpawnLevel", VarGet( "HeapSpawnLevelStage2" ) );
				VarSet( "CurrentHeapSpawn", VarGet( "HeapSpawnStage2" ) );
				ClockworkMinionSays( 2, VarGet( "MinionNextPhase" ));
			}
		} 

		//Every little while spawn some more baddies
		DoEvery("SpawnFarClockworkAttackSquad", 1.5, SpawnFarClockworkAttackSquad ); //TO DO would we like to add clockwork from a far?
		DoEvery("SpawnNearbyHeapAttackSquad", 1.0, SpawnNearbyHeapAttackSquad ); //Temp

		//Have the minions Yap occasionally
		if( DoEvery("MinionSays", 0.3f, 0 ) )  
		{
			if( RandomFraction( 0.0f, 1.0f ) > 0.5f )
				ClockworkMinionSays( 1, VarGet( "MinionFront1" ) );
			else if( RandomFraction( 0.0f, 1.0f ) > 0.5f )
				ClockworkMinionSays( 1, VarGet( "MinionFront2" ) );
			else if( RandomFraction( 0.0f, 1.0f ) > 0.5f )
				ClockworkMinionSays( 1, VarGet( "MinionFront3" ) );
		}

		//Check to see if Clockwork have built the Paladin
		if( EntityExists( VarGet( "UnfinishedPaladin" ) ) )
		{
			//If unfinished is fully healthy, Paladin is born! Create him and go to next phase
			if( GetHealth( VarGet( "UnfinishedPaladin" ) ) > 0.97 )
			{	
				ClockworkSucceed();
			}
		}	
		//If Unfinished Paladin is dead, start building him again
		else 
		{
			ClockworkMinionSays( 1, VarGet( "MinionRebuildPaladin") );
			SpawnUnfinishedPaladin(); 
		}
	}

////////////////////////////////////////////////////////////////////////////////////////
	STATE("HeroesAlmostSucceed") ///////////////////////////////////////////////////////////

	//Check to see if Clockwork have built the Paladin
	if( EntityExists( VarGet( "UnfinishedPaladin" ) ) )
	{
		//If unfinished is fully healthy, Paladin is born! Create him and go to next phase
		if( GetHealth( VarGet( "UnfinishedPaladin" ) ) > 0.97 )
		{	
			ClockworkSucceed();
		}
	}	
	//If Unfinished Paladin is dead, event is over
	else 
	{
		//FanFare!  DO something that allows them to know they have succeeded
		//TO DO : Hand out rewards
		GiveRewardToAllInRadius( VarGet("Reward"), EncounterPos( MyEncounter(), 1 ) , VarGetFraction("RewardRadius") );
		Kill( "Heaps", 0 );  //Get rid of the heaps
		CloseMyEncounterAndEndScript();

		// send alert
		SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("ClearMessage"), 1,
																"zonename", StringLocalizeMenuMessages(GetMapName())));

	}

////////////////////////////////////////////////////////////////////////////////////////
	STATE("ClockworkSucceeded") ///////////////////////////////////////////////////////////


	//When the Paladin is dead, we're done. Yay.
	if( 0 == EntityExists( VarGet( "Paladin" ) ) )
	{
		CloseMyEncounterAndEndScript();
	}

	END_STATES

	ON_SIGNAL("Win")
		SET_STATE( "HeroesAlmostSucceed" );
	END_SIGNAL

	ON_SIGNAL("Lose")
		SET_STATE( "ClockworkSucceeded" );
	END_SIGNAL
}