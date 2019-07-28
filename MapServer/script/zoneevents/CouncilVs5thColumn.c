// MISSION SCRIPT

// This is an encounter script that runs the Ghostship through Independence Port
//

#include "scriptutil.h"

static void showState( void )
{
	DebugString(StringAdd("In state: ", VarGet(STATE_VAR)));
}

void SpawnFarCouncilAttackSquad( STRING target )
{
	ENCOUNTERGROUP group;
	char spawndef[256];
	char * numSpot;
	char buf[256];

	//TO DO make a generic randomify spawn function
	strcpy( spawndef, VarGet("AroundCouncilSpawndef") ); 
	numSpot = strrchr( spawndef, 'Q' ); 
	itoa( RandomNumber( 0, 2 ), buf , 10 );
	*numSpot = buf[0];

	group = FindEncounterGroupByRadius( 
		target,
		VarGetNumber( "ATTACK_DOUGHNUT_INNER"), 
		VarGetNumber( "ATTACK_DOUGHNUT_OUTER"), 
		VarGet("FarCouncilSpawnLayout"),//"AROUND" 
		1, 
		0 );  

	if( !StringEqual( LOCATION_NONE, group ) )
	{
		Spawn( 1, "CouncilAttackers", spawndef, group, VarGet("FarCouncilSpawnLayout"), 0, 0);  
		AddAIBehavior(group, "Combat");
		SetAIMoveTarget( group, target, LOW_PRIORITY, 0, 1, 20.0f );
		DetachSpawn( "CouncilAttackers" );
	}
}

void Spawn5thDefenseSquad( STRING target )
{
	ENCOUNTERGROUP group;
	char spawndef[256];
	char * numSpot;
	char buf[256];

	//TO DO make a generic randomify spawn function
	strcpy( spawndef, VarGet("Around5thSpawndef") );
	numSpot = strrchr( spawndef, 'Q' ); 
	itoa( RandomNumber( 0, 2 ), buf , 10 );
	*numSpot = buf[0];
	
	group = FindEncounterGroupByRadius( 
		target,
		0, 
		VarGetNumber( "DEFENSE_DOUGHNUT_OUTER"), 
		VarGet("Near5thSpawnLayout"),//"AROUND" 
		1, 
		0 );  

	if( !StringEqual( LOCATION_NONE, group ) )
	{
		Spawn( 1, "5thDefenders", spawndef, group, VarGet("Near5thSpawnLayout"), 0, 0);  
		AddAIBehavior(group, "Combat");
		SetAIMoveTarget( group, target, LOW_PRIORITY, 0, 1, 20.0f );
		SetAITeamVillain(group);
		DetachSpawn( "5thDefenders" );
	}
}

static char attackTargets[][256] = 
{
	"marker:CouncilVs5thTarget_01",
	"marker:CouncilVs5thTarget_02",
	"marker:CouncilVs5thTarget_03",
};

static void HealWalkers(void)
{
	int i; 
	int numEnts = NumEntitiesInTeam("CouncilAttackers");

	for( i = 1 ; i <= numEnts ; i++ )
	{
		SetHealth( GetEntityFromTeam("CouncilAttackers", i), 1.0, 1 );
	}
}

void EventThink(void)
{
	LOCATION attackTarget = attackTargets[ rand()%(sizeof(attackTargets) / sizeof((attackTargets)[0])) ];

	//Keep a minimum number of 5th Column and Council
	if( NumEntitiesInTeam( "5thDefenders" ) < VarGetNumber( "Min5thDefenders" ) ) 
	{
		Spawn5thDefenseSquad( attackTarget );
	}

	if( NumEntitiesInTeam( "CouncilAttackers" ) < VarGetNumber( "MinCouncilAttackers" ) )
	{
		SpawnFarCouncilAttackSquad( attackTarget );
	}
	
}

void CouncilVs5thColumn(void)
{   
	INITIAL_STATE      
//////////////////////////////////////////////////////////////////////////
	ON_ENTRY  //////////////////////////////////////////////////////////////////////////
	{
		//1. Generate all the spawndef names:
		STRING rootname;
		STRING aroundCouncilSpawndef;
		STRING around5thSpawndef;
		STRING rumbleSpawndef;

		rootname = "SpawnDefs\\ZoneEvents\\Expansion3\\";
		rootname = StringAdd(rootname, GetShortMapName());
		((char*)rootname)[strlen( rootname )-1] = 'X';

		aroundCouncilSpawndef = StringAdd(rootname, "_CouncilAround_D1_VQ.spawndef");
		VarSet( "AroundCouncilSpawndef", aroundCouncilSpawndef );

		around5thSpawndef = StringAdd(rootname, "_5thAround_D1_VQ.spawndef");
		VarSet( "Around5thSpawndef", around5thSpawndef );

		//TO DO : add multiple versions?
		rumbleSpawndef = StringAdd(rootname, "_Rumble_D1_V0.spawndef");

		//Get all the Rumbles in the level and replace them with 5th v Council
		OverrideEncounterGroupsInMap( "Rumble", rumbleSpawndef, 1.0 );

		//Get all the spawns in this radius, and reserve them.
		ReserveEncountersInRadius( "marker:CouncilVs5th", VarGetFraction("EncounterClearRadius" ) );

		//Any Encounter groups set up as canspawn in the map, set to ManualSpawn, and named CouncilVs5thOnly
		//Are allowed to spawn as normal.  This takes precedence over the reserved encounters above since it's 
		//called after it
		DisableManualSpawn( "CouncilVs5thOnly" );

	}
	SET_STATE("Rumbling");

	END_ENTRY

////////////////////////////////////////////////////////////////////////////////////////
	STATE("Rumbling") ////////////////////////////////////////////////////////
       
	DoEvery( "EventThink", VarGetFraction( "RefreshFrequency"), EventThink ); 

	DoEvery( "HealWalkers", 0.3f, HealWalkers);

	//Ratchet down 5th if Council is winning
	if( DoEvery( "CouncilWinningTimer", 1.0f, 0) )
	{
		if( VarGetNumber( "CouncilWinning" ) == 1 )
		{
			NUMBER min5th;
			min5th = VarGetNumber( "Min5thDefenders" );
			min5th--;
			VarSetNumber( "Min5thDefenders", min5th );
		}
	}

	 
	END_STATES

	ON_SIGNAL("CouncilWinning")
		VarSetNumber("CouncilWinning", 1);
	END_SIGNAL
}