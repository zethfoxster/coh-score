
#include "scriptutil.h"

// Documentation at: http://code:8081/display/cs/Data+NPC+Encounter+Script

typedef struct {
	NUMBER ID;
	NUMBER BunkerID;
	FRACTION health;
	FRACTION target;
} DataNPCGeometryState;

int						DataNPCAlreadyRunning = false;
DataNPCGeometryState	**DataNPCGeometryList = NULL;



/////////////////////////////////////////////////////////////////////////
// Marker and Geometry Processing
/////////////////////////////////////////////////////////////////////////

// given the bunker name, return the index of that bunker in the bunker list array
int DataNPCFindBunkerIndex(STRING name)
{
	int i, count;
	if (!StringEmpty(name)) {
		count = VarGetArrayCount("BunkerList");
		for (i = 0; i < count; i++)
		{
			if (StringEqual(VarGetArrayElement("BunkerList", i), name))
				return i;
		}
	}
	return -1;
}

int DataNPCMarkerProcessor(SCRIPTELEMENT elem)
{
	SCRIPTMARKER marker = GetScriptMarkerFromScriptElement(elem);

	if (!StringEmpty(FindMarkerProperty(marker, "Bunker")))
	{
		DataNPCGeometryState *state = malloc(sizeof(DataNPCGeometryState));

		state->ID = DynamicGeometryFind(FindMarkerGeoName(marker), LOC_ORIGIN, -1);
		if (state->ID) {
			state->BunkerID = DataNPCFindBunkerIndex(FindMarkerProperty(marker, "Bunker"));
			state->health = state->target = 50.0f;
			DynamicGeometrySetHP(state->ID, state->health, 0);

			eaPush(&DataNPCGeometryList, state);
		} else {
			// couldn't find this dyn geom group
			free (state);
		}
	}
	return true; 
}

void DataNPCSetAllTargets(NUMBER location, FRACTION target)
{
	int i, count;
	count = eaSize(&DataNPCGeometryList);
	for (i = 0; i < count; i++)
	{
		if (DataNPCGeometryList[i]) {
			if (location >= 0)
			{ 
				if (DataNPCGeometryList[i]->BunkerID == location)
					DataNPCGeometryList[i]->target = target;
			} else {
				DataNPCGeometryList[i]->target = target;
			}
		}
	}
}

