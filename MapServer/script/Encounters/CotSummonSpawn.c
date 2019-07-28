// SPAWNDEF SCRIPT

// In this script the CoT are doing a demon summoning.  After the encounter
// goes Inactive, a timer starts.  If the key summoner is not interrupted
// during this time, a demon is summoned and kills the summoner.

// Parameters:
//    COTSUMMON_TIME - time in minutes it takes to summon the demon
//    COTSUMMON_VILLAIN - the particular demon that will be summoned
//	  COTSUMMON_LEVEL - the level the demon will be created at - OPTIONAL
//	  COTSUMMON_NAME - the name of the demon
//	  COTSUMMON_SAYBEFORE - what the summoner says when he starts summoning
//	  COTSUMMON_SAYAFTER - what the summoner says after summoning

#include "scriptutil.h"

void CotSummonSpawn()
{
	INITIAL_STATE
		if (EncounterState(MyEncounter()) >= ENCOUNTER_INACTIVE)
			SET_STATE("Summoning");

	STATE("Summoning")
		STRING summoner = ActorFromEncounterPos(MyEncounter(), ENCOUNTER_KEYVILLAIN);
		ON_ENTRY
			AddAIBehavior(summoner, "DoNothing(AnimBit(COTSummon),CombatOverride(Defensive))");
			Say(summoner, VarGet("COTSUMMON_SAYBEFORE"), 2);
			TimerSet("SummonTime", VarGetFraction("COTSUMMON_TIME"));
		END_ENTRY

		if (EntityExists(GetAIWasHitBy(summoner)) || EntityExists(summoner) == 0)
			EndScript();
		else if (TimerElapsed("SummonTime"))
			SET_STATE("KillSummoner")

	STATE("KillSummoner")
		// if the demon get summoned have it kill the summoner first
		STRING demonpos, summoner;
		int level;

		summoner = ActorFromEncounterPos(MyEncounter(), ENCOUNTER_KEYVILLAIN);
		ON_ENTRY
			demonpos = EncounterPos(MyEncounter(), ENCOUNTER_KEYVICTIM);

			if (VarIsEmpty("COTSUMMON_LEVEL"))
				level = EncounterLevel(MyEncounter());
			else
				level = VarGetNumber("COTSUMMON_LEVEL");

			CreateVillain("Demon", VarGet("COTSUMMON_VILLAIN"), level, VarGet("COTSUMMON_NAME"), demonpos);
			SetAITeamVillain("Demon");
			SetAIAttackTarget("Demon", summoner, HIGH_PRIORITY);
			SetEncounterComplete(MyEncounter(), 0); // the encounter is a failure if the demon gets summoned
			AttachSpawn(MyEncounter(), "Demon");
		END_ENTRY

		if (EntityExists(summoner) == 0)
			SET_STATE("FreeDemon")
		else if (EntityExists("Demon") == 0)
			EndScript();

	STATE("FreeDemon")
		AddAIBehavior(MyEncounter(), "Wander");
		EndScript();

	END_STATES
}
