// ZONE SCRIPT
//

// Used for testing new features of the script system

#include "scriptutil.h"
#include "timing.h"

#define COV_WARBURG_LAUNCHB_IMAGES "sp_icon_code_full.tga"
#define COV_WARBURG_LAUNCHC_IMAGES "sp_icon_code_fullC.tga"
#define COV_WARBURG_LAUNCHN_IMAGES "sp_icon_code_fullB.tga"

#define COV_WARBURG_LAUNCH_WPICON	"map_enticon_rocket"
#define COV_WARBURG_LAUNCHB_WPICON	"map_enticon_code_full"
#define COV_WARBURG_LAUNCHC_WPICON	"map_enticon_code_fullC"
#define COV_WARBURG_LAUNCHN_WPICON	"map_enticon_code_fullB"

enum {
	COV_WARBURGFOOTBALL_PAGE_HELLO = 1,
	COV_WARBURGFOOTBALL_PAGE_LEAVE = 3,
	COV_WARBURGFOOTBALL_PAGE_ACCEPTMISSION,
	COV_WARBURGFOOTBALL_PAGE_INSTRUCTIONS,
};

enum {
	COV_WARBURG_GLOWIE_NUCLEAR = 1,
	COV_WARBURG_GLOWIE_BIOLOGICAL,
	COV_WARBURG_GLOWIE_CHEMICAL,
	COV_WARBURG_GLOWIE_LAUNCH,
	COV_WARBURG_GLOWIE_NUM
};

static StringArray playerList;		// list of players who have a launch code - used to check for timeout on their timers

void CoVWarburgFootballAddToList(StringArray *list, ENTITY player)
{
	char *pEntString = NULL;
	int idx = StringArrayFind(*list, player);
		
	// check to see if they're already on the list
	if (idx == -1)
	{	
		pEntString = (char *) malloc(strlen(player) + 1);
		strcpy(pEntString, player);
		eaInsert(list, pEntString, 0);
	}
}


ENTITY CoVWarburgFootballRemoveFromList(StringArray *list, ENTITY player)
{
	int idx = StringArrayFind(*list, player);
	char *pEntString = NULL;
	STRING retval;

	if (idx != -1)
	{	
		pEntString = eaRemove(list, idx);
		if (pEntString) 
		{
			retval = StringAdd(pEntString, "");
			SAFE_FREE(pEntString);
		}
	}
	return retval;
}

void CoVWarburgFootballEndGame(ENTITY player)
{

	// remove tokens
	EntityRemoveRewardToken(player, "WarburgCode1");
	EntityRemoveRewardToken(player, "WarburgCode2");
	EntityRemoveRewardToken(player, "WarburgCode3");
	EntityRemoveRewardToken(player, "WarburgLaunchCode");
	EntityRemoveRewardToken(player, "WarburgScientist");
	
	// remove glow
//	EntityRemoveTempPower(player, "SilentPowers", "V_Ore_Exposure");
//	EntityRemoveTempPower(player, "SilentPowers", "V_Sample_Exposure");
	EntityRemoveTempPower(player, "SilentPowers", "V_Ore_Exposure"); // replace with real power

	// remove all waypoints
	ClearWaypoint(player, VarGetNumber("NuclearWaypoint"));
	ClearWaypoint(player, VarGetNumber("ChemicalWaypoint"));
	ClearWaypoint(player, VarGetNumber("BiologicalWaypoint"));
	ClearWaypoint(player, VarGetNumber("LaunchWaypoint"));

	// revert UI
	ScriptUIHide("Collection:BiologicalWar", player);
	ScriptUIHide("Collection:ChemicalWar", player);
	ScriptUIHide("Collection:NuclearWar", player);
	ScriptUIShow("Collection:CodeWar", player);

	// remove from timer list
	CoVWarburgFootballRemoveFromList(&playerList, player);
}


void CoVWarburgFootballUpdateTimers()
{
	int i;
 
	for (i = 0; i < eaSize(&playerList); i++)
	{
		ENTITY player = playerList[i];

		if (EntityHasRewardToken(player, VarGet("PlayingGameToken")) &&
			EntityGetRewardToken(player, VarGet("PlayingGameToken")) + (60 * VarGetNumber("TimerLength")) < CurrentTime())
		{
			// timer has expired
			SendPlayerAttentionMessage(player, VarGet("TimeUpString"));
			CoVWarburgFootballEndGame(player); // player is no longer valid, since it is removed from the list in this step
			i--; // player has been removed from list, so the index must be adjusted for that
		}
	}
}


int CoVWarburgFootballOnContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
 
	switch (link)
	{
	case COV_WARBURGFOOTBALL_PAGE_HELLO:	// HELLO
		// headshot
		sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));

		// check to see if player is playing the game 
		if (EntityHasRewardToken(player, VarGet("PlayingGameToken")))
		{
			// first time player
			if (IsEntityHero(player)) {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("HeroContactReturnSpeech"), player));
				AddResponse(response, StringLocalizeWithVars(VarGet("HeroContactInstructionResponse"), player), COV_WARBURGFOOTBALL_PAGE_INSTRUCTIONS);
			} else {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("VillainContactReturnSpeech"), player));
				AddResponse(response, StringLocalizeWithVars(VarGet("VillainContactInstructionResponse"), player), COV_WARBURGFOOTBALL_PAGE_INSTRUCTIONS);
			}
			AddResponse(response, StringLocalizeWithVars(VarGet("ContactExitText"), player), COV_WARBURGFOOTBALL_PAGE_LEAVE);

		} else {
			// first time player
			if (IsEntityHero(player)) {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("HeroContactStartSpeech"), player));
				AddResponse(response, StringLocalizeWithVars(VarGet("HeroContactAcceptResponse"), player), COV_WARBURGFOOTBALL_PAGE_ACCEPTMISSION);
				AddResponse(response, StringLocalizeWithVars(VarGet("HeroContactInstructionResponse"), player), COV_WARBURGFOOTBALL_PAGE_INSTRUCTIONS);
				AddResponse(response, StringLocalizeWithVars(VarGet("HeroContactDeclineResponse"), player), COV_WARBURGFOOTBALL_PAGE_LEAVE);
			} else {
				strcat(response->dialog, StringLocalizeWithVars( VarGet("VillainContactStartSpeech"), player));
				AddResponse(response, StringLocalizeWithVars(VarGet("VillainContactAcceptResponse"), player), COV_WARBURGFOOTBALL_PAGE_ACCEPTMISSION);
				AddResponse(response, StringLocalizeWithVars(VarGet("VillainContactInstructionResponse"), player), COV_WARBURGFOOTBALL_PAGE_INSTRUCTIONS);
				AddResponse(response, StringLocalizeWithVars(VarGet("VillainContactDeclineResponse"), player), COV_WARBURGFOOTBALL_PAGE_LEAVE);
			}
		}
		return 1;
	case COV_WARBURGFOOTBALL_PAGE_ACCEPTMISSION:
		sprintf(response->dialog, getContactHeadshotScriptSMF(target, player));

		if (IsEntityHero(player)) {
			strcat(response->dialog, StringLocalizeWithVars(VarGet("HeroContactAcceptSpeech"), player));
		} else {
			strcat(response->dialog, StringLocalizeWithVars(VarGet("VillainContactAcceptSpeech"), player));
		}

		// ScriptUIEntStartUsing(VarGetNumber("UIID"), player);
		EntityGrantRewardToken(player, VarGet("PlayingGameToken"), 1);
		AddResponse(response, StringLocalizeWithVars(VarGet("ContactExitText"), player), COV_WARBURGFOOTBALL_PAGE_LEAVE);
		return 1;
	case COV_WARBURGFOOTBALL_PAGE_INSTRUCTIONS:
		sprintf(response->dialog,  getContactHeadshotScriptSMF(target, player));

		if (IsEntityHero(player)) {
			strcat(response->dialog, StringLocalizeWithVars(VarGet("HeroContactInstructionSpeech"), player));
		} else {
			strcat(response->dialog, StringLocalizeWithVars(VarGet("VillainContactInstructionSpeech"), player));
		}

		AddResponse(response, StringLocalizeWithVars(VarGet("ContactBackText"), player), COV_WARBURGFOOTBALL_PAGE_HELLO);
		AddResponse(response, StringLocalizeWithVars(VarGet("ContactExitText"), player), COV_WARBURGFOOTBALL_PAGE_LEAVE);
		return 1;
	case COV_WARBURGFOOTBALL_PAGE_LEAVE:
	default:
		return 0;
	}
}


int CoVWarburgOnVillainContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	if (IsEntityVillain(player)) {
		return CoVWarburgFootballOnContactClick( player, target, link, response);
	} else {
		if (TimerElapsed("VillainContactTalk")) {
			Say(target, VarGet("VillainGoAway"), 0);
			TimerSet("VillainContactTalk", 0.25f);
		}
		return 0;
	}
}