void DataNPCUpdateGeometryState(void)
{
	int count, i;

	// update bunker geometry state
	count = eaSize(&DataNPCGeometryList);
	for (i = 0; i < count; i++)
	{
		DataNPCGeometryState *state = DataNPCGeometryList[i];
		if (state && state->health != state->target) {
			NUMBER oldHealth = (NUMBER) state->health;
			NUMBER newHealth = 0;
			FRACTION delta = VarGetFraction("BunkerChangeRate");

			// move towards target
			if (fabs(state->health - state->target) < delta) {
				state->health = state->target;
			} else {
				if (state->health > state->target)
				{
					delta = -delta;
				}
				state->health += delta;
			}

			// update health ingame
			newHealth = (NUMBER) state->health;
			if (oldHealth != newHealth)
			{
				DynamicGeometrySetHP(state->ID, newHealth, 0);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////
// RV Status Reading 
/////////////////////////////////////////////////////////////////////////

void DataNPCUpdateBunkerStatus(void)
{
	int state = MapGetDataToken("RVBunkerState");
	int oldstate = VarGetNumber("LastStatus");
	int i, oldbunker, newbunker, herocount, villaincount;
	int heroWins = MapGetDataToken("RVHeroScore");
	int villainWins = MapGetDataToken("RVVillainScore");

	herocount = villaincount = 0;
	if (oldstate != state)
	{ 
		for (i = 0; i < 16; i++) 
		{
			newbunker = (state & (3 << (i*2))) >> (i*2);
			oldbunker = (oldstate & (3 << (i*2))) >> (i*2);

			if (newbunker == 1) // hero
			{
				herocount++;
				if (newbunker != oldbunker) {
					if (VarGetNumber("TakeOverChat") && i < VarGetArrayCount("HeroesTakeBase")) 
						Say(VarGet("DataNPC"), StringLocalize(VarGetArrayElement("HeroesTakeBase", i)), 0);
					DataNPCSetAllTargets(i, 100.0f);
				}
			} else if (newbunker == 2) { // villain
				villaincount++;
				if (newbunker != oldbunker) {
					if (VarGetNumber("TakeOverChat") && i < VarGetArrayCount("VillainsTakeBase")) 
						Say(VarGet("DataNPC"), StringLocalize(VarGetArrayElement("VillainsTakeBase", i)), 0);
					DataNPCSetAllTargets(i, 0.0f);
				}
			} else {
				DataNPCSetAllTargets(i, 50.0f);
			}
		}

		VarSetNumber("LastStatus", state);
		VarSetNumber("HeroCount", herocount);
		VarSetNumber("VillainCount", villaincount);
	}

	if (heroWins != VarGetNumber("HeroWins"))
	{
		if (VarGetNumber("TakeOverChat") && !StringEmpty(VarGet("HeroesWin"))) 
			Say(VarGet("DataNPC"), StringLocalize(VarGet("HeroesWin")), 0);
		VarSetNumber("HeroWins", heroWins);
	}

	if (villainWins != VarGetNumber("VillainWins"))
	{
		if (VarGetNumber("TakeOverChat") && !StringEmpty(VarGet("VillainsWin"))) 
			Say(VarGet("DataNPC"), StringLocalize(VarGet("VillainsWin")), 0);
		VarSetNumber("VillainWins", villainWins);
	}

}

int DataNPCOnRightSide(ENTITY player)
{
	return ((VarGetNumber("OnlyHeroes") && IsEntityOnBlueSide(player)) ||
			(VarGetNumber("OnlyVillains") && IsEntityOnRedSide(player)));
}

/////////////////////////////////////////////////////////////////////////
// Contact Interaction
/////////////////////////////////////////////////////////////////////////

int DataNPCOnContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{

	switch (link)
	{
	case 1:	// HELLO
		sprintf(response->dialog, getContactHeadshotScriptSMF(target, player));

		// check to see if this is a restricted info dude
		if (DataNPCOnRightSide(player)) {
			strcat(response->dialog, StringLocalizeWithVars( VarGet("InfoText"), player));
			strcat(response->dialog, "<br><br>");
			strcat(response->dialog, StringLocalizeWithVars( VarGet("HeroBaseText"), player));
			strcat(response->dialog, " ");
			strcat(response->dialog, VarGet("HeroCount"));
			strcat(response->dialog, "<br><br>");
			strcat(response->dialog, StringLocalizeWithVars( VarGet("VillainBaseText"), player));
			strcat(response->dialog, " ");
			strcat(response->dialog, VarGet("VillainCount"));

			if (TimerElapsed("ContactTalkTimer"))
			{
				Say(VarGet("DataNPC"), StringLocalizeWithVars(VarGet("OutLoudInfoText"), player), 0);
				TimerSet("ContactTalkTimer", 0.25f);
			}
		} else {
			strcat(response->dialog, StringLocalizeWithVars( VarGet("RestrictedText"), player));
		}
		AddResponse(response, StringLocalizeWithVars(VarGet("Leave"), player), 3);
		return 1;
		break;
	default:
		return 0;
	}
}

void DataNPCShutdown()
{
	if (VarGetNumber("IAmGeometryMaster")) 
	{
		if (DataNPCGeometryList) 
			eaDestroy(&DataNPCGeometryList);
		DataNPCGeometryList = NULL;
		DataNPCAlreadyRunning = false;
	}
}

/////////////////////////////////////////////////////////////////////////
// M A I N
/////////////////////////////////////////////////////////////////////////

void ScriptDataNPC()
{
	INITIAL_STATE    
 	{
		ENTITY DataNPC;


		DataNPC = ActorFromEncounterPos( MyEncounter(), VarGetNumber("DataNPCEncounterPosition")); 
		if( StringEqual( DataNPC, TEAM_NONE ) )
		{
			EndScript(); //Error
		}
		else
		{
			OnScriptedContactInteract(DataNPCOnContactClick, DataNPC);
			VarSet( "DataNPC", DataNPC );
			SET_STATE("Waiting");
		}
		TimerSet("ContactTalkTimer", 0.25f);

		if (!DataNPCAlreadyRunning)
		{

			/////////////////////////////////////////////////////////////////////////
			// Setup Bunkers
			/////////////////////////////////////////////////////////////////////////

			// Geometry Swap Setup
			eaCreate(&DataNPCGeometryList);
			eaSetSize(&DataNPCGeometryList, 0);
 			ForEachScriptMarker(DataNPCMarkerProcessor);

			DataNPCAlreadyRunning = true;
			VarSetNumber("IAmGeometryMaster", 1);
		}

		VarSetNumber("HeroCount", 0);
		VarSetNumber("VillainCount", 0);

	}

	////////////////////////////////////////////////////////////////////
	STATE("Waiting")
	{
		// maybe someone's around?
		int i;
		NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);

		for (i = 1; i <= count; i++)
		{
			ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);
			
			if (IsEntityPlayer(player) && DataNPCOnRightSide(player) && 
				DistanceBetweenEntities(player, VarGet("DataNPC")) < VarGetFraction("CloseDistance"))
			{
				// a player is close
				Say(VarGet("DataNPC"), StringLocalizeWithVars(VarGet("Greeting"), player), 0);
				SET_STATE("PeopleNear");
			} 
		}
		DoEvery("UpdateTimer", 0.25f, DataNPCUpdateBunkerStatus);
		DataNPCUpdateGeometryState();
	}

	////////////////////////////////////////////////////////////////////
	STATE("PeopleNear")
	{
		int i;
		int closeCount = 0;
		NUMBER count = NumEntitiesInTeam(ALL_PLAYERS);

		for (i = 1; i <= count; i++)
		{
			ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);
			
			if (IsEntityPlayer(player) && DistanceBetweenEntities(player, VarGet("DataNPC")) < VarGetFraction("CloseDistance") + 5.0f)
			{
				// a player is close
				closeCount++;
			}
		}

		DoEvery("UpdateTimer", 0.25f, DataNPCUpdateBunkerStatus);
		DataNPCUpdateGeometryState();

		if (closeCount == 0)
		{
			SET_STATE("Waiting");
		}
	}

	END_STATES

	ON_STOPSIGNAL
		DataNPCShutdown();
	END_SIGNAL
}


void DataNPCInit()
{
	SetScriptName( "DataNPC" );
	SetScriptFunc( ScriptDataNPC );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "DataNPCEncounterPosition",							NULL,		0 );
	SetScriptVar( "Greeting",											NULL,		SP_MULTIVALUE );
	SetScriptVar( "RestrictedText",										NULL,		SP_MULTIVALUE );
	SetScriptVar( "InfoText",											NULL,		SP_MULTIVALUE );
	SetScriptVar( "HeroBaseText",										NULL,		SP_MULTIVALUE );
	SetScriptVar( "VillainBaseText",									NULL,		SP_MULTIVALUE );
	SetScriptVar( "OutLoudInfoText",									NULL,		SP_MULTIVALUE );
	SetScriptVar( "BunkerList",											NULL,		0 );
	SetScriptVar( "HeroesTakeBase",										NULL,		0 );
	SetScriptVar( "VillainsTakeBase",									NULL,		0 );
	SetScriptVar( "HeroesWin",											NULL,		SP_MULTIVALUE );
	SetScriptVar( "VillainsWin",										NULL,		SP_MULTIVALUE );

	SetScriptVar( "Leave",												NULL,		SP_MULTIVALUE );
	SetScriptVar( "OnlyHeroes",											NULL,		0 );
	SetScriptVar( "OnlyVillains",										NULL,		0 );

	SetScriptVar( "CloseDistance",										"25.0",		SP_OPTIONAL );
	SetScriptVar( "TakeOverChat",										"1",		SP_OPTIONAL );
	SetScriptVar( "BunkerChangeRate",									"1.0",		SP_OPTIONAL );

	SetScriptSignal( "End Event", "EndScript" );
}
