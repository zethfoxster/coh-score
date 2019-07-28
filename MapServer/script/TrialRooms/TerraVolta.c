// MISSION SCRIPT

// This script runs the overall situation in Terra Volta
//
// We wait until the players have reached the entryway outside the main reactor.
// Then we start spawning bad guys at regular intervals until the players have
// repelled everybody.

#include "scriptutil.h"

// Parameters:
//		STATUS_REACTOR
//		STATUS_INITIAL

#define TV_SPAWNDEF_BOSSES			"StrikeTeams/Trial_Rooms/spawndefs/Trialroom_03_01/TV_Core_SR_D01_V1.spawndef"
#define TV_SPAWNDEF_MIXED			"StrikeTeams/Trial_Rooms/spawndefs/Trialroom_03_01/TV_Core_SR_D01_V0.spawndef"
#define TV_SPAWNDEF_MINIONS			"StrikeTeams/Trial_Rooms/spawndefs/Trialroom_03_01/TV_Core_SR_D01_V2.spawndef"
#define TV_SPAWNDEF_SUPER_MINIONS	"StrikeTeams/Trial_Rooms/spawndefs/Trialroom_03_01/TV_Core_SR_D01_V3.spawndef"

#define TV_MAX_WAVE					9

void TVNextWave() 
{
	int var = VarGetNumber("WaveCount");
	VarSetNumber("WaveCount", var + 1);
}

int TVLevel() 
{
	int var = MissionLevel();

	if (var < 45)
		var--;
	else if (var < 32)
		var -= 2;

	return var;
}

static void spawnInvaders(void)
{ 
  
	switch (VarGetNumber("WaveCount")) {
		case 0:
			if ((STATE_MINS > 1.0) || (NumEntitiesInTeam("ReactorAttackers") < 1)) {
				// spawn and move to next wave
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MINIONS, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
				DetachSpawn("ReactorAttackers");
				TVNextWave();
			}
			break;
		case 1:
			if ((STATE_MINS > 2.0) || (NumEntitiesInTeam("ReactorAttackers") < 1)) {
				// spawn and move to next wave
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MINIONS, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
				Spawn(1, "ReactorAttackers", TV_SPAWNDEF_MIXED, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
				DetachSpawn("ReactorAttackers");
				TVNextWave();
			}
			break;
		case 2:
			if ((STATE_MINS > 5.0) || (NumEntitiesInTeam("ReactorAttackers") < 1)) {
				// spawn and move to next wave
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MINIONS, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
				Spawn(1, "ReactorAttackers", TV_SPAWNDEF_BOSSES, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
				DetachSpawn("ReactorAttackers");
				TVNextWave();
			}
			break;
		case 3:
			if ((STATE_MINS > 8.0) || (NumEntitiesInTeam("ReactorAttackers") < 1)) {
				// spawn and move to next wave
				Spawn(3, "ReactorAttackers", TV_SPAWNDEF_MIXED, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
				DetachSpawn("ReactorAttackers");
				TVNextWave();
			}
			break;
		case 4:
			if ((STATE_MINS > 11.0) || (NumEntitiesInTeam("ReactorAttackers") < 1)) {
				// spawn and move to next wave
				Spawn(3, "ReactorAttackers", TV_SPAWNDEF_SUPER_MINIONS, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
				DetachSpawn("ReactorAttackers");
				TVNextWave();
			}
			break;
		case 5:
			if ((STATE_MINS > 14.0) || (NumEntitiesInTeam("ReactorAttackers") < 1)) {
				// spawn and move to next wave
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MINIONS, "EncounterGroup:TV_Wave", NULL, TVLevel()+1, MissionNumHeroes());
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MIXED, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
				DetachSpawn("ReactorAttackers");
				TVNextWave();
			}
			break;
		case 6:
			if ((STATE_MINS > 17.0) || (NumEntitiesInTeam("ReactorAttackers") < 1)) {
				// spawn and move to next wave
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MINIONS, "EncounterGroup:TV_Wave", NULL, TVLevel()+1, MissionNumHeroes());
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MIXED, "EncounterGroup:TV_Wave", NULL, TVLevel()+1, MissionNumHeroes());
				Spawn(1, "ReactorAttackers", TV_SPAWNDEF_BOSSES, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
				DetachSpawn("ReactorAttackers");
				TVNextWave();
			}
			break;
		case 7:
			if ((STATE_MINS > 20.0) || (NumEntitiesInTeam("ReactorAttackers") < 1)) {
				// spawn and move to next wave
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MINIONS, "EncounterGroup:TV_Wave", NULL, TVLevel()+2, MissionNumHeroes());
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MIXED, "EncounterGroup:TV_Wave", NULL, TVLevel()+1, MissionNumHeroes());
				Spawn(1, "ReactorAttackers", TV_SPAWNDEF_BOSSES, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
				DetachSpawn("ReactorAttackers");
				TVNextWave();
			}
			break;
		case 8:
			if ((STATE_MINS > 25.0) || (NumEntitiesInTeam("ReactorAttackers") < 1)) {
				// spawn and move to next wave
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MINIONS, "EncounterGroup:TV_Wave", NULL, TVLevel()+2, MissionNumHeroes());
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MIXED, "EncounterGroup:TV_Wave", NULL, TVLevel()+1, MissionNumHeroes());
				Spawn(1, "ReactorAttackers", TV_SPAWNDEF_BOSSES, "EncounterGroup:TV_Wave", NULL, TVLevel()+1, MissionNumHeroes());
				DetachSpawn("ReactorAttackers");
				TVNextWave();
			}
			break;
		case TV_MAX_WAVE:
			if ((STATE_MINS > 30.0) || (NumEntitiesInTeam("ReactorAttackers") < 1)) {
				// spawn and move to next wave
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MINIONS, "EncounterGroup:TV_Wave", NULL, TVLevel()+2, MissionNumHeroes());
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_MIXED, "EncounterGroup:TV_Wave", NULL, TVLevel()+2, MissionNumHeroes());
				Spawn(2, "ReactorAttackers", TV_SPAWNDEF_BOSSES, "EncounterGroup:TV_Wave", NULL, TVLevel()+1, MissionNumHeroes());
				DetachSpawn("ReactorAttackers");
				TVNextWave();
			}
			break;
	}
