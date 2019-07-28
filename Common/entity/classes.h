/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#if (!defined(CLASSES_H__)) || (defined(CLASSES_PARSE_INFO_DEFINITIONS)&&!defined(CLASSES_PARSE_INFO_DEFINED))
#define CLASSES_H__

// CLASSES_PARSE_INFO_DEFINITIONS must be defined before including this file
// to get the ParseInfo structures needed to read in files.

#include <stdlib.h> // for offsetof

#ifdef CLASSES_PARSE_INFO_DEFINITIONS
#define CHARACTER_ATTRIBS_PARSE_INFO_DEFINITIONS
#endif

#include "character_attribs.h"

/***************************************************************************/
/***************************************************************************/

typedef struct StashTableImp* StashTable;
typedef const struct StashTableImp *cStashTable;
typedef struct PowerCategory PowerCategory;
typedef struct PowerDictionary PowerDictionary;

/***************************************************************************/
/***************************************************************************/

typedef struct NamedTable
{
	const char *pchName;
	const float *pfValues;
} NamedTable;

#ifdef CLASSES_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseNamedTable[] =
{
	{ "{",                 TOK_START,    0 },
	{ "Name",              TOK_STRING(NamedTable, pchName, 0)  },
	{ "Values",            TOK_F32ARRAY(NamedTable, pfValues) },
	{ "}",                 TOK_END,      0 },
	{ "", 0, 0 }
};
#endif

/***************************************************************************/
/***************************************************************************/

typedef enum Category
{
	kCategory_Primary=0,
	kCategory_Secondary,
	kCategory_Pool,
	kCategory_Epic,
	/* Add new categories which a character has here */

	kCategory_Count
} Category;

typedef enum ClassesDiminish
{

	kClassesDiminish_Inner,
	kClassesDiminish_Outer,

	kClassesDiminish_Count
};
/***************************************************************************/
/***************************************************************************/

typedef struct CharacterClass
{
	// Defines the character class, which sets up the allowable powers and
	// default hit points and defense for the character.

	const char *pchName;

	const char *pchDisplayName;
	const char *pchDisplayHelp;
	const char *pchDisplayShortHelp;
	const char *pchIconName;
		// Various strings for user-interaction

	const char *pchIcon;
		// The icon for this archetype

	const char **pchAllowedOriginNames;
		// This determins what origins can be picked by the player

	const char **pchSpecialRestrictions;
		// This is a string array of special restrictions (e.g. Kheldian, Avian, etc)

	char *pchStoreRestrictions;
		// This is a string of store restrictions 

	char *pchLockedTooltip;
	// This message is displayed to the user if this AT is locked describing what they can do to unlock it

	char *pchProductCode;
	// Product code that will be used for the purchase option if this AT is locked

	const char *pchReductionClass;
	// Class that is used for mission difficulty reduction purposes

	bool bReduceAsAV;
	// Class uses AV flag for mission difficulty reduction purposes

	int *piLevelUpRespecs;
		// This is an int array of levels at which the character respecs
		// instead of just picking the new power/slots

	bool bConnectHPAndStatus;
		// Gang together hit points and status points. Modifications to 
		//   hit points will affect status points and vice-versa. Hit points 
		//   are are set to be the same as status points.

	size_t offDefiantHitPointsAttrib;
		// Byte offset to the attribute in the CharacterAttributes struct.
		// If non-zero, points to the attrib which is used as hitpoints
		//   after the character has been defeated.
		// Once the actual hitpoint attrib has reached zero, this attrib
		//   is then used to determine if the character has actually been
		//   defeated.

	float fDefiantScale;
		// Scale applied to damage before it's removed from the
		//   DefiantHitPointsAttrib.

	const PowerCategory *pcat[kCategory_Count];
		// Categories of powers available to the character

	const CharacterAttributes **ppattrBase;
		// The base values for each attribute

	const CharacterAttributes **ppattrMin;
		// The min values allowed for each attribute

	const CharacterAttributes **ppattrStrengthMin;
		// The min values allowed for and attrib's strength

	const CharacterAttributes **ppattrResistanceMin;
		// The min values allowed for and attrib's resistance

	int iNumLevels;
		// How many levels of data each of the attrib arrays have in them.

	const CharacterAttributes *pattrMax;
		// The default max values allowed for each attribute. A character may
		// have these buffed/debuffed.

	const CharacterAttributes *pattrMaxMax;
		// The maximum value that an attrib's max value can be buffed to.

	const CharacterAttributes *pattrStrengthMax;
		// The maximum value that an attrib's strength can be buffed to.

	const CharacterAttributes *pattrResistanceMax;
		// The maximum value that an attrib's resistance can be buffed to.


	// Tables for diminishing returns
	const CharacterAttributes **ppattrDiminStr[kClassesDiminish_Count];
	const CharacterAttributes **ppattrDiminCur[kClassesDiminish_Count];
	const CharacterAttributes **ppattrDiminRes[kClassesDiminish_Count];
		

	const NamedTable **ppNamedTables;
	cStashTable namedStash;
		// Tables used by powers for scaling powers by level

	// Temporaries and other internals

	const char *pchTempCategory[kCategory_Count];
		// These strings are the names for the various categories and power
		// sets available to this class. They aren't used except to look up
		// the real pointer to these things (above).

	const CharacterAttributesTable **ppTempAttribMax;
	const CharacterAttributesTable **ppTempAttribMaxMax;
	const CharacterAttributesTable **ppTempStrengthMax;
	const CharacterAttributesTable **ppTempResistanceMax;
		// These tables are created by reading in the files and are then
		//   transformed into the more convenient data structures above.

	int iNumBytesTables;
		// The size of the data pointed to by each of the pattr... arrays
		// Used for shared memory stuff. Pay no attention to it.

} CharacterClass;

