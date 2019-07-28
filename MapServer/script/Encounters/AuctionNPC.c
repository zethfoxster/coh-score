
#include "scriptutil.h"

// Documentation at: http://code:8081/display/cs/Auction+NPC+Encounter+Script


int AuctionNPCOnContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	switch (link)
	{
		case 1:	// HELLO
		default:
			// open auction house window
			OpenAuction(target, player);
			break;
	}
	return 0;
}

static void AuctionNPCAttachContact()
{
	ENTITY AuctionNPC;

	//Find the NPC, and Set Script to Ready
	AuctionNPC = ActorFromEncounterPos( MyEncounter(), VarGetNumber("AuctionNPCEncounterPosition")); 
	if( StringEqual( AuctionNPC, TEAM_NONE ) )
	{
		EndScript(); //Error
	}
	else
	{
		// setup auction action!
		OnScriptedContactInteract(AuctionNPCOnContactClick, AuctionNPC);
		VarSet( "AuctionNPC", AuctionNPC );
		SET_STATE("Ready");
	}
}

void ScriptAuctionNPC()
{
	INITIAL_STATE    
	{
		ON_ENTRY 
			// take control of the encounter
			ScriptControlsCompletion( MyEncounter() ); 

			// check to see if we should spawn the contact
			if (IsInventionEnabled() && MapHasDataToken("EnableAuction"))
				AuctionNPCAttachContact();
			else
			{
				Kill( MyEncounter(), false );
				EndScript(); //Error
			}
		END_ENTRY
		SET_STATE("Ready");
	}

	////////////////////////////////////////////////////////////////////
	STATE("Ready")
	{
		if (DoEvery("DisableCheck", 1.0f, NULL))
		{
			if (!MapHasDataToken("EnableAuction"))
			{
				Kill( MyEncounter(), false );
				EndScript(); //Error
			}
		}
	}

	END_STATES
}

void AuctionNPCInit()
{
	SetScriptName( "AuctionNPC" );
	SetScriptFunc( ScriptAuctionNPC );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "AuctionNPCEncounterPosition",						NULL,		0 );

	SetScriptSignal( "End Event", "EndScript" );
}
