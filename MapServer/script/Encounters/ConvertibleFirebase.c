
#include "scriptutil.h"

int ConvertibleFirebaseControlComputerClick( ENTITY player, ENTITY target )
{
	if ( VarGetNumber("AllowBaseToBeTaken") == 0 ) {
		if( NumEntitiesInTeam( MyEncounter() ) <= VarGetNumber( "NumberOfEntsNotNeedingDestruction" ) ) 
		{
			Say( target, VarGet( "ControlComputerDefeatedMessage" ), 2 );
		} else {
			Say( target, VarGet( "ControlComputerNotDefeatedMessage" ), 2 );
		}
	} else {
		//If You are in control of this firebase, say so
		if( GetAIGang( player ) == GetAIGang( VarGet("FirebaseBrain" ) ) )
		{
			Say( target, VarGet( "ControlComputerAlreadyOwnedMessage" ), 2 );
		}
		//Else if the only guys left in the encounter is the Computer and the Brain, take it over
		else if( NumEntitiesInTeam( MyEncounter() ) <= VarGetNumber( "NumberOfEntsNotNeedingDestruction" ) ) 
		{
			RegenerateEncounter( MyEncounter() );
			SetAIGang( MyEncounter(), GetAIGang( player ) );
			SetAIAlly( MyEncounter(), GetAIAlly( player ) );
			Say( target, VarGet( "ControlComputerSuccessMessage" ), 2 );
			SendPlayerAttentionMessage( player, VarGet("SuccessBannerMessage") );
			if (IsEntityOnTeam( player ) ) {
				VarSetNumber("OnTeam", 1);
				VarSet("Owner", "");
			} else {
				VarSetNumber("OnTeam", 0);
				VarSet("Owner", player);
			}
		}
		//else Fail in you efforts
		else
		{
			Say( target, VarGet( "ControlComputerFailureMessage" ), 2 );

			//TEMP DEBUG
			if (0) {
				const char * buf = 0;
				int i, n = NumEntitiesInTeam( MyEncounter() );

				buf = StringAdd( buf, "DEBUGMSG: Turrets+Bases remaining: ");
				buf = StringAdd( buf, NumberToString((NumEntitiesInTeam( MyEncounter() )-2 ) ) );
				Say( target, buf, 2 ); 
				buf = 0;

				for (i = 0; i < NumEntitiesInTeam(MyEncounter()); i++)
				{	
					ENTITY ent = GetEntityFromTeam(MyEncounter(), i + 1 );
					buf = StringAdd( buf, " ENT: ");
					buf = StringAdd( buf, GetDisplayName( ent ));
					buf = StringAdd( buf, " HP: ");
					buf = StringAdd( buf, FractionToString( GetHealth( ent ) ) );
				}
				Say( target, buf, 2 );
			}
			//END TEMP DEBUG
		}
	}
	return 0;
}

