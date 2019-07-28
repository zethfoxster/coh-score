// MISSION SCRIPT

// This script runs the overall situation in Eden 
// All it does is change the mission objective strings

#include "scriptutil.h"

void EdenMission(void)
{
	INITIAL_STATE
		ON_ENTRY
			SetMissionStatus(VarGet("DESTROY_ROCK_WALL"));
		END_ENTRY

		if (ObjectiveIsComplete("RockWall")) {
			SET_STATE("DestroyMoldWall")
		}

	STATE("DestroyMoldWall")
		ON_ENTRY
			SetMissionStatus(VarGet("DESTROY_MOLD_WALL"));
		END_ENTRY

		if (ObjectiveIsComplete("MoldWall")) {
			SET_STATE("DefeatCrystalTitan")
		}

	STATE("DefeatCrystalTitan")
		ON_ENTRY
			SetMissionStatus(VarGet("DEFEAT_CRYSTAL_TITAN"));
		END_ENTRY

		if (ObjectiveIsComplete("CrystalTitan")) {
			SET_STATE("FreeHeroes")
		}

	STATE("FreeHeroes")
		ON_ENTRY
			SetMissionStatus(VarGet("FREE_HEROES"));
		END_ENTRY

	END_STATES
}

