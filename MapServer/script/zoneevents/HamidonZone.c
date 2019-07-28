// LOCATION SCRIPT

// This is a location script that runs the Snake Pit event
//

#include "scriptutil.h"

#define CLEANUP_PL							"RunIntoDoor(Timer(60),CombatOverride(Passive)),DestroyMe"
#define KILL_PL								"DestroyMe"

#define HAMIDON_WAVE_COUNT		3

static int gHamidonWaves[HAMIDON_WAVE_COUNT];

void HamidonZoneOnEntityDefeat(ENTITY player, ENTITY victor)
{
	// check for desired type of critter
	if (IsEntityCritter(player))
	{
		STRING name = GetVillainDefClassName(player);

		if (StringEqual(name, VarGet("MonsterClass")))
		{
			int count = VarGetNumber("CurrentKillCount");
			count++;
			VarSetNumber("CurrentKillCount", count);

			// checking to see how many of this class have been killed
			if (count >= VarGetNumber("MaxMonsters"))
			{
				// over the max, start hami
				OnEntityDefeated ( NULL );	// no more counting
				SET_STATE("HamidonOut");				
			} 
			else if (count >= VarGetNumber("MinMonsters"))
			{
				// more than the minimum, check to see if percentage rolled
				if (RandomNumber(0, 100) < VarGetNumber("MonsterPercent"))
				{
					OnEntityDefeated ( NULL );	// no more counting
					SET_STATE("HamidonOut");				
				}
			}
		}
	}
}


void HamidonZone(void)
{  
	int i, count;
	ENCOUNTERGROUP group = "first"; 

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

		// if there are too many running, don't spawn another
		if (ScriptCountRunningInstances("HamidonZone") > 1)
		{
			EndScript();
			return;
		}
 
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		SET_STATE("CountingMonsters");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("CountingMonsters") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
		
			// re-enable zone spawns
			SetDisableMapSpawns(false);

			// Setup kill count callback
			OnEntityDefeated ( HamidonZoneOnEntityDefeat );

			VarSetNumber("CurrentKillCount", 0);

			for (i = 0; i < HAMIDON_WAVE_COUNT; i++)
			{
				gHamidonWaves[i] = false;
			}

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////
	STATE("HamidonOut") ////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// clear kill count callback
			OnEntityDefeated ( NULL );

			// make everyone run away
			count = NumEntitiesInTeam( ALL_CRITTERS );		
			for( i = 1 ; i <= count ; i++ )
			{
				ENTITY e = GetEntityFromTeam(ALL_CRITTERS, i); 
				STRING name = GetVillainDefClassName(e);

				if (StringEqual(name, VarGet("MonsterClass")))
				{
					SetAIPriorityList(e, CLEANUP_PL);
				}
			}

			// shutdown spawns in zone
			SetDisableMapSpawns(true);


			// spawn hamidon and mitos
			group = FindEncounterGroupByRadius( 
				"marker:HamidonLocation",
				0, 
				30, 
				"Hamidon", 
				1, 
				0 );  
			if (stricmp(group, "location:none") != 0) 
			{
				Spawn( 1, "Hamidon", "UseCanSpawns", group, "Hamidon", 54, 0 );
				VarSet("HamidonEnt", ActorFromEncounterPos(group, 40));
				VarSetFraction("LastWave", 0.75f);
			} else {
				// failed
				SET_STATE("CountingMonsters");
			}

			// reset timers
			TimerRemove("MonsterCleanup");
			TimerRemove("HamiCheck");

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if (DoEvery("MonsterCleanup", 1.1f, NULL))
		{			
			// make everyone run away
			count = NumEntitiesInTeam( ALL_CRITTERS );		
			for( i = 1 ; i <= count ; i++ )
			{
				ENTITY e = GetEntityFromTeam(ALL_CRITTERS, i); 
				STRING name = GetVillainDefClassName(e);

				if (StringEqual(name, VarGet("MonsterClass")))
				{
					SetAIPriorityList(e, KILL_PL);
				}
			}
		}

		if (DoEvery("HamiCheck", 0.1f, NULL)) 
		{
			FRACTION health = GetHealth( VarGet( "HamidonEnt" ) );
			FRACTION lastWave = VarGetFraction("LastWave");

			// check for hamidon's death
			if( health <= 0.00f )
			{
				SET_STATE("HamidonKilled");	
			} else {
				// check for hamidon mitos
				if (health < lastWave)
				{
					STRING spawnLayout;
					NUMBER wave = RandomNumber(0, HAMIDON_WAVE_COUNT-1);
					NUMBER waveStart = wave;

					while (gHamidonWaves[wave])
					{
						wave++;
						if (wave >= HAMIDON_WAVE_COUNT)
							wave = 0;
						if (wave == waveStart)
						{
							// out of waves
							wave = HAMIDON_WAVE_COUNT;
							break;
						}
					}
					
					if (wave >= 0 && wave < HAMIDON_WAVE_COUNT)
					{
						spawnLayout = StringAdd("MitoWave", NumberToString(wave+1));

						// spawn new wave
						group = FindEncounterGroupByRadius( 
							"marker:HamidonLocation",
							0, 
							30, 
							spawnLayout, 
							1, 
							0 );  

						if (stricmp(group, "location:none") != 0) 
						{
							Spawn( 1, "Mitos", "UseCanSpawns", group, spawnLayout, 54, 0 );
							JoinAITeam( "Hamidon", spawnLayout ); //Adds Mitos to Hami's AI team

							VarSetFraction("LastWave", lastWave - 0.25f);
							gHamidonWaves[wave] = true;
						}
					}
				}
			}				
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("HamidonKilled") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// clear remaining mitos
			Kill("Mitos", false);

			TimerRemove("MonsterCleanup");
			TimerRemove("HamiCheck");

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if (DoEvery("HamiCheck", 0.25f, NULL))
		{ 
			// clean up hami encounter
			Kill("Hamidon", false);

			SET_STATE("CountingMonsters");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		Kill("Hamidon", false);
		Kill("Mitos", false);

		// re-enable zone spawns
		SetDisableMapSpawns(false);

		EndScript();

	END_STATES

	ON_STOPSIGNAL
		Kill("Hamidon", false);
		Kill("Mitos", false);

		// re-enable zone spawns
		SetDisableMapSpawns(false);

		EndScript();
	END_SIGNAL

	ON_SIGNAL("EndScript")
		SET_STATE( "EventShutdown" );
	END_SIGNAL

	ON_SIGNAL("CallHami")
		SET_STATE( "HamidonOut" );
	END_SIGNAL

}


void HamidonZoneInit()
{
	SetScriptName( "HamidonZone" );
	SetScriptFunc( HamidonZone );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "MinMonsters",							"5",						SP_OPTIONAL );
	SetScriptVar( "MaxMonsters",							"50",						SP_OPTIONAL );
	SetScriptVar( "MonsterPercent",							"4",						SP_OPTIONAL );
	SetScriptVar( "MonsterClass",							"",							0 );

	SetScriptSignal( "End Event", "EndScript" );		
	SetScriptSignal( "Hami", "CallHami" );		
}