void ConvertibleFirebase()
{
	INITIAL_STATE
	{
		ENTITY controlComputer;
		ENTITY firebaseBrain;
		
		controlComputer	= ActorFromEncounterPos( MyEncounter(), VarGetNumber( "PositionOfControlComputer" ) ); 
		firebaseBrain	= ActorFromEncounterPos( MyEncounter(), VarGetNumber( "PositionOfFirebaseBrain"   ) );

		if( StringEqual( TEAM_NONE, controlComputer ) || StringEqual( TEAM_NONE, firebaseBrain ) )
		{
			//ScriptError( "This firebase doesn't have the right stuff" );
			EndScript();
		}
		else
		{
			OnPlayerInteractWithNPCOrGlowie( ConvertibleFirebaseControlComputerClick, controlComputer);
			VarSet( "FirebaseBrain", firebaseBrain );
			SetEncounterActive( MyEncounter() );
			ScriptControlsCompletion( MyEncounter() );
			SET_STATE("Running");
		}
	}

	////////////////////////////////////////////////////////////////////
	STATE("Running")
	{
		//Occasionally respawn the defenses of the firebase 
		if( DoEvery( "TIMER", VarGetFraction("FrequencyOfDefenseRespawn"), 0 ) )//TO DO variablize
		{
			if( RandomNumber(1,100) < VarGetFraction( "ChanceOfDefenseRespawn" ) ) //TO DO variablize
			{
				RegenerateEncounter( MyEncounter() );
				if ( VarGetNumber("AllowBaseToBeTaken") != 0 ) 
				{
					SetAIGang(MyEncounter(), GetAIGang( VarGet( "FirebaseBrain" ) ) );
				} else {
					SetAIGang( MyEncounter(), 0 );
					SetAIAlly( MyEncounter(), kAllyID_Monster );
				}
			}
		}

		if (DoEvery ("TeamTimer", 0.02f, 0))
		{
			if ( VarGetNumber("AllowBaseToBeTaken") != 0 ) 
			{
				STRING owner = VarGet("Owner");

				// for debug purposes
				VarSetNumber("CurrentEntCount", NumEntitiesInTeam( MyEncounter() ));

				if (!StringEqual(owner, "")) 
				{
					if (IsEntityPlayer(owner)) // ironically, this is the same as checking if the player is on the map
					{
						// the owner is still around - Check to see if the firebase is not on a team
						if (!VarGetNumber("OnTeam") && IsEntityOnTeam(owner))
						{	
							// if player has joined team, switch ownership
							if ( GetAIGang( VarGet("FirebaseBrain" ) ) != GetAIGang( owner ) )
							{		
								SetAIGang( MyEncounter(), GetAIGang( owner ) );
								SetAIAlly( MyEncounter(), GetAIAlly( owner ) );
								VarSetNumber("OnTeam", 1);
								VarSet("Owner", "");
							}
						}
					} else {
						// if player has left, revert firebase to unowned
						SetAIGang( MyEncounter(),0 );
						SetAIAlly( MyEncounter(), kAllyID_Monster );
						VarSet("Owner", "");
						VarSetNumber("OnTeam", 0);
					}
				}
			}
		} else {
			// firebase can't be taken... check to see if the defenses are down
			if( NumEntitiesInTeam( MyEncounter() ) <= VarGetNumber( "NumberOfEntsNotNeedingDestruction" ) ) 
			{
				SetAIAlly( MyEncounter(), kAllyID_Hero );
			}
		}
	}

	END_STATES
}

void ConvertibleFirebaseInit()
{
	SetScriptName( "ConvertibleFirebase" );
	SetScriptFunc( ConvertibleFirebase );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "PositionOfControlComputer",				"",		0 );
	SetScriptVar( "PositionOfFirebaseBrain",				"",		0 );
	SetScriptVar( "FrequencyOfDefenseRespawn",				"",		0 );
	SetScriptVar( "ChanceOfDefenseRespawn",					"",		0 );
	SetScriptVar( "NumberOfEntsNotNeedingDestruction",		"",		0 );

	SetScriptVar( "AllowBaseToBeTaken",						"",		0 );

	SetScriptVar( "ControlComputerSuccessMessage",			"",		SP_OPTIONAL | SP_MULTIVALUE );
	SetScriptVar( "ControlComputerFailureMessage",			"",		SP_OPTIONAL | SP_MULTIVALUE );
	SetScriptVar( "SuccessBannerMessage",					"",		SP_OPTIONAL | SP_MULTIVALUE );
	SetScriptVar( "ControlComputerAlreadyOwnedMessage",		"",		SP_OPTIONAL | SP_MULTIVALUE );
	SetScriptVar( "ControlComputerDefeatedMessage",			"",		SP_OPTIONAL | SP_MULTIVALUE );
	SetScriptVar( "ControlComputerNotDefeatedMessage",		"",		SP_OPTIONAL | SP_MULTIVALUE );

	SetScriptSignal( "End Event", "EndScript" );
}
