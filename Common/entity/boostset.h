/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#if (!defined(BOOSTSET_H__)) || (defined(BOOSTSET_PARSE_INFO_DEFINITIONS)&&!defined(BOOSTSET_PARSE_INFO_DEFINED))
#define BOOSTSET_H__

#include <stdlib.h>     // for offsetof
#include "stdtypes.h"

typedef struct BasePower BasePower;
typedef struct Power Power;
typedef struct BoostSet BoostSet;
typedef struct Character Character;
typedef struct Boost Boost;

typedef struct BoostList
{
	const BasePower **ppBoosts;
} BoostList;

typedef struct BoostSetBonus
{
	const char *pchDisplayName;
		// The display name of the bonus

	int iMinBoosts;
		// The number of distinct boosts required to activate this bonus

	int iMaxBoosts;
		// The max number of distinct boosts required to keep this bonus.  0 means no max.

	const char **ppchRequires;
		// A power eval statement that may limit the availability of this bonus.  If empty, it is not evaluated.

	const BasePower **ppAutoPowers;
		// The auto powers granted by this bonus

	const BasePower *pBonusPower;
		// The general power granted by this bonus (may be a mix of boost attribs and additional attribs)

	const BoostSet *pBoostSetParent;
		// Backpointer to the parent BoostSet
} BoostSetBonus;

typedef struct BoostSet
{
	const char *pchName;
		// The internal name of the set

	const char *pchDisplayName;
		// The display name of the set

	const char *pchGroupName;
		// The display name of the set group

	const char **ppchConversionGroups;
		// The name of the conversion groups this boostset belongs to

	const BasePower **ppPowers;
		// The powers that can use this set

	const BoostList **ppBoostLists;
		// The boosts that make up this set

	const BoostSetBonus **ppBonuses;
		// The bonuses granted by this set

	int iMinLevel;
		// Minimum level for this boostset

	int iMaxLevel;
		// Maximum level for this boostset

	const char *pchStoreProduct;
		// Product code that must be available for this Boost Set to be a valid conversion

} BoostSet;

typedef struct BoostSetDictionary
{
	const BoostSet **ppBoostSets;
	cStashTable haItemNames;
	cStashTable htBasePowerToGroupNameMap;
	cStashTable htBasePowerToSetNameMap;
	cStashTable htConversionLists;
} BoostSetDictionary;

extern SHARED_MEMORY BoostSetDictionary g_BoostSetDict;


typedef struct ConversionSetCost
{
	const char *pchConversionSetName;
	int tokensInSet;
	int tokensOutSet;
	int allowAttuned;
} ConversionSetCost;

typedef struct ConversionSets
{
	const ConversionSetCost **ppConversionSetCosts;
} ConversionSets;

extern SHARED_MEMORY ConversionSets g_ConversionSets;

void boostset_RecompileBonuses(Character *c, Power *ppow, int deleteAllAutos);
bool boostset_Finalize(BoostSetDictionary *bdict, bool shared_memory, const char * filename, bool bErrorOverwrite);
void boostset_Validate();
int boostset_Compare(const BoostSet **a, const BoostSet **b, const void *context); // used for stablesort
int boostset_CompareName(const BoostSet **a, const BoostSet **b, const void *context); // ditto
const ConversionSetCost *boostset_findConversionSet(const char *setName);
void boostset_DestroyDictHashes(BoostSetDictionary *bdict);

const BoostSet *boostset_FindByName(char *pName);

Boost * boostSet_getBoostsetIfConversionParametersVerified(Character * c, int idx, const char * conversionSet);
int boostset_hasValidConversion(Entity *e, const char *conversionSet, int level);
int boostset_Convert(Entity * e, int idx, const char *conversionSet);


#ifdef BOOSTSET_PARSE_INFO_DEFINITIONS

TokenizerParseInfo ParseBoostList[] =
{
	{ "{",                TOK_START,     0 },
	{ "Boosts",           TOK_LINKARRAY(BoostList, ppBoosts, &g_basepower_InfoLink) },
	{ "}",                TOK_END,       0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseBoostSetBonus[] =
{
	{ "{",                TOK_START,     0 },
	{ "DisplayText",      TOK_STRING(BoostSetBonus, pchDisplayName, 0) },
	{ "MinBoosts",        TOK_INT(BoostSetBonus, iMinBoosts, 0)    },
	{ "MaxBoosts",        TOK_INT(BoostSetBonus, iMaxBoosts, 0)    },
	{ "Requires",         TOK_STRINGARRAY(BoostSetBonus, ppchRequires),     }, 
	{ "AutoPowers",       TOK_LINKARRAY(BoostSetBonus, ppAutoPowers, &g_basepower_InfoLink) },
	{ "BonusPower",       TOK_LINK(BoostSetBonus, pBonusPower, 0, &g_basepower_InfoLink) },
	{ "}",                TOK_END,       0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseBoostSet[] =
{
	{ "{",					TOK_START,       0 },
	{ "Name",				TOK_STRUCTPARAM | TOK_STRING(BoostSet, pchName, 0)},
	{ "DisplayName",		TOK_STRING(BoostSet, pchDisplayName, 0) },
	{ "GroupName",			TOK_STRING(BoostSet, pchGroupName, 0) },
	{ "ConversionGroups",	TOK_STRINGARRAY(BoostSet, ppchConversionGroups),     }, 
	{ "Powers",				TOK_LINKARRAY(BoostSet, ppPowers, &g_basepower_InfoLink) },
	{ "BoostLists",			TOK_STRUCT(BoostSet, ppBoostLists, ParseBoostList)},
	{ "Bonuses",			TOK_STRUCT(BoostSet, ppBonuses, ParseBoostSetBonus)},
	{ "MinLevel",			TOK_INT(BoostSet, iMinLevel, 0) },
	{ "MaxLevel",			TOK_INT(BoostSet, iMaxLevel, 0) },
	{ "StoreProduct",		TOK_STRING(BoostSet, pchStoreProduct, 0) },
	{ "}",					TOK_END,         0 },
	{ "", 0, 0 }	
};

TokenizerParseInfo ParseBoostSetDictionary[] =
{
	{ "BoostSet", TOK_STRUCT(BoostSetDictionary, ppBoostSets, ParseBoostSet)},
	{ "", 0, 0 }
};


TokenizerParseInfo ParseConversionSetCost[] =
{
	{ "{",					TOK_START,       0 },
	{ "Name",				TOK_STRUCTPARAM | TOK_STRING(ConversionSetCost, pchConversionSetName, 0)},
	{ "InSet",				TOK_INT(ConversionSetCost, tokensInSet, 0)    },
	{ "OutSet",				TOK_INT(ConversionSetCost, tokensOutSet, 0)    },
	{ "AllowAttuned",		TOK_INT(ConversionSetCost, allowAttuned, 0)   },			
	{ "}",					TOK_END,         0 },
	{ "", 0, 0 }	
};

TokenizerParseInfo ParseConversionSet[] =
{
	{ "ConversionSet", TOK_STRUCT(ConversionSets, ppConversionSetCosts, ParseConversionSetCost)},
	{ "", 0, 0 }
};
#endif


#ifdef BOOSTSET_PARSE_INFO_DEFINITIONS
#define BOOSTSET_PARSE_INFO_DEFINED
#endif

#endif /* #ifndef BOOSTSET_H__ */

/* End of File */
