/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <assert.h>

#include "earray.h"
#include "textparser.h"

#include "power_system.h"

TokenizerParseInfo ParseSchedule[] =
{
	{ "{",                         TOK_START,  0 },
	{ "FreeBoostSlotsOnPower",     TOK_INTARRAY(Schedule, piFreeBoostSlotsOnPower)     },
	{ "PoolPowerSet",              TOK_INTARRAY(Schedule, piPoolPowerSet)              },
	{ "EpicPowerSet",              TOK_INTARRAY(Schedule, piEpicPowerSet)              },
	{ "Power",                     TOK_INTARRAY(Schedule, piPower)                     },
	{ "AssignableBoost",           TOK_INTARRAY(Schedule, piAssignableBoost)           },
	{ "InspirationCol",            TOK_INTARRAY(Schedule, piInspirationCol)            },
	{ "InspirationRow",            TOK_INTARRAY(Schedule, piInspirationRow)            },
	{ "}",                         TOK_END,    0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseExperienceTable[] =
{
	{ "{",                     TOK_START,  0 },
	{ "ExperienceRequired",    TOK_INTARRAY(ExperienceTable, piRequired)      },
	{ "DefeatPenalty",         TOK_INTARRAY(ExperienceTable, piDefeatPenalty) },
	{ "}",                     TOK_END,    0 },
	{ "", 0, 0 }
};

StaticDefineInt PowerSystemEnum[] =
{
	DEFINE_INT
	{ "kPowers", kPowerSystem_Powers },
	{ "kPower",  kPowerSystem_Powers },
	{ "Powers",  kPowerSystem_Powers },
	{ "Power",   kPowerSystem_Powers },
	DEFINE_END
};

TokenizerParseInfo ParseSchedules[] =
{
	{ "{",                TOK_START,  0 },
	{ "Powers",           TOK_EMBEDDEDSTRUCT(Schedules, aSchedules, ParseSchedule)},
	{ "}",                TOK_END,    0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseExperienceTables[] =
{
	{ "{",                TOK_START,  0 },
	{ "Powers",           TOK_EMBEDDEDSTRUCT(ExperienceTables, aTables, ParseExperienceTable)},
	{ "}",                TOK_END,    0 },
	{ "", 0, 0 }
};

int g_aiMaxLevels[2][2] = 
{
	{	// Heroes
		MAX_PLAYER_SECURITY_LEVEL, // kPowerSystem_Powers
		MAX_PLAYER_WISDOM_LEVEL,   // kPowerSystem_Skills
	},
	{	// Villains
		MAX_PLAYER_SECURITY_LEVEL, // kPowerSystem_Powers
		MAX_PLAYER_WISDOM_LEVEL,   // kPowerSystem_Skills
	},
};

int g_aiRequiredSecurityLevel[] = 
{
	0,    // kPowerSystem_Powers
	14,   // kPowerSystem_Skills
};

/**********************************************************************func*
 * CountForLevel
 *
 */
int CountForLevel(int iLevel, const int *piSchedule)
{
	int i = 0;

	if(!piSchedule)
		return 0;

	if(iLevel<0)
		iLevel = 0;

	for(i=0;
		i<eaiSize(&piSchedule) && piSchedule[i]<=iLevel;
		i++)
	{
		// Nothing to do
	}

	return i;
}

/* End of File */
