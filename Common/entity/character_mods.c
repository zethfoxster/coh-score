/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <stddef.h> // for offsetof
#include "stashtable.h"
#include <assert.h>

#include "earray.h"
#include "entai.h" // for AI_LOG
#include "utils.h"
#include "timing.h"
#include "mathutil.h"

#include "classes.h"
#include "powers.h"
#include "boostset.h"
#include "character_base.h"
#include "character_level.h"
#include "character_combat.h"

#include "attribmod.h"
#include "combat_mod.h"
#include "character_mods.h"
#include "character_animfx.h"
#include "entity.h"

void SubtractFromMaxAbsorbMods(Character *p, float amount)
{
	AttribModListIter iter;
	AttribMod *absorbMod;

	absorbMod = modlist_GetFirstMod(&iter, &p->listMod);

	while (amount > 0.0f && absorbMod)
	{
		if (absorbMod->ptemplate->offAspect == offsetof(CharacterAttribSet, pattrMax)
			&& absorbMod->offAttrib == offsetof(CharacterAttributes, fAbsorb))
		{
			float lost = MIN(amount, absorbMod->fMagnitude);

			absorbMod->fMagnitude -= lost;
			amount -= lost;

			if (amount > 0.0f) // we ran the attrib mod to zero
			{
				// If fAbsorb could be kStackType_Suppress, you'd need to add stuff here.

				modlist_RemoveAndDestroyMod(&p->listMod, absorbMod);
			}
		}

		absorbMod = modlist_GetNextMod(&iter);
	}
}

/**********************************************************************func*
 * character_AccrueMods
 *
 */
