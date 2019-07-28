// ZONE SCRIPT
//

#include "scriptutil.h"

void SpawnContact( STRING layout, STRING spawn, STRING def )
{
	ENCOUNTERGROUP group; 
	group = FindEncounter( 0, 0, 0, 0, 0, layout, 0, 0 );
	if ( !StringEqual(group, LOCATION_NONE) )  //Will just fail on every map except dance party
	{
		ENTITY contact;
		Spawn( 1, "Contact", spawn, group, 0, 1, 0 );  

		contact = ActorFromEncounterPos( group, 1 );

		if( !StringEqual( contact, TEAM_NONE ) )
		{
			// add on click handler
			OnScriptedContactInteractFromContactDef( def, contact );
			ScriptRegisterNewContact( ScriptGetContactHandle( def ), contact );
		}
	}
}

static void SpawnContacts()
{
	//Try to spawn the contacts (if the location doesn't exist, don't spawn them.
	if ( !VarGetNumber("ContactsSpawned") ) 
	{
		SpawnContact( VarGet("HeroMissionContactLayout"),    VarGet("HeroMissionContactSpawnDef"),    VarGet("HeroMissionContact")    ); 
		SpawnContact( VarGet("VillainMissionContactLayout"), VarGet("VillainMissionContactSpawnDef"), VarGet("VillainMissionContact") ); 
		SpawnContact( VarGet("HeroStoreContactLayout"),      VarGet("HeroStoreContactSpawnDef"),      VarGet("HeroStoreContact")      ); 
		SpawnContact( VarGet("VillainStoreContactLayout"),   VarGet("VillainStoreContactSpawnDef"),   VarGet("VillainStoreContact")   ); 
		SpawnContact( VarGet("HeroWeddingContactLayout"),    VarGet("HeroWeddingContactSpawnDef"),    VarGet("HeroWeddingContact")    );
		SpawnContact( VarGet("VillainWeddingContactLayout"), VarGet("VillainWeddingContactSpawnDef"), VarGet("VillainWeddingContact") );
		SpawnContact( VarGet("HeroWidowContactLayout"),		 VarGet("HeroWidowContactSpawnDef"),      VarGet("HeroWidowContact")      );
		SpawnContact( VarGet("VillainWidowContactLayout"),   VarGet("VillainWidowContactSpawnDef"),   VarGet("VillainWidowContact")   );

		VarSetNumber("ContactsSpawned", 1);
	}
}


void SpringFlingEventEnd(int freeThings)
{
	// remove contacts (if they exist)
	Kill("Contact", false);
	VarSetNumber("ContactsSpawned", 0);

	// remove map tokens
	MapRemoveDataToken(VarGet("ContactMapToken"));

	// remove callbacks
	OnPlayerEnteringMap( NULL );
}


//////////////////////////////////////////////////////////////////////////////////////
// E N T E R   C A L L B A C K
//////////////////////////////////////////////////////////////////////////////////////
// Sets up contact for players who don't have it!
int SpringFlingOnEnterMap( ENTITY player )
{

	// check to see if the player has any of the event contacts
	if (IsEntityHero(player))
	{
		int contactHandle = ScriptGetContactHandle( VarGet("HeroMissionContact") );

		if ( !DoesPlayerHaveContact( player, contactHandle ) ) 
		{
			ScriptGivePlayerNewContact(player, contactHandle );
			SendPlayerDialog( player, StringLocalize( VarGet( "HeroMissionContactPopupOnIssue" ) ) );
		}
	}
	else //You're a villain
	{
		int contactHandle = ScriptGetContactHandle( VarGet("VillainMissionContact") );

		if ( !DoesPlayerHaveContact( player, contactHandle ) ) 
		{
			ScriptGivePlayerNewContact(player, contactHandle );
			SendPlayerDialog( player, StringLocalize( VarGet( "VillainMissionContactPopupOnIssue" ) ) );
		}
	}

	// check to see if the player has any of the Red Widow contacts
	if (IsEntityHero(player))
	{
		int contactHandle = ScriptGetContactHandle( VarGet("HeroWidowContact") );

		if ( !DoesPlayerHaveContact( player, contactHandle ) && GetLevel(player) >= 30) 
		{
			ScriptGivePlayerNewContact(player, contactHandle );
			SendPlayerDialog( player, StringLocalize( VarGet( "HeroWidowContactPopupOnIssue" ) ) );
		}
	}
	else //You're a villain
	{
		int contactHandle = ScriptGetContactHandle( VarGet("VillainWidowContact") );

		if ( !DoesPlayerHaveContact( player, contactHandle ) && GetLevel(player) >= 30) 
		{
			ScriptGivePlayerNewContact(player, contactHandle );
			SendPlayerDialog( player, StringLocalize( VarGet( "VillainWidowContactPopupOnIssue" ) ) );
		}
	}

	return true;
}

