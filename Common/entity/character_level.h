/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_LEVEL_H__
#define CHARACTER_LEVEL_H__

#ifndef POWER_SYSTEM_H__
#include "power_system.h"
#endif

typedef struct Character Character;


void character_SetLevel(Character* p, int iLevel);
	// Forces a character to the given level.

int character_ExperienceNeeded(Character *p);
	// Calculates the number of experience points needed for the next
	//   level.

int character_AdditionalExperienceNeededForLevel(Character *p, int iLevel);
// Calculates the additional number of experience points needed for the specified
//   level.

int character_CalcExperienceLevel(Character *p);
int character_CalcExperienceLevelByExp( int iExperience);
	// Calculates which level the character has XP for. Unless sidekicked,
	//   this is the level they will fight at. It may be higher than
	//   the characetr's real level since leveling requires that you go
	//   back to your signature contact.

bool character_CanLevel(Character *p);
	// Returns true if the character has enough XP to go up a level.

bool character_CapExperienceDebt(Character *p);
	// Cap the experience debt of the character.

void character_ApplyAndReduceDebt(Character *p, int *piReceived, int *piDebtWorkedOff);
	// piReceived is input for how much experience the character is receiving
	// piReceived is output for how much experience the character should receive (i.e. the docked amount)
	// piDebtWorked off is output (only) for how much debt was worked off
	// updates the character's debt and debt-related badge stats

int character_GiveExperienceNoDebt(Character *p, int iReceived);
	// gives the character iReceived experience, without docking for debt
	// updates the character's experience and experience related badge stats
	// returns the actual amount of experience received (capped e.g. by max level)

void character_GiveExperience(Character *p, int iReceived, int *piActuallyReceived, int *piDebtWorkedOff);
	// combination of ApplyAndReduceDebt and GiveExperienceNoDebt above
	// gives the character iReceived experience docked for the character's debt
	// outputs the docked/capped experience actually received and how much debt was worked off

int character_IncreaseExperienceNoDebt(Character *p, int iIncreaseTo);
	// wrapper for GiveExperienceNoDebt above, but attempts to increase the
	//   character's experience to the given value (will not decrease)
	// returns the amount of experience actually added

void character_ApplyDefeatPenalty(Character *p, float fDebtPercentage);
	// Modify the character when defeated (XP debt, for example), given how 
	//   much of the standard debt shoul dbe applied (0.0 will give no debt,
	//   0.5 gives half debt, 1.0 gives the standard debt.)

int character_CountPoolPowerSetsBought(Character *p);
int character_CountEpicPowerSetsBought(Character *p);
int character_CountPowersBought(Character *p);
int character_CountBoostsBought(Character *p);

bool character_CanBuyPowerForLevel(Character *p, int iLevel);
int  character_CountPowersForLevel(Character *p, int iLevel);
bool character_CanBuyBoostForLevel(Character *p, int iLevel);
int  character_CountBuyBoostForLevel(Character *p, int iLevel);
bool character_CanBuyPoolPowerSetForLevel(Character *p, int iLevel);
bool character_CanBuyEpicPowerSetForLevel(Character *p, int iLevel);

bool character_CanBuyPower(Character *p);
bool character_CanBuyBoost(Character *p);
bool character_CanBuyPoolPowerSet(Character *p);
bool character_CanBuyEpicPowerSet(Character *p);

int character_GetLevel(Character *p);
int character_PowersGetMaximumLevel(Character *p);
void character_SetExperiencePoints(Character *p, int ival);
int character_GetExperiencePoints(Character *p);
int character_AddExperiencePoints(Character *p, int ival);
int character_GetExperienceDebt(Character *p);
void character_SetExperienceDebt(Character *p, int iVal);
int character_AddExperienceDebt(Character *p, int iVal);

//  1-based level info
int character_GetCombatLevelExtern(Character *p);       // Optimistic, includes side-kick adjust
int character_GetExperienceLevelExtern(Character *p);   // Optimistic, no side-kick adjust
int character_GetSecurityLevelExtern(Character *p);     // Pessimistic, player has actually leveled to here
void character_SetLevelExtern(Character *p, int iLevel);

//
// Functions which work across all power systems or only in a specific
//    power system.
//

void character_LevelFinalize(Character *p, bool grantAutoIssuePowers);
	// Updates/Resizes the character's various inventories/states for
	//   its current level.

void character_LevelUp(Character *p);
void character_LevelDown(Character *p);
	// Forcibly increase the character's level by one and update all it's
	//   stats and inventory sizes.

void character_GiveCappedExperience(Character *p, int iReceived, int *piActuallyReceived, int *piDebtWorkedOff, int *piLeftOver);
	// Handle capped XP, which gets applied 100% to debt. Returns how
	//   much XP was actually received (always 0), how much debt was
	//   worked off, and how much is left over if the debt is paid off.
	// Only operates on KPowerSystem_Powers.


//
// Shorthand for standard power system advancement. These default to using
//   kPowerSystem_Powers.
//

extern int gArchitectMapMinLevel;
extern int gArchitectMapMaxLevel;
extern int gArchitectMapMinLevelAuto;
extern int gArchitectMapMaxLevelAuto;
#endif /* #ifndef CHARACTER_LEVEL_H__ */

/* End of File */