void character_AccrueMods(Character *p, float fRate, bool *pbDisallowDefeat)
{
    static StashTable perf_from_powername = 0;
    static PerformanceInfo **mod_perfs = NULL;
    
	AttribMod *pmod;
	float fHitPoints;
	float fAbsorb;
	float fEndurance;
	float fRage;
	CharacterAttribSet attribset;
	const CharacterAttributes *pattrClassMax;
	bool bRemoveSleepMods = false;
	float fLastMaxHitPoints = p->attrMax.fHitPoints;
	AttribModListIter iter;
    bool bIsDefiant = character_IsDefiant(p);
	int iPass;

	// OK, this hokey-pokey with attrTmp is to coalesce the memsets into a
	// single contiguous memset.
	CharacterAttributes attrTmp[3];
#define attrAbsolute    attrTmp[0]
#define attrResBuffs    attrTmp[1]
#define attrResDebuffs  attrTmp[2]

	pattrClassMax = &p->pclass->pattrMax[p->iCombatLevel];

	// Copy the base info into attrMax
	p->attrMax = *pattrClassMax;

	pmod = modlist_GetFirstMod(&iter, &p->listMod);

	// This skips accruing mods if there were no mods last tick and there
	// are no mods again this tick. There's no way the character's stats can
	// change in this case.
	if(pmod == NULL && p->attrMax.fHitPoints == fLastMaxHitPoints)
	{
		if(p->bNoModsLastTick)
		{
			float fMaxHp = p->attrMax.fHitPoints;

			// Praetorians in the tutorial start out not being able to go
			// over 50% health. This is controlled by a DbFlag on the ent
			// but we cache it on the character.
			if(p->bHalfMaxHealth)
				fMaxHp = fMaxHp / 2.0f;

			// HP and End are changed by the Regen tick, so they always need
			//   to be clamped. They're the only things which could change
			//   outside of attrib mods, and will always change upwards in
			//   the Regen tick.
			if(p->attrCur.fHitPoints > fMaxHp)
				p->attrCur.fHitPoints = fMaxHp;

			if(p->attrCur.fEndurance > p->attrMax.fEndurance)
				p->attrCur.fEndurance = p->attrMax.fEndurance;

			return;
		}

		p->bNoModsLastTick = true;
	}
	else
	{
		p->bNoModsLastTick = false;
	}

	memset(&attrTmp, 0, sizeof(CharacterAttributes)*3);

	// Get the base values for all the attribs
	// Make sure to save and restore counters
	fHitPoints    = p->attrCur.fHitPoints;
	fEndurance    = p->attrCur.fEndurance;
	fRage         = p->attrCur.fRage;

	p->attrCur = *p->pclass->ppattrBase[0];

	p->attrCur.fHitPoints        = fHitPoints;
	p->attrCur.fEndurance        = fEndurance;
	p->attrCur.fRage             = fRage;

	// Add any level specific combat mods
	p->attrCur.fToHit += combatmod_LevelToHitMod(p);

	// Reset accrued mods from last time
	SetModBase(&p->attrMod);

	// Reset current str
	SetCharacterAttributesToOne(&p->attrStrength);

	attribset.pattrMod = &p->attrMod;
	attribset.pattrMax = &p->attrMax;
	attribset.pattrStrength = &p->attrStrength;
	attribset.pattrResistance = &p->attrResistance;
	attribset.pattrAbsolute = &attrAbsolute;

    if(!perf_from_powername)
        perf_from_powername = stashTableCreateWithStringKeys(1024, StashDefault);
    // powers reset, clear the stashtable
    if(timing_state.resetperfinfo_init)
        stashTableClear(perf_from_powername);

	// the mods are not in any particular order in the list but we need to
	// process all the mods for attrMax first because that set of attribs
	// gets overwritten at the top of this function. if we don't do this
	// anything which works as a percentage of attrMax, or is clamped by
	// attrMax might be wrong
	// ARM NOTE:  Should we move ClampMax into this loop, between the two iterations?
    PERFINFO_AUTO_START("mods processing",1);
	for(iPass = 0; iPass < 2; iPass++)
	{
		bool bMaxOnly = true;
		pmod = modlist_GetFirstMod(&iter, &p->listMod);

		// First pass is just the mods whose template affects attrMax
		// Second pass is the opposite - all mods who are NOT for attrMax
		if(iPass)
			bMaxOnly = false;

		while(pmod!=NULL)
		{
			bool process = false;
			const char *powname = pmod->ptemplate->ppowBase->pchName;

			// ----------
			// Per-power performance start
			// ----------
#define PER_POWER_MONITORING 0
#if PER_POWER_MONITORING
			if(timing_state.autoStackDepth < timing_state.autoStackMaxDepth)
			{
				StashElement se;
				PerformanceInfo *perf;
				if(!stashFindElement(perf_from_powername, powname, &se)
					|| !(perf = stashElementGetPointer(se)))
				{
					// our pointer was cleared beneath us.
					// this is probably a timing reset
					if(!perf)
						stashTableClear(perf_from_powername);

					// need a pointer to the memory in the stashtable
					// because on reset the pointers are cleared automatically
					stashAddPointerAndGetElement(perf_from_powername, powname,NULL,TRUE, &se);
					timerAutoTimerAlloc((PerformanceInfo**)stashElementGetPointerRef(se), powname);
				}
				perf = stashElementGetPointer(se);
				PERFINFO_AUTO_START_STATIC(powname, &perf, 1);
			}
#endif

			// template says which set of attribmods in the set gets modified
			// so we can separate out the attrMax mods vs the rest
			if(bMaxOnly && pmod && pmod->ptemplate && pmod->ptemplate->offAspect == offsetof(CharacterAttribSet, pattrMax))
				process = true;
			else if (!bMaxOnly && pmod && pmod->ptemplate && pmod->ptemplate->offAspect != offsetof(CharacterAttribSet, pattrMax))
				process = true;

			if(process && mod_Process(pmod, p, &attribset, &attrResBuffs, &attrResDebuffs, &p->attrResistance, &bRemoveSleepMods, pbDisallowDefeat, fRate))
			{
				// Remove it from the list
				// AI_LOG(AI_LOG_POWERS, (p->entParent, "AccruMod: %s (Timed out and removed.)\n", pmod->ptemplate->pchName));
				if (pmod && pmod->ptemplate && pmod->ptemplate->ppowBase && pmod->ptemplate->ppowBase->bShowBuffIcon)
					p->entParent->team_buff_update = true;

				if (pmod && pmod->ptemplate && pmod->ptemplate->eStack == kStackType_Suppress
					&& !pmod->bSuppressedByStacking)
				{
					MarkLargestModForPromotionOutOfSuppression(p, pmod);
				}

				modlist_RemoveAndDestroyCurrentMod(&iter);
			}

			// ----------
			// per-power performance end
			// ----------
#if PER_POWER_MONITORING
			PERFINFO_AUTO_STOP();
#endif

			pmod = modlist_GetNextMod(&iter);
		}
	}

	PromoteModsOutOfSuppression(p);

    PERFINFO_AUTO_STOP();

	if(bRemoveSleepMods)
	{
		character_RemoveSleepMods(p);
	}

	// Track the amount lost to both the MaxMax clamp and to the cur calculations
	// To be removed from the attrib mods themselves!
	fAbsorb = p->attrMax.fAbsorb;

	// Clamp the max values so that they don't go beyond the max maxes
	//   allowed for the class at this level
	PERFINFO_AUTO_START("clamp max", 1);
	ClampMax(p, &attribset);
	PERFINFO_AUTO_STOP();

	// After calculating the max HitPoints, we scale current HP
	//  against the old max, if needed
	// This is done before calculating a new cur so that people
	//  who had their max HP go up and have proportional damage
	//  pending won't get unjustly killed.
	// People who had max go down and have absolute damage
	//  pending might get killed... but they were taking damage
	//  anyway.
	if(fLastMaxHitPoints && p->attrMax.fHitPoints != fLastMaxHitPoints)
	{
		float f = p->attrCur.fHitPoints / fLastMaxHitPoints;
		f *= p->attrMax.fHitPoints;

		if(f<1.0f && p->attrCur.fHitPoints>0.0f)
			f = 1.0f;
		p->attrCur.fHitPoints = f;
	}

	PERFINFO_AUTO_START("calc cur", 1);
	CalcCur(p, &attribset);
	PERFINFO_AUTO_STOP();

	PERFINFO_AUTO_START("sub from absorb", 1);
	SubtractFromMaxAbsorbMods(p, fAbsorb - p->attrMax.fAbsorb);
	PERFINFO_AUTO_STOP();

	if(p->pclass->offDefiantHitPointsAttrib)
	{
		float fOverkill = 1.0f - p->attrCur.fHitPoints;

		if(fOverkill > 0.0f)
		{
			float *pf = ATTRIB_GET_PTR(&p->attrCur, p->pclass->offDefiantHitPointsAttrib);

			fOverkill *= p->pclass->fDefiantScale;

            // if not already defiant, throw away the extra damage
            if(!bIsDefiant)
            {
                *pf -= fOverkill;
            }

			if(*pf < 0.0f)
			{
				// The player is dead
				p->attrCur.fHitPoints = 0.0f;
			}
			else
			{
				p->attrCur.fHitPoints = 1.0f;
			}
		}
	}

	PERFINFO_AUTO_START("clamp cur", 1);
	ClampCur(p);
	PERFINFO_AUTO_STOP();

	PERFINFO_AUTO_START("calc res", 1);
	CalcResistance(p, &attribset, &attrResBuffs, &attrResDebuffs);
	PERFINFO_AUTO_STOP();

	// Non-pet critters use this strength directly. Players and pets have
	//   their strengths clamped after factoring each power's enhancements
	//   in AccrueBoosts.
	if(ENTTYPE(p->entParent)==ENTTYPE_CRITTER &&
		p->entParent->erCreator==0)
	{
		PERFINFO_AUTO_START("clamp str", 1);
		ClampStrength(p, attribset.pattrStrength);
		PERFINFO_AUTO_STOP();
	}
}

