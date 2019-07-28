// MISSION SCRIPT
// Encounter Script
//
#include "scriptutil.h"

#include "entity.h"
#include "character_combat.h"
Entity* EntTeamInternal(TEAM team, int index, int* num);


void CathedralOfPainRularuu(void)
{  
			TEAM team = VarGet("Rularuu");
			int n = NumEntitiesInTeam(team);
			Entity* e = NULL;
			int shield = 0;
			int count;

			for (count = 0; !e && count < n; count++)
			{
				e = EntTeamInternal(team, count, NULL);
			}

			if (e && character_IsNamedTogglePowerActive(e->pchar, VarGet("RularuuInvulnerablePower")))
				shield = 1;

	INITIAL_STATE   
	{
		ON_ENTRY
		{
			ENTITY rularuu		= GetEntityFromTeam( MyEncounter(), 1 );
			ENTITY willforge	= GetEntityFromTeam( MyEncounter(), 2 );
			ENTITY willforge3	= GetEntityFromTeam( MyEncounter(), 3 );
			ENTITY willforge4	= GetEntityFromTeam( MyEncounter(), 4 );

			VarSet( "Rularuu",    rularuu    );
			VarSet( "WillForge",  willforge  );
			VarSet( "WillForge3", willforge3 );
			VarSet( "WillForge4", willforge4 );

			ScriptControlsCompletion( MyEncounter() ); //Not this script, though, the CathedralOfPain.c script
			SetEncounterActive( MyEncounter() );

			SetAITogglePowerRightNow( VarGet( "Rularuu" ), VarGet( "RularuuInvulnerablePower" ), 1 );
		}
		END_ENTRY
		{
			if( EncounterState( MyEncounter() ) == ENCOUNTER_ACTIVE )
				SET_STATE("InvulnerableRularuu");

			//Shouldn't be possible, but just in case...
			if ( !GetHealth( VarGet( "Rularuu" ) ) )
			{
				SET_STATE( "DeadRularuu" );
			}
		}
	}

	STATE("InvulnerableRularuu")
	{
		ON_ENTRY
		{
			ENTITY Rularuu = GetEntityFromTeam( MyEncounter(), 1 );

			SetHealth( VarGet( "WillForge" ), 1.0, 0 );
			SetAIUsePowerRightNow( VarGet( "WillForge" ), 0, VarGet( "WillForgeSummonMonsterPower" ) );
			
			SetHealth( VarGet( "WillForge3" ), 1.0, 0 );
			SetAIUsePowerRightNow( VarGet( "WillForge3" ), 0, VarGet( "WillForgeSummonMonsterPower" ) );
			
			SetHealth( VarGet( "WillForge4" ), 1.0, 0 );
			SetAIUsePowerRightNow( VarGet( "WillForge4" ), 0, VarGet( "WillForgeSummonMonsterPower" ) );

			SetAITogglePowerRightNow( VarGet( "Rularuu" ), VarGet( "RularuuInvulnerablePower" ), 1 );
			VarSetNumber( "OneCastingDone", 0 );
		}
		END_ENTRY
		{
			NUMBER petCount;
			petCount = NumTargetableEntitiesInTeam( MyEncounter() );  //No Willforges etc, because they can't be fought
			petCount -= 1; //Don't count Rularuu himself (itself?)

			if ( petCount > 0 && !shield )
			{
				SetAITogglePowerRightNow( VarGet( "Rularuu" ), VarGet( "RularuuInvulnerablePower" ), 1 );
			}

			if ( VarGetNumber( "OneCastingDone" ) && petCount <= 0 )
			{
				SET_STATE( "VulnerableRularuu" );
			}
			else
			{
				if( !VarGetNumber("OneCastingDone") && petCount > 0 )
					VarSetNumber( "OneCastingDone", 1 );

				if( DoEvery( "WillForgeSummonMoreMinionsTimer", VarGetFraction( "MinutesBetweenWillForgeSummonings" ), 0 ) )
				{
					if( petCount < VarGetNumber( "MaxWillForgeMinionsAtATime" ) )
					{
						SetAIUsePowerRightNow( VarGet( "WillForge" ), 0, VarGet( "WillForgeSummonMonsterPower" ) );
						SetAIUsePowerRightNow( VarGet( "WillForge3" ), 0, VarGet( "WillForgeSummonMonsterPower" ) );
						SetAIUsePowerRightNow( VarGet( "WillForge4" ), 0, VarGet( "WillForgeSummonMonsterPower" ) );
					}
				}
			}

			//Shouldn't be possible, but just in case...
			if ( !GetHealth( VarGet( "Rularuu" ) ) )
			{
				SET_STATE( "DeadRularuu" );
			}
		}
	}

	STATE("VulnerableRularuu")
	{
		ON_ENTRY
		{
			TimerSet( "VulnerableRularuuTimer", VarGetFraction( "MinutesRularuuIsVulnerableAfterWillForgeIsDown" ) );
			SetHealth( VarGet("Willforge"), 0, 0 );
			SetHealth( VarGet("Willforge3"), 0, 0 );
			SetHealth( VarGet("Willforge4"), 0, 0 );
			SetAITogglePowerRightNow( VarGet( "Rularuu" ), VarGet( "RularuuInvulnerablePower" ), 0 );

			BroadcastAttentionMessage(VarGet( "ShieldDownMessage" ), VarGet( "Rularuu" ), VarGetFraction("ShieldDownMessageRadius") );	

		}
		END_ENTRY

			if ( shield )
			{
				SetAITogglePowerRightNow( VarGet( "Rularuu" ), VarGet( "RularuuInvulnerablePower" ), 0 );
			}

			if ( !GetHealth( VarGet( "Rularuu" ) ) )
			{
				SET_STATE( "DeadRularuu" );
			}
			else if ( TimerElapsed( "VulnerableRularuuTimer" ) )
			{
				SET_STATE( "InvulnerableRularuu" );
			}
	}

	STATE("DeadRularuu")
	{
		// spawn the workbench
		ON_ENTRY
		{
			CreateVillainNearEntity("Workbench", VarGet("WorkbenchEntity"), 1, 
				NULL, VarGet( "Rularuu" ), 20.0f);
			CreateVillain("Workbench", VarGet("WorkbenchEntity"), 1, NULL, VarGet( "Rularuu" ));
		}
		END_ENTRY

		SetEncounterComplete( MyEncounter(), 1 );
	}

	END_STATES
}



void CathedralOfPainRularuuInit()
{
	SetScriptName( "CathedralOfPainRularuu" );
	SetScriptFunc( CathedralOfPainRularuu );
	SetScriptType( SCRIPT_ENCOUNTER );		

	//Rularuu   = EP 1
	//WillForge = EP 2
	//WillForge3 = EP 3
	//WillForge4 = EP 4

	SetScriptVar( "WillForgeSummonMonsterPower",					"TO DO",	0 );
	SetScriptVar( "RularuuInvulnerablePower",						"TO DO",	0 );
	SetScriptVar( "MinutesRularuuIsVulnerableAfterWillForgeIsDown", "0.1",		0 );
	SetScriptVar( "MaxWillForgeMinionsAtATime",						"50",		0 );
	SetScriptVar( "MinutesBetweenWillForgeSummonings",				"0.1",		0 );
	SetScriptVar( "ShieldDownMessage",								"",			0 );
	SetScriptVar( "ShieldDownMessageRadius",						"400.0",	SP_OPTIONAL );
	SetScriptVar( "WorkbenchEntity",								"",			0 );


}