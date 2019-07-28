// MISSION SCRIPT

// This is a location script for Croatoa for the Battle between the Red Caps and Cabal
//

#include "scriptutil.h"

static void CroatoaBattleSpawnAttackSquads( const char *spawndef, const char *spawnlayout)
{

	ENCOUNTERGROUP group; 

	group = FindEncounterGroupByRadius( 
		MyLocation(),
		0, 
		9999, 
		spawnlayout,
		1, 
		0 );  

	if( !StringEqual( LOCATION_NONE, group ) )
	{
		Spawn( 1, "CroatoaBattle", spawndef, group, spawnlayout, 0, 0);
		AddAIBehavior(group, "Combat");
		SetAIMoveTarget( group, MyLocation(), LOW_PRIORITY, 0, 1, 20.0f );
		DetachSpawn( group );
	}
}


void CroatoaBattle(void)
{   
	INITIAL_STATE      
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
			// send alert
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
				"zonename", StringLocalizeMenuMessages(GetMapName())));
		}
		END_ENTRY

		SET_STATE("Wave");

		VarSetNumber("WaveCount", 4);

	////////////////////////////////////////////////////////////////////////////////////////
	STATE("Wave") ////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  //////////////////////////////////////////////////////////////////////////
		{
			VarSetNumber("WaveCount", VarGetNumber("WaveCount") - 1);

			// spawn everything
			CroatoaBattleSpawnAttackSquads(VarGet("TuathaSpawndef"), VarGet("TuathaSpawnLayout"));
			CroatoaBattleSpawnAttackSquads(VarGet("CabalSpawndef"), VarGet("CabalSpawnLayout"));

			// check for giants
			if (VarGetNumber("WaveCount") == 0)
//			if (RandomNumber(0, 100) < VarGetNumber("ChanceOfGiants"))
			{
				CroatoaBattleSpawnAttackSquads(VarGet("RedCapGiantSpawndef"), VarGet("RedCapSpawnLayout"));
				CroatoaBattleSpawnAttackSquads(VarGet("FirbolgGiantSpawndef"), VarGet("FirbolgSpawnLayout"));
			} else {
				CroatoaBattleSpawnAttackSquads(VarGet("RedCapSpawndef"), VarGet("RedCapSpawnLayout"));
				CroatoaBattleSpawnAttackSquads(VarGet("FirbolgSpawndef"), VarGet("FirbolgSpawnLayout"));
			}

		}
		END_ENTRY

		VarSetNumber("CroatoaBattleCount", NumEntitiesInTeam("CroatoaBattle"));

		// have we completely run out of guys?
		if (NumEntitiesInTeam("CroatoaBattle") <= 0) {
			if (VarGetNumber("WaveCount") > 0)
			{
				SET_STATE("Lull");
			} else {
				SET_STATE("Cleanup");
			}
		}

		// has long enough passed to bring out next wave?
		if (DoEvery("LullTimer", 5, NULL) && VarGetNumber("WaveCount") > 0)
		{
			SET_STATE("Lull");
		}

	////////////////////////////////////////////////////////////////////////////////////////
	STATE("Lull") ////////////////////////////////////////////////////////

		if (DoEvery("LullTimer", 3, NULL))
			SET_STATE("Wave");

	////////////////////////////////////////////////////////////////////////////////////////
	STATE("Cleanup") ////////////////////////////////////////////////////////
    
		// send alert
		SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("ClearMessage"), 1,
															"zonename", StringLocalizeMenuMessages(GetMapName())));

		Kill("CroatoaBattle", false);

		EndScript();

	 
	END_STATES

	ON_SIGNAL("EndScript")
		SET_STATE( "Cleanup" );
	END_SIGNAL


}