/**********************************************************************func*
 * character_AccrueBoosts
 *
 */
void character_AccrueBoosts(Character *p, Power *ppow, float fRate)
{
	int i, iIdx;
	int iLevelEff;
	CharacterAttributes attrBoosts = {0};

	assert(p!=NULL);
	assert(ppow!=NULL);

	if(g_ulPowersDebug & 1)
		printf("Handicap: AccrueBoosts for %s\n", p->entParent->name);

	// This sets the actual set to use. Anyone who calls this function
	//   (players) will be using the power's scratchpad. Everyone else
	//   (critters) will be using the Character::attrStrength.
	ppow->pattrStrength = &ppow->attrStrength;

	ppow->bBonusAttributes = false;

	iLevelEff = character_CalcExperienceLevel(p);

	power_DestroyAllChanceMods(ppow);

	ppow->attrStrength = p->attrStrength;

	// If this power is "ignoring strength" then set the strengths which
	//   might affect the AttribMods in the power to the default of 1.0.
	//   This leaves the other strengths (such as RechargeTime, Range, etc.)
	//   alone so they can be buffed by other powers.
	if(ppow->ppowBase->bIgnoreStrength)
	{
		for(i=eaiSize(&ppow->ppowBase->piAttribCache)-1;
			i>=0;
			i--)
		{
			size_t offAttrib = ppow->ppowBase->piAttribCache[i];
			if(offAttrib < sizeof(CharacterAttributes))
			{
				float *pf = ATTRIB_GET_PTR(ppow->pattrStrength, offAttrib);
				*pf = 1.0f;
			}
		}
	}

	if(!p->bIgnoreBoosts)
	{
		for(i=eaSize(&ppow->ppBoosts)-1; i>=0; i--)
		{
			if(ppow->ppBoosts[i]!=NULL && ppow->ppBoosts[i]->ppowBase!=NULL)
			{
				int j;
				float fEff = boost_Effectiveness(ppow->ppBoosts[i], p->entParent, iLevelEff, p->entParent->erOwner);
				int iLevelBoost = ppow->ppBoosts[i]->iLevel+ppow->ppBoosts[i]->iNumCombines;

				if(fEff > 0.0f)
				{
					if (ppow->ppBoosts[i]->ppowBase->bBoostBoostable)
						iLevelBoost = ppow->ppBoosts[i]->iLevel;
					else if (ppow->ppBoosts[i]->ppowBase->bBoostUsePlayerLevel)
						iLevelBoost = MIN(iLevelEff, ppow->ppBoosts[i]->ppowBase->iMaxBoostLevel - 1);

					for(j=eaSize(&ppow->ppBoosts[i]->ppowBase->ppTemplateIdx)-1; j>=0; j--)
					{
						float fMagnitude;
						const AttribModTemplate *ptemplate = ppow->ppBoosts[i]->ppowBase->ppTemplateIdx[j];

						if(TemplateHasFlag(ptemplate, Boost))
						{
							if(ptemplate->boostModAllowed && !power_IsValidBoostMod(ppow, ptemplate))
								continue;

							// Theoretically, we should use mod_Fill here. However,
							//   boosts are defined in a very limited way so it's been
							//   optimized down to the basics.
							if(!TemplateHasAttrib(ptemplate, kSpecialAttrib_PowerChanceMod))
							{
								int iDelta;
								CombatMod *pcm;

								// Boosts use the level of the boost for magnitude lookup
								if(ptemplate->iVarIndex < 0)
									fMagnitude = class_GetNamedTableValue(p->pclass, ptemplate->pchTable, iLevelBoost);
								else
									fMagnitude = ppow->ppBoosts[i]->afVars[ptemplate->iVarIndex];

								fMagnitude *= ptemplate->fScale;

								// MAK - I modified this from IsExemplar to just testing for combat level limiting.
								// I believe this is the intent
								if (p->iCombatLevel < iLevelEff)
								{
									if(g_ulPowersDebug & 1) printf("Handicap: combat level:%d, Effective level: %d, mag: %f\n", p->iCombatLevel, iLevelEff, fMagnitude);
									fMagnitude = boost_HandicapExemplar(p->iCombatLevel, iLevelEff, fMagnitude);
								}
								else
								{
									if(g_ulPowersDebug & 1) printf("Handicap: No Handicapping for %s: combat level:%d, Effective level: %d\n", p->entParent->name, p->iCombatLevel, iLevelEff);
								}

								fMagnitude *= fEff;

								combatmod_Get(p, iLevelEff, iLevelEff, &pcm, &iDelta);
								fMagnitude = fMagnitude*combatmod_Magnitude(pcm, iDelta);

								// Theoretically, we should use mod_Process here. However,
								//   nothing special is ever done with boosts at this point
								//   so it's been optimized down to its bare essentials.
								for (iIdx = eaiSize(&ptemplate->piAttrib) - 1; iIdx >= 0; iIdx--)
								{
									size_t offAttrib = ptemplate->piAttrib[iIdx];
									float *pf;
									if (ptemplate->offAspect == offsetof(CharacterAttribSet, pattrStrength) &&
										TemplateHasFlag(ptemplate, BoostIgnoreDiminishing))
									{
										pf = ATTRIB_GET_PTR(&ppow->attrStrength, offAttrib);
									}
									else
									{
										pf = ATTRIB_GET_PTR(&attrBoosts, offAttrib);
									}
									*pf += fMagnitude;
								}
							}
							else
							{
								ModFillContext mfc = { 0 };
								mfc.fEffectiveness = mfc.fTickChance = mfc.fToHit = 1.0;
								mfc.pPow = ppow;
								mfc.fTickChance = ptemplate->fTickChance;
								for (iIdx = eaiSize(&ptemplate->piAttrib) - 1; iIdx >= 0; iIdx--)
								{
									// Special handling of PowerChanceMod boosts
									AttribMod *pmod = attribmod_Create();
									assert(pmod!=NULL);

									// mod_Fill uses level of character to determine magnitude, so this may need to change
									PERFINFO_AUTO_START("mod_Fill", 1);
										mod_Fill(pmod, ptemplate, iIdx, p, p, &mfc);
									PERFINFO_AUTO_STOP();
									eaPush(&ppow->ppmodChanceMods, pmod);
								}
							}
						}
						else
						{
							ppow->bBonusAttributes = true;
						}
					}
				}
			}
		}
		for(i=eaSize(&ppow->ppGlobalBoosts)-1; i>=0; i--)
		{
			if(ppow->ppGlobalBoosts[i]!=NULL && ppow->ppGlobalBoosts[i]->ppowBase!=NULL)
			{
				int j;
				float fEff = boost_Effectiveness(ppow->ppGlobalBoosts[i], p->entParent, iLevelEff, p->entParent->erOwner);
				int iLevelBoost = ppow->ppGlobalBoosts[i]->iLevel+ppow->ppGlobalBoosts[i]->iNumCombines;

				if(ppow->ppGlobalBoosts[i]->ppowBase->bBoostIgnoreEffectiveness || fEff > 0.0f)
				{
					for(j=eaSize(&ppow->ppGlobalBoosts[i]->ppowBase->ppTemplateIdx)-1; j>=0; j--)
					{
						float fMagnitude;
						const AttribModTemplate *ptemplate = ppow->ppGlobalBoosts[i]->ppowBase->ppTemplateIdx[j];

						if(TemplateHasFlag(ptemplate, Boost))
						{
							if(ptemplate->boostModAllowed && !power_IsValidBoostMod(ppow, ptemplate))
								continue;

							// Theoretically, we should use mod_Fill here. However,
							//   boosts are defined in a very limited way so it's been
							//   optimized down to the basics.
							if(!TemplateHasAttrib(ptemplate, kSpecialAttrib_PowerChanceMod))
							{
								int iDelta;
								CombatMod *pcm;

								// Boosts use the level of the boost for magnitude lookup
								if(ptemplate->iVarIndex < 0)
									fMagnitude = class_GetNamedTableValue(p->pclass, ptemplate->pchTable, iLevelBoost);
								else
									fMagnitude = ppow->ppGlobalBoosts[i]->afVars[ptemplate->iVarIndex];

								fMagnitude *= ptemplate->fScale;

								// MAK - I modified this from IsExemplar to just testing for combat level limiting.
								// I believe this is the intent
								if (p->iCombatLevel < iLevelEff)
								{
									if(g_ulPowersDebug & 1) printf("Handicap: combat level:%d, Effective level: %d, mag: %f\n", p->iCombatLevel, iLevelEff, fMagnitude);
									fMagnitude = boost_HandicapExemplar(p->iCombatLevel, iLevelEff, fMagnitude);
								}
								else
								{
									if(g_ulPowersDebug & 1) printf("Handicap: No Handicapping for %s: combat level:%d, Effective level: %d\n", p->entParent->name, p->iCombatLevel, iLevelEff);
								}

								fMagnitude *= fEff;

								combatmod_Get(p, iLevelEff, iLevelEff, &pcm, &iDelta);
								fMagnitude = fMagnitude*combatmod_Magnitude(pcm, iDelta);

								// Theoretically, we should use mod_Process here. However,
								//   nothing special is ever done with boosts at this point
								//   so it's been optimized down to its bare essentials.
								for (iIdx = eaiSize(&ptemplate->piAttrib) - 1; iIdx >= 0; iIdx--)
								{
									size_t offAttrib = ptemplate->piAttrib[iIdx];
									float *pf;
									if (ptemplate->offAspect == offsetof(CharacterAttribSet, pattrStrength) &&
										TemplateHasFlag(ptemplate, BoostIgnoreDiminishing))
									{
										pf = ATTRIB_GET_PTR(&ppow->attrStrength, offAttrib);
									}
									else
									{
										pf = ATTRIB_GET_PTR(&attrBoosts, offAttrib);
									}
									*pf += fMagnitude;
								}
							}
							else
							{
								ModFillContext mfc = { 0 };
								mfc.fEffectiveness = mfc.fTickChance = mfc.fToHit = 1.0;
								mfc.pPow = ppow;
								mfc.fTickChance = ptemplate->fTickChance;
								for (iIdx = eaiSize(&ptemplate->piAttrib) - 1; iIdx >= 0; iIdx--)
								{
									// Special handling of PowerChanceMod boosts
									AttribMod *pmod = attribmod_Create();
									assert(pmod!=NULL);

									// mod_Fill uses level of character to determine magnitude, so this may need to change
									PERFINFO_AUTO_START("mod_Fill", 1);
									mod_Fill(pmod, ptemplate, iIdx, p, p, &mfc);
									PERFINFO_AUTO_STOP();
									eaPush(&ppow->ppmodChanceMods, pmod);
								}
							}
						}
						else
						{
							ppow->bBonusAttributes = true;
						}
					}
				}
			}
		}
	}

	// Get a list of bonus powers from this power's sets
	boostset_RecompileBonuses(p, ppow, false);

	// Do same accum work here as the above loop, with minor tweaks
	for(i=eaSize(&ppow->ppBonuses)-1; i>=0; i--)
	{
		if(ppow->ppBonuses[i]->pBonusPower)
		{
			int j;
			
			// Bonuses don't vary with level
			float fEff = 1.0f; //boost_Effectiveness(ppow->ppBoosts[i], iLevelEff);

			for(j=eaSize(&ppow->ppBonuses[i]->pBonusPower->ppTemplateIdx)-1; j>=0; j--)
			{
				float fMagnitude;
				const AttribModTemplate *ptemplate = ppow->ppBonuses[i]->pBonusPower->ppTemplateIdx[j];
				
				if(TemplateHasFlag(ptemplate, Boost))
				{
					int iDelta;
					CombatMod *pcm;

					// Bonuses use the level of the character for magnitude lookup
					if(ptemplate->iVarIndex<0)
						fMagnitude = class_GetNamedTableValue(p->pclass, ptemplate->pchTable, iLevelEff);  // set bonuses use effective level of character (not enhancements)
					else
						fMagnitude = ppow->ppBoosts[i]->afVars[ptemplate->iVarIndex];

					fMagnitude *= ptemplate->fScale;

					fMagnitude *= fEff;

					combatmod_Get(p, iLevelEff, iLevelEff, &pcm, &iDelta);
					fMagnitude = fMagnitude*combatmod_Magnitude(pcm, iDelta);

					// Theoretically, we should use mod_Process here. However,
					//   nothing special is ever done with boosts at this point
					//   so it's been optimized down to its bare essentials.
					for (iIdx = eaiSize(&ptemplate->piAttrib) - 1; iIdx >= 0; iIdx--)
					{
						size_t offAttrib = ptemplate->piAttrib[iIdx];
						float *pf;
						if (ptemplate->offAspect == offsetof(CharacterAttribSet, pattrStrength) &&
							TemplateHasFlag(ptemplate, BoostIgnoreDiminishing))
						{
							pf = ATTRIB_GET_PTR(&ppow->attrStrength, offAttrib);
						}
						else
						{
							pf = ATTRIB_GET_PTR(&attrBoosts, offAttrib);
						}
						*pf += fMagnitude;
					}
				}
			}
		}
	}

	// Add the boost to the base strength and apply diminishing returns
	AggregateStrength(p, ppow->ppowBase, &ppow->attrStrength, &attrBoosts);

	PERFINFO_AUTO_START("clamp str", 1);

	// Set strength to 1.0 for attributes which have strength disallowed.
	for(i=eaiSize(&ppow->ppowBase->pStrengthsDisallowed)-1; i>=0; i--)
	{
		size_t offAttr = ppow->ppowBase->pStrengthsDisallowed[i];
		if(offAttr>=0 && offAttr<sizeof(CharacterAttributes))
		{
			float *pf = ATTRIB_GET_PTR(ppow->pattrStrength, offAttr);
			*pf = 1.0f;
		}
	}

	ClampStrength(p, ppow->pattrStrength);

	PERFINFO_AUTO_STOP();
}

