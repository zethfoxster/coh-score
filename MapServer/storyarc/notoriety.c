/*\
 *
 *	notoriety.h/c - Copyright 2003, 2004 Cryptic Studios
 *		All Rights Reserved
 * 		Confidential property of Cryptic Studios
 *  
 *	deals with notoriety adjustment of tasks - allows players
 *  to go to special 'notoriety' contacts and change the level tasks
 *  are generated at
 *
 */

#include "storyarcprivate.h"
#include "character_base.h"
#include "character_level.h"
#include "entPlayer.h"
#include "team.h"
#include "entity.h"
#include "taskforce.h"
#include "staticMapInfo.h"
#include "dbcomm.h"

typedef struct DifficultySettings
{
	int* cost; // cost to change difficulty settings

	F32 exemplarXPMod; // modifier to explemplared peoples XP

	int maxTeamNoAV; // largest team size that can get away with no Arch Villains
	int maxTeamNoAVPlusOne; // largest team size that can get away with an even level AV

} DifficultySettings;


TokenizerParseInfo ParseDifficultySettings[] =
{
	{ "{",                   TOK_START,  0 },
	{ "Cost",				 TOK_INTARRAY(DifficultySettings, cost)   },
	{ "ExemplarXPMod",		 TOK_F32(DifficultySettings, exemplarXPMod, 1)      },
	{ "MaxTeamNoAV",		 TOK_INT(DifficultySettings, maxTeamNoAV, 5)		 },
	{ "MaxTeamNoAVPlusOne",  TOK_INT(DifficultySettings, maxTeamNoAVPlusOne, 7) },
	{ "}",                   TOK_END,    0 },
	{ "", 0, 0 }
};

DifficultySettings g_difficultySettings = {0};

static void difficultyCostLoad(void)
{
	if (g_difficultySettings.cost)
	{
		ParserDestroyStruct(ParseDifficultySettings, &g_difficultySettings);
		memset(&g_difficultySettings, 0, sizeof(g_difficultySettings));
	}
	ParserLoadFiles(NULL, "defs/DifficultyCost.def", "DifficultyCost.bin", PARSER_SERVERONLY, 
		ParseDifficultySettings, &g_difficultySettings, NULL, NULL, NULL, NULL);
	if (eaiSize(&g_difficultySettings.cost) < MAX_PLAYER_SECURITY_LEVEL )
	{
		ErrorFilenamef("defs/DifficultyCost.def", "Difficulty cost file doesn't define enough player levels");
	}
}

void difficultyLoad(void)
{
	difficultyCostLoad();
}

F32 exemplarXPMod(void)
{
	return g_difficultySettings.exemplarXPMod;
}

void difficultyUpdateTask(Entity* player, StoryTaskHandle *sahandle)
{
	if (player)
	{
		StoryDifficulty *pPlayerDiff = playerDifficulty(player);
		if(pPlayerDiff)
		{
			StoryInfo* info = StoryArcLockPlayerInfo(player);
			int i, n = eaSize(&info->tasks);
			for (i = 0; i < n; i++)
			{
				if( isSameStoryTask(sahandle, &info->tasks[i]->sahandle ) )
					difficultyCopy(&info->tasks[i]->difficulty, pPlayerDiff);
			}
			StoryArcUnlockPlayerInfo(player);
		}
	}
}

StoryDifficulty *playerDifficulty(Entity* player)
{
	if (!player || !player->storyInfo) 
		return 0;
	return &player->storyInfo->difficulty;
}


int difficultyDoAVReduction(StoryDifficulty * pDifficulty, int teamSize)
{
	if ( teamSize > g_difficultySettings.maxTeamNoAV  || pDifficulty->alwaysAV)
		return 0;
	return 1;
}

int difficultyAVLevelAdjust(StoryDifficulty * pDifficulty, int avLevel, int teamSize)
{
	int plusOne = (teamSize > g_difficultySettings.maxTeamNoAVPlusOne)?1:0;
	return MAX(0,avLevel + plusOne);
}