void SpringFlingEvent(void)
{
	//////////////////////////////////////////////////////////////////////////////////////
	// S E T U P
	//////////////////////////////////////////////////////////////////////////////////////
	INITIAL_STATE

		ON_ENTRY 
		{
			STRING thisMapName = GetShortMapName();
			bool giveContact = true;

			MapSetDataToken(VarGet("ContactMapToken"), 1);

			// setup mission contacts
			if( StringEqual( thisMapName, "V_City_01_01" ) ) //Not in Mercy
				giveContact = false;
			if( StringEqual( thisMapName, "City_01_01" ) ) //Not in Atlas
				giveContact = false;
			if( StringEqual( thisMapName, "City_01_03" ) ) //Not in Galaxy
				giveContact = false;
			if( StringEqual( thisMapName, "V_City_00_01" ) ) //Not in Breakout
				giveContact = false;
			if( StringEqual( thisMapName, "City_00_01" ) ) //Not in Outbreak
				giveContact = false;
			if( StringEqual( thisMapName, "P_City_00_01" ) ) //Not in Nova Praetoria
				giveContact = false;
			if( StringEqual( thisMapName, "P_City_00_05" ) ) //Not in Precinct Five
				giveContact = false;
			if( StringEqual( thisMapName, "N_City_00_01" ) ) //Not in Destroyed Galaxy
				giveContact = false;


			SpawnContacts();

			if( giveContact )
				OnPlayerEnteringMap( SpringFlingOnEnterMap );
		}
		END_ENTRY

		SET_STATE("DoSpringFling");


	//////////////////////////////////////////////////////////////////////////////////////
	// DoSpringFling
	//////////////////////////////////////////////////////////////////////////////////////
	STATE("DoSpringFling") ////////////////////////////////////////////////////////
		ON_ENTRY 
		{ 
	
		}
		END_ENTRY

		//Currently, the spring fling does nothing but create an issue contacts

	//////////////////////////////////////////////////////////////////////////////////////
	// S H U T D O W N
	//////////////////////////////////////////////////////////////////////////////////////
	STATE("Shutdown") ////////////////////////////////////////////////////////
		SpringFlingEventEnd(true);
		EndScript();
	END_STATES

	ON_SIGNAL("End")
		SET_STATE("Shutdown");
	END_SIGNAL

	ON_STOPSIGNAL
		SpringFlingEventEnd(true);
	END_SIGNAL
}

void SpringFlingInit()
{
	SetScriptName( "SpringFlingEvent" );
	SetScriptFunc( SpringFlingEvent );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptSignal( "End", "End" );		

	SetScriptVar( "HeroMissionContactLayout",				NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroMissionContactSpawnDef",				NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroMissionContact",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroMissionContactPopupOnIssue",			NULL,		SP_DONTDISPLAYDEBUG );
	
	SetScriptVar( "VillainMissionContactLayout",			NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainMissionContactSpawnDef",			NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainMissionContact",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainMissionContactPopupOnIssue",		NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "HeroStoreContactLayout",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroStoreContactSpawnDef",				NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroStoreContact",						NULL,		SP_DONTDISPLAYDEBUG );
	
	SetScriptVar( "VillainStoreContactLayout",				NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainStoreContactSpawnDef",			NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainStoreContact",					NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "HeroWeddingContactLayout",				NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroWeddingContactSpawnDef",				NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroWeddingContact",						NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "VillainWeddingContactLayout",			NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainWeddingContactSpawnDef",			NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainWeddingContact",					NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "HeroWidowContactLayout",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroWidowContactSpawnDef",				NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroWidowContact",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroWidowContactPopupOnIssue",			NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "VillainWidowContactLayout",				NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainWidowContactSpawnDef",			NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainWidowContact",					NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainWidowContactPopupOnIssue",		NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "ContactMapToken",						NULL,		SP_DONTDISPLAYDEBUG );
}
