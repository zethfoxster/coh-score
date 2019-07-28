#include "entity.h"
#include "entPlayer.h"
#include "MARTY.h"
#include "timing.h"
#include "dbcomm.h"
#include "earray.h"
#include "power_system.h"
#include "structDefines.h"
#include "character_level.h"
#include "log.h"
#include "mission.h"
#include "teamCommon.h"
#include "cmdserver.h"
#include "buddy_server.h"

MARTYMods g_MARTYMods;

StaticDefineInt ParseMARTYExperienceTypes[] =
{
	DEFINE_INT
	{"Architect",MARTY_ExperienceArchitect},
	{"Combined",MARTY_ExperienceCombined},
	DEFINE_END
};

typedef enum MARTY_Actions
{
	MARTY_Action_Log,
	MARTY_Action_Throttle,
}MARTY_Actions;

StaticDefineInt ParseMARTYActions[] =
{
	DEFINE_INT
	{"Log",MARTY_Action_Log},
	{"Throttle",MARTY_Action_Throttle},
	DEFINE_END
};

void HandleThrottleCheck(MARTY_RewardData *marty, MARTYExpMods **actionList, int xp, int oldXpSum, int charLevel, int numMin, Entity *e)
{
	int i;
	if (!e || !e->pchar)
		return;
	for (i = 0; i < eaSize(&actionList); ++i)
	{
		int doAction = 0;
		int didAction = 0;
		switch (numMin)
		{
		case 1:
			didAction = actionList[i]->perLevelThrottles[charLevel]->expGain1Min && (oldXpSum > actionList[i]->perLevelThrottles[charLevel]->expGain1Min);
			doAction = actionList[i]->perLevelThrottles[charLevel]->expGain1Min && (xp > actionList[i]->perLevelThrottles[charLevel]->expGain1Min);
			break;
		case 5:
			didAction = actionList[i]->perLevelThrottles[charLevel]->expGain5Min && (oldXpSum > actionList[i]->perLevelThrottles[charLevel]->expGain5Min);
			doAction = actionList[i]->perLevelThrottles[charLevel]->expGain5Min && (xp > actionList[i]->perLevelThrottles[charLevel]->expGain5Min);
			break;
		case 15:
			didAction = actionList[i]->perLevelThrottles[charLevel]->expGain15Min && (oldXpSum > actionList[i]->perLevelThrottles[charLevel]->expGain15Min);
			doAction = actionList[i]->perLevelThrottles[charLevel]->expGain15Min && (xp > actionList[i]->perLevelThrottles[charLevel]->expGain15Min);
			break;
		case 30:
			didAction = actionList[i]->perLevelThrottles[charLevel]->expGain30Min && (oldXpSum > actionList[i]->perLevelThrottles[charLevel]->expGain30Min);
			doAction = actionList[i]->perLevelThrottles[charLevel]->expGain30Min && (xp > actionList[i]->perLevelThrottles[charLevel]->expGain30Min);
			break;
		case 60:
			didAction = actionList[i]->perLevelThrottles[charLevel]->expGain60Min && (oldXpSum > actionList[i]->perLevelThrottles[charLevel]->expGain60Min);
			doAction = actionList[i]->perLevelThrottles[charLevel]->expGain60Min && (xp > actionList[i]->perLevelThrottles[charLevel]->expGain60Min);
			break;
		case 120:
			didAction = actionList[i]->perLevelThrottles[charLevel]->expGain120Min && (oldXpSum > actionList[i]->perLevelThrottles[charLevel]->expGain120Min);
			doAction = actionList[i]->perLevelThrottles[charLevel]->expGain120Min && (xp > actionList[i]->perLevelThrottles[charLevel]->expGain120Min);
			break;
		default:
			devassert(0);
		}
		if (doAction)
		{
			if (actionList[i]->action == MARTY_Action_Throttle && server_state.MARTY_enabled)
			{
				marty->expThrottled = 1;
			}

			if (!didAction)
			{
				if (actionList[i]->expType == MARTY_ExperienceArchitect)
				{
					LOG(LOG_MARTY, LOG_LEVEL_IMPORTANT, 0, 
						"Player %s[level: %i|combat: %i](account %s) tripped MARTY %s (%s). Earned %i xp in %i min interval playing arc id(%i).", 
						e->name, character_CalcExperienceLevel(e->pchar), character_CalcCombatLevel(e->pchar), e->auth_name, 
						actionList[i]->action == MARTY_Action_Throttle ? "Throttle" : "Log", actionList[i]->logAppendMsg, 
						xp, numMin, g_ArchitectTaskForce ? g_ArchitectTaskForce->architect_id : -1);
				}
				else
				{
					LOG(LOG_MARTY, LOG_LEVEL_IMPORTANT, 0, 
						"Player %s[level: %i|combat: %i](account %s) tripped MARTY %s (%s). Earned %i xp in %i min interval playing (%s|%s).", 
						e->name, character_CalcExperienceLevel(e->pchar), character_CalcCombatLevel(e->pchar), e->auth_name, 
						actionList[i]->action == MARTY_Action_Throttle ? "Throttle" : "Log", actionList[i]->logAppendMsg, 
						xp, numMin, g_activemission && g_activemission->task ? g_activemission->task->filename : "none",
						g_activemission && g_activemission->task ? g_activemission->task->logicalname : "none");
				}
			}
		}
	}
}
void clearMARTYHistory(Entity *e)
{
	int i;
	for (i = 0; i < MARTY_ExperienceTypeCount; ++i)
	{
		int j;
		MARTY_RewardData *marty = &e->pl->MARTY_rewardData[i];
		for (j = 0; j < MAX_MARTY_EXP_MINUTE_RECORDS; ++j)
		{
			 marty->expCircleBuffer[j] = 0;
		}
		marty->expMinuteAccum = marty->exp5MinSum = marty->exp15MinSum = marty->exp30MinSum = marty->exp60MinSum = marty->exp120MinSum = 0;
	}
}

