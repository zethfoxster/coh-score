/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#if (!defined(COMBAT_MOD_H__)) || (defined(COMBAT_MOD_PARSE_INFO_DEFINITIONS)&&!defined(COMBAT_MOD_PARSE_INFO_DEFINED))
#define COMBAT_MOD_H__

#include <stdlib.h>   // for offsetof

// COMBAT_MOD_PARSE_INFO_DEFINITIONS must be defined before including this file
// to get the ParseInfo structures needed to read in files.

/***************************************************************************/
/***************************************************************************/

typedef struct CombatMod
{
	// Holds a set of percentage modifiers used to vary combat between
	// characters of different level.

	const float *pfToHit;
	const float *pfMagnitude;
	const float *pfDuration;
	const float *pfAccuracy;

} CombatMod;


#ifdef COMBAT_MOD_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseCombatMod[] =
{
	{ "{",           TOK_START,    0 },
	{ "ToHit",       TOK_F32ARRAY(CombatMod, pfToHit)     },
	{ "Magnitude",   TOK_F32ARRAY(CombatMod, pfMagnitude) },
	{ "Duration",    TOK_F32ARRAY(CombatMod, pfDuration)  },
	{ "Accuracy",    TOK_F32ARRAY(CombatMod, pfAccuracy)  },
	{ "}",           TOK_END,      0 },
	{ "", 0, 0 }
};
#endif


typedef struct CombatMods
{
	CombatMod Higher;
	CombatMod Lower;

	int iMaxSize;
	int iMinSize;

} CombatMods;

#ifdef COMBAT_MOD_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseCombatMods[] =
{
	{ "{",                   TOK_START,  0 },
	{ "MinSize",            TOK_INT(CombatMods, iMinSize,   0) },
	{ "MaxSize",            TOK_INT(CombatMods, iMaxSize, 100) },

	{ "HigherLevel",         TOK_EMBEDDEDSTRUCT(CombatMods, Higher, ParseCombatMod) },
	{ "LowerLevel",          TOK_EMBEDDEDSTRUCT(CombatMods, Lower, ParseCombatMod) },
	{ "}",                   TOK_END,    0 },
	{ "", 0, 0 }
};
#endif

typedef struct CombatModsTable
{
	float fPvPToHitMod;
	float fPvPElusivityMod;
	const float *pfToHitLevelMods;
	const CombatMods **ppMods;
} CombatModsTable;


#ifdef COMBAT_MOD_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseCombatModsTable[] =
{
	{ "PvPToHitMod",		TOK_F32(CombatModsTable, fPvPToHitMod,  0) },
	{ "PvPElusivityMod",	TOK_F32(CombatModsTable, fPvPElusivityMod,  0) },
	{ "ToHitLevelMod",		TOK_F32ARRAY(CombatModsTable, pfToHitLevelMods) },
	{ "CombatMods",			TOK_STRUCT(CombatModsTable, ppMods, ParseCombatMods) },
	{ "", 0, 0 }
};
#endif


/***************************************************************************/
/***************************************************************************/

typedef struct Character Character;

bool combatmod_Ignored(Character *pSrc, Character *pTarget);

void combatmod_Get(Character *pSrc, int iSelfLevel, int iTargetLevel, const CombatMod **ppcm, int *piDelta);

float combatmod_ToHit(CombatMod *pcm, int iDelta);
float combatmod_Duration(CombatMod *pcm, int iDelta);
float combatmod_Magnitude(CombatMod *pcm, int iDelta);
float combatmod_Accuracy(CombatMod *pcm, int iDelta);

float combatmod_PvPToHitMod(Character *pSrc, Character *pTarget);
float combatmod_PvPElusivityMod(Character *pSrc);
float combatmod_LevelToHitMod(Character *pSrc);

/***************************************************************************/
/***************************************************************************/

extern SHARED_MEMORY CombatModsTable g_CombatModsPlayer;
extern SHARED_MEMORY CombatModsTable g_CombatModsVillain;


/***************************************************************************/
/***************************************************************************/

#ifdef COMBAT_MOD_PARSE_INFO_DEFINITIONS
#define COMBAT_MOD_PARSE_INFO_DEFINED
#endif

#endif /* #ifndef COMBAT_MOD_H__ */

/* End of File */