//	Spawn(1, "ReactorAttackers", "spawndefs/missions/encounters/SkyRaiders_V0.spawndef", "EncounterGroup:ReactorAttack2");
//	DetachSpawn("ReactorAttackers");
//	Spawn(1, "ReactorAttackers", "spawndefs/missions/encounters/SkyRaiders_V0.spawndef", "EncounterGroup:ReactorAttack3");
//	DetachSpawn("ReactorAttackers");
//	Spawn(1, "ReactorAttackers", "spawndefs/missions/encounters/SkyRaiders_V2.spawndef", "EncounterGroup:ReactorAttack4");
//	DetachSpawn("ReactorAttackers");
//	Spawn(1, "ReactorAttackers", "spawndefs/missions/encounters/SkyRaiders_V1.spawndef", "EncounterGroup:ReactorAttack5");
//	DetachSpawn("ReactorAttackers");
	DebugString("Spawning Invaders");
}

static void showState(void)
{
	DebugString(StringAdd("In state: ", VarGet(STATE_VAR)));
}

void TerraVoltaMission(void)
{
	INITIAL_STATE
		ON_ENTRY
			SetMissionStatus(VarGet("STATUS_GUARD"));
			DoEvery("Heartbeat", 0.01f, showState);
		END_ENTRY

		if (ObjectiveIsComplete("SaveGuard")) {
			SET_STATE("Story")
		}

		// failsafe switch
		if (NumEntitiesInArea("MissionRoom:3", ALL_PLAYERS))
		{
			DebugString("Player in core");
			SET_STATE("Core")
		}

		if (ObjectiveHasFailed("ProtectCore")) 
		{	
			SET_STATE("Failed")
		}

	STATE("Story")
		ON_ENTRY
			SetMissionStatus(VarGet("STATUS_STORY"));
		END_ENTRY

		if (ObjectiveIsComplete("DiscoverStory")) {
			SET_STATE("Scientists")
		}

		// failsafe switch
		if (NumEntitiesInArea("MissionRoom:3", ALL_PLAYERS))
		{
			DebugString("Player in core");
			SET_STATE("Core")
		}

		if (ObjectiveHasFailed("ProtectCore")) 
		{	
			SET_STATE("Failed")
		}

	STATE("Scientists")
		ON_ENTRY
			SetMissionStatus(VarGet("STATUS_SCIENTISTS"));
		END_ENTRY

		if (ObjectiveIsComplete("SaveScientist")) {
			SET_STATE("Bomb")
		}

		// failsafe switch
		if (NumEntitiesInArea("MissionRoom:3", ALL_PLAYERS))
		{
			DebugString("Player in core");
			SET_STATE("Core")
		}

		if (ObjectiveHasFailed("ProtectCore")) 
		{	
			SET_STATE("Failed")
		}

	STATE("Bomb")
		ON_ENTRY
			SetMissionStatus(VarGet("STATUS_BOMB"));
		END_ENTRY

		if (ObjectiveIsComplete("Bombs")) {
			SET_STATE("Corridor")
		}

		// failsafe switch
		if (NumEntitiesInArea("MissionRoom:3", ALL_PLAYERS))
		{
			DebugString("Player in core");
			SET_STATE("Core")
		}

		if (ObjectiveHasFailed("ProtectCore")) 
		{	
			SET_STATE("Failed")
		}

	STATE("Corridor")
		ON_ENTRY
			SetMissionStatus(VarGet("STATUS_CORRIDOR"));
		END_ENTRY

		if (NumEntitiesInArea("MissionRoom:3", ALL_PLAYERS))
		{
			DebugString("Player in core");
			SET_STATE("Core")
		}

		if (ObjectiveHasFailed("ProtectCore")) 
		{	
			SET_STATE("Failed")
		}

	STATE("Core")
		ON_ENTRY
			SetMissionStatus(VarGet("STATUS_CORE"));
			VarSetNumber("WaveCount", 0);
			Spawn(1, "ReactorAttackers", TV_SPAWNDEF_MINIONS, "EncounterGroup:TV_Wave", NULL, TVLevel(), MissionNumHeroes());
		END_ENTRY

		DoEvery("SpawnInvaders", 0.1f, spawnInvaders);
		
		if (VarGetNumber("WaveCount") > TV_MAX_WAVE && NumEntitiesInTeam("ReactorAttackers") < 1)
		{
			SetMissionComplete(1);
			SetMissionStatus("");
			EndScript();
		}

		if (ObjectiveHasFailed("ProtectCore")) 
		{	
			SET_STATE("Failed")
		}

		if (NumEntitiesInArea("MissionRoom:2", ALL_PLAYERS))
		{
			DebugString("Player in hall");
		}

	STATE("Failed")
		if (STATE_MINS > 0.5) {
			SetMissionComplete(0);
			SetMissionStatus("");
			EndScript();	
		}

	END_STATES


	ON_SIGNAL("WinGame")
			SetMissionComplete(1);
			SetMissionStatus("");
			EndScript();
	END_SIGNAL

	ON_SIGNAL("LoseGame")
			SetMissionComplete(0);
			SetMissionStatus("");
			EndScript();
	END_SIGNAL

}

