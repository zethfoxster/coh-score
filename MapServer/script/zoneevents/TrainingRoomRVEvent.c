// ZONE SCRIPT
//


#include "scriptutil.h"

static GLOWIEDEF TrainingRoomRVEventPresent = NULL;
static int *TrainingRoomRVEventLocationUsed = NULL;
static int *TrainingRoomRVEventContacts = NULL;
static StashTable TrainingRoomRVEventPresentEnts;


// don't run on city zones that don't have the marker we expect
static int CorrectCityZone()
{
	ENCOUNTERGROUP group = "first"; 
	group = FindEncounterGroupByRadius( 
		LOC_ORIGIN,
		0, 
		9999.0, 
		"VillainLeveler", 
		1, 
 		0 );  
	if (stricmp(group, "location:none") != 0) {
		return 1;
	} else {
		return 0;
	}
}

static int StoreOnContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	switch (link)
	{
	case 1:	// HELLO
		OpenStore(target, player, VarGet("Store"));
		return 0;
		break;
	default:
		return 0;
	}
}

static int LevelerOnClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));
	AddResponse(response, StringLocalizeWithVars(VarGet("Leave"), player), 3);
	switch (link)
	{
		case 1:	// HELLO
			// check to see if the player has already been here
			if (EntityHasRewardToken(player, VarGet("Token")))
			{
				if (IsEntityHero(player))
					strcat(response->dialog, StringLocalizeWithVars( VarGet("ReturnTextHero"), player));
				else 
					strcat(response->dialog, StringLocalizeWithVars( VarGet("ReturnTextVillain"), player));
			} else {
				if (IsEntityHero(player))
					strcat(response->dialog, StringLocalizeWithVars( VarGet("LevelUpTextHero"), player));
				else 
					strcat(response->dialog, StringLocalizeWithVars( VarGet("LevelUpTextVillain"), player));
				EntityGrantRewardToken(player, VarGet("Token"), 1);
				EntitySetXP(player, VarGetNumber("XP"));
				EntityAddInfluence(player, VarGetNumber("Influence"));
			}
			return 1;
			break;
		case 3: // exit
		default:
			return 0;
	}


	return 0;
}


static void TrainingRoomRVEventCreateSpawns()
{
	ENCOUNTERGROUP group = "first"; 

	// spawn contact hero one
	group = FindEncounterGroupByRadius( 
		LOC_ORIGIN,
		0, 
		9999.0, 
		"VillainLeveler", 
		1, 
 		0 );  
	if (stricmp(group, "location:none") != 0) {
		Spawn( 1, "Villains", "scripts.loc/SpawnDefs/V_PvP_05_01/Villain_Leveler_D0_V0.spawndef", group, "VillainLeveler", 1, 0 );  

		// create stores
		OnScriptedContactInteract(StoreOnContactClick, ActorFromEncounterPos(group, 4));
		OnScriptedContactInteract(StoreOnContactClick, ActorFromEncounterPos(group, 5));
		OnScriptedContactInteract(StoreOnContactClick, ActorFromEncounterPos(group, 6));

		// create trainer
		OnScriptedContactInteractFromContactDef("scripts.loc/V_Contacts/Recluses_Victory/Ice_Mistral.contact", ActorFromEncounterPos(group, 2));
		OnScriptedContactInteractFromContactDef("scripts.loc/V_Contacts/Recluses_Victory/Silver_Mantis.contact", ActorFromEncounterPos(group, 3));

		// create leveler
		OnScriptedContactInteract(LevelerOnClick, ActorFromEncounterPos(group, 1));

	}

		// spawn contact hero one
	group = FindEncounterGroupByRadius( 
		LOC_ORIGIN,
		0, 
		9999.0, 
		"HeroLeveler", 
		1, 
 		0 );  
	if (stricmp(group, "location:none") != 0) {
		Spawn( 1, "Heroes", "scripts.loc/SpawnDefs/V_PvP_05_01/Hero_Leveler_D0_V0.spawndef", group, "HeroLeveler", 1, 0 );  

		// create stores
		OnScriptedContactInteract(StoreOnContactClick, ActorFromEncounterPos(group, 4));
		OnScriptedContactInteract(StoreOnContactClick, ActorFromEncounterPos(group, 5));
		OnScriptedContactInteract(StoreOnContactClick, ActorFromEncounterPos(group, 6));

		// create trainer
		OnScriptedContactInteractFromContactDef("scripts.loc/V_Contacts/Recluses_Victory/Minx.contact", ActorFromEncounterPos(group, 2));
		OnScriptedContactInteractFromContactDef("scripts.loc/V_Contacts/Recluses_Victory/Swan.contact", ActorFromEncounterPos(group, 3));

		// create leveler
		OnScriptedContactInteract(LevelerOnClick, ActorFromEncounterPos(group, 1));

	}

}

void TrainingRoomRVEventEnd(int freeThings)
{

	// remove contacts (if they exist)
	Kill("Villains", false);
	Kill("Heroes", false);

}



//////////////////////////////////////////////////////////////////////////////////////
// M A I N 
//////////////////////////////////////////////////////////////////////////////////////

void TrainingRoomRVEvent(void)
{

	//////////////////////////////////////////////////////////////////////////////////////
	// S E T U P
	//////////////////////////////////////////////////////////////////////////////////////
	INITIAL_STATE

		ON_ENTRY 
			if (!CorrectCityZone()) {
				EndScript();
			} else {
				// spawn things
				TrainingRoomRVEventCreateSpawns();
			}

		END_ENTRY


	//////////////////////////////////////////////////////////////////////////////////////
	// S H U T D O W N
	//////////////////////////////////////////////////////////////////////////////////////
	STATE("Shutdown") ////////////////////////////////////////////////////////

		TrainingRoomRVEventEnd(true);
		EndScript();
	END_STATES
		

	ON_SIGNAL("End")
		SET_STATE("Shutdown");
	END_SIGNAL

	ON_STOPSIGNAL
		TrainingRoomRVEventEnd(true);
	END_SIGNAL
}

void TrainingRoomRVEventInit()
{
	SetScriptName( "TrainingRoomRVEvent" );
	SetScriptFunc( TrainingRoomRVEvent );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptSignal( "End", "End" );		

	SetScriptVar( "Store",									NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "Token",									NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ReturnTextVillain",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ReturnTextHero",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "LevelUpTextHero",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "LevelUpTextVillain",						NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "XP",										NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "Influence",								NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "Leave",									NULL,		SP_DONTDISPLAYDEBUG );
}
