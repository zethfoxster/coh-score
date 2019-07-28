// MISSION SCRIPT

// This script run the Clockwork attack in Kings Row
//


#include "scriptutil.h"

// Parameters:
//		NONE_YET

static char attackTargets[][256] = 
{
	"marker:ClockworkAttack_01",
	"marker:ClockworkAttack_02",
};
#define MIN_CLOCKWORK_ATTACKERS 70
#define ATTACK_DOUGHNUT_INNER 50
#define ATTACK_DOUGHNUT_OUTER 200
#define INITIAL_CLOCKWORK_SQUADS 8
#define BABBAGES_SPAWNED 4


static void showState(void)
{
	DebugString(StringAdd("In state: ", VarGet(STATE_VAR)));
}

LOCATION getAttackTarget( int reset )
{
	static LOCATION attackTarget;

	if( !attackTarget || reset )
		attackTarget = attackTargets[ rand()%(sizeof(attackTargets) / sizeof((attackTargets)[0])) ];

	return attackTarget;
}

void SpawnFarClockworkAttackSquad(void)
{
	if ( NumEntitiesInTeam("ClockworkAttackers") < MIN_CLOCKWORK_ATTACKERS )
	{
		ENCOUNTERGROUP group;
		STRING spawndef;

		if (STATE_MINS > 3.0)
			spawndef = "SpawnDefs/ZoneEvents/ClockworkAttack/ClockworkFirstWave_D1_V0.spawndef";
		else if (STATE_MINS > 3.0)
			spawndef = "SpawnDefs/ZoneEvents/ClockworkAttack/ClockworkFirstWave_D1_V0.spawndef";
		else if (STATE_MINS > 1.0)
			spawndef = "SpawnDefs/ZoneEvents/ClockworkAttack/ClockworkFirstWave_D1_V0.spawndef";

		group = FindEncounterGroupByRadius( getAttackTarget(0), ATTACK_DOUGHNUT_INNER, ATTACK_DOUGHNUT_OUTER, "Around", 1, 0 );
		Spawn( 1, "ClockworkAttackers", spawndef, group, 1, 0 ); 
		DetachSpawn("ClockworkAttackers");
		SetAIAttackTarget("ClockworkAttackers", "TargetBuilding", 0);
	}
}

void ClockworkAttack(void)
{
	int i;

	INITIAL_STATE

	ON_ENTRY  //////////////////////////////////////////////////////////////////////////
	//Randomly pick a building to be attacked
	getAttackTarget(1);

	//TO DO Spawn the building encounter

	//Create some clockwork to come attack the building
	for (i = 1; i <= INITIAL_CLOCKWORK_SQUADS; i++)
		SpawnFarClockworkAttackSquad();
	END_ENTRY

	STATE("ClockworkAttacking") ////////////////////////////////////////////////////////

	//Check to see if the clockwork have won.  If so, spawn baddies
	if( 0 == NumEntitiesInTeam("TargetBuilding") )
		SET_STATE("ClockworkSucceed")

	//Every little while spawn some more
	DoEvery("SpawnFarClockworkAttackSquad", 1.0, SpawnFarClockworkAttackSquad );
	
	STATE("ClockworkSucceed") ///////////////////////////////////////////////////////////
	//Destroy the building
	//TO DO: how, exactly? destroyNearbyBuilding

	//Create some clockwork to come attack the building
	for (i = 1 ; i <= BABBAGES_SPAWNED ; i++)
	{
		///TO DO SpawnClockworkBabbage();
	}
	SET_STATE("ClockworkRampage")

	STATE("ClockworkRampage") ///////////////////////////////////////////////////////////

	END_STATES
	

}