#ifdef CLASSES_PARSE_INFO_DEFINITIONS

extern StaticDefine ParsePowerDefines[]; // defined in load_def.c

TokenizerParseInfo ParseCharacterClass[] =
{
	{ "{",                   		TOK_START,			0 },
	{ "Name",                		TOK_STRING(CharacterClass, pchName, 0)              },
	{ "DisplayName",         		TOK_STRING(CharacterClass, pchDisplayName, 0)       },
	{ "DisplayHelp",         		TOK_STRING(CharacterClass, pchDisplayHelp, 0)       },
	{ "AllowedOrigins",      		TOK_STRINGARRAY(CharacterClass, pchAllowedOriginNames)},
	{ "SpecialRestrictions",		TOK_STRINGARRAY(CharacterClass, pchSpecialRestrictions)},
	{ "StoreRequires",				TOK_STRING(CharacterClass, pchStoreRestrictions, 0)},
	{ "LockedTooltip",				TOK_STRING(CharacterClass, pchLockedTooltip, 0)},
	{ "ProductCode",				TOK_STRING(CharacterClass, pchProductCode, 0)},
	{ "ReductionClass",				TOK_STRING(CharacterClass, pchReductionClass, 0)},
	{ "ReduceAsArchvillain",		TOK_BOOLFLAG(CharacterClass, bReduceAsAV, 0)},
	{ "LevelUpRespecs",				TOK_INTARRAY(CharacterClass, piLevelUpRespecs)},
	{ "DisplayShortHelp",    		TOK_STRING(CharacterClass, pchDisplayShortHelp, 0)  },
	{ "Icon",						TOK_STRING(CharacterClass, pchIcon, 0)              },
	{ "PrimaryCategory",			TOK_STRING(CharacterClass, pchTempCategory[kCategory_Primary], 0)   },
	{ "SecondaryCategory",			TOK_STRING(CharacterClass, pchTempCategory[kCategory_Secondary], 0) },
	{ "PowerPoolCategory",			TOK_STRING(CharacterClass, pchTempCategory[kCategory_Pool], 0)      },
	{ "EpicPoolCategory",			TOK_STRING(CharacterClass, pchTempCategory[kCategory_Epic], 0)      },
	{ "AttribMin",					TOK_STRUCT(CharacterClass, ppattrMin,				ParseCharacterAttributes)      },
	{ "AttribBase",					TOK_STRUCT(CharacterClass, ppattrBase,				ParseCharacterAttributes)      },
	{ "StrengthMin",				TOK_STRUCT(CharacterClass, ppattrStrengthMin,		ParseCharacterAttributes)      },
	{ "ResistanceMin",				TOK_STRUCT(CharacterClass, ppattrResistanceMin,		ParseCharacterAttributes)      },
	{ "AtrribDiminStrIn",			TOK_STRUCT(CharacterClass, ppattrDiminStr[kClassesDiminish_Inner],		ParseCharacterAttributes)      },
	{ "AtrribDiminStrOut",			TOK_STRUCT(CharacterClass, ppattrDiminStr[kClassesDiminish_Outer],		ParseCharacterAttributes)      },
	{ "AtrribDiminCurIn",			TOK_STRUCT(CharacterClass, ppattrDiminCur[kClassesDiminish_Inner],		ParseCharacterAttributes)      },
	{ "AtrribDiminCurOut",			TOK_STRUCT(CharacterClass, ppattrDiminCur[kClassesDiminish_Outer],		ParseCharacterAttributes)      },
	{ "AtrribDiminResIn",			TOK_STRUCT(CharacterClass, ppattrDiminRes[kClassesDiminish_Inner],		ParseCharacterAttributes)      },
	{ "AtrribDiminResOut",			TOK_STRUCT(CharacterClass, ppattrDiminRes[kClassesDiminish_Outer],		ParseCharacterAttributes)      },
	{ "AttribMaxTable",				TOK_STRUCT(CharacterClass, ppTempAttribMax,		ParseCharacterAttributesTable) },
	{ "AttribMaxMaxTable",			TOK_STRUCT(CharacterClass, ppTempAttribMaxMax,	ParseCharacterAttributesTable) },
	{ "StrengthMaxTable",			TOK_STRUCT(CharacterClass, ppTempStrengthMax,		ParseCharacterAttributesTable) },
	{ "ResistanceMaxTable",			TOK_STRUCT(CharacterClass, ppTempResistanceMax,	ParseCharacterAttributesTable) },
	{ "ModTable",					TOK_STRUCT(CharacterClass, ppNamedTables,        ParseNamedTable)            },
	{ "ConnectHPAndStatus",			TOK_BOOL(CharacterClass, bConnectHPAndStatus, 0), BoolEnum},
	{ "ConnectHPAndIntegrity",		TOK_REDUNDANTNAME|TOK_BOOL(CharacterClass, bConnectHPAndStatus, 0), BoolEnum},
	{ "DefiantHitPointsAttrib",		TOK_AUTOINT(CharacterClass, offDefiantHitPointsAttrib, 0), ParsePowerDefines },
	{ "DefiantScale",				TOK_F32(CharacterClass, fDefiantScale, 1.0f)  },

	// These are for shared memory stuff. Ignore them.
	{ "_FinalAttrMax_",				TOK_POINTER(CharacterClass, pattrMax, iNumBytesTables) },
	{ "_FinalAttrMaxMax_",			TOK_POINTER(CharacterClass, pattrMaxMax, iNumBytesTables)    },
	{ "_FinalAttrStrengthMax_",		TOK_POINTER(CharacterClass, pattrStrengthMax, iNumBytesTables)    },
	{ "_FinalAttrResistanceMax_",	TOK_POINTER(CharacterClass, pattrResistanceMax, iNumBytesTables)    },

	{ "}",                        TOK_END,      0 },
	{ "", 0, 0 }
};
#endif