static int difficultyCostToChange(Entity* player)
{
	int zerolevel = character_GetExperienceLevelExtern(player->pchar) - 1;
	if (zerolevel < 0 || zerolevel >= eaiSize(&g_difficultySettings.cost))
		return 0;
	return g_difficultySettings.cost[zerolevel];
}


void difficultyCopy( StoryDifficulty *pDifficultyDest, StoryDifficulty *pDifficultySrc )
{
	if( !pDifficultyDest || !pDifficultySrc)
		return;

	if (pDifficultySrc->levelAdjust >= MIN_LEVEL_ADJUST && pDifficultySrc->levelAdjust <= MAX_LEVEL_ADJUST )
		pDifficultyDest->levelAdjust = pDifficultySrc->levelAdjust;

	if (pDifficultySrc->teamSize >= MIN_TEAM_SIZE && pDifficultySrc->levelAdjust <= MAX_TEAM_SIZE )
		pDifficultyDest->teamSize = pDifficultySrc->teamSize;

	pDifficultyDest->alwaysAV = 0;
	if ( pDifficultySrc->alwaysAV == 1 )
		pDifficultyDest->alwaysAV = 1;

	pDifficultyDest->dontReduceBoss = 0;
	if ( pDifficultySrc->dontReduceBoss == 1  )
		pDifficultyDest->dontReduceBoss = 1;
}

void difficultySet( StoryDifficulty *pDifficulty, int levelAdjust, int teamSize, int alwaysAV, int dontReduceBoss )
{
	if (levelAdjust >= MIN_LEVEL_ADJUST && levelAdjust <= MAX_LEVEL_ADJUST )
		pDifficulty->levelAdjust = levelAdjust;

	if (teamSize >= MIN_TEAM_SIZE && levelAdjust <= MAX_TEAM_SIZE )
		pDifficulty->teamSize = teamSize;

	if ( alwaysAV == 0 || alwaysAV == 1 )
		pDifficulty->alwaysAV = alwaysAV;

	if ( dontReduceBoss == 0 || dontReduceBoss == 1 )
		pDifficulty->dontReduceBoss = dontReduceBoss;
}

void difficultySetFromPlayer(StoryDifficulty *pDifficulty, Entity* player )
{
	if (!player || !player->storyInfo) 
	{
		return;
	}
	else
	{
		int onFB = TaskForceIsOnFlashback(player);
		int onAE = TaskForceIsOnArchitect(player);
		int tfScalable = TaskForceIsScalable(player);
		if (!player->taskforce || onFB ||  onAE || tfScalable ) 
		{
			difficultyCopy( pDifficulty, &player->storyInfo->difficulty);
		}
		else if (player->taskforce)
		{
			difficultySet( pDifficulty, MAX(0, player->storyInfo->difficulty.levelAdjust), 
										MAX(1, player->storyInfo->difficulty.teamSize), 1, 1);
		}
	}
}

void difficultySetForPlayer(Entity* player, int levelAdjust, int teamSize, int alwaysAV, int dontReduceBoss )
{
	StoryInfo* info;
	int i;

	if (!player || !player->storyInfo) 
		return;

	// Get the players storyinfo, not the taskforce
	info = StoryArcLockPlayerInfo(player);
	difficultySet( &info->difficulty, levelAdjust, teamSize, alwaysAV, dontReduceBoss );

	// Update the players tasks appropriately
	for (i = eaSize(&info->tasks)-1; i >= 0; i--)
	{
		int isTeamTask = TaskIsTeamTask(info->tasks[i], player);

		// If task is complete or is currently running, dont update it
		if (TASK_COMPLETE(info->tasks[i]->state) || (isTeamTask && player->teamup->map_id))
			continue;

		// Update task notoriety and send it to the team if neccessary
		difficultySet( &info->tasks[i]->difficulty, levelAdjust, teamSize, alwaysAV, dontReduceBoss );

		if (isTeamTask && teamLock(player, CONTAINER_TEAMUPS))
		{
            difficultySet( &player->teamup->activetask->difficulty, levelAdjust, teamSize, alwaysAV, dontReduceBoss );
			teamUpdateUnlock(player, CONTAINER_TEAMUPS);
		}
	}
	StoryArcUnlockPlayerInfo(player);
	TaskStatusSendNotorietyUpdate(player);
}