/**********************************************************************func*
 * character_RemoveSleepMods
 *
 */
void character_RemoveSleepMods(Character *p)
{
	AttribMod *pmod;
	AttribModListIter iter;

	pmod = modlist_GetFirstMod(&iter, &p->listMod);
	while(pmod!=NULL)
	{
		if(pmod->offAttrib == offsetof(CharacterAttributes, fSleep))
		{
			// Only cancel things which affect sleeping, not strength, res,
			//   or maxes.
			if(pmod->ptemplate->offAspect == offsetof(CharacterAttribSet, pattrMod)
				|| pmod->ptemplate->offAspect == offsetof(CharacterAttribSet, pattrAbsolute))
			{
				// If scale*magnitude is less than zero, then the attrib mod
				//   is actually being used to resist sleeping. Don't
				//   cancel the buff.
				// Also don't cancel the buff if it hasn't affected them yet.
				if(pmod->ptemplate->fScale*pmod->ptemplate->fMagnitude >= 0
					&& pmod->bContinuingFXOn)
				{
					mod_CancelFX(pmod, p);
					modlist_RemoveAndDestroyCurrentMod(&iter);
					if (pmod->ptemplate->ppowBase && pmod->ptemplate->ppowBase->bShowBuffIcon)
						p->entParent->team_buff_update = true;
				}
			}
		}

		pmod = modlist_GetNextMod(&iter);
	}
}

