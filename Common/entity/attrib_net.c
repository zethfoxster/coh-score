/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <stddef.h>   // for offsetof
#include "textparser.h"
#include "character_attribs.h"
#include "character_base.h"
#include "attrib_net.h"

extern TokenizerParseInfo ParseCharacterAttributes[]; // in character_attribs.h


TokenizerParseInfo ParseBasicCharacterAttributesLowRes[] =
{
	{ "HitPoints",           TOK_FLOAT_ROUNDING(FLOAT_ONES)		| TOK_F32(CharacterAttributes, fHitPoints, 0) },
	{ "Absorb",              TOK_FLOAT_ROUNDING(FLOAT_ONES)		| TOK_F32(CharacterAttributes, fAbsorb, 0) },
	{ "Endurance",           TOK_FLOAT_ROUNDING(FLOAT_FIVES)	| TOK_F32(CharacterAttributes, fEndurance, 0) },
	{ "Meter",               TOK_FLOAT_ROUNDING(FLOAT_TENTHS)	| TOK_F32(CharacterAttributes, fMeter, 0) },
	{ "StealthRadiusPlayer", TOK_FLOAT_ROUNDING(FLOAT_ONES)		| TOK_F32(CharacterAttributes, fStealthRadiusPlayer, 0) },
	{ 0 }
};

TokenizerParseInfo ParseBasicCharacterAttributesHighRes[] =
{
	{ "HitPoints",           TOK_FLOAT_ROUNDING(FLOAT_TENTHS)	| TOK_F32(CharacterAttributes, fHitPoints, 0) },
	{ "Absorb",              TOK_FLOAT_ROUNDING(FLOAT_TENTHS)   | TOK_F32(CharacterAttributes, fAbsorb, 0) },
	{ "Endurance",           TOK_FLOAT_ROUNDING(FLOAT_TENTHS)	| TOK_F32(CharacterAttributes, fEndurance, 0) },
	{ "Meter",               TOK_FLOAT_ROUNDING(FLOAT_HUNDRETHS) | TOK_F32(CharacterAttributes, fMeter, 0) },
	{ "PerceptionRadius",    TOK_FLOAT_ROUNDING(FLOAT_ONES)		| TOK_F32(CharacterAttributes, fPerceptionRadius, 0) },
	{ "StealthRadiusPlayer", TOK_FLOAT_ROUNDING(FLOAT_ONES)		| TOK_F32(CharacterAttributes, fStealthRadiusPlayer, 0) },
	{ "Confused",            TOK_FLOAT_ROUNDING(FLOAT_TENTHS)	| TOK_F32(CharacterAttributes, fConfused, 0) },
	{ 0 }
};

TokenizerParseInfo ParseBasicCharacterAttributesHighestRes[] =
{
	{ "HitPoints",           TOK_FLOAT_ROUNDING(FLOAT_TENTHS)	| TOK_F32(CharacterAttributes, fHitPoints, 0) },
	{ "Absorb",              TOK_FLOAT_ROUNDING(FLOAT_TENTHS)   | TOK_F32(CharacterAttributes, fAbsorb, 0) },
	{ "Endurance",           TOK_FLOAT_ROUNDING(FLOAT_TENTHS)	| TOK_F32(CharacterAttributes, fEndurance, 0) },
	{ "Meter",               TOK_FLOAT_ROUNDING(FLOAT_HUNDRETHS) | TOK_F32(CharacterAttributes, fMeter, 0) },
	{ "PerceptionRadius",    TOK_FLOAT_ROUNDING(FLOAT_ONES)		| TOK_F32(CharacterAttributes, fPerceptionRadius, 0) },
	{ "StealthRadiusPlayer", TOK_FLOAT_ROUNDING(FLOAT_ONES)		| TOK_F32(CharacterAttributes, fStealthRadiusPlayer, 0) },
	{ "Confused",            TOK_FLOAT_ROUNDING(FLOAT_TENTHS)	| TOK_F32(CharacterAttributes, fConfused, 0) },
	{ "ToHit",               TOK_FLOAT_ROUNDING(FLOAT_HUNDRETHS) | TOK_F32(CharacterAttributes, fToHit, 0) },
	{ 0 }
};

TokenizerParseInfo SendLevels[] =
{
	{ "Level",              TOK_MINBITS(4)	| TOK_INT(Character, iLevel, 0) },
	{ "CombatLevel",        TOK_MINBITS(4)	| TOK_INT(Character, iCombatLevel, 0) },
	{ 0 }
};

TokenizerParseInfo SendBasicCharacter[] =
{
	// Sends *partial* list (ParseBasicCharacterAttributes)
	{ "CurrentAttribs",         TOK_EMBEDDEDSTRUCT(Character, attrCur, ParseBasicCharacterAttributesLowRes) },
	{ "MaxAttribs",             TOK_EMBEDDEDSTRUCT(Character, attrMax, ParseBasicCharacterAttributesLowRes) },

	// This is sorta hackery, since this is not technically an embedded structure, it's just describing more of the same
	{ "SendLevels",             TOK_NULLSTRUCT(SendLevels) },
	{ 0 }
};

TokenizerParseInfo SendFullCharacter[] =
{
	// Sends *full* list (ParseCharacterAttributes)
	{ "CurrentAttribs",         TOK_EMBEDDEDSTRUCT(Character, attrCur, ParseBasicCharacterAttributesHighRes) },
	{ "MaxAttribs",             TOK_EMBEDDEDSTRUCT(Character, attrMax, ParseBasicCharacterAttributesHighRes) },

	// This is sorta hackery, since this is not technically an embedded structure, it's just describing more of the same
	{ "SendLevels",             TOK_NULLSTRUCT(SendLevels) },

	{ "ExperiencePoints",       TOK_MINBITS(16)	| TOK_INT(Character, iExperiencePoints, 0) },
	{ "ExperienceDebt",         TOK_MINBITS(16)	| TOK_INT(Character, iExperienceDebt, 0) },
	{ "ExperienceRest",         TOK_MINBITS(16)	| TOK_INT(Character, iExperienceRest, 0) },
	{ "InfluencePoints",        TOK_MINBITS(16)	| TOK_INT(Character, iInfluencePoints, 0) },
	{ 0 }
};

TokenizerParseInfo SendCharacterToSelf[] =
{
	// Sends *full* list (ParseCharacterAttributes)
	{ "CurrentAttribs",         TOK_EMBEDDEDSTRUCT(Character, attrCur, ParseBasicCharacterAttributesHighestRes) },
	{ "MaxAttribs",             TOK_EMBEDDEDSTRUCT(Character, attrMax, ParseBasicCharacterAttributesHighRes) },

	// This is sorta hackery, since this is not technically an embedded structure, it's just describing more of the same
	{ "SendLevels",             TOK_NULLSTRUCT(SendLevels) },

	{ "ExperiencePoints",       TOK_MINBITS(16)	| TOK_INT(Character, iExperiencePoints, 0) },
	{ "ExperienceDebt",         TOK_MINBITS(16)	| TOK_INT(Character, iExperienceDebt, 0) },
	{ "ExperienceRest",         TOK_MINBITS(16)	| TOK_INT(Character, iExperienceRest, 0) },
	{ "InfluencePoints",        TOK_MINBITS(16)	| TOK_INT(Character, iInfluencePoints, 0) },
	{ 0 }
};


/* End of File */