static void difficultyContactAddResponseLink(ContactResponse* response, const char * text, int link, int cost ) 
{
	const char * linkstr = StaticDefineIntRevLookup(ParseContactLink, link);
	strcatf(response->dialog, "<a href=%s>", linkstr );
	strcatf(response->dialog, "<tr width=80%><td highlight=0><a href=%s>", linkstr);
	strcat(response->dialog, text);
	if( cost > 0 )
		strcatf( response->dialog, saUtilLocalize(0, "DifficultyCost", cost) );
	strcat(response->dialog, "</a></td></tr></a>");
}

static int difficultyChargeForChange(Entity * player, Contact* contact, ContactResponse* response, const char * text, int cost )
{
	strcpy(response->dialog, ContactHeadshot(contact, player, 0));
	if (player->pchar->iInfluencePoints >= cost) // success
	{
		ent_AdjInfluence(player, -cost, NULL);
		strcat(response->dialog, text );
		return 1;
	}
	else // failure
	{
		strcat(response->dialog, saUtilLocalize(player,"NotorietyNotEnoughInfluence", player, player->pchar->iInfluencePoints));
		return 0;
	}
}

int difficultyContact(Entity* player, Contact* contact, int link, ContactResponse* response)
{
	StoryDifficulty *pDiff = playerDifficulty(player);
	int i, cost = difficultyCostToChange(player);
	char colorString[8], nameString[64];
	const MapSpec *spec;

	memset(response, 0, sizeof(ContactResponse));
	if (player->storyInfo->interactCell) 
		return 0;

	strcpy(colorString, "Blue");

	if (spec = MapSpecFromMapId(db_state.base_map_id))
	{
		if (spec->teamArea == MAP_TEAM_VILLAINS_ONLY)
		{
			strcpy(colorString, "Red");
		}
		else if (spec->teamArea == MAP_TEAM_PRAETORIANS)
		{
			strcpy(colorString, "Gold");
		}
	}

	switch (link)
	{
	case CONTACTLINK_HELLO:
	case CONTACTLINK_MAIN:
		// hello, etc.
		strcpy(response->dialog, ContactHeadshot(contact, player, 0));

		sprintf_s(nameString, 64, "%sDifficultyCurrentIntro", colorString);
		strcat(response->dialog, saUtilLocalize(player, nameString, player));
		
		if (pDiff->levelAdjust == 0)
		{
			strcat(response->dialog, saUtilLocalize(player, "DifficultyCurrentLevelSame"));
		}
		else
		{
			strcat(response->dialog, saUtilLocalize(player, "DifficultyCurrentLevel", pDiff->levelAdjust ));
		}
	
		if (pDiff->teamSize <= 1)
		{
			sprintf_s(nameString, 64, "%sDifficultTeamSizeOne", colorString);
			strcat(response->dialog, saUtilLocalize(player, nameString));
		}
		else
		{
			sprintf_s(nameString, 64, "%sDifficultTeamSize", colorString);
			strcat(response->dialog, saUtilLocalize(player, nameString, pDiff->teamSize));
		}

		sprintf_s(nameString, 64, "%sDifficultyBossAV", colorString);
		strcat(response->dialog, saUtilLocalize(player, nameString, pDiff->dontReduceBoss?"DoesString":"DoesNotString", pDiff->alwaysAV?"DoesString":"DoesNotString" ));
		
		strcat(response->dialog, "<br><br>");
		strcat(response->dialog, "<linkhoverbg #118866aa><link white><linkhover white><table>");

		// change level
		difficultyContactAddResponseLink( response, saUtilLocalize(player, "DifficultyIWantChangeLevel"), CONTACTLINK_DIFFICULTY_LEVEL, cost );

		// change team size
		sprintf_s(nameString, 64, "%sDifficultyIWantChangeTeamSize", colorString);
		difficultyContactAddResponseLink( response, saUtilLocalize(player, nameString), CONTACTLINK_DIFFICULTY_TEAMSIZE, cost );
		
		// change Boss
		if( pDiff->dontReduceBoss )
			difficultyContactAddResponseLink( response, saUtilLocalize(player, "DifficultyIWantDontFightBossesSolo"), CONTACTLINK_DIFFICULTY_DISALLOW_BOSS, cost );
		else
			difficultyContactAddResponseLink( response, saUtilLocalize(player, "DifficultyIWantFightBossesSolo"), CONTACTLINK_DIFFICULTY_ALLOW_BOSS, cost );

		if( pDiff->alwaysAV )
		{
			sprintf_s(nameString, 64, "%sDifficultyIWantNoAV", colorString);
			difficultyContactAddResponseLink( response, saUtilLocalize(player, nameString), CONTACTLINK_DIFFICULTY_NEVER_AV, cost );
		}
		else
		{
			sprintf_s(nameString, 64, "%sDifficultyIWantAV", colorString);
			difficultyContactAddResponseLink( response, saUtilLocalize(player, nameString), CONTACTLINK_DIFFICULTY_ALWAYS_AV, cost );
		}

		strcat(response->dialog, "</table>");
		break;

	case CONTACTLINK_DIFFICULTY_LEVEL:
		strcpy(response->dialog, ContactHeadshot(contact, player, 0));
		strcat(response->dialog, saUtilLocalize(player,"DifficultyChangeLevel", player));
		strcat(response->dialog, "<br><br>");
		strcat(response->dialog, "<linkhoverbg #118866aa><link white><linkhover white><table>");
		
		for( i = CONTACTLINK_DIFFICULTY_LEVEL_MINUS1; i <= CONTACTLINK_DIFFICULTY_LEVEL_PLUS4; i++ )
		{
			int level = i-CONTACTLINK_DIFFICULTY_LEVEL_MINUS1-1;
			if( level == pDiff->levelAdjust )
				strcatf(response->dialog, "<tr width=80%><td highlight=0>%s</td></tr>", saUtilLocalize(player,"DifficultyLevel", level ));
			else
				difficultyContactAddResponseLink( response, saUtilLocalize(player,"DifficultyLevel", level ), i, cost );
		}
		strcat(response->dialog, "</table>");
		break;

	case CONTACTLINK_DIFFICULTY_LEVEL_MINUS1:
	case CONTACTLINK_DIFFICULTY_LEVEL_EVEN:
	case CONTACTLINK_DIFFICULTY_LEVEL_PLUS1:
	case CONTACTLINK_DIFFICULTY_LEVEL_PLUS2:
	case CONTACTLINK_DIFFICULTY_LEVEL_PLUS3:
	case CONTACTLINK_DIFFICULTY_LEVEL_PLUS4:
		{
			int newlevel = 	(link - CONTACTLINK_DIFFICULTY_LEVEL_MINUS1) - 1;

			if( (pDiff->levelAdjust != newlevel) && difficultyChargeForChange(player, contact, response, saUtilLocalize(player, "DifficultyChangedLevel", newlevel), cost) )
				difficultySetForPlayer(player, newlevel, 0, -1, -1 );
			AddResponse(response, saUtilLocalize(player,"DifficultyWhatAreMySettings"), CONTACTLINK_MAIN);
			break;
		}
	case CONTACTLINK_DIFFICULTY_TEAMSIZE:
		strcpy(response->dialog, ContactHeadshot(contact, player, 0));
		sprintf_s(nameString, 64, "%sDifficultyChangeTeamsize", colorString);
		strcat(response->dialog, saUtilLocalize(player, nameString, player));
		strcat(response->dialog, "<br><br>");
		strcat(response->dialog, "<linkhoverbg #118866aa><link white><linkhover white><table>");

		for( i = CONTACTLINK_DIFFICULTY_TEAMSIZE_1; i <= CONTACTLINK_DIFFICULTY_TEAMSIZE_8; i++ )
		{
			int teamsize = i - CONTACTLINK_DIFFICULTY_TEAMSIZE_1 + 1;
			const char *string;

			sprintf_s(nameString, 64, "%sDifficultyTeamSize", colorString);
			string = saUtilLocalize(player, nameString, teamsize);

			if (teamsize == pDiff->teamSize)
				strcatf(response->dialog, "<tr width=80%><td highlight=0>%s</td></tr>", string);
			else
				difficultyContactAddResponseLink(response, string, i, cost);
		}
		strcat(response->dialog, "</table>");
		break;

	case CONTACTLINK_DIFFICULTY_TEAMSIZE_1:
	case CONTACTLINK_DIFFICULTY_TEAMSIZE_2:
	case CONTACTLINK_DIFFICULTY_TEAMSIZE_3:
	case CONTACTLINK_DIFFICULTY_TEAMSIZE_4:
	case CONTACTLINK_DIFFICULTY_TEAMSIZE_5:
	case CONTACTLINK_DIFFICULTY_TEAMSIZE_6:
	case CONTACTLINK_DIFFICULTY_TEAMSIZE_7:
	case CONTACTLINK_DIFFICULTY_TEAMSIZE_8:
		{
			int newteamsize = link - CONTACTLINK_DIFFICULTY_TEAMSIZE_1 + 1;
			if (pDiff->teamSize != newteamsize)
			{
				const char *string;

				sprintf_s(nameString, 64, "%sDifficultyChangedTeamSize", colorString);
				string = saUtilLocalize(player, nameString, newteamsize);

				if (difficultyChargeForChange(player, contact, response, string, cost))
				{
					difficultySetForPlayer(player, -2, newteamsize, -1, -1);
				}
			}
			AddResponse(response, saUtilLocalize(player, "DifficultyWhatAreMySettings"), CONTACTLINK_MAIN);
			break;
		}
	case 	CONTACTLINK_DIFFICULTY_ALLOW_BOSS:
  		if( !pDiff->dontReduceBoss && difficultyChargeForChange(player, contact, response, saUtilLocalize(player,"DifficultyChangedFightBosses"), cost) )
			difficultySetForPlayer(player, -2, 0, -1, 1 );
		AddResponse(response, saUtilLocalize(player,"DifficultyWhatAreMySettings"), CONTACTLINK_MAIN);
		break;
	case  CONTACTLINK_DIFFICULTY_DISALLOW_BOSS:
		if( pDiff->dontReduceBoss && difficultyChargeForChange(player, contact, response, saUtilLocalize(player,"DifficultyChangedNoFightBosses"), cost) )
			difficultySetForPlayer(player, -2, 0, -1, 0 );
		AddResponse(response, saUtilLocalize(player,"DifficultyWhatAreMySettings"), CONTACTLINK_MAIN);
		break;
	case  CONTACTLINK_DIFFICULTY_ALWAYS_AV:
		if (!pDiff->alwaysAV)
		{
			const char *string;

			sprintf_s(nameString, 64, "%sDifficultyChangedAlwaysAV", colorString);
			string = saUtilLocalize(player, nameString);

			if (difficultyChargeForChange(player, contact, response, string, cost) )
			{
				difficultySetForPlayer(player, -2, 0, 1, -1);
			}
		}
		AddResponse(response, saUtilLocalize(player, "DifficultyWhatAreMySettings"), CONTACTLINK_MAIN);
		break;
	case CONTACTLINK_DIFFICULTY_NEVER_AV:
		if (pDiff->alwaysAV)
		{
			const char *string;

			sprintf_s(nameString, 64, "%sDifficultyChangedNeverAV", colorString);
			string = saUtilLocalize(player, nameString);

			if (difficultyChargeForChange(player, contact, response, string, cost))
			{
				difficultySetForPlayer(player, -2, 0, 0, -1);
			}
		}
		AddResponse(response, saUtilLocalize(player, "DifficultyWhatAreMySettings"), CONTACTLINK_MAIN);
		break;

	case CONTACTLINK_BYE:
	default:
		return 0;
	}

	AddResponse(response, saUtilLocalize(player,"Leave"), CONTACTLINK_BYE);
	return 1;
}


