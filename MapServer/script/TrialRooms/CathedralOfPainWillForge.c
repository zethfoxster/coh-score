// MISSION SCRIPT
// Encounter Script
//
#include "scriptutil.h"

void CathedralOfPainWillForge(void)
{  
	INITIAL_STATE 
	{
		ON_ENTRY
		{
			ENTITY obelisk = GetEntityFromTeam( MyEncounter(), 1 );
			ENTITY willforge = GetEntityFromTeam( MyEncounter(), 2 );

			VarSet( "Obelisk", obelisk );
			VarSet( "WillForge", willforge );

			ScriptControlsCompletion( MyEncounter() ); //Not this script, though, the CathedralOfPain.c script
			
			SetAITogglePowerRightNow( VarGet( "Obelisk" ), VarGet( "ObeliskInvulnerablePower" ), 1 );

			SetEncounterActive( MyEncounter() );
		}
		END_ENTRY
		{	
			if( EncounterState( MyEncounter() ) == ENCOUNTER_ACTIVE )
				SET_STATE("InvulnerableObelisk");
		}
	}

	STATE("InvulnerableObelisk")
	{
		ON_ENTRY
		{
			ENTITY obelisk = GetEntityFromTeam( MyEncounter(), 1 );

			SetHealth( VarGet( "WillForge" ), 1.0, 0 );
			SetAIUsePowerRightNow( VarGet( "WillForge" ), 0, VarGet( "WillForgeSummonMonsterPower" ) );

			SetAITogglePowerRightNow( VarGet( "Obelisk" ), VarGet( "ObeliskInvulnerablePower" ), 1 );
			VarSetNumber( "OneCastingDone", 0 );
			//Reset timer on the next time
			TimerSet("WillForgeSummonMoreMinionsTimer", VarGetFraction( "MinutesBetweenWillForgeSummonings" ) );
		}
		END_ENTRY
		{
			NUMBER petCount;
			petCount = NumEntitiesInTeam( MyEncounter() );  //Max is for all monsters total
			petCount -= 2; //Obelisk and WillForge don't count

			if ( VarGetNumber( "OneCastingDone" ) && petCount <= 0 )
			{
				SET_STATE( "VulnerableObelisk" );
			}
			else
			{
				if( !VarGetNumber("OneCastingDone") && petCount > 0 )
					VarSetNumber( "OneCastingDone", 1 );

				//If you haven't gone off for some reason or its time to go off again
				if( !VarGetNumber( "OneCastingDone" ) || DoEvery( "WillForgeSummonMoreMinionsTimer", VarGetFraction( "MinutesBetweenWillForgeSummonings" ), 0 ) )
				{
					if( petCount < VarGetNumber( "MaxWillForgeMinionsAtATime" ) )
					{
						SetAIUsePowerRightNow( VarGet( "WillForge" ), 0, VarGet( "WillForgeSummonMonsterPower" ) );
					}
				}
			}
			//Shouldn't be possible, but just in case...
			if ( !GetHealth( VarGet( "Obelisk" ) ) )
			{
				SET_STATE( "DeadObelisk" );
			}
		}
	}

	STATE("VulnerableObelisk")
	{
		ON_ENTRY 
		{
			ENCOUNTERGROUP natterlings;
			TimerSet( "VulnerableObeliskTimer", VarGetFraction( "MinutesObeliskIsVulnerableAfterWillForgeIsDown" ) );
			SetHealth( VarGet("Willforge"), 0, 0 );
			SetAITogglePowerRightNow( VarGet( "Obelisk" ), VarGet( "ObeliskInvulnerablePower" ), 0 );
			natterlings = FindEncounter( VarGet("Obelisk" ), 0, 200, 0, 0, 0, "ObeliskNatterlings", 0 );
			
			Spawn( 1, "Natterlings", VarGet( "ObeliskNatterlingSpawndef" ), natterlings, NULL, 50, 0 );
			SetEncounterActive( natterlings );
			JoinAITeam( VarGet("Obelisk" ), "Natterlings" );
			DetachSpawn( "Natterlings" );
		
			BroadcastAttentionMessage(VarGet( "ShieldDownMessage" ), VarGet( "Obelisk" ), VarGetFraction("ShieldDownMessageRadius") );	
		}
		END_ENTRY

		if ( !GetHealth( VarGet( "Obelisk" ) ) )
		{
			SET_STATE( "DeadObelisk" );
		}
		else if ( TimerElapsed( "VulnerableObeliskTimer" ) )
		{
			SET_STATE( "InvulnerableObelisk" );
		}
	}

	STATE("DeadObelisk")
	{
		ON_ENTRY
		{
			TimerSet( "DeadObeliskTimer", VarGetFraction( "MinutesTillDeadObeliskResurrects" ) );
		}
		END_ENTRY

		if( EncounterState( MyEncounter() ) != ENCOUNTER_COMPLETED && TimerElapsed( "DeadObeliskTimer" ) ) 
		{
			SetHealth( VarGet("Obelisk"), 1.0, 0 );
			SET_STATE( "InvulnerableObelisk" );
		}
	}

	END_STATES
} 



void CathedralOfPainWillForgeInit()
{
	SetScriptName( "CathedralOfPainWillForge" );
	SetScriptFunc( CathedralOfPainWillForge );
	SetScriptType( SCRIPT_ENCOUNTER );		

	//Obelisk   = EP 1
	//WillForge = EP 2 

	SetScriptVar( "WillForgeSummonMonsterPower",					"TO DO",	0 );
	SetScriptVar( "ObeliskInvulnerablePower",						"TO DO",	0 );
	SetScriptVar( "MinutesTillDeadObeliskResurrects",				"1.0",		0 );
	SetScriptVar( "MinutesObeliskIsVulnerableAfterWillForgeIsDown", "0.1",		0 );
	SetScriptVar( "MaxWillForgeMinionsAtATime",						"50",		0 );
	SetScriptVar( "MinutesBetweenWillForgeSummonings",				"0.1",		0 );
	SetScriptVar( "ObeliskNatterlingSpawndef",						"V_Contacts/BaseMissions/Spawndefs/ObeliskNatterlings_D10_V0.spawndef",	0 );
	SetScriptVar( "ShieldDownMessage",								"",			0 );
	SetScriptVar( "ShieldDownMessageRadius",						"400.0",	SP_OPTIONAL );

}