int displayMARTYSum(Entity *e, int experienceType, int interval)
{
	MARTY_RewardData *marty = &e->pl->MARTY_rewardData[experienceType];
	switch (interval)
	{
	case 0:
		return marty->expMinuteAccum;
	case 1:
		return marty->expCircleBuffer[marty->expRingBufferLocation+MAX_MARTY_EXP_MINUTE_RECORDS-1];
	case 5:
		return marty->exp5MinSum;
	case 15:
		return marty->exp15MinSum;
	case 30:
		return marty->exp30MinSum;
	case 60:
		return marty->exp60MinSum;
	case 120:
		return marty->exp120MinSum;
	default:
		devassert(0);
		return 0;
	}
}

void MARTY_Tick(Entity *e)
{
	//	if this second ends overrolls
	if (e && e->pl)
	{
		int currentMinute = dbSecondsSince2000() / SECONDS_PER_MINUTE;
		int i;
		int sentPlayerMessage = 0;
		int charLevel = character_CalcExperienceLevel(e->pchar);
		for (i = 0; i < MARTY_ExperienceTypeCount; ++i)
		{
			MARTY_RewardData *marty = &e->pl->MARTY_rewardData[i];
			if (currentMinute != marty->expCurrentMinute)
			{
				int xp = marty->expMinuteAccum;
				MARTYExpMods **actionList = NULL;
				int j;
				int oldThrottled = 0;
				int oldSum = 0;
				int min5Pos = (marty->expRingBufferLocation + MAX_MARTY_EXP_MINUTE_RECORDS - 5) % MAX_MARTY_EXP_MINUTE_RECORDS;
				int min15Pos = (marty->expRingBufferLocation + MAX_MARTY_EXP_MINUTE_RECORDS - 15) % MAX_MARTY_EXP_MINUTE_RECORDS;
				int min30Pos = (marty->expRingBufferLocation + MAX_MARTY_EXP_MINUTE_RECORDS - 30) % MAX_MARTY_EXP_MINUTE_RECORDS;
				int min60Pos = (marty->expRingBufferLocation + MAX_MARTY_EXP_MINUTE_RECORDS - 60) % MAX_MARTY_EXP_MINUTE_RECORDS;
				int min120Pos = (marty->expRingBufferLocation + MAX_MARTY_EXP_MINUTE_RECORDS - 120) % MAX_MARTY_EXP_MINUTE_RECORDS;

				oldThrottled = marty->expThrottled;
				marty->expThrottled = 0;

				for (j = 0; j < eaSize(&g_MARTYMods.perActionType); ++j)
				{
					MARTYExpMods *expMod = g_MARTYMods.perActionType[j];
					if (expMod->expType == i)
					{
						eaPush(&actionList, expMod);
					}
				}

				HandleThrottleCheck(marty, actionList, xp, marty->expCircleBuffer[marty->expRingBufferLocation], charLevel, 1, e);

				oldSum = marty->exp5MinSum;
				marty->exp5MinSum += xp;
				marty->exp5MinSum -= marty->expCircleBuffer[min5Pos];
				HandleThrottleCheck(marty, actionList, marty->exp5MinSum, oldSum, charLevel, 5, e);

				oldSum = marty->exp15MinSum;
				marty->exp15MinSum += xp;
				marty->exp15MinSum -= marty->expCircleBuffer[min15Pos];
				HandleThrottleCheck(marty, actionList, marty->exp15MinSum, oldSum, charLevel, 15,e );

				oldSum = marty->exp30MinSum;
				marty->exp30MinSum += xp;
				marty->exp30MinSum -= marty->expCircleBuffer[min30Pos];
				HandleThrottleCheck(marty, actionList, marty->exp30MinSum, oldSum, charLevel, 30,e);

				oldSum = marty->exp60MinSum;
				marty->exp60MinSum += xp;
				marty->exp60MinSum -= marty->expCircleBuffer[min60Pos];
				HandleThrottleCheck(marty, actionList, marty->exp60MinSum, oldSum, charLevel, 60,e);

				oldSum = marty->exp120MinSum;
				marty->exp120MinSum += xp;
				marty->exp120MinSum -= marty->expCircleBuffer[min120Pos];
				HandleThrottleCheck(marty, actionList, marty->exp120MinSum, oldSum, charLevel, 120,e);

				marty->expCircleBuffer[marty->expRingBufferLocation] = xp;
				marty->expRingBufferLocation = (marty->expRingBufferLocation + 1) % MAX_MARTY_EXP_MINUTE_RECORDS;

				//  send message to player if throttling state changes
				if(!sentPlayerMessage && marty->expThrottled != oldThrottled)
				{
					if(marty->expThrottled)
					{
						if (i == MARTY_ExperienceArchitect)
							sendInfoBox(e, INFO_REWARD, "MARTYArchitectThrottleOn");
						else
							sendInfoBox(e, INFO_REWARD, "MARTYCombinedThrottleOn");
					}
					else
					{
						if (i == MARTY_ExperienceArchitect)
							sendInfoBox(e, INFO_REWARD, "MARTYArchitectThrottleOff");
						else
							sendInfoBox(e, INFO_REWARD, "MARTYCombinedThrottleOff");
					}
					sentPlayerMessage = 1;
				}

				//	accumulate/clear
				marty->expMinuteAccum = 0;
				marty->expCurrentMinute = currentMinute;
				eaDestroy(&actionList);
			}
		}
	}
}