/**********************************************************************func*
 * character_HandleConditionalFXMods
 *
 */
void character_HandleConditionalFXMods(Character *pchar)
{
	AttribMod *pmod;
	AttribModListIter iter;

	pmod = modlist_GetFirstMod(&iter, &pchar->listMod);
	while(pmod!=NULL)
	{
		if(pmod->offAttrib<sizeof(CharacterAttributes))
		{
			float *pf = ATTRIB_GET_PTR(&pchar->attrCur, pmod->offAttrib);
			const char *pchConditionalFX = basepower_GetAttribModFX(TemplateGetSub(pmod->ptemplate, pFX, pchConditionalFX), pmod->fx->pchConditionalFX);

			if(*pf>0.0f)
			{
				if(pchConditionalFX)
				{
					Entity *eSrc = erGetEnt(pmod->erSource);
					Entity *ePrevTarget = erGetEnt(pmod->erPrevTarget);
					if(!eSrc || eSrc->pchar!=pchar)
					{
						eSrc = pchar->entParent;
					}

					character_PlayMaintainedFX(pchar,
						ePrevTarget ? ePrevTarget->pchar : 0,
						eSrc->pchar,
						pchConditionalFX,
						pmod->uiTint,
						0.0,
						PLAYFX_NOT_ATTACHED, PLAYFX_NO_TIMEOUT,
						PLAYFX_CONTINUING|(pmod->fx->bImportant ? PLAYFX_IMPORTANT : 0));
				}
				character_AddStickyStates(pchar, TemplateGetSub(pmod->ptemplate, pFX, piConditionalBits));
			}
			else
			{
				if(pchConditionalFX)
					character_KillMaintainedFX(pchar, pchar, pchConditionalFX);
			}
		}

		pmod = modlist_GetNextMod(&iter);
	}
}

/* End of File */
