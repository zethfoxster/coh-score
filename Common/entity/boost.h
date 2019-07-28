/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#if (!defined(BOOST_H__)) || (defined(BOOST_PARSE_INFO_DEFINITIONS)&&!defined(BOOST_PARSE_INFO_DEFINED))
#define BOOST_H__

#include "RewardSlot.h"

// Can't include powers.h due to circular include
typedef struct CharacterOrigin CharacterOrigin;
typedef struct BasePower BasePower;
typedef struct Power Power;
typedef struct Entity Entity;
typedef struct AttribModTemplate AttribModTemplate;

#define POWER_VAR_MAX_COUNT 4
#define POWERUP_COST_SLOTS_MAX_COUNT 10

//------------------------------------------------------------
//  Info for about how powered up a boost is
// - character_Send/ReceivePowers
//----------------------------------------------------------
typedef struct PowerupSlot
{ 
	RewardSlot reward;
	bool filled;
} PowerupSlot;

bool powerupslot_Valid( PowerupSlot const *slot );
int powerupslot_ValidCount(PowerupSlot const *slots, int size);
PowerupSlot* powerupslot_Init( PowerupSlot *slot );

typedef int BoostType;
typedef struct uiEnhancement uiEnhancement;

typedef struct Boost
{
	const BasePower *ppowBase;

	int iLevel;
		// The base level of this boost.

	int iNumCombines;
		// The number of times this boost has been combined.

	Power *ppowParent;
		// The Power which contains this Boost. If NULL, the boost is
		// in the inventory.

	Power *ppowParentRespec;
		// The Power which contains this Boost before a respec. If NULL, the boost is
		// in the inventory.  This should only be not NULL during an active respec.

	int idx;
		// The index of the Boost within the Power or Inventory.

	float fTimer;
		// Timer used for enhancements which have non-boost attribs on them.
		// It handles how often the non-boost attribs go off (controlled
		// by the Boost's fActivationPeriod.

	float afVars[POWER_VAR_MAX_COUNT];
		// Values for each of the power's vars. (Defined in the BasePower.)

	PowerupSlot aPowerupSlots[POWERUP_COST_SLOTS_MAX_COUNT];
		// slots needed to activate this

	uiEnhancement *pUI;

} Boost;


Boost *boost_Create(const BasePower *ppowBase, int idx, Power *ppowParent, int iLevel, int iNumCombines, float const *afVars, int numVars, PowerupSlot const *aPowerupSlots, int numPowerupSlots );

Boost *boost_Clone(const Boost *pboost);
void boost_Destroy(Boost *pboost);
float boost_CombineChance(const Boost *pboostA, const Boost *pboostB);
float boost_BoosterChance(const Boost *pbA);
float boost_EffectivenessLevel(int iBoostLevel, int iLevel);
float boost_BoosterEffectivenessLevelByCombines(int numCombines);
float boost_BoosterEffectivenessLevel(const Boost *pbA);
float boost_Effectiveness(const Boost *pboost, Entity* e, int iLevel, bool isBoostingAPet);
bool boost_IsValidToSlotAtLevel(const Boost *pboost, Entity* e, int iLevel, bool isBoostingAPet);
int power_MinBoostUseLevel(const BasePower *ppowBase, int iBoostLevel);
int power_MaxBoostUseLevel(const BasePower *ppowBase, int iBoostLevel);
float boost_HandicapExemplar(int iCombatLevel, int iXPLevel, float fMagnitude);

bool boost_IsValidCombination(const Boost *pboostA, const Boost *pboostB);
bool boost_IsBoostable(Entity* e, const Boost *pboostA);
bool boost_IsCatalyzable(const Boost *pboostA);
bool boost_IsValidBoostForOrigin(const Boost *pboost, const BasePower *ppowBase, const CharacterOrigin *porigin);

bool power_IsValidBoost(const Power *ppow, const Boost *pboost);
bool power_IsValidBoostIgnoreOrigin(const Power *ppow, const Boost *pboost);
int power_ValidBoostSlotRequired(const Power *ppow, const Boost *pboost);
bool power_IsValidBoostMod(const Power *ppow, const AttribModTemplate *mod);

bool boost_AddPowerupslot( Boost *b, const RewardSlot *slot );

bool boost_IsOriginSpecific(const BasePower *ppowBase);

extern SHARED_MEMORY float *g_pfBoostCombineChances;
extern SHARED_MEMORY float *g_pfBoostSameSetCombineChances;

extern SHARED_MEMORY float *g_pfBoostEffectivenessAbove;
extern SHARED_MEMORY float *g_pfBoostEffectivenessBelow;

extern SHARED_MEMORY float *g_pfBoostCombineBoosterChances;
extern SHARED_MEMORY float *g_pfBoostEffectivenessBoosters;

typedef struct ExemplarHandicaps
{
	const float *pfLimits;
	const float *pfHandicaps;
	const float *pfPreClamp;
	const float *pfPostClamp;
} ExemplarHandicaps;

extern SHARED_MEMORY ExemplarHandicaps g_ExemplarHandicaps;

#ifdef BOOST_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseBoostChanceTable[] =
{
	{ "{",                TOK_START,    0 },
	{ "CombineChances",   TOK_EARRAY | TOK_F32_X, 0 },
	{ "}",                TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseBoostEffectivenessTable[] =
{
	{ "{",                TOK_START,    0 },
	{ "Effectiveness",    TOK_EARRAY | TOK_F32_X, 0 },
	{ "}",                TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseBoostExemplarTable[] =
{
	{ "{",                TOK_START,    0 },
	{ "Limits",           TOK_F32ARRAY(ExemplarHandicaps, pfLimits)    },
	{ "Weights",          TOK_F32ARRAY(ExemplarHandicaps, pfHandicaps) },
	{ "PreClamp",         TOK_F32ARRAY(ExemplarHandicaps, pfPreClamp)  },
	{ "PostClamp",        TOK_F32ARRAY(ExemplarHandicaps, pfPostClamp) },
	{ "}",                TOK_END,      0 },
	{ "", 0, 0 }
};

#endif


// The number of times a boost can be combined
#define BOOST_MAX_COMBINES 2

#ifdef BOOST_PARSE_INFO_DEFINITIONS
#define BOOST_PARSE_INFO_DEFINED
#endif

#endif /* #ifndef BOOST_H__ */

/* End of File */