void validateMARTYMods(MARTYMods *mods, char *filename)
{
	int i;
	for (i = 0; i < eaSize(&mods->perActionType); ++i)
	{
		MARTYExpMods *actionExpMod = mods->perActionType[i];
		if (eaSize(&actionExpMod->perLevelThrottles) != MAX_PLAYER_LEVEL)
		{
			ErrorFilenamef(filename, "Number of level mods does not match number of player levels. Found %i. Expected %i.", eaSize(&actionExpMod->perLevelThrottles), MAX_PLAYER_LEVEL);
		}
		else
		{
			int j;
			for (j = 0; j < MAX_PLAYER_LEVEL; ++j)
			{
				MARTYLevelMods *levelMod = actionExpMod->perLevelThrottles[j];
				if (j != levelMod->level)
				{
					ErrorFilenamef(filename, "Level does not match expected. %i experience mod. Level %i", i, j);
				}
			}
		}
	}
}

void MARTY_NormalizeExpHistory(Entity *e, int oldLevel)
{
	if (e && e->pl && server_state.MARTY_enabled)
	{
		if (EAINRANGE(oldLevel, g_MARTYMods.levelupShifts))
		{
			int i;
			for (i = 0; i < MARTY_ExperienceTypeCount; ++i)
			{
				int j;
				for (j = 0; j < MAX_MARTY_EXP_MINUTE_RECORDS; ++j)
				{
					e->pl->MARTY_rewardData[i].expCircleBuffer[j] *= g_MARTYMods.levelupShifts[oldLevel]->mod;
				}
				e->pl->MARTY_rewardData[i].exp5MinSum *= g_MARTYMods.levelupShifts[oldLevel]->mod;
				e->pl->MARTY_rewardData[i].exp15MinSum *= g_MARTYMods.levelupShifts[oldLevel]->mod;
				e->pl->MARTY_rewardData[i].exp30MinSum *= g_MARTYMods.levelupShifts[oldLevel]->mod;
				e->pl->MARTY_rewardData[i].exp60MinSum *= g_MARTYMods.levelupShifts[oldLevel]->mod;
				e->pl->MARTY_rewardData[i].exp120MinSum *= g_MARTYMods.levelupShifts[oldLevel]->mod;
			}
		}
	}
}