int CoVWarburgOnHeroContactClick(ENTITY player, ENTITY target, NUMBER link, ContactResponse* response)
{
	if (IsEntityHero(player)) {  
		return CoVWarburgFootballOnContactClick( player, target, link, response);
	} else {
		if (TimerElapsed("HeroContactTalk")) {
			Say(target, VarGet("HeroGoAway"), 0);
			TimerSet("HeroContactTalk", 0.25f);
		}
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// D E V I C E S
///////////////////////////////////////////////////////////////////////////////////

void CoVWarburgDeviceCompleteHandleFragments(ENTITY player)
{
	int count = 0;

	// checking to see if proxy salvage was used
	if (EntityHasRewardToken(player, "WarburgCode3"))
		count = 0;
	else if (EntityHasRewardToken(player, "WarburgCode2"))
		count = 1;
	else if (EntityHasRewardToken(player, "WarburgCode1"))
		count = 2;
	else 
		count = 3;

	if (count)
		EntityGrantSalvage(player, VarGet("ProxySalvage"), -count);

	// removing tokens
	EntityRemoveRewardToken(player, "WarburgCode1");
	EntityRemoveRewardToken(player, "WarburgCode2");
	EntityRemoveRewardToken(player, "WarburgCode3");

}

int CoVWarburgDeviceHasCodes(ENTITY player)
{
	int count = EntityCountSalvage(player, VarGet("ProxySalvage"));

	if ((EntityHasRewardToken(player, "WarburgCode3")) ||
		(EntityHasRewardToken(player, "WarburgCode2") && count > 0) ||
		(EntityHasRewardToken(player, "WarburgCode1") && count > 1) ||
		(count > 2))
		return true;
	else 
		return false;
}

int CoVWarburgDeviceInteract(ENTITY player, ENTITY target)
{
	if (EntityHasRewardToken(player, "WarburgLaunchCode"))
	{
		SendPlayerDialog(player, StringLocalize(VarGet("AlreadyHaveCode")));
		return true;
	} else if (CoVWarburgDeviceHasCodes(player))
	{
		return false;
	} else {
		SendPlayerDialog(player, StringLocalize(VarGet("DeviceNotEnoughCodes")));
		return true;
	}
}

int CoVWarburgNuclearComplete(ENTITY player, ENTITY target)
{
	int count = 0;

	if (CoVWarburgDeviceHasCodes(player))
	{
		SendPlayerDialog(player, VarGet("NuclearDeviceComplete"));

		CoVWarburgDeviceCompleteHandleFragments(player);

		EntityGrantRewardToken(player, "WarburgLaunchCode", COV_WARBURG_GLOWIE_NUCLEAR);
		VarSetNumber(EntVar(player, "TimeToLaunch"), 300 + timerSecondsSince2000());
		ScriptUIShow("Collection:NuclearWar", player);
		ScriptUIHide("Collection:CodeWar", player);

		ClearWaypoint(player, VarGetNumber("NuclearWaypoint"));
		ClearWaypoint(player, VarGetNumber("ChemicalWaypoint"));
		ClearWaypoint(player, VarGetNumber("BiologicalWaypoint"));
		SetWaypoint(player, VarGetNumber("LaunchWaypoint"));

		// timer setting
		EntityGrantRewardToken(player, VarGet("PlayingGameToken"), CurrentTime());
		CoVWarburgFootballAddToList(&playerList, player);

		// Add glow
		EntityGrantReward(player, "V_OreExposure");		// add in glow power here

	} else {
		SendPlayerDialog(player, StringLocalize(VarGet("DeviceNotEnoughCodes")));
	}

	return true;
}

int CoVWarburgBiologicalComplete(ENTITY player, ENTITY target)
{
	if (CoVWarburgDeviceHasCodes(player))
	{
		SendPlayerDialog(player, VarGet("BiologicalDeviceComplete"));

		CoVWarburgDeviceCompleteHandleFragments(player);

		EntityGrantRewardToken(player, "WarburgLaunchCode", COV_WARBURG_GLOWIE_BIOLOGICAL);
		VarSetNumber(EntVar(player, "TimeToLaunch"), 300 + timerSecondsSince2000());
		ScriptUIShow("Collection:BiologicalWar", player);
		ScriptUIHide("Collection:CodeWar", player);
		ClearWaypoint(player, VarGetNumber("NuclearWaypoint"));
		ClearWaypoint(player, VarGetNumber("ChemicalWaypoint"));
		ClearWaypoint(player, VarGetNumber("BiologicalWaypoint"));
		SetWaypoint(player, VarGetNumber("LaunchWaypoint"));

		// timer setting
		EntityGrantRewardToken(player, VarGet("PlayingGameToken"), CurrentTime());
		CoVWarburgFootballAddToList(&playerList, player);

		// Add glow
		EntityGrantReward(player, "V_OreExposure");		// add in glow power here


	} else {
		SendPlayerDialog(player, StringLocalize(VarGet("DeviceNotEnoughCodes")));
	}

	return true;
}

int CoVWarburgChemicalComplete(ENTITY player, ENTITY target)
{
	if (CoVWarburgDeviceHasCodes(player))
	{
		SendPlayerDialog(player, VarGet("ChemicalDeviceComplete"));

		CoVWarburgDeviceCompleteHandleFragments(player);

		EntityGrantRewardToken(player, "WarburgLaunchCode", COV_WARBURG_GLOWIE_CHEMICAL);
		VarSetNumber(EntVar(player, "TimeToLaunch"), 300 + timerSecondsSince2000());
		ScriptUIShow("Collection:ChemicalWar", player);
		ScriptUIHide("Collection:CodeWar", player);
		ClearWaypoint(player, VarGetNumber("NuclearWaypoint"));
		ClearWaypoint(player, VarGetNumber("ChemicalWaypoint"));
		ClearWaypoint(player, VarGetNumber("BiologicalWaypoint"));
		SetWaypoint(player, VarGetNumber("LaunchWaypoint"));

		// timer setting
		EntityGrantRewardToken(player, VarGet("PlayingGameToken"), CurrentTime());
		CoVWarburgFootballAddToList(&playerList, player);

		// Add glow
		EntityGrantReward(player, "V_OreExposure");		// add in glow power here

	} else {
		SendPlayerDialog(player, StringLocalize(VarGet("DeviceNotEnoughCodes")));
	}

	return true;
}

int CoVWarburgLaunchInteract(ENTITY player, ENTITY target)
{
	if (EntityHasRewardToken(player, "WarburgLaunchCode"))
	{
		return false;
	} else {
		SendPlayerDialog(player, StringLocalize(VarGet("DeviceNoLaunchCode")));
		return true;
	}
}

int CoVWarburgLaunchComplete(ENTITY player, ENTITY target)
{
	ENCOUNTERGROUP group = "first"; 

	if (EntityHasRewardToken(player, "WarburgLaunchCode"))
	{
		int type = EntityGetRewardToken(player, "WarburgLaunchCode");

		// give appropriate rewards
		switch (type) {
			case COV_WARBURG_GLOWIE_BIOLOGICAL:
				EntityGrantReward(player, VarGet("BioReward"));
				break;
			case COV_WARBURG_GLOWIE_CHEMICAL:
				EntityGrantReward(player, VarGet("ChemReward"));
				break;
			case COV_WARBURG_GLOWIE_NUCLEAR:
				EntityGrantReward(player, VarGet("NukeReward"));
				break;
		}

		// give badge credit
		GiveBadgeCredit(player, kScriptBadgeTypeRockets);

		CoVWarburgFootballEndGame(player);

		// launch missle code here
		if (VarGetNumber("RocketLaunch") + (60 * VarGetNumber("LaunchTimeout")) < CurrentTime())
		{
			SetAIPriorityList("Rocket", "PL_Rocket_Launch");
			VarSetNumber("RocketLaunch", CurrentTime());
			VarSetNumber("RocketReload", 1);

			// Spawn Explosion
			{
				ENCOUNTERGROUP group = "first"; 

				group = FindEncounterGroupByRadius( 
					"coord:0.0,0.0,0.0",
					0, 
					9999, 
					"RocketExplosion", 
					1, 
 					0 );  
		 
				if (stricmp(group, "location:none") != 0) {
					Spawn( 1, "RocketExplosion", "scripts.loc/SpawnDefs/V_PvP_04_01/Rocket_Explosion_D0_V0.spawndef", 
							group, "RocketExplosion", 1, 0 );  
				}
			}
		}		

	} else {
		SendPlayerDialog(player, StringLocalize(VarGet("DeviceNoLaunchCode")));
	}

	return true;
}



void CoVWarburgFootballOnEntityDefeat(ENTITY player, ENTITY victor)
{
	int count, start;
	int j;
	char *token = NULL;
	ENTITY killer;
	ENTITY team;

	// don't need to bother with this if it isn't a player
	if (IsEntityPlayer(victor))
	{
		// check for PK
		if (IsEntityPlayer(player))
		{
			// find out which token to lose
			if (EntityHasRewardToken(player, "WarburgLaunchCode"))
			{
//				EntityRemoveRewardToken(player, "WarburgCode3");

				// removing waypoints
				ClearWaypoint(player, VarGetNumber("NuclearWaypoint"));
				ClearWaypoint(player, VarGetNumber("ChemicalWaypoint"));
				ClearWaypoint(player, VarGetNumber("BiologicalWaypoint"));
				ClearWaypoint(player, VarGetNumber("LaunchWaypoint"));

				// giving the old code back to player, to break down the launch code
				EntityGrantRewardToken(player, "WarburgCode2", 1);
				EntityGrantRewardToken(player, "WarburgCode1", 1);

				token = "WarburgLaunchCode";
				EntityRemoveTempPower(player, "SilentPowers", "V_Ore_Exposure"); // replace with real power

				// fix UI
				ScriptUIHide("Collection:BiologicalWar", player);
				ScriptUIHide("Collection:ChemicalWar", player);
				ScriptUIHide("Collection:NuclearWar", player);
				ScriptUIShow("Collection:CodeWar", player);

				// remove timer
				CoVWarburgFootballRemoveFromList(&playerList, player);
			}
			else if (EntityHasRewardToken(player, "WarburgCode3"))
			{
				token = "WarburgCode3";
			} 
			else if (EntityHasRewardToken(player, "WarburgCode2"))
			{
				token = "WarburgCode2";
			} 
			else if (EntityHasRewardToken(player, "WarburgCode1"))
			{
				token = "WarburgCode1";
			} 
			else
			{
				return; // killed player had no tokens
			} 
			EntityRemoveRewardToken(player, token);

			// get list of all players on killing team
			team = GetPlayerTeamFromPlayer(victor);
			count = NumEntitiesInTeam(team);

			// randomly order list
			start = RandomNumber(1, count);


			// go through player team list and see if they have the one we've got
			for (j = 1; j <= count; j++) {
				start++;
				if (start > count)
					start = 1;

				killer = GetEntityFromTeam(team, start);

				if (!EntityHasRewardToken(killer, "WarburgCode3") && 
					DistanceBetweenEntities(killer, player) < 600.0f )
				{
					// if they don't, give it to them
					if (EntityHasRewardToken(killer, "WarburgCode2"))
					{
						EntityGrantRewardToken(killer, "WarburgCode3", 2);
					} 
					else if (EntityHasRewardToken(killer, "WarburgCode1"))
					{
						EntityGrantRewardToken(killer, "WarburgCode2", 1);
					} 
					else
					{
						EntityGrantRewardToken(killer, "WarburgCode1", 1);
					} 
					return;  // only one token gets awarded
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
// M A P   C A L L B A C K S
/////////////////////////////////////////////////////////////////////////////////

// called when the player leaves the map
int CoVWarburgFootballOnLeaveMap(ENTITY player)
{
	CoVWarburgFootballEndGame(player);
	
	// clear contact waypoints
	if (IsEntityHero(player)) {
		ClearWaypoint(player, VarGetNumber("HeroContact"));
	} else {
		ClearWaypoint(player, VarGetNumber("VillainContact"));
	}

	return 0;
}
// called when the player enters the map
int CoVWarburgFootballOnEnterMap(ENTITY player)
{
	// set up UI
	ScriptUIShow("Collection:CodeWar", player);
		
	EntityRemoveRewardToken(player, "WarburgScientist");

	// set contact waypoints
	if (IsEntityHero(player)) {
		SetWaypoint(player, VarGetNumber("HeroContact"));
	} else {
		SetWaypoint(player, VarGetNumber("VillainContact"));
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// M A I N    F U N C T I O N 
/////////////////////////////////////////////////////////////////////////////////

void CoVWarburgFootballScript(void)
{
	ENCOUNTERGROUP group = "first"; 
	static GLOWIEDEF def[COV_WARBURG_GLOWIE_NUM] = { NULL, NULL, NULL, NULL };
	char buf[128];


	INITIAL_STATE

		ON_ENTRY 
		{
			//////////////////////////////////////////////////////////////////////////
			// put stuff that should run once on initialization here
			VarSetNumber("RocketLaunch", CurrentTime());

			// setup timers for contact
			TimerSet("HeroContactTalk", 0.25f);
			TimerSet("VillainContactTalk", 0.25f);

			//////////////////////////////////////////////////////////////////////////
			// create UI

			ScriptUITitle(ONENTER("WarburgTitleUI"), "EventName", "EventInfo");
			ScriptUICollectItems(ONENTER("CodeCollectUI"), 3, "Token:WarburgCode1", "Code 1", "sp_icon_code_01.tga",
															 "Token:WarburgCode2", "Code 2", "sp_icon_code_02.tga",
															 "Token:WarburgCode3", "Code 3", "sp_icon_code_03.tga");

			// create B UI
			ScriptUITimer("LaunchTimerUI", PERPLAYER("TimeToLaunch"), "TimeRemaining", "TimeRemainingToolTip");
			ScriptUICollectItems("BiologicalLaunchUI", 1, "Token:WarburgLaunchCode", "CodeBInfo", COV_WARBURG_LAUNCHB_IMAGES);

			// create C UI
			ScriptUICollectItems("ChemicalLaunchUI", 1, "Token:WarburgLaunchCode", "CodeCInfo", COV_WARBURG_LAUNCHC_IMAGES);

			// create N UI
			ScriptUICollectItems("NuclearLaunchUI", 1, "Token:WarburgLaunchCode", "CodeNInfo", COV_WARBURG_LAUNCHN_IMAGES);

			ScriptUICreateCollection("CodeWar", 2, "WarburgTitleUI", "CodeCollectUI");
			ScriptUICreateCollection("BiologicalWar", 3, "WarburgTitleUI", "LaunchTimerUI", "BiologicalLaunchUI");
			ScriptUICreateCollection("ChemicalWar", 3, "WarburgTitleUI", "LaunchTimerUI", "ChemicalLaunchUI");
			ScriptUICreateCollection("NuclearWar", 3, "WarburgTitleUI", "LaunchTimerUI", "NuclearLaunchUI");

			//////////////////////////////////////////////////////////////////////////
			// Setting up the player lists
			//////////////////////////////////////////////////////////////////////////
			eaCreate(&playerList);

			//////////////////////////////////////////////////////////////////////////
			// Setup contacts

			// villain contact
			group = FindEncounterGroupByRadius( 
				"coord:0.0,0.0,0.0",
				0, 
				9999, 
				"NPC_Villain_Contact", 
				1, 
 				0 );  
			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Contacts", "scripts.loc/SpawnDefs/V_PvP_04_01/NPC_Contacts_D0_V1.spawndef", group, "NPC_Villain_Contact", 1, 0 );  

				// add on click handler
				OnScriptedContactInteract(CoVWarburgOnVillainContactClick, ActorFromEncounterPos(group, 1));

				// map markers
				VarSetNumber("VillainContact", CreateWaypoint(EncounterPos(group, 1), VarGet("VillainContactName"), "map_enticon_contact", NULL, false, false, -1.0f));
			}

			// hero contact
			group = FindEncounterGroupByRadius( 
				"coord:0.0,0.0,0.0",
				0, 
				9999, 
				"NPC_Hero_Contact", 
				1, 
 				0 );  
			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "Contacts", "scripts.loc/SpawnDefs/V_PvP_04_01/NPC_Contacts_D0_V0.spawndef", group, "NPC_Hero_Contact", 1, 0 );  

				// add on click handler
				OnScriptedContactInteract(CoVWarburgOnHeroContactClick, ActorFromEncounterPos(group, 1));

				// map markers
				VarSetNumber("HeroContact", CreateWaypoint(EncounterPos(group, 1), VarGet("HeroContactName"), "map_enticon_contact", NULL, false, false, -1.0f));
			}

			// set up glowies
			if (def[COV_WARBURG_GLOWIE_NUCLEAR] == NULL) {
				def[COV_WARBURG_GLOWIE_NUCLEAR] = GlowieCreateDef(StringLocalize(VarGet("NuclearDeviceName")), VarGet("NuclearDeviceModel"), 
																	VarGet("DeviceInteract"), VarGet("DeviceInterrupt"), 
																	VarGet("DeviceComplete"), VarGet("DeviceInteractBar"), 
																	VarGetNumber("DeviceInteractTime"));
				def[COV_WARBURG_GLOWIE_BIOLOGICAL] = GlowieCreateDef(StringLocalize(VarGet("BiologicalDeviceName")), VarGet("BiologicalDeviceModel"), 
																	VarGet("DeviceInteract"), VarGet("DeviceInterrupt"), 
																	VarGet("DeviceComplete"), VarGet("DeviceInteractBar"), 
																	VarGetNumber("DeviceInteractTime"));
				def[COV_WARBURG_GLOWIE_CHEMICAL] = GlowieCreateDef(StringLocalize(VarGet("ChemicalDeviceName")), VarGet("ChemicalDeviceModel"), 
																	VarGet("DeviceInteract"), VarGet("DeviceInterrupt"), 
																	VarGet("DeviceComplete"), VarGet("DeviceInteractBar"), 
																	VarGetNumber("DeviceInteractTime"));
				def[COV_WARBURG_GLOWIE_LAUNCH] = GlowieCreateDef(StringLocalize(VarGet("LaunchDeviceName")), VarGet("LaunchDeviceModel"), 
																	VarGet("DeviceInteract"), VarGet("DeviceInterrupt"), 
																	VarGet("DeviceComplete"), VarGet("DeviceInteractBar"), 
																	VarGetNumber("DeviceInteractTime"));
			}


			// Create device holders
			sprintf(buf, "marker:%s", VarGet("NuclearDeviceLocation"));
			GlowiePlace(def[COV_WARBURG_GLOWIE_NUCLEAR], buf, true, CoVWarburgDeviceInteract, CoVWarburgNuclearComplete);
			VarSetNumber("NuclearWaypoint", CreateWaypoint(buf, VarGet("NuclearDeviceName"), COV_WARBURG_LAUNCHN_WPICON, NULL, false, false, -1.0f));

			sprintf(buf, "marker:%s", VarGet("ChemicalDeviceLocation"));
			GlowiePlace(def[COV_WARBURG_GLOWIE_CHEMICAL], buf, true, CoVWarburgDeviceInteract, CoVWarburgChemicalComplete);
			VarSetNumber("ChemicalWaypoint", CreateWaypoint(buf, VarGet("ChemicalDeviceName"), COV_WARBURG_LAUNCHC_WPICON, NULL, false, false, -1.0f));

			sprintf(buf, "marker:%s", VarGet("BiologicalDeviceLocation"));
			GlowiePlace(def[COV_WARBURG_GLOWIE_BIOLOGICAL], buf, true, CoVWarburgDeviceInteract, CoVWarburgBiologicalComplete);
			VarSetNumber("BiologicalWaypoint", CreateWaypoint(buf, VarGet("BiologicalDeviceName"), COV_WARBURG_LAUNCHB_WPICON, NULL, false, false, -1.0f));

			sprintf(buf, "marker:%s", VarGet("LaunchDeviceLocation"));
			GlowiePlace(def[COV_WARBURG_GLOWIE_LAUNCH], buf, true, CoVWarburgLaunchInteract, CoVWarburgLaunchComplete);
			VarSetNumber("LaunchWaypoint", CreateWaypoint(buf, VarGet("LaunchDeviceName"), COV_WARBURG_LAUNCH_WPICON, NULL, true, false, -1.0f));


			// Spawn Rocket
			{
				ENCOUNTERGROUP group = "first"; 

				group = FindEncounterGroupByRadius( 
					"coord:0.0,0.0,0.0",
					0, 
					9999, 
					"Rocket", 
					1, 
 					0 );  
		 
				if (stricmp(group, "location:none") != 0) {
					Spawn( 1, "Rocket", "scripts.loc/SpawnDefs/V_PvP_04_01/Rocket_Moving_D0_V0.spawndef", 
							group, "Rocket", 1, 0 );  
				}
			}


			//////////////////////////////////////////////////////////////////////////
			///Script Hooks
			OnEntityDefeated ( CoVWarburgFootballOnEntityDefeat );

			OnPlayerExitingMap( CoVWarburgFootballOnLeaveMap );

			OnPlayerEnteringMap( CoVWarburgFootballOnEnterMap );

		}
		END_ENTRY

		// check timers
		CoVWarburgFootballUpdateTimers();

		// checking for waypoints
		{
			int i, count = NumEntitiesInTeam(ALL_PLAYERS);

			for (i = 1; i <= count; i++)
			{
				ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);

				if (EntityHasRewardToken(player, "WarburgCode3") && 
					EntityGetRewardToken(player, "WarburgCode3") > 1)
				{
					EntityGrantRewardToken(player, "WarburgCode3", 1);
					SetWaypoint(player, VarGetNumber("NuclearWaypoint"));
					SetWaypoint(player, VarGetNumber("ChemicalWaypoint"));
					SetWaypoint(player, VarGetNumber("BiologicalWaypoint"));
				}
			}
		}

		// checking for rocket reload
		// launch missle code here
		if (VarGetNumber("RocketReload") && VarGetNumber("RocketLaunch") + (60 * VarGetNumber("LaunchTimeout")) - 20 < CurrentTime())
		{
			SetAIPriorityList("Rocket", "PL_Rocket_Reload");
			VarSetNumber("RocketReload", 0);
		}		


	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventEnd") ////////////////////////////////////////////////////////

		// destroy waypoints
		DestroyWaypoint(VarGetNumber("HeroContact"));
		DestroyWaypoint(VarGetNumber("VillainContact"));
		DestroyWaypoint(VarGetNumber("NuclearWaypoint"));
		DestroyWaypoint(VarGetNumber("ChemicalWaypoint"));
		DestroyWaypoint(VarGetNumber("BiologicalWaypoint"));
		DestroyWaypoint(VarGetNumber("LaunchWaypoint"));

		// clean up lists
		eaDestroyEx(&playerList, NULL);

		// clean up handlers
		OnPlayerExitingMap( NULL );
		OnPlayerEnteringMap( NULL );
		OnPlayerJoinsTeam( NULL );
		OnPlayerLeavesTeam(	NULL );
		OnTeamRewarded ( NULL );

		EndScript();

	END_STATES

	ON_SIGNAL("End")
		EndScript();
	END_SIGNAL

	ON_SIGNAL("Fragments")
	{
		int i, count = NumEntitiesInTeam(ALL_PLAYERS);

		for (i = 1; i <= count; i++)
		{
			ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);

			EntityGrantRewardToken(player, "WarburgCode1", 1);
			EntityGrantRewardToken(player, "WarburgCode2", 1);
			EntityGrantRewardToken(player, "WarburgCode3", 2); 
		}
	}
	END_SIGNAL

	ON_SIGNAL("Nuke")
	{
		int i, count = NumEntitiesInTeam(ALL_PLAYERS);

		for (i = 1; i <= count; i++)
		{
			ENTITY player = GetEntityFromTeam(ALL_PLAYERS, i);

			CoVWarburgNuclearComplete(player, player);
		}
	}
	END_SIGNAL


	ON_SIGNAL("Launch")
		SetAIPriorityList("Rocket", "PL_Rocket_Launch");
		// Spawn Explosion
		{
			ENCOUNTERGROUP group = "first"; 

			group = FindEncounterGroupByRadius( 
				"coord:0.0,0.0,0.0",
				0, 
				9999, 
				"RocketExplosion", 
				1, 
 				0 );  
		 
			if (stricmp(group, "location:none") != 0) {
				Spawn( 1, "RocketExplosion", "scripts.loc/SpawnDefs/V_PvP_04_01/Rocket_Explosion_D0_V0.spawndef", 
						group, "RocketExplosion", 1, 0 );  
				DetachSpawn("RocketExplosion");
			}
		}
	END_SIGNAL

	ON_SIGNAL("Reload")
		SetAIPriorityList("Rocket", "PL_Rocket_Reload");
	END_SIGNAL

	ON_SIGNAL("Restart")
		SET_STATE( STATE_INITIAL );
	END_SIGNAL

	ON_STOPSIGNAL
		EndScript();
	END_SIGNAL
}

void CoVWarburgFootballInit()
{
	SetScriptName( "CoVWarburgFootball" );
	SetScriptFunc( CoVWarburgFootballScript );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptSignal( "End Event", "EndScript" );		
	SetScriptSignal( "Launch", "Launch" );		
	SetScriptSignal( "Restart", "Restart" );		
	SetScriptSignal( "Reload", "Reload" );		

	SetScriptSignal( "Fragments", "Fragments" );		
	SetScriptSignal( "Nuke", "Nuke" );		


	SetScriptVar( "EventName",								"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "EventInfo",								"",			SP_DONTDISPLAYDEBUG );

	SetScriptVar( "HeroContactName",						"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactName",						"",			SP_DONTDISPLAYDEBUG );

	SetScriptVar( "FragmentInfo",							"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "FragmentToolTip",						"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "CodeBInfo",								"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "CodeBToolTip",							"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "CodeCInfo",								"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "CodeCToolTip",							"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "CodeNInfo",								"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "CodeNToolTip",							"",			SP_DONTDISPLAYDEBUG );

	SetScriptVar( "NuclearDeviceName",						"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NuclearDeviceModel",						"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NuclearDeviceLocation",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NuclearDeviceComplete",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "NukeReward",								"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BiologicalDeviceName",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BiologicalDeviceModel",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BiologicalDeviceLocation",				"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BiologicalDeviceComplete",				"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "BioReward",								"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ChemicalDeviceName",						"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ChemicalDeviceModel",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ChemicalDeviceLocation",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ChemicalDeviceComplete",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ChemReward",								"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "LaunchDeviceName",						"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "LaunchDeviceModel",						"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "LaunchDeviceLocation",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "LaunchTimeout",							"",			SP_DONTDISPLAYDEBUG );

	SetScriptVar( "DeviceInteract",							"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "DeviceInteractBar",						"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "DeviceInteractTime",						"5",		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "DeviceInterrupt",						"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "DeviceComplete",							"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "DeviceNotEnoughCodes",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "DeviceNoLaunchCode",						"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "DeviceAlreadyLauched",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "AlreadyHaveCode",						"",			SP_DONTDISPLAYDEBUG );

	SetScriptVar( "TimerLength",							"5",		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TimeRemaining",							"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TimeRemainingToolTip",					"",			SP_DONTDISPLAYDEBUG );
	SetScriptVar( "TimeUpString",							"",			SP_DONTDISPLAYDEBUG );

	SetScriptVar( "PlayingGameToken",						"",			SP_DONTDISPLAYDEBUG );

	SetScriptVar( "HeroContactStartSpeech",					NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG);
	SetScriptVar( "HeroContactAcceptResponse",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroContactDeclineResponse",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroContactInstructionResponse",			NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroContactAcceptSpeech",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroContactInstructionSpeech",			NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroContactReturnSpeech",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactStartSpeech",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactAcceptResponse",			NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactDeclineResponse",			NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactInstructionResponse",		NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactAcceptSpeech",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactInstructionSpeech",		NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainContactReturnSpeech",				NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );

	SetScriptVar( "ContactBackText",						NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "ContactExitText",						NULL,		SP_MULTIVALUE | SP_DONTDISPLAYDEBUG );
	SetScriptVar( "VillainGoAway",							NULL,		SP_DONTDISPLAYDEBUG );
	SetScriptVar( "HeroGoAway",								NULL,		SP_DONTDISPLAYDEBUG );

	SetScriptVar( "ProxySalvage",							NULL,		SP_DONTDISPLAYDEBUG );

}
