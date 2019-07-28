
#include "scriptutil.h"

// Documentation at: http://code:8081/display/cs/Contact+NPC+Encounter+Script


//////////////////////////////////////////////////////////////////////////////////////
// E N T E R   C A L L B A C K
//////////////////////////////////////////////////////////////////////////////////////
// Sets up contact for players who don't have it!
int ContactNPCOnEnterMap(ENTITY player)
{
	int contactHandle = ScriptGetContactHandle( VarGet("ContactDef") );

	if (!DoesPlayerHaveContact(player, contactHandle)) 
	{
		// give contact
		ScriptGivePlayerNewContact(player, contactHandle);
	}

	return true;
}

static void ContactNPCAttachContact()
{
	ENTITY ContactNPC;

	//Find the NPC, and Set Script to Ready
	ContactNPC = ActorFromEncounterPos( MyEncounter(), VarGetNumber("ContactNPCEncounterPosition")); 
	if( StringEqual( ContactNPC, TEAM_NONE ) )
	{
		EndScript(); //Error
	}
	else
	{
		OnScriptedContactInteractFromContactDef(VarGet("ContactDef"), ContactNPC);
		ScriptRegisterNewContact(ScriptGetContactHandle(VarGet("ContactDef")), PointFromEntity(ContactNPC));

		VarSet( "ContactNPC", ContactNPC );
		SET_STATE("Ready");

		if (VarGetNumber("IssueOnZoneEnter"))
		{
			OnPlayerEnteringMap( ContactNPCOnEnterMap );
		}
	}
}

void ScriptContactNPC()
{
	INITIAL_STATE    
	{
		ON_ENTRY 
			// take control of the encounter
			ScriptControlsCompletion( MyEncounter() ); 

			// check to see if we should spawn the contact
			if (VarGetNumber("CheckForInvention"))
			{
				if (IsInventionEnabled())
				{
					ContactNPCAttachContact();
				} else {
					Kill( MyEncounter(), false );
					EndScript(); //Error
				}
			} else {
				ContactNPCAttachContact();
			}

		END_ENTRY
		SET_STATE("Ready");
	}

	////////////////////////////////////////////////////////////////////
	STATE("Ready")
	{

	}

	END_STATES
}

void ContactNPCInit()
{
	SetScriptName( "ContactNPC" );
	SetScriptFunc( ScriptContactNPC );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "ContactNPCEncounterPosition",						NULL,		0 );

	SetScriptVar( "ContactDef",											NULL,		0 );

	SetScriptVar( "CheckForInvention",									0,			0);
	SetScriptVar( "IssueOnZoneEnter",									0,			SP_OPTIONAL);

	SetScriptSignal( "End Event", "EndScript" );
}