/***************************************************************************/
/***************************************************************************/

typedef struct CharacterClasses
{
	// Defines the set of character classes.

	const CharacterClass **ppClasses;

} CharacterClasses;

#ifdef CLASSES_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseCharacterClasses[] =
{
	{ "Class", TOK_STRUCT(CharacterClasses, ppClasses, ParseCharacterClass)},
	{ "", 0, 0 }
};
#endif

/***************************************************************************/
/***************************************************************************/

extern SHARED_MEMORY CharacterClasses g_CharacterClasses;
extern SHARED_MEMORY CharacterClasses g_VillainClasses;

void classes_Repack(CharacterClasses *pclasses, const PowerDictionary *pdict);
	// Given the CharacterClasses and the PowerDictionary, look up the power
	// referenced by name in the class and initialized the lookup fields
	// in CharacterClass to point to the right place.

bool classes_Finalize(CharacterClasses *pclasses, bool shared_memory);
	// Convert into more optimized table lookups

const CharacterClass *classes_GetPtrFromName(const CharacterClasses *pclasses, const char *pch);
	// Returns a pointer to the class associated with the given name.

int classes_GetIndexFromName(const CharacterClasses *pclasses, const char *pch);
	// Returns a index to the class associated with the given name.

int classes_GetIndexFromPtr(const CharacterClasses *pclasses, const CharacterClass *pclass);
	// Returns a index to the class associated with the given pointer.

NamedTable *class_GetNamedTable(CharacterClass *pclass, const char *pch);
	// Returns a pointer to the table with the given name.

float class_GetNamedTableValue(const CharacterClass *pclass, const char *pch, int iIdx);
	// Returns the value at a given index in the table with the given name.

bool class_MatchesSpecialRestriction(const CharacterClass *pclass, const char * keyword);

bool class_IsRespecLevelUp(const CharacterClass *pclass, int level);
	// Returns true if levelling up from the given level should cause a respec
	// as well as the normal advancement things.

bool class_UsesAVStyleReduction(const char* characterClassName);

const char* class_GetReduced(const char* characterClassName);

#ifdef CLASSES_PARSE_INFO_DEFINITIONS
#define CLASSES_PARSE_INFO_DEFINED
#endif

#endif /* #ifndef CLASSES_H__ */

/* End of File */
