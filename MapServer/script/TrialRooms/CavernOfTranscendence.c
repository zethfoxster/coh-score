// MISSION SCRIPT

// This script runs the overall situation in the Cavern of Transcendence
//

//
// Add description here!
//

#include "utils.h"

#include "scriptutil.h"

// Parameters:
//		STATUS_REACTOR
//		STATUS_INITIAL

#define CAVERN_TEAM						"Magmen"
#define CAVERN_TEAM_MIN					60

typedef struct CavernOuterSpawnData {
	char	spawndef[32];
	int		useInner;
} CavernOuterSpawnData;

#define CAVERN_INNER_SPAWN_COUNT		5
char *CavernInnerSpawns[CAVERN_INNER_SPAWN_COUNT] =
{
	"StrikeTeams\\Trial_Rooms\\spawndefs\\Trialroom_01_02\\Trial_Magmite_around_D10_V0.spawndef",
	"StrikeTeams\\Trial_Rooms\\spawndefs\\Trialroom_01_02\\Trial_Magmite_around_D10_V1.spawndef",
	"StrikeTeams\\Trial_Rooms\\spawndefs\\Trialroom_01_02\\Trial_Magmite_around_D10_V2.spawndef",
	"StrikeTeams\\Trial_Rooms\\spawndefs\\Trialroom_01_02\\Trial_Magmite_around_D10_V3.spawndef",
	"StrikeTeams\\Trial_Rooms\\spawndefs\\Trialroom_01_02\\Trial_Magmite_around_D10_V4.spawndef",
};

#define CAVERN_OUTER_SPAWN_COUNT		5
char *CavernOuterSpawns[CAVERN_OUTER_SPAWN_COUNT] =
{
	"StrikeTeams\\Trial_Rooms\\spawndefs\\Trialroom_01_02\\Trial_Magmite_Phalanx_D10_V0.spawndef",
	"StrikeTeams\\Trial_Rooms\\spawndefs\\Trialroom_01_02\\Trial_Magmite_Phalanx_D10_V1.spawndef",
	"StrikeTeams\\Trial_Rooms\\spawndefs\\Trialroom_01_02\\Trial_Magmite_Phalanx_D10_V2.spawndef",
	"StrikeTeams\\Trial_Rooms\\spawndefs\\Trialroom_01_02\\Trial_Magmite_Phalanx_D10_V3.spawndef",
	"StrikeTeams\\Trial_Rooms\\spawndefs\\Trialroom_01_02\\Trial_Magmite_Phalanx_D10_V4.spawndef",
};

#define CAVERN_OUTER_LOC_COUNT			8
CavernOuterSpawnData CavernOuterSpawnsData[CAVERN_OUTER_LOC_COUNT] =
{
	{ "EncounterGroup:Outer1", 0 },
	{ "EncounterGroup:Outer4", 0 },
	{ "EncounterGroup:Outer8", 1 },
	{ "EncounterGroup:Outer6", 0 },
	{ "EncounterGroup:Outer3", 0 },
	{ "EncounterGroup:Outer7", 1 },
	{ "EncounterGroup:Outer2", 0 },
	{ "EncounterGroup:Outer5", 0 }
};

// returns the next spawn location then increments the counter for the next spawn.  Wraps around
//		if we've gone past the bottom of the list
int CavernNextSpawnLoc() 
{
	int retval = VarGetNumber("OuterSpawn");
	int var = retval + 1;
	if (var >= CAVERN_OUTER_LOC_COUNT) {
		var = 0;
	}
	VarSetNumber("OuterSpawn", var);
	return retval;
}

static void CavernOfTranscendenceSpawnInner(void)
{ 
	char	innerSpawnLoc[128];
	int		retval = VarGetNumber("InnerSpawn");

	if (VarGetNumber("InnerSpawn") < 8) {
		sprintf(innerSpawnLoc, "EncounterGroup:Inner%d", retval+1);
	//	DebugString(innerSpawnLoc);
		Spawn(1, CAVERN_TEAM, CavernInnerSpawns[rand()%CAVERN_INNER_SPAWN_COUNT], 
				innerSpawnLoc, NULL, MissionLevel(), MissionNumHeroes());
		SetAIPriorityList(CAVERN_TEAM, "PL_UseAIConfig");
		DetachSpawn(CAVERN_TEAM);
		VarSetNumber("InnerSpawn", retval + 1);
	}
}

static void CavernOfTranscendenceSpawnOuter(void)
{ 
//	char	innerSpawnLoc[128];
	int		spawnLoc;

	// spawn count check... 
	if (VarGetNumber("InnerSpawn") > 7 && NumEntitiesInTeam(CAVERN_TEAM) < CAVERN_TEAM_MIN)
	{
//		sprintf(innerSpawnLoc, "Spawning Invaders Num = %d", NumEntitiesInTeam(CAVERN_TEAM));
//		DebugString(innerSpawnLoc);

		// Cavern Need Magmen!
		spawnLoc = CavernNextSpawnLoc();
		if (CavernOuterSpawnsData[spawnLoc].useInner) {
			Spawn(1, CAVERN_TEAM, CavernInnerSpawns[rand()%CAVERN_INNER_SPAWN_COUNT], 
					CavernOuterSpawnsData[spawnLoc].spawndef, NULL, MissionLevel(), MissionNumHeroes());
			SetAIPriorityList(CAVERN_TEAM, "PL_UseAIConfig");
			DetachSpawn(CAVERN_TEAM);
		} else {
			Spawn(1, CAVERN_TEAM, CavernOuterSpawns[rand()%CAVERN_OUTER_SPAWN_COUNT], 
					CavernOuterSpawnsData[spawnLoc].spawndef, NULL, MissionLevel(), MissionNumHeroes());
			SetAIPriorityList(CAVERN_TEAM, "PL_UseAIConfig");
			DetachSpawn(CAVERN_TEAM);
		}
	}
}

void CavernOfTranscendenceMission(void)
{

	INITIAL_STATE
		ON_ENTRY
			// spawn inner circle
			VarSetNumber("InnerSpawn", 0);
			VarSetNumber("OuterSpawn", rand() % CAVERN_OUTER_SPAWN_COUNT);
			SetMissionStatus(VarGet("STATUS_TOUCH"));
		END_ENTRY

		DoEvery("SpawnInner", 0.5f, CavernOfTranscendenceSpawnInner);
		DoEvery("SpawnOuter", 0.6f, CavernOfTranscendenceSpawnOuter);


		if (ObjectiveIsComplete("Obelisks")) {
			SET_STATE("Rescue")
		}

	STATE("Rescue")
		ON_ENTRY
			SetMissionStatus(VarGet("STATUS_RESCUE"));
		END_ENTRY

		if (ObjectiveIsComplete("Sam")) {
			SetMissionComplete(1);
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

