/***************************************************************************
 *     Copyright (c) 2013 Abandonware Holdings, Ltd.
 *     All Rights Reserved
 ***************************************************************************/

/* N6 note: What a huge mess this file is. The logic is all spread out,
 * mixing validation and data normalization in seemingly random functions.
 *
 * I'm attempting to clean it up and standardize on the following
 * conventions:
 *
 * Prefix indicates which level of the hierarchy this function works with.
 * powerdict_, effectgroup_, legacyattribmod_, params_, etc.
 *
 * Suffix indicates the purpose and context in which it is used:
 *
 * FixupForBin - Used after text data is loaded, to do any processing that
 *     needs to happen before the bin files are written. Should modify the
 *     structures to include whatever data needs to go into the bins. Not called
 *     if data is already shared or is loaded from bin files.
 *
 * Validate - Read-only validator to run after parsing the text data and fixing
 *     it up. Should print warnings for data problems, and return false if the
 *     data is horribly broken to the point where the parent level needs to skip
 *     this record entirely. Not called when loading from bin files.
 *
 * Finalize - Done for both text and bin files. Set up any runtime-only
 *     portions of the structure. Don't set pointers here because this
 *     happens before powers are relocated to shared memory. Not called if data
 *     is already shared.
 *
 * SetBack/ForwardPointers - Update intra-structure links and index arrays.
 *     Done multiple times as needed, both before and after shared memory
 *     relocation. Not called if data is already shared.
 *
 * FinalizeUnshared - Update per-process runtime data. Called even if data is
 *     mapped from shared memory.
 *
 * FinalizeShared - Extra finalization after being moved to shared memory.
 *     Called only if data is loaded from text/bin files. This is called late,
 *     after all the other defs have loaded, so that references to data which
 *     depends on the powers dictionary and loads after it can be fixed.
 */

#include <assert.h>
#include <float.h>
#include <string.h>

#include "file.h"  // for isDevelopmentMode
#include "error.h"
#include "earray.h"
#include "mathutil.h"
#include "StashTable.h"

#include "powers.h"
#include "attribmod.h"
#include "boostset.h"
#include "classes.h"
#include "VillainDef.h"

#include "character_eval.h"
#include "character_combat_eval.h"
#include "character_attribs.h"
#include "character_inventory.h"
#include "loaddefcommon.h"
#include "SharedMemory.h"
#include "fileutil.h"
#include "FolderCache.h"

#include "MessageStore.h"
#include "MessageStoreUtil.h"
#include "textparser.h"

#include "npc.h"
#include "pnpcCommon.h"
#include "costume_data.h"

#ifdef SERVER
#include "dbghelper.h"
#include "cmdserver.h"
#include "langServerUtil.h"

#define XLATE localizedPrintf(0,
#endif

#ifdef CLIENT
#include "cmdgame.h"
#include "sprite_text.h"
#include "fx.h"

#ifndef TEST_CLIENT
extern void uiEnhancementAddHelpText(Boost *pBoost, BasePower *pBase, int iLevel, StuffBuff *pBuf);
#endif

#define XLATE textStd(
#endif

extern StaticDefine ParsePowerDefines[]; // defined in load_def.c
extern TokenizerParseInfo ParseAttribModTemplate[];
extern TokenizerParseInfo ParseEffectGroup[];
extern TokenizerParseInfo ParseLegacyAttribModTemplate[];

PowerMode g_eRaidAttackerMode;
PowerMode g_eRaidDefenderMode;
PowerMode g_eOnCoopTeamMode;

bool s_bNeedSharedFinalization = false;
static StashTable s_PowersLoadOnly = 0;
static SharedMemoryHandle *s_pPowerdictHandle = NULL;
static const char **s_combatPhaseNames = 0;

// #define HASH_POWERS // Define to create and user name hashes to find powers.

/**********************************************************************func*
 * less
 *
 */
static int less(const void *a, const void *b)
{
	int i = *(int *)a;
	int j = *(int *)b;

	return j-i;
}


#define CHECK_TRANSLATION_CAT(pch) \
	if(pch && pch[0]!='\0') \
{ \
	char *pchXlated = XLATE pch); \
	if(stricmp(pch, pchXlated)==0) \
{ \
	pchXlated = XLATE "'%s'", pch); \
	ErrorFilenamef(pcat->pchSourceFile, "BAD TRANSLATION: %14s in field %-20s of category %s\n", pchXlated, #pch, pcat->pchName); \
} \
}


#define CHECK_TRANSLATION_SET(pch) \
	if(pch && pch[0]!='\0') \
{ \
	char *pchXlated = XLATE pch); \
	if(stricmp(pch, pchXlated)==0) \
{ \
	pchXlated = XLATE "'%s'", pch); \
	ErrorFilenamef(pset->pchSourceFile, "BAD TRANSLATION: %14s in field %-20s of power set %s\n", pchXlated, #pch, pset->pchFullName); \
} \
}


#define CHECK_TRANSLATION_POW(pch) \
	if(pch && pch[0]!='\0') \
{ \
	char *pchXlated = XLATE pch); \
	if(stricmp(pch, pchXlated)==0) \
{ \
	pchXlated = XLATE "'%s'", pch); \
	ErrorFilenamef(ppow->pchSourceFile, "BAD TRANSLATION: %14s in field %-30s of power %s\n", pchXlated, #pch, ppow->pchSourceName); \
} \
}

static void powerset_FixupForBin(const BasePowerSet *psetBase)
{
	if (psetBase->ppSpecializeRequires)
	{
		chareval_Validate(psetBase->ppSpecializeRequires, psetBase->pchSourceFile);
	}
}

static void power_FixupForBinFX(BasePower *ppow, PowerFX *fx)
{
	//
	// Do some sanity checking on the timing and animations
	//
	if(fx->iFramesBeforeHit==-1)
	{
		if(fx->iFramesAttack>35)
			ErrorFilenamef(fx->pchSourceFile, "STEVE: Long anim (%d frames) with no FramesBeforeHit: (%s)\n", fx->iFramesAttack, ppow->pchFullName);
		fx->iFramesBeforeHit = 15;
	}

	if(fx->iInitialFramesBeforeHit==-1)
		fx->iInitialFramesBeforeHit = fx->iFramesBeforeHit;

#ifdef CLIENT
	// Checks all attack and hitfx for a death condition
	if( game_state.fxdebug )
	{
		FxDebugAttributes attribs;

		if( fx->pchHitFX )
		{
			if( FxDebugGetAttributes( 0, fx->pchHitFX, &attribs ) )
			{
				if( attribs.lifeSpan <= 0 && !(attribs.type & FXTYPE_TRAVELING_PROJECTILE) )
					printf( "FX %s used by %s as a oneshot HitFX, but wont die\n", fx->pchHitFX, ppow->pchFullName );
			}
			else
			{
				printf("Nonexistent Fx(%s %s)\n", ppow->pchFullName, fx->pchHitFX);
			}
		}

		if( fx->pchSecondaryAttackFX )
		{
			if( FxDebugGetAttributes( 0, fx->pchSecondaryAttackFX, &attribs ) )
			{
				if( attribs.lifeSpan <= 0 && !(attribs.type & FXTYPE_TRAVELING_PROJECTILE) )
					printf( "FX %s used by %s as a oneshot ChainFX, but wont die\n", fx->pchSecondaryAttackFX, ppow->pchFullName );
			}
			else
			{
				printf("Nonexistent Fx(%s %s)\n", ppow->pchFullName, fx->pchSecondaryAttackFX);
			}
		}

		if( fx->pchAttackFX )
		{
			if( FxDebugGetAttributes( 0, fx->pchAttackFX, &attribs ) )
			{
				if( attribs.lifeSpan <= 0 && !(attribs.type & FXTYPE_TRAVELING_PROJECTILE) )
					printf( "FX %s used by %s as a oneshot AttackFX, but wont die\n", fx->pchAttackFX, ppow->pchFullName );
			}
			else
			{
				printf("Nonexistent Fx(%s %s)\n", ppow->pchFullName, fx->pchAttackFX);
			}
		}

		if( fx->pchInitialAttackFX )
		{
			if( FxDebugGetAttributes( 0, fx->pchInitialAttackFX, &attribs ) )
			{
				if( attribs.lifeSpan <= 0 && !(attribs.type & FXTYPE_TRAVELING_PROJECTILE) )
					printf( "FX %s used by %s as a oneshot InitialAttackFX, but wont die\n", fx->pchInitialAttackFX, ppow->pchFullName );
			}
			else
			{
				printf("Nonexistent Fx(%s %s)\n", ppow->pchFullName, fx->pchInitialAttackFX);
			}
		}

		if( fx->pchBlockFX )
		{
			if( FxDebugGetAttributes( 0, fx->pchBlockFX, &attribs ) )
			{
				if( attribs.lifeSpan <= 0 && !(attribs.type & FXTYPE_TRAVELING_PROJECTILE) )
					printf( "FX %s used by %s as a oneshot BlockFX, but wont die\n", fx->pchBlockFX, ppow->pchFullName );
			}
			else
			{
				printf("Nonexistent Fx(%s %s)\n", ppow->pchName, fx->pchBlockFX);
			}
		}

		if( fx->pchDeathFX)
		{
			if( FxDebugGetAttributes( 0, fx->pchDeathFX, &attribs ) )
			{
				if( attribs.lifeSpan <= 0 && !(attribs.type & FXTYPE_TRAVELING_PROJECTILE) )
					printf( "FX %s used %s as a oneshot HitFX, but wont die", fx->pchDeathFX, ppow->pchFullName );
			}
			else
			{
				printf("Nonexistent Fx(%s %s)\n", ppow->pchFullName, fx->pchDeathFX);
			}
		}
	}
#endif

	if(fx->pchInitialAttackFX==NULL && fx->pchAttackFX!=NULL)
		fx->pchInitialAttackFX=ParserAllocString(fx->pchAttackFX);

	if(fx->piInitialAttackBits==NULL && fx->piAttackBits!=NULL)
		eaiCopy(&fx->piInitialAttackBits, &fx->piAttackBits);

	// Don't print warnings if defaulted.
	if(fx->iFramesAttack==-1)
	{
		fx->iFramesAttack = 35;
	}
	else if(ppow->eType == kPowerType_Click || ppow->eType == kPowerType_Toggle)
	{
		float fAnimTime = fx->iFramesAttack/30.0f;
		if(ppow->fTimeToActivate-ppow->fInterruptTime-fAnimTime > 0.01f
			|| ppow->fTimeToActivate-ppow->fInterruptTime-fAnimTime < -0.01f)
		{
			if(fx->pchIgnoreAttackTimeErrors && *fx->pchIgnoreAttackTimeErrors)
			{
				// Suppressed
			}
			else
			{
				//ErrorFilenamef(ppow->pchSourceFile, "(%s.%s.%s)\n  TimeToActivate (%1.2fs) != FramesAttack (%1.2fs)\nIf you are certain that this is correct, you may ignore this error by putting \"IgnoreAttackTimeErrors %s\" in the power definition.\n",
				//	pcat->pchName, pset->pchName, ppow->pchName,
				//	ppow->fTimeToActivate-ppow->fInterruptTime,
				//	fAnimTime,
				//	getUserName());
#ifdef TEST_CLIENT
				printf("%s: Animation Powers (%s)\n  TimeToActivate (%1.2fs) != FramesAttack (%1.2fs)\n",
					ppow->pchSourceFile, ppow->pchFullName, ppow->fTimeToActivate-ppow->fInterruptTime, fAnimTime);
#else
				ErrorFilenamef(ppow->pchSourceFile,
					"(%s)\n  TimeToActivate (%1.2fs) != FramesAttack (%1.2fs)\nIf you are certain that this is correct, you may ignore this error by putting \"IgnoreAttackTimeErrors %s\" in the power definition.\n",
					ppow->pchFullName, ppow->fTimeToActivate-ppow->fInterruptTime,
					fAnimTime, getUserName());
#endif
			}
		}
		else
		{
			if(fx->pchIgnoreAttackTimeErrors && *fx->pchIgnoreAttackTimeErrors)
			{
				ErrorFilenamef(ppow->pchSourceFile, "(%s) %s used IgnoreAttackTimeErrors and didn't need to!  It should be removed.\n",
					ppow->pchFullName,
					fx->pchIgnoreAttackTimeErrors);
			}
		}
	}

	if(fx->bDelayedHit)
	{
		if(fx->fProjectileSpeed==0.0f)
			fx->fProjectileSpeed = 60.0f;
	}
	else
	{
		fx->fProjectileSpeed = FLT_MAX;
	}
}

static void UpdateEvalFlags(int *evalFlags, const char **ppchExpr,
							const BasePower *ppow, bool bIsRequires, bool bIsDelayed)
{
	int i;
	if (!ppchExpr)
		return;

	for(i = eaSize(&ppchExpr) - 1; i >= 0; i--)
	{
		const char* requiresOperand = ppchExpr[i];
		if(strstriConst(requiresOperand, "@ToHit") || strstriConst(requiresOperand, "@ToHitRoll") ||
			strstriConst(requiresOperand, "@AlwaysHit") || strstriConst(requiresOperand, "@ForceHit"))
				*evalFlags |= ATTRIBMOD_EVAL_HIT_ROLL;
		if(strstriConst(requiresOperand, "@Value") || strstriConst(requiresOperand, "@Scale") ||
			strstriConst(requiresOperand, "@Effectiveness") || strstriConst(requiresOperand, "@Strength") ||
			strstriConst(requiresOperand, "@StdResult")) {
				if (bIsRequires)
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) uses Expression-only operands in an ApplicationRequires.\n", ppow->pchFullName);
				else
					*evalFlags |= ATTRIBMOD_EVAL_CALC_INFO;
		}
		if(strstriConst(requiresOperand, "@CustomFX"))
			*evalFlags |= ATTRIBMOD_EVAL_CUSTOMFX;
		if(strstriConst(requiresOperand, "@TargetsHit")) {
			if (bIsDelayed)
				*evalFlags |= ATTRIBMOD_EVAL_TARGETSHIT;
			else
				ErrorFilenamef(ppow->pchSourceFile, "Power (%s) uses @TargetsHit in a non-delayed expression.\n", ppow->pchFullName);
		}
		if(strstriConst(requiresOperand, "@Chance") || strstriConst(requiresOperand, "@ChanceMods"))
			*evalFlags |= ATTRIBMOD_EVAL_CHANCE;
		if(strstriConst(requiresOperand, "@ChainJump") || strstriConst(requiresOperand, "@ChainEff"))
			*evalFlags |= ATTRIBMOD_EVAL_CHAIN;
	}

	if (bIsDelayed)
		*evalFlags |= ATTRIBMOD_EVAL_DELAY;

#if SERVER
	combateval_Validate(dbg_BasePowerStr(ppow), ppchExpr, ppow->pchSourceFile);
#endif
}

#define PARAMCASE(t) \
	case ParamType_##t: \
	{ \
		AttribModParam_##t *param = (AttribModParam_##t*)TemplateGetParams(ptemplate, ##t);
#define PARAMCASEEND \
	} \
	break
#define PARAMGETLO(t) AttribModParam_##t##_loadonly *paramlo = ParserGetLoadOnly(param)

static void params_FixupForBin(BasePower *ppow, AttribModTemplate *ptemplate)
{
	switch (*(int*)ptemplate->pParams)
	{
		PARAMCASE(Phase) {
			int i, phaseidx;
			AttribModParam_Phase_loadonly *paramlo = ParserGetLoadOnly(param);

			if (TemplateHasAttrib(ptemplate, kSpecialAttrib_CombatPhase)) {
				// Combat phases are dynamic, so set them up if this is the first time
				if (!s_combatPhaseNames) {
					eaCreateConst(&s_combatPhaseNames);
					eaInsertConst(&s_combatPhaseNames, "NoPrime", 0);
					eaInsertConst(&s_combatPhaseNames, "Friendly", 1);
					eaInsertConst(&s_combatPhaseNames, "Prime", 2);
				}
				for (i = eaSize(&paramlo->ppchCombatPhaseNames) - 1; i >= 0; i--) {
					for (phaseidx = eaSize(&s_combatPhaseNames) - 1; phaseidx >= 0; phaseidx--) {
						if (!stricmp(paramlo->ppchCombatPhaseNames[i], s_combatPhaseNames[phaseidx]))
							break;
					}
					if (phaseidx < 0) {
						phaseidx = eaSize(&s_combatPhaseNames);
						devassert(phaseidx < 32);
						eaPushConst(&s_combatPhaseNames, paramlo->ppchCombatPhaseNames[i]);
					}
					eaiPush(&param->piCombatPhases, phaseidx);
				}
			}

			if (TemplateHasAttrib(ptemplate, kSpecialAttrib_VisionPhase)) {
				// Vision phases however are statically defined
				// TODO: Maybe make the parse handle this itself with a StaticIntDefine

				for (i = eaSize(&paramlo->ppchVisionPhaseNames) - 1; i >= 0; i--) {
					for (phaseidx = eaSize(&g_visionPhaseNames.visionPhases) - 1; phaseidx >= 0; phaseidx--) {
						if (!stricmp(paramlo->ppchVisionPhaseNames[i], g_visionPhaseNames.visionPhases[phaseidx])) {
							eaiPush(&param->piVisionPhases, phaseidx);
							break;
						}
					}

					if (phaseidx < 0)
						ErrorFilenamef(ppow->pchSourceFile, "Power (%s): Vision Phase '%s' not found in visionPhases.def!", ppow->pchFullName, paramlo->ppchVisionPhaseNames[i]);
				}
			}

			if (TemplateHasAttrib(ptemplate, kSpecialAttrib_ExclusiveVisionPhase)) {
				for (phaseidx = eaSize(&g_exclusiveVisionPhaseNames.visionPhases) - 1; phaseidx >= 0; phaseidx--) {
					if (!stricmp(paramlo->pchExclusiveVisionPhaseName, g_exclusiveVisionPhaseNames.visionPhases[phaseidx])) {
						param->iExclusiveVisionPhase = phaseidx;
						break;
					}
				}
				if (phaseidx < 0)
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s): Vision Phase '%s' not found in visionPhasesExclusive.def!", ppow->pchFullName, paramlo->pchExclusiveVisionPhaseName);
			}

		} PARAMCASEEND;
	}

}

static void effectgroup_FixupForBin(BasePower *ppow, EffectGroup *pEffect,
									bool *bAllNearGround, bool *hasUntilShutOffMods,
									bool *hasOnlyUntilShutOffMods)
{
	int imod, i, iSize;

	// Get eval flags for effect-wide requires
	UpdateEvalFlags(&pEffect->evalFlags, pEffect->ppchRequires, ppow, true, false);

	// Go through all attribmod templates in effect group
	iSize = eaSize(&pEffect->ppTemplates);
	for(imod=0; imod<iSize; imod++)
	{
		AttribModTemplate *ptemplate = cpp_const_cast(AttribModTemplate*)(pEffect->ppTemplates[imod]);

		if (ptemplate->pParams)
			params_FixupForBin(ppow, ptemplate);

		if(ptemplate->fDuration>=ATTRIBMOD_DURATION_FOREVER)
		{
			*hasUntilShutOffMods = true;

			if(ptemplate->eApplicationType != kModApplicationType_OnActivate)
				ErrorFilenamef(ppow->pchSourceFile, "kUntilShutOff AttribMod not set to ApplicationType OnActivate in power (%s).\n",ppow->pchFullName);

			ptemplate->eApplicationType = kModApplicationType_OnActivate;
		}
		else
			*hasOnlyUntilShutOffMods = false;

		// Determine if the duration and magnitude should be
		//   affected by combat mods or resistance.
		if(ptemplate->eType == kModType_Constant ||
			(ptemplate->eType == kModType_Expression && ptemplate->ppchDuration && ptemplate->ppchMagnitude))
		{
			if (!TemplateHasFlag(ptemplate, IgnoreCombatMods)
				&& !TemplateHasFlag(ptemplate, CombatModDuration)
				&& !TemplateHasFlag(ptemplate, CombatModMagnitude))
			{
				TemplateSetFlag(ptemplate, CombatModDuration);
				TemplateSetFlag(ptemplate, CombatModMagnitude);
			}
			if (!TemplateHasFlag(ptemplate, IgnoreResistance)
				&& !TemplateHasFlag(ptemplate, ResistDuration)
				&& !TemplateHasFlag(ptemplate, ResistMagnitude))
			{
				TemplateSetFlag(ptemplate, ResistDuration);
				TemplateSetFlag(ptemplate, ResistMagnitude);
			}
		}
		else if(ptemplate->eType == kModType_Duration
			|| (ptemplate->eType == kModType_Expression && ptemplate->ppchDuration))
		{
			if (!TemplateHasFlag(ptemplate, IgnoreCombatMods)
				&& !TemplateHasFlag(ptemplate, CombatModDuration)
				&& !TemplateHasFlag(ptemplate, CombatModMagnitude))
				TemplateSetFlag(ptemplate, CombatModDuration);
			if (!TemplateHasFlag(ptemplate, IgnoreResistance)
				&& !TemplateHasFlag(ptemplate, ResistDuration)
				&& !TemplateHasFlag(ptemplate, ResistMagnitude))
				TemplateSetFlag(ptemplate, ResistDuration);
		}
		else if(ptemplate->eType == kModType_Magnitude
			|| ptemplate->eType == kModType_SkillMagnitude
			|| (ptemplate->eType == kModType_Expression && ptemplate->ppchMagnitude))
		{
			if (!TemplateHasFlag(ptemplate, IgnoreCombatMods)
				&& !TemplateHasFlag(ptemplate, CombatModDuration)
				&& !TemplateHasFlag(ptemplate, CombatModMagnitude))
				TemplateSetFlag(ptemplate, CombatModMagnitude);
			if (!TemplateHasFlag(ptemplate, IgnoreResistance)
				&& !TemplateHasFlag(ptemplate, ResistDuration)
				&& !TemplateHasFlag(ptemplate, ResistMagnitude))
				TemplateSetFlag(ptemplate, ResistMagnitude);
		}

		*bAllNearGround = *bAllNearGround && TemplateHasFlag(ptemplate, NearGround);

		UpdateEvalFlags(&pEffect->evalFlags, ptemplate->ppchDelayedRequires, ppow, true, true);
		UpdateEvalFlags(&pEffect->evalFlags, ptemplate->ppchMagnitude, ppow, false, TemplateHasFlag(ptemplate, DelayEval));
		UpdateEvalFlags(&pEffect->evalFlags, ptemplate->ppchDuration, ppow, false, TemplateHasFlag(ptemplate, DelayEval));

		// Make sure there are enough target counts for marker locations
		if (ptemplate->pTargetInfo && ptemplate->pTargetInfo->ppchMarkerNames)
		{
			int i, szdiff = eaSize(&ptemplate->pTargetInfo->ppchMarkerNames) -
				eaiSize(&ptemplate->pTargetInfo->piMarkerCount);
			for (i = 0; i < szdiff; i++)
				eaiPush(&ptemplate->pTargetInfo->piMarkerCount, -1);
		}

		if (TemplateHasFlag(ptemplate, DelayEval) && ptemplate->eStack == kStackType_Maximize)
			ErrorFilenamef(ppow->pchSourceFile, "Power (%s) uses kStackType_Maximize and has DelayEval.\n", ppow->pchFullName);

		if (TemplateHasFlag(ptemplate, DelayEval) && ptemplate->eStack == kStackType_Suppress)
			ErrorFilenamef(ppow->pchSourceFile, "Power (%s) uses kStackType_Suppress and has DelayEval.\n", ppow->pchFullName);

		if (TemplateHasAttrib(ptemplate, offsetof(CharacterAttributes, fAbsorb)) && ptemplate->eStack == kStackType_Suppress)
		{
			ErrorFilenamef(ppow->pchSourceFile, "Power (%s) uses kStackType_Suppress on an Absorb mod, this isn't supported. Changing to kStackType_Stack.\n", ppow->pchFullName);
			ptemplate->eStack = kStackType_Stack;
		}

		// Update the power's attrib cache
		for (i = 0; i < eaiSize(&ptemplate->piAttrib); i++) {
			eaiSortedPushUnique(&ppow->piAttribCache, ptemplate->piAttrib[i]);
		}
	} // for each attrib mod template

	// Designer laziness fixuppery
	if(pEffect->fRadiusOuter == 0.0f && pEffect->fRadiusInner < 0.0f)
	{
		pEffect->fRadiusInner = 0.0f;;
	}

	// If this group has any child groups, process them
	for (i = 0; i < eaSize(&pEffect->ppEffects); i++) {
		effectgroup_FixupForBin(ppow, (EffectGroup*)pEffect->ppEffects[i],
			bAllNearGround, hasUntilShutOffMods, hasOnlyUntilShutOffMods);
	}
}

#define NEW_PARAM(type, name) AttribModParam_##type *name = ParserAllocStruct(sizeof(AttribModParam_##type)); \
	ParserInitStruct(name, sizeof(AttribModParam_##type), ParseAttribModParam_##type); \
	ParserCopyMeta(pLegacy, name); \
	name->eType = ParamType_##type
#define NEW_PARAMLO(type, name, loname) AttribModParam_##type *name = ParserAllocStruct(sizeof(AttribModParam_##type)); \
	AttribModParam_##type##_loadonly *loname = ParserAcquireLoadOnlyEx(s_PowersLoadOnly, name, ParseAttribModParam_##type); \
	ParserCopyMeta(pLegacy, name); \
	ParserCopyMeta(pLegacy, loname); \
	ParserInitStruct(name, sizeof(AttribModParam_##type), ParseAttribModParam_##type); \
	name->eType = ParamType_##type
#define MOVEPTR(src, dest) (dest) = (src); (src) = NULL
#define PUSHPTRC(src, dest) eaPushConst(&(dest), (src)); (src) = NULL
static void legacyattribmod_MigrateParams(BasePower *ppow, LegacyAttribModTemplate *pLegacy,
										  EffectGroup *pnewEffect, AttribModTemplate *pnewTemplate)
{
	// --- GrantPower, RevokePower
	if ( (pLegacy->offAttrib == kSpecialAttrib_GrantPower ||
		  pLegacy->offAttrib == kSpecialAttrib_GrantBoostedPower ||
		  pLegacy->offAttrib == kSpecialAttrib_RevokePower)
		&& pLegacy->pchReward )
	{
		NEW_PARAM(Power, params);

		PUSHPTRC(pLegacy->pchReward, params->ps.ppchPowerNames);
		if (pLegacy->pchParams) {
			if(stricmp(pLegacy->pchParams, "all") == 0)
				TemplateSetFlag(pnewTemplate, RevokeAll);
			else
				params->iCount = atoi(pLegacy->pchParams);
		} else
			params->iCount = 1;

		pnewTemplate->pParams = params;
		return;
	}

	// --- EntCreate
	if ( pLegacy->offAttrib == kSpecialAttrib_EntCreate
		&& pLegacy->pchEntityDef )
	{
		NEW_PARAMLO(EntCreate, params, paramslo);

		MOVEPTR(pLegacy->pchEntityDef, paramslo->pchEntityDef);
		MOVEPTR(pLegacy->pchPriorityList, params->pchPriorityList);

		TemplateSetFlag(pnewTemplate, CopyBoosts);

		pnewTemplate->pParams = params;
		return;
	}

	// --- RechargePower
	if ( pLegacy->offAttrib == kSpecialAttrib_RechargePower
		&& pLegacy->pchReward )
	{
		NEW_PARAM(Power, params);

		if (!strchr(pLegacy->pchReward, '.')) {
			PUSHPTRC(pLegacy->pchReward, params->ps.ppchCategoryNames);
		} else if (strchr(pLegacy->pchReward, '.') == strrchr(pLegacy->pchReward, '.')) {
			PUSHPTRC(pLegacy->pchReward, params->ps.ppchPowersetNames);
		} else {
			PUSHPTRC(pLegacy->pchReward, params->ps.ppchPowerNames);
		}
		pnewTemplate->pParams = params;
		return;
	}

	// --- SetCostume
	if (pLegacy->offAttrib == kSpecialAttrib_SetCostume &&
		pLegacy->pchCostumeName)
	{
		NEW_PARAMLO(Costume, params, paramslo);

		if (pLegacy->pchParams)
			params->iPriority = atoi(pLegacy->pchParams);

		MOVEPTR(pLegacy->pchCostumeName, paramslo->pchCostumeName);
		pnewTemplate->pParams = params;
		return;
	}

	// --- Phase
	if (pLegacy->offAttrib == kSpecialAttrib_CombatPhase &&
		pLegacy->pchParams)
	{
		NEW_PARAMLO(Phase, params, paramslo);
		PUSHPTRC(pLegacy->pchParams, paramslo->ppchCombatPhaseNames);
		pnewTemplate->pParams = params;
		return;
	}
	if (pLegacy->offAttrib == kSpecialAttrib_VisionPhase &&
		pLegacy->pchParams)
	{
		NEW_PARAMLO(Phase, params, paramslo);
		PUSHPTRC(pLegacy->pchParams, paramslo->ppchVisionPhaseNames);
		pnewTemplate->pParams = params;
		return;
	}
	if (pLegacy->offAttrib == kSpecialAttrib_ExclusiveVisionPhase &&
		pLegacy->pchParams)
	{
		NEW_PARAMLO(Phase, params, paramslo);
		MOVEPTR(pLegacy->pchParams, paramslo->pchExclusiveVisionPhaseName);
		pnewTemplate->pParams = params;
		return;
	}

	// --- Reward
	if ( (pLegacy->offAttrib == kSpecialAttrib_Reward ||
		  pLegacy->offAttrib == kSpecialAttrib_RewardSource ||
		  pLegacy->offAttrib == kSpecialAttrib_RewardSourceTeam)
		&& pLegacy->pchReward )	{
		NEW_PARAM(Reward, params);

		PUSHPTRC(pLegacy->pchReward, params->ppchRewards);
		pnewTemplate->pParams = params;
		return;
	}

	// --- Teleport
	if ( pLegacy->offAttrib == offsetof(CharacterAttributes, fTeleport)
		&& pLegacy->pchParams )	{
		NEW_PARAM(Teleport, params);

		MOVEPTR(pLegacy->pchParams, params->pchDestination);
		pnewTemplate->pParams = params;
		return;
	}

	// --- AddBehavior
	if ( pLegacy->offAttrib == kSpecialAttrib_AddBehavior
		&& eaSize(&pLegacy->ppchPrimaryStringList) == 1 ) {
		NEW_PARAM(Behavior, params);

		MOVEPTR(pLegacy->ppchPrimaryStringList, params->ppchBehaviors);
		pnewTemplate->pParams = params;
		return;
	}

	// --- SZEValue
	if ( pLegacy->offAttrib == kSpecialAttrib_SetSZEValue &&
		eaSize(&pLegacy->ppchPrimaryStringList) > 0 &&
		eaSize(&pLegacy->ppchSecondaryStringList) > 0 ) {
		NEW_PARAM(SZEValue, params);

		MOVEPTR(pLegacy->ppchPrimaryStringList, params->ppchScriptID);
		MOVEPTR(pLegacy->ppchSecondaryStringList, params->ppchScriptValue);
		pnewTemplate->pParams = params;
		return;
	}

	// --- Token
	if ( (pLegacy->offAttrib == kSpecialAttrib_TokenAdd ||
		  pLegacy->offAttrib == kSpecialAttrib_TokenSet ||
		  pLegacy->offAttrib == kSpecialAttrib_TokenClear)
		&& pLegacy->pchReward )	{
		NEW_PARAM(Token, params);

		PUSHPTRC(pLegacy->pchReward, params->ppchTokens);
		pnewTemplate->pParams = params;
		return;
	}

	// --- ChanceMods
	if ( (pLegacy->offAttrib == kSpecialAttrib_GlobalChanceMod ||
		  pLegacy->offAttrib == kSpecialAttrib_PowerChanceMod)
	    && pLegacy->pchReward ) {
		NEW_PARAM(EffectFilter, params);

		PUSHPTRC(pLegacy->pchReward, params->ppchTags);
		pnewTemplate->pParams = params;
	}
}

static void effectgroup_ClearTemplates(EffectGroup *peff)
{
	int i = 0;

	if (!peff->ppTemplates)
		return;
	for (i = eaSize(&peff->ppTemplates) - 1; i >= 0; i--) {
		ParserDestroyStruct(ParseAttribModTemplate, (AttribModTemplate*)peff->ppTemplates[i]);
	}
	eaDestroyConst(&peff->ppTemplates);
	peff->ppTemplates = NULL;
}

// Add an effect group and template derived from a legacy template to
// a power. Tries to coalesce similar effects where possible.
static void legacyattribmod_Add(BasePower *ppow, EffectGroup *peffect,
								AttribModTemplate *ptemplate)
{
	EffectGroup *trialeffA, *trialeffB;
	AttribModTemplate *trialmodA, *trialmodB;
	bool bMergedEffect = false, bMergedTemplate = false;
	int i;

	trialeffA = ParserAllocStruct(sizeof(EffectGroup));
	trialeffB = ParserAllocStruct(sizeof(EffectGroup));

	// First see if there are any effect groups that are equivalent
	ParserCopyStruct(ParseEffectGroup, peffect, sizeof(EffectGroup), trialeffA);
	effectgroup_ClearTemplates(trialeffA);

	for (i = eaSize(&ppow->ppEffects) - 1; i >= 0; i--) {
		ParserCopyStruct(ParseEffectGroup, ppow->ppEffects[i], sizeof(EffectGroup), trialeffB);
		effectgroup_ClearTemplates(trialeffB);

		if ( StructCompare(ParseEffectGroup, trialeffA, trialeffB) == 0 &&
			 (EffectHasFlag(trialeffA, LinkedChance) ||
			 trialeffA->fChance == 1 && trialeffA->fProcsPerMinute == 0) )
		{
			// Effect groups are the same and not chance dependent, so ditch the new one
			ParserDestroyStruct(ParseEffectGroup, peffect);
			ParserFreeStruct(peffect);
			peffect = (EffectGroup*)ppow->ppEffects[i];
			bMergedEffect = true;
			break;
		}
	}

	ParserDestroyStruct(ParseEffectGroup, trialeffA);
	ParserDestroyStruct(ParseEffectGroup, trialeffB);
	ParserFreeStruct(trialeffA);
	ParserFreeStruct(trialeffB);

	if (bMergedEffect) {
		// Okay, we merged the effect group. Now see if this group
		// has any mod teampltes that are identical except for the
		// attribute.
		trialmodA = ParserAllocStruct(sizeof(AttribModTemplate));
		trialmodB = ParserAllocStruct(sizeof(AttribModTemplate));

		ParserCopyStruct(ParseAttribModTemplate, ptemplate, sizeof(AttribModTemplate), trialmodA);
		eaiDestroy(&trialmodA->piAttrib);
		trialmodA->piAttrib = 0;

		for (i = eaSize(&peffect->ppTemplates) - 1; i >= 0; i--) {
			// piAttrib entries must be unique, don't consolidate if it would add a duplicate.
			// ptemplate will only ever have a single attrib since it was constructed from a
			// legacy template.
			if (TemplateHasAttrib(peffect->ppTemplates[i], ptemplate->piAttrib[0]))
				continue;

			ParserCopyStruct(ParseAttribModTemplate, peffect->ppTemplates[i], sizeof(AttribModTemplate), trialmodB);
			eaiDestroy(&trialmodB->piAttrib);
			trialmodB->piAttrib = 0;

			if ( StructCompare(ParseAttribModTemplate, trialmodA, trialmodB) == 0 &&
				 (EffectHasFlag(peffect, LinkedChance) ||
				 trialmodA->fTickChance == 1) )
			{
				// Sweet. We can just shove all the attribs into the existing template
				eaiPushArray(&peffect->ppTemplates[i]->piAttrib, &ptemplate->piAttrib);
				ParserDestroyStruct(ParseAttribModTemplate, ptemplate);
				ParserFreeStruct(ptemplate);
				ptemplate = (AttribModTemplate*)peffect->ppTemplates[i];
				bMergedTemplate = true;
				break;
			}
		}

		ParserDestroyStruct(ParseAttribModTemplate, trialmodA);
		ParserDestroyStruct(ParseAttribModTemplate, trialmodB);
		ParserFreeStruct(trialmodA);
		ParserFreeStruct(trialmodB);
	}

	if (!bMergedTemplate)
		eaPushConst(&peffect->ppTemplates, ptemplate);
	if (!bMergedEffect)
		eaPushConst(&ppow->ppEffects, peffect);
}

static void power_FixupForBin(BasePower *ppow, int **crcFullNameList)
{
	int iSizeVars;
	bool bAllNearGround;
	bool hasUntilShutOffMods = false;
	bool hasOnlyUntilShutOffMods = true;
	int imod, ieff, ifx;
	int iEffSize;
	BasePower_loadonly *ppowlo = ParserGetLoadOnly(ppow);
	bool bLegacyPower = ppowlo ? (eaSize(&ppowlo->ppLegacyTemplates) > 0) : false;

	ppow->crcFullName = powerdict_GetCRCOfBasePower(ppow);
	assert(ppow->crcFullName);	//	make sure it doesn't CRC to 0 (which we're reserving to test for NULLs)
	assert(-1 == eaiFind(crcFullNameList, ppow->crcFullName));
	eaiPush(crcFullNameList, ppow->crcFullName);

	// Change degrees to radians
	ppow->fArc = RAD(ppow->fArc);
	ppow->fPositionYaw = RAD(ppow->fPositionYaw);

	// Fix up invention vars
	iSizeVars = eaSize(&ppow->ppVars);
	if(iSizeVars > POWER_VAR_MAX_COUNT)
	{
		ErrorFilenamef(ppow->pchSourceFile, "Too many vars defined for power (%s).\n",ppow->pchFullName);
		eaSetSize(&cpp_const_cast(PowerVar**)(ppow->ppVars), POWER_VAR_MAX_COUNT);
	}
	for(imod = 0; imod<iSizeVars; imod++)
	{
		if(ppow->ppVars[imod]->iIndex >= POWER_VAR_MAX_COUNT)
		{
			ErrorFilenamef(ppow->pchSourceFile, "Invalid index for vars on power (%s).\n",ppow->pchFullName);
			(cpp_const_cast(PowerVar*)(ppow->ppVars[imod]))->iIndex = -1;
		}
	}

	power_FixupForBinFX(ppow, &ppow->fx);
	for(ifx = eaSize(&ppow->customfx)-1; ifx >= 0; --ifx)
		power_FixupForBinFX(ppow, cpp_const_cast(PowerFX*)(&ppow->customfx[ifx]->fx));

	// Sort BoostsAllowed so it's easier to compare them.
	qsort(cpp_const_cast(BoostType*)(ppow->pBoostsAllowed), eaiSize(&(int *)ppow->pBoostsAllowed), sizeof(int), less);

	//
	// Validate the power requires.
	//
	if(ppow->ppchBuyRequires)
	{
		chareval_Validate(ppow->ppchBuyRequires,ppow->pchSourceFile);
	}
	if(ppow->ppchActivateRequires)
	{
		chareval_Validate(ppow->ppchActivateRequires,ppow->pchSourceFile);
	}
	if(ppow->ppchConfirmRequires)
	{
		chareval_Validate(ppow->ppchConfirmRequires,ppow->pchSourceFile);
	}
	if(ppow->ppchSlotRequires)
	{
		// not a character evaluation statement, can't run this anymore.
		// TODO:  Write a validator for this type of evaluation statement.
		//chareval_Validate(ppow->ppchSlotRequires,ppow->pchSourceFile);
	}
	if(ppow->ppchRewardRequires)
	{
		chareval_Validate(ppow->ppchRewardRequires,ppow->pchSourceFile);
	}
	if(ppow->ppchHighlightEval)
	{
		chareval_Validate(ppow->ppchHighlightEval,ppow->pchSourceFile);
	}
	if(ppow->ppchAuctionRequires)
	{
		chareval_Validate(ppow->ppchAuctionRequires,ppow->pchSourceFile);
	}

#if SERVER
	if(ppow->ppchTargetRequires)
	{
		combateval_Validate(dbg_BasePowerStr(ppow),ppow->ppchTargetRequires,ppow->pchSourceFile);
	}
#endif

	if (ppow->iNumAllowed > 1)
	{
		if (ppow->eType == kPowerType_GlobalBoost)
		{
			ErrorFilenamef(ppow->pchSourceFile, "GlobalBoost (%s) has NumAllowed %d which should be 1\nYou cannot have two copies of the same GlobalBoost\n",ppow->pchFullName, ppow->iNumAllowed);
			ppow->iNumAllowed = 1;
		}
		else if (strstriConst(ppow->pchFullName, "Temporary_Powers") == NULL
					&& ppow->bDoNotSave == 0)
		{
			ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has NumAllowed %d, but isn't allowed to have more than one.  NumAllowed only works for powers in the Temporary Powers category or DoNotSave kTrue.\n", ppow->pchFullName, ppow->iNumAllowed);
			ppow->iNumAllowed = 1;
		}
	}

	if(ppow->bIsEnvironmentHit && (ppow->eAIReport != kAIReport_Never))
	{
		ErrorFilenamef(ppow->pchSourceFile, "Power (%s) IsEnvironmentHit but not set to AIReport Never.\nThis will cause unexpected AI results because hits will be counted as attacks.\nPower is being automatically changed to AIReport Never, please adjust data so this change is explict.",ppow->pchFullName);
		ppow->eAIReport = kAIReport_Never;
	}

	if (!ppow->ppEffects)
		eaCreate((EffectGroup***)&ppow->ppEffects);

	if (bLegacyPower) {
		// A few fixups that we only want to apply to legacy powers. We expect better of
		// designers now, and will make them fix the defs if they're lazy!

		// Because Design is incredibly lazy, I'm doing a fixup on
		//   bShowBuffIcon if it's not specifically set.
		// If this is a pet getting stuck to the caster, then we want
		//   to see the icon (e.g. Fire Imps)
		// (Search for "ShowBuffIcon" for more fixups below)
		if(ppow->bShowBuffIcon == -1)
		{
			if(eaiSize(&(int*)ppow->pAffected) == 1
				&& ppow->pAffected[0] == kTargetType_Caster)
			{
				ppow->bShowBuffIcon = true;
			}
		}
	}

	// Migrate legacy AttribMod templates into effect groups
	iEffSize = ppowlo ? eaSize(&ppowlo->ppLegacyTemplates) : 0;
	for (ieff = 0; ieff < iEffSize; ieff++) {
		LegacyAttribModTemplate *pLegacy = (LegacyAttribModTemplate*)ppowlo->ppLegacyTemplates[ieff];
		EffectGroup *pnewEffect;
		AttribModTemplate *pnewTemplate;

		if (pLegacy->offAttrib == kSpecialAttrib_PowerRedirect &&
			pLegacy->ppchPrimaryStringList)
		{
			// Migrate power redirects to new system
			PowerRedirect *pRedirect = ParserAllocStruct(sizeof(PowerRedirect));
			pRedirect->pchName = pLegacy->ppchPrimaryStringList[0];

			// Some REALLY stupid legacy powers put the redirect mods in the middle
			// of the list...
			if (ppow->ppEffects) {
				int i;
				for (i = eaSize(&ppow->ppEffects) - 1; i >= 0; i--) {
					ParserDestroyStruct(ParseEffectGroup, (void*)ppow->ppEffects[i]);
				}
				eaDestroyConst(&ppow->ppEffects);
				ppow->ppEffects = NULL;
			}

			if (eaSize(&pLegacy->eg.ppchRequires) == 1 &&
			  !(strcmp(pLegacy->eg.ppchRequires[0], "1")))
			{
				// Legacy "default" redirection, simply omit the requires, but set
				// it as the default for power info.
				pRedirect->bShowInInfo = true;
			} else {
				pRedirect->ppchRequires = pLegacy->eg.ppchRequires;
				pLegacy->eg.ppchRequires = NULL;
			}

			// Source file is the legacy mod
			ParserCopyMeta(pLegacy, pRedirect);

			eaPushConst(&ppow->ppRedirect, pRedirect);

			eaDestroyConst(&pLegacy->ppchPrimaryStringList);
			pLegacy->ppchPrimaryStringList = NULL;
			continue;
		} else if (ppow->ppRedirect) {
			// Legacy power redirectors tend to have extra random attribmods that
			// need to be omitted.
			continue;
		}

		// More ShowBuffIcon fixups for lazy Designers.
		// The logic in this is probably broken, but it has been for a while, so apply it to
		// legacy powers only.
		if(ppow->bShowBuffIcon == -1)
		{
			// If this attribmod doesn't target the caster
			//   and its an EntCreate, shut off the buff icon.
			// If the pet does anything to the target, it will use
			//   its own power which will have its own icon.
			if(pLegacy->am.eTarget != kModTarget_Caster
				&& pLegacy->offAttrib == kSpecialAttrib_EntCreate)
			{
				ppow->bShowBuffIcon = false;
			}
			else
			{
				// If there are any AttribMods on this power
				//   which aren't EntCreates, then we need to see
				//   the icon no matter what.
				ppow->bShowBuffIcon = true;
			}
		}

		pnewEffect = ParserAllocStruct(sizeof(EffectGroup));
		pnewTemplate = ParserAllocStruct(sizeof(AttribModTemplate));
		ParserCopyStruct(ParseEffectGroup, &pLegacy->eg, sizeof(EffectGroup), pnewEffect);
		ParserCopyStruct(ParseAttribModTemplate, &pLegacy->am, sizeof(AttribModTemplate), pnewTemplate);

		// Copy development metadata from the file it actually came from
		ParserCopyMeta(pLegacy, pnewEffect);
		ParserCopyMeta(pLegacy, pnewTemplate);

		// Default these since they're not loaded directly into the legacy template
		pnewEffect->fChance = pnewTemplate->fTickChance = 1.0;

		eaiCreate(&pnewTemplate->piAttrib);
		eaiPush(&pnewTemplate->piAttrib, pLegacy->offAttrib);

		// Since Chance has been split, need to try to divine which kind the power
		// designer intended.
		if (pnewEffect->fProcsPerMinute > 0.0f) {
			// PPM-based procs must apply up front
			pnewEffect->fChance = pLegacy->fChance;
		} else if (pnewTemplate->fPeriod != 0.0f) {
			// A specified apply period always checks per tick
			pnewTemplate->fTickChance = pLegacy->fChance;
		} else if (pnewTemplate->eType == kModType_Duration ||
			(pnewTemplate->eType == kModType_Expression && pnewTemplate->ppchDuration)) {
			// Continuous kDuration mods always check up front
			pnewEffect->fChance = pLegacy->fChance;
		} else if ((pnewTemplate->eType == kModType_Magnitude ||
			pnewTemplate->eType == kModType_Expression) && // Magnitude mods ...
			pnewTemplate->fDuration > 0 &&				   // if they have a duration
			!pLegacy->bCancelOnMiss)					   // and are not CancelOnMiss
		{
			pnewEffect->fChance = pLegacy->fChance;        // check up front as well
		} else if (pnewTemplate->fDuration == 0 && !pLegacy->bDelayEval) {
			// One tick only effects
			pnewEffect->fChance = pLegacy->fChance;
		} else {
			// Everything else is per tick
			pnewTemplate->fTickChance = pLegacy->fChance;
		}

		// Migrate messages
		if (pLegacy->msgs.pchDisplayAttackerHit ||
			pLegacy->msgs.pchDisplayDefenseFloat ||
			pLegacy->msgs.pchDisplayFloat ||
			pLegacy->msgs.pchDisplayVictimHit)
		{
			// Just move them, pLegacy is going to be destroyed anyway
			AttribModMessages *pnewMessages = ParserAllocStruct(sizeof(AttribModMessages));
			MOVEPTR(pLegacy->msgs.pchDisplayAttackerHit, pnewMessages->pchDisplayAttackerHit);
			MOVEPTR(pLegacy->msgs.pchDisplayDefenseFloat, pnewMessages->pchDisplayDefenseFloat);
			MOVEPTR(pLegacy->msgs.pchDisplayFloat, pnewMessages->pchDisplayFloat);
			MOVEPTR(pLegacy->msgs.pchDisplayVictimHit, pnewMessages->pchDisplayVictimHit);

			ParserCopyMeta(pLegacy, pnewMessages);

			pnewTemplate->pMessages = pnewMessages;
		}

		// Migrate FX
		if (pLegacy->fx.pchConditionalFX ||
			pLegacy->fx.pchContinuingFX ||
			pLegacy->fx.piConditionalBits ||
			pLegacy->fx.piContinuingBits)
		{
			AttribModFX *pnewFX = ParserAllocStruct(sizeof(AttribModFX));
			MOVEPTR(pLegacy->fx.pchConditionalFX, pnewFX->pchConditionalFX);
			MOVEPTR(pLegacy->fx.pchContinuingFX, pnewFX->pchContinuingFX);
			MOVEPTR(pLegacy->fx.piConditionalBits, pnewFX->piConditionalBits);
			MOVEPTR(pLegacy->fx.piContinuingBits, pnewFX->piContinuingBits);

			ParserCopyMeta(pLegacy, pnewFX);

			pnewTemplate->pFX = pnewFX;
		}

		// Migrate all the various flags
		if (!pLegacy->bShowFloaters)
			TemplateSetFlag(pnewTemplate, NoFloaters);
		if (pLegacy->bIgnoresBoostDiminishing)
			TemplateSetFlag(pnewTemplate, BoostIgnoreDiminishing);
		if (pLegacy->bCancelOnMiss)
			TemplateSetFlag(pnewTemplate, CancelOnMiss);
		if (pLegacy->bNearGround)
			TemplateSetFlag(pnewTemplate, NearGround);
		if (!pLegacy->bAllowStrength)
			TemplateSetFlag(pnewTemplate, IgnoreStrength);
		if (!pLegacy->bAllowResistance)
			TemplateSetFlag(pnewTemplate, IgnoreResistance);
		if (!pLegacy->bAllowCombatMods)
			TemplateSetFlag(pnewTemplate, IgnoreCombatMods);
		if (pLegacy->bBoostTemplate)
			TemplateSetFlag(pnewTemplate, Boost);
		if (pLegacy->bMatchExactPower)
			TemplateSetFlag(pnewTemplate, StackExactPower);
		if (pLegacy->bDisplayTextOnlyIfNotZero)
			TemplateSetFlag(pnewTemplate, HideZero);
		if (pLegacy->bVanishEntOnTimeout)
			TemplateSetFlag(pnewTemplate, VanishEntOnTimeout);
		if (pLegacy->bDoNotTintCostume)
			TemplateSetFlag(pnewTemplate, DoNotTintCostume);
		if (pLegacy->bKeepThroughDeath)
			TemplateSetFlag(pnewTemplate, KeepThroughDeath);
		if (pLegacy->bDelayEval)
			TemplateSetFlag(pnewTemplate, DelayEval);
		if (pLegacy->bStackByAttrib)
			TemplateSetFlag(pnewTemplate, StackByAttribAndKey);
		if (pLegacy->bLinkedChance) {
			pnewEffect->iFlags |= EFFECTGROUP_FLAG_VALUE(LinkedChance);
//			ErrorFilenamef(ppow->pchSourceFile, "WARNING: Power (%s) has an AttribMod that uses LinkedChance. This is deprecated and should be migrated to effect groups.", ppow->pchFullName);
		}

		if (pLegacy->bUseDurationResistance > 0)
			TemplateSetFlag(pnewTemplate, ResistDuration);
		if (pLegacy->bUseMagnitudeResistance > 0)
			TemplateSetFlag(pnewTemplate, ResistMagnitude);
		if (pLegacy->bUseDurationCombatMods > 0)
			TemplateSetFlag(pnewTemplate, CombatModDuration);
		if (pLegacy->bUseMagnitudeCombatMods > 0)
			TemplateSetFlag(pnewTemplate, CombatModMagnitude);

		if (pLegacy->pchIgnoreSuppressErrors)
			TemplateSetFlag(pnewTemplate, IgnoreSuppressErrors);

		if (pLegacy->bDelayEval && pLegacy->eg.ppchRequires) {
			// DelayEval Requires are equivalent to DelayedRequires, move them
			// out of the effect group
			pnewTemplate->ppchDelayedRequires = pnewEffect->ppchRequires;
			pnewEffect->ppchRequires = NULL;
		}

		// Effect-specific crap
		if (pLegacy->offAttrib == kSpecialAttrib_CombatModShift &&
			pLegacy->pchParams && !stricmp(pLegacy->pchParams, "DoNotDisplayShift"))
			TemplateSetFlag(pnewTemplate, DoNotDisplayShift);

		if ((pLegacy->offAttrib == kSpecialAttrib_TokenAdd ||
			pLegacy->offAttrib == kSpecialAttrib_TokenSet) &&
			pLegacy->pchParams && !stricmp(pLegacy->pchParams, "NoTime"))
			TemplateSetFlag(pnewTemplate, NoTokenTime);

		// If this is a marker-targeted power, move the parameters into the
		// extended targeting info.
		if (pLegacy->am.eTarget == kModTarget_Marker &&
			pLegacy->pchParams) {
				AttribModTargetInfo *pnewTarget = ParserAllocStruct(sizeof(AttribModTargetInfo));
				char *pch;
				pch = strtok((char*)pLegacy->pchParams, ":");
				PUSHPTRC(pLegacy->pchParams, pnewTarget->ppchMarkerNames);
				pch = strtok((char*)pLegacy->pchParams, ":");
				if (pch) {
					eaiPush(&pnewTarget->piMarkerCount, atoi(pch));
				} else {
					eaiPush(&pnewTarget->piMarkerCount, -1);
				}

				ParserCopyMeta(pLegacy, pnewTarget);
				pnewTemplate->pTargetInfo = pnewTarget;
		}

		legacyattribmod_MigrateParams(ppow, pLegacy, pnewEffect, pnewTemplate);

		legacyattribmod_Add(ppow, pnewEffect, pnewTemplate);
		ParserDestroyStruct(ParseLegacyAttribModTemplate, (LegacyAttribModTemplate*)pLegacy);
	}

	//
	// Sanitize effects
	//
	bAllNearGround = true;

	iEffSize = eaSize(&ppow->ppEffects);
	for (ieff=0; ieff<iEffSize; ieff++)
	{
		effectgroup_FixupForBin(ppow, (EffectGroup*)ppow->ppEffects[ieff],
			&bAllNearGround, &hasUntilShutOffMods, &hasOnlyUntilShutOffMods);
	}

	// Check that at least one redirect is set as default
	if (eaSize(&ppow->ppRedirect) > 0) {
		bool bFound = false;

		// Redirected powers don't have any effects to be checked for near ground flags,
		// so don't default this to true.
		bAllNearGround = false;

		if (eaSize(&ppow->ppEffects) > 0) {
			ErrorFilenamef(ppow->pchSourceFile, "Power (%s) uses power redirection, but has effects that will never be used.\n", ppow->pchFullName);
		}

		for (ieff = 0; ieff < eaSize(&ppow->ppRedirect); ieff++) {
			if (ppow->ppRedirect[ieff]->bShowInInfo)
				bFound = true;
		}
		if (!bFound)
			((PowerRedirect**)ppow->ppRedirect)[ieff-1]->bShowInInfo = true;
	}
	else if(hasOnlyUntilShutOffMods && ppow->fActivatePeriod < ACTIVATE_PERIOD_FOREVER)
	{
		ErrorFilenamef(ppow->pchSourceFile, "ActivatePeriod too short for power (%s) which only has kUntilShutOff AttribMods.\n",ppow->pchFullName);
		ppow->fActivatePeriod = ACTIVATE_PERIOD_FOREVER;
	}
	else if(!hasOnlyUntilShutOffMods && ppow->fActivatePeriod >= ACTIVATE_PERIOD_FOREVER)
		ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has non kUntilShutOff AttribMods and a very long ActivatePeriod.\n",ppow->pchFullName);

	if(hasUntilShutOffMods && !((ppow->eType == kPowerType_Auto) || (ppow->eType == kPowerType_Toggle)))
	{
		ErrorFilenamef(ppow->pchSourceFile, "kUntilShutOff AttribMods in non-Auto or Toggle power (%s).\n",ppow->pchFullName);
	}

	if(ppow->bTargetNearGround<0)
	{
		ppow->bTargetNearGround = bAllNearGround;
	}

#ifndef TEST_CLIENT
	// Doing this in a separate loop to make the errors easier to read.
	if (!msGetHideTranslationErrors())
	{
		CHECK_TRANSLATION_POW(ppow->pchDisplayAttackerAttackFloater);
		CHECK_TRANSLATION_POW(ppow->pchDisplayAttackerAttack);
		CHECK_TRANSLATION_POW(ppow->pchDisplayAttackerHit);
		CHECK_TRANSLATION_POW(ppow->pchDisplayHelp);
		CHECK_TRANSLATION_POW(ppow->pchDisplayName);
		CHECK_TRANSLATION_POW(ppow->pchDisplayShortHelp);
		CHECK_TRANSLATION_POW(ppow->pchDisplayVictimHit);
	}
#endif

}

static void params_SetForwardPointers(AttribModTemplate *ptemplate)
{
	switch (*(int*)ptemplate->pParams)
	{
		PARAMCASE(Power) {
			powerspec_Resolve(&param->ps);
		} PARAMCASEEND;
		PARAMCASE(EntCreate) {
			powerspec_Resolve(&param->ps);
		} PARAMCASEEND;
	}
}

static void effectgroup_SetForwardPointers(BasePower *ppow, EffectGroup *pEffect)
{
	int i, iSize;

	iSize = eaSize(&pEffect->ppEffects);
	for(i=0; i<iSize; i++)
	{
		effectgroup_SetForwardPointers(ppow, (EffectGroup*)pEffect->ppEffects[i]);
	}

	iSize = eaSize(&pEffect->ppTemplates);
	for(i=0; i<iSize; i++)
	{
		AttribModTemplate *mod = (AttribModTemplate*)pEffect->ppTemplates[i];
		mod->ppowBaseIndex = eaSize(&ppow->ppTemplateIdx);
		eaPushConst(&ppow->ppTemplateIdx, mod);
		if (mod->pParams)
			params_SetForwardPointers(mod);
	}
}

/**********************************************************************func*
* powerdict_SetForwardPointers
*
* Constructs the pcat->pset->ppow pointers
*
*/
void powerdict_SetForwardPointers(PowerDictionary *pdict)
{
	int icat;
	int iSizeCat;

	// First pass is just to build up the internal arrays
	iSizeCat = eaSize(&pdict->ppPowerCategories);
	for(icat=0; icat<iSizeCat; icat++)
	{
		int iset;
		PowerCategory *pcat = (PowerCategory*)pdict->ppPowerCategories[icat];
		int iSizeSets = eaSize(&pcat->ppPowerSetNames);

		eaSetSizeConst(&cpp_const_cast(BasePowerSet**)(pcat->ppPowerSets),0);

		for(iset=0; iset<iSizeSets; iset++)
		{
			int ipow;
			int iSizePows;
			BasePowerSet *pset = (BasePowerSet*)powerdict_GetBasePowerSetByFullName(pdict,pcat->ppPowerSetNames[iset]);

			if (!pset)
			{
				ErrorFilenamef(pcat->pchSourceFile,
					"Power Category %s contains reference to nonexistent PowerSet %s!",
					pcat->pchName,pcat->ppPowerSetNames[iset]);
				continue;
			}

			iSizePows = eaSize(&pset->ppPowerNames);

			eaSetSizeConst(&pset->ppPowers,0);

			for(ipow=0; ipow<iSizePows; ipow++)
			{
				char powername[512];
				BasePower *ppow = (BasePower*)powerdict_GetBasePowerByFullName(pdict,pset->ppPowerNames[ipow]);
				
				if (!ppow)
				{
					ErrorFilenamef(pset->pchSourceFile,
						"Powerset %s contains reference to nonexistent power %s! This breaks the available levels of powers!",
						pset->pchFullName,pset->ppPowerNames[ipow]);
					continue;
				}

				sprintf(powername,"%s.%s",pset->pchFullName,ppow->pchName);
				
				if (strcmp(powername,ppow->pchFullName) != 0)
				{
					ppow = (BasePower*)powerdict_GetBasePowerByFullName(pdict,powername);
				}

				if (!ppow)
				{
					ErrorFilenamef(pset->pchSourceFile,
						"Powerset %s is missing power %s (duplicate of %s). Rebuild your bins, and get a programmer if that fails",
						pset->pchFullName,powername,pset->ppPowerNames[ipow]);
					continue;
				}

				eaPushConst(&pset->ppPowers,ppow);
			}
			eaPushConst(&pcat->ppPowerSets,pset);
		}
	}

	// Do a second pass for things that need the powerset / power arrays to be complete
	for(icat=0; icat<iSizeCat; icat++)
	{
		int iset;
		PowerCategory *pcat = (PowerCategory*)pdict->ppPowerCategories[icat];
		int iSizeSets = eaSize(&pcat->ppPowerSetNames);

		for(iset=0; iset<iSizeSets; iset++)
		{
			int ipow;
			BasePowerSet *pset = (BasePowerSet*)pcat->ppPowerSets[iset];
			int iSizePows = eaSize(&pset->ppPowerNames);

			for(ipow=0; ipow<iSizePows; ipow++)
			{
				int ieff;
				BasePower *ppow = (BasePower*)pset->ppPowers[ipow];
				int iSizeEff = eaSize(&ppow->ppEffects);

				// Rebuild template index
				eaSetSizeConst(&ppow->ppTemplateIdx,0);

				for(ieff=0; ieff<iSizeEff; ieff++)
				{
					effectgroup_SetForwardPointers(ppow, (EffectGroup*)ppow->ppEffects[ieff]);
				}

				// Update power redirection pointers
				for (ieff = eaSize(&ppow->ppRedirect) - 1; ieff >= 0; ieff--)
				{
					PowerRedirect *pRedir = (PowerRedirect*)ppow->ppRedirect[ieff];
					const BasePower *temp = powerdict_GetBasePowerByFullName(pdict, pRedir->pchName);

					if(temp)
						pRedir->ppowBase = temp;
				}
			}
		}
	}
}

/**********************************************************************func*
* powerdict_DuplicateMissingPowers
*
* This function sets up the power dictionary loading process for the rest of the
* loading process, and duplicates any powers needed to fulfill inter-set references
*
*/
void powerdict_DuplicateMissingPowers(PowerDictionary *pdict)
{
	int icat;
	int iSizeCat;

	iSizeCat = eaSize(&pdict->ppPowerCategories);
	for(icat=0; icat<iSizeCat; icat++)
	{
		int iset;
		PowerCategory *pcat = (PowerCategory*)pdict->ppPowerCategories[icat];
		int iSizeSets = eaSize(&pcat->ppPowerSetNames);

		eaSetSizeConst(&pcat->ppPowerSets,0);

		for(iset=0; iset<iSizeSets; iset++)
		{
			int ipow;
			int iSizePows;
			BasePowerSet *pset = (BasePowerSet*)powerdict_GetBasePowerSetByFullName(pdict,pcat->ppPowerSetNames[iset]);

			if (!pset)
			{
				Errorf("Power Category %s contains reference to nonexistent PowerSet %s!",
					pcat->pchName,pcat->ppPowerSetNames[iset]);
				continue;
			}

			iSizePows = eaSize(&pset->ppPowerNames);

			eaSetSizeConst(&pset->ppPowers,0);

			for(ipow=0; ipow<iSizePows; ipow++)
			{
				char powername[512];
				BasePower *ppow = (BasePower*)powerdict_GetBasePowerByFullName(pdict,pset->ppPowerNames[ipow]);

				if (!ppow)
				{
					ErrorFilenamef(pset->pchSourceFile,
						"Powerset %s contains reference to nonexistent power %s! This breaks the available levels of powers!",
						pset->pchFullName,pset->ppPowerNames[ipow]);
					continue;
				}
					

				sprintf(powername,"%s.%s",pset->pchFullName,ppow->pchName);

				if (strcmp(powername,ppow->pchFullName) != 0)
				{
					BasePower *newpow = (BasePower*)powerdict_GetBasePowerByFullName(pdict,powername);

					if (!newpow)
					{					
						// If the power name doesn't match expected name, this refers to a power
						// in another set, and needs to be duplicated for this set
						BasePower *newpow = ParserAllocStruct(sizeof(BasePower));
						ParserCopyStruct(ParseBasePower,ppow,sizeof(BasePower),newpow);
						newpow->pchSourceName = ParserAllocString(newpow->pchFullName);
						StructFreeStringConst(newpow->pchFullName);
						newpow->pchFullName = ParserAllocString(powername);

						eaPushConst(&pdict->ppPowers,newpow);
					}
					ppow = newpow;
				}
				else if (!ppow->pchSourceName)
				{
					ppow->pchSourceName = ParserAllocString(ppow->pchFullName);
				}
				
				eaPushConst(&pset->ppPowers,ppow);

			}
			eaPushConst(&pcat->ppPowerSets,pset);
		}		
	}
}

/**********************************************************************func*
 * powerdict_FixPowerSetErrors
 *
 * This function is called after the power dict hierarchy is set up, but BEFORE
 * any bin files are written. It will never be called in production mode
 *
 */
void powerdict_FixPowerSetErrors(PowerDictionary *pdict)
{
	int icat;
	int iSizeCat;

	assert(pdict!=NULL);

	iSizeCat = eaSize(&pdict->ppPowerCategories);
	for(icat=0; icat<iSizeCat; icat++)
	{
		int iset;
		PowerCategory *pcat = (PowerCategory*)pdict->ppPowerCategories[icat];
		int iSizeSets = eaSize(&pcat->ppPowerSets);

		for(iset=0; iset<iSizeSets; iset++)
		{
			int ipow;
			BasePowerSet *pset = (BasePowerSet*)pcat->ppPowerSets[iset];
			int iSizePows = eaSize(&pset->ppPowers);

			for(ipow=0; ipow<iSizePows; ipow++)
			{
				BasePower *ppow = (BasePower*)pset->ppPowers[ipow];

				if(ppow)
				{
					// All the powers in a power set have to be in the same
					//   power system.
					ppow->eSystem = pset->eSystem;

					if(stricmp(pcat->pchName, "Prestige")==0)
					{
						ppow->bFree = true;
						ppow->iForceLevelBought = 0;
					}

					// Default Free-ness of the power for inherent powers
					if(stricmp(pcat->pchName, "Inherent")==0)
					{
						ppow->bFree = true;
						ppow->bAutoIssue = true;
					}

					if(stricmp(ppow->pchName, "Inherent")==0)
					{
						ppow->bFree = true;
						ppow->bAutoIssue = true;
					}

					if(stricmp(pcat->pchName, "Incarnate")==0)
					{
						ppow->bFree = true;
					}

					// Set max boosts for temporary powers to zero since you
					//   can't slot them. Disallow all kinds of boosts in them.
					// Temporary powers are also free.
					if(stricmp(pcat->pchName, "Temporary_Powers")==0)
					{
						ppow->iMaxBoosts = 0;
						if(ppow->eType != kPowerType_Boost && ppow->eType != kPowerType_GlobalBoost)
							eaiDestroy(&ppow->pBoostsAllowed);

						ppow->bFree = true;
					}
#if CLIENT
#ifndef TEST_CLIENT

					if (!isProductionMode())
					{
						if (ppow->eType == kPowerType_Boost)
						{
							uiEnhancementAddHelpText(NULL, ppow, 1, NULL);
						}
					}
#endif
#endif
					if(ppow->bAutoIssue && !ppow->bFree)
					{
						ErrorFilenamef(ppow->pchSourceFile, "AutoIssue power (%s.%s.%s) not marked as free. It should be marked with \"Free  kTrue\".", pcat->pchName, pset->pchName, ppow->pchName);
						ppow->bFree = true;
					}
				}
				else
				{
					ErrorFilenamef(pset->pchSourceFile, "Set %s.%s has empty power. Check set itself as well.", pcat->pchName, pset->pchName);
				} // power valid check
			} // for each power
		} // for each set
	} // for each category
}

static const DimReturnSet *pDimSetDefault = NULL;

/***************************************************************************
* Lots of parameter-related validation to make sure designers aren't stupid.
* This is kind of ugly due to all the macros, but beats typing everything
* a hundred times.
*/

#define REQUIREPARAM(t, ...) \
	if (TemplateHasAttribFromList(ptemplate, __VA_ARGS__)) { \
		if (!TemplateGetParams(ptemplate, t)) { \
		ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has an AttribMod that needs " #t " params, but is missing them.", ppow->pchFullName); \
			return false; \
		} \
	}

static bool template_Validate(const BasePower *ppow, const AttribModTemplate *ptemplate)
{
	REQUIREPARAM(Costume, kSpecialAttrib_SetCostume);
	REQUIREPARAM(EntCreate, kSpecialAttrib_EntCreate);
	REQUIREPARAM(Power, kSpecialAttrib_GrantPower, kSpecialAttrib_GrantBoostedPower, kSpecialAttrib_RevokePower, kSpecialAttrib_RechargePower, kSpecialAttrib_ExecutePower);
	REQUIREPARAM(Reward, kSpecialAttrib_Reward, kSpecialAttrib_RewardSource, kSpecialAttrib_RewardSourceTeam);
	REQUIREPARAM(Phase, kSpecialAttrib_CombatPhase, kSpecialAttrib_VisionPhase, kSpecialAttrib_ExclusiveVisionPhase);
	REQUIREPARAM(Behavior, kSpecialAttrib_AddBehavior);
	REQUIREPARAM(SZEValue, kSpecialAttrib_SetSZEValue);
	REQUIREPARAM(Token, kSpecialAttrib_TokenAdd, kSpecialAttrib_TokenSet, kSpecialAttrib_TokenClear);
	REQUIREPARAM(EffectFilter, kSpecialAttrib_GlobalChanceMod, kSpecialAttrib_CancelMods);

	if (ptemplate->eType == kModTarget_Marker &&
		!(ptemplate->pTargetInfo &&
		eaSize(&ptemplate->pTargetInfo->ppchMarkerNames) > 0)) {
			ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has an AttribMod that targets a Marker, but is missing extended targeting info.", ppow->pchFullName);
			return false;
	}

	return true;
}

#define PARAMATTRIBS(t, ...) \
	if (!TemplateHasAttribFromList(ptemplate, __VA_ARGS__)) \
		ErrorFilenamef(ppow->pchSourceFile, "WARNING: Power (%s) has a " #t " param on an AttribMod that doesn't use it.", ppow->pchFullName)

static bool powerspec_Validate(const BasePower *ppow, const PowerSpec *pSpec)
{
	int i;

	for (i = eaSize(&pSpec->ppchCategoryNames) - 1; i >= 0; i--)
	{
		if (!powerdict_GetCategoryByName(&g_PowerDictionary, pSpec->ppchCategoryNames[i])) {
			ErrorFilenamef(ppow->pchSourceFile, "Power (%s) references invalid power category %s.", ppow->pchFullName, pSpec->ppchCategoryNames[i]);
			return false;
		}
	}

	for (i = eaSize(&pSpec->ppchPowersetNames) - 1; i >= 0; i--)
	{
		if (!powerdict_GetBasePowerSetByFullName(&g_PowerDictionary, pSpec->ppchPowersetNames[i])) {
			ErrorFilenamef(ppow->pchSourceFile, "Power (%s) references invalid powerset %s.", ppow->pchFullName, pSpec->ppchPowersetNames[i]);
			return false;
		}
	}

	for (i = eaSize(&pSpec->ppchPowerNames) - 1; i >= 0; i--)
	{
		if (!powerdict_GetBasePowerByFullName(&g_PowerDictionary, pSpec->ppchPowerNames[i])) {
			ErrorFilenamef(ppow->pchSourceFile, "Power (%s) references invalid power %s.", ppow->pchFullName, pSpec->ppchPowerNames[i]);
			return false;
		}
	}

	return true;
}

#define REQUIREPARAMFIELD(t, n, f) \
	if (!TemplateGetParam(ptemplate, t, f)) { \
		ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has an AttribMod that is missing the required field " n " from param " #t ".", ppow->pchFullName); \
		return false; \
	}
#define REQUIREPARAMFIELDLO(t, n, f) \
	{ \
		AttribModParam_##t##_loadonly *paramlo = ParserGetLoadOnly(param); \
		if (!paramlo || !paramlo->f) { \
			ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has an AttribMod that is missing the required field " n " from param " #t ".", ppow->pchFullName); \
			return false; \
		} \
	}

static bool params_Validate(const BasePower *ppow, const AttribModTemplate *ptemplate)
{
	switch (*(int*)ptemplate->pParams) {
		PARAMCASE(Costume) {
			PARAMATTRIBS(Costume, kSpecialAttrib_SetCostume);
			REQUIREPARAMFIELDLO(Costume, "Costume", pchCostumeName);

			// Can't validate yet because npc defs aren't loaded. Do it in FinalizeShared.
		} PARAMCASEEND;

		PARAMCASE(Reward) {
			REQUIREPARAMFIELD(Reward, "Reward", ppchRewards);
		} PARAMCASEEND;

		PARAMCASE(EntCreate) {
			AttribModParam_EntCreate_loadonly *paramlo = ParserGetLoadOnly(param);
			PARAMATTRIBS(EntCreate, kSpecialAttrib_EntCreate);

			if (TemplateHasFlag(ptemplate, PseudoPet)) {
				REQUIREPARAMFIELD(EntCreate, "DisplayName", pchDisplayName);
			} else {
				if (!paramlo ||
					(!paramlo->pchEntityDef && !paramlo->pchClass)) {
						ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has an AttribMod that is missing either EntityDef or Class from param EntCreate.", ppow->pchFullName);
						return false;
				}
				if (!paramlo->pchEntityDef) {
					REQUIREPARAMFIELD(EntCreate, "DisplayName", pchDisplayName);
				}
			}

			if (!powerspec_Validate(ppow, &param->ps))
				return false;

			// Can't validate yet because villain defs aren't loaded. Do it in FinalizeShared.
		} PARAMCASEEND;

		PARAMCASE(Power) {
			PARAMATTRIBS(Power, kSpecialAttrib_GrantPower, kSpecialAttrib_RevokePower,
				kSpecialAttrib_RechargePower, kSpecialAttrib_ExecutePower);
			if (!powerspec_Validate(ppow, &param->ps))
				return false;
		} PARAMCASEEND;

		PARAMCASE(Phase) {
			PARAMATTRIBS(Phase, kSpecialAttrib_CombatPhase, kSpecialAttrib_VisionPhase,
				kSpecialAttrib_ExclusiveVisionPhase);

			if (TemplateHasAttrib(ptemplate, kSpecialAttrib_CombatPhase)) {
				REQUIREPARAMFIELD(Phase, "CombatPhase", piCombatPhases);
			}
			if (TemplateHasAttrib(ptemplate, kSpecialAttrib_VisionPhase)) {
				REQUIREPARAMFIELD(Phase, "VisionPhase", piVisionPhases);
			}
			if (TemplateHasAttrib(ptemplate, kSpecialAttrib_ExclusiveVisionPhase)) {
				if (param->iExclusiveVisionPhase == -1) {
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has an AttribMod that is missing the required field ExclusiveVisionPhase from param Phase.", ppow->pchFullName);
					return false;
				}
			}
		} PARAMCASEEND;

		PARAMCASE(Teleport) {
			PARAMATTRIBS(Teleport, offsetof(CharacterAttributes, fTeleport));
		} PARAMCASEEND;

		PARAMCASE(Behavior) {
			PARAMATTRIBS(Behavior, kSpecialAttrib_AddBehavior);
			REQUIREPARAMFIELD(Behavior, "Behavior", ppchBehaviors);
		} PARAMCASEEND;

		PARAMCASE(SZEValue) {
			PARAMATTRIBS(SZEValue, kSpecialAttrib_SetSZEValue);
			REQUIREPARAMFIELD(SZEValue, "ScriptID", ppchScriptID);
			REQUIREPARAMFIELD(SZEValue, "ScriptValue", ppchScriptValue);
			chareval_Validate(param->ppchScriptID, ppow->pchSourceFile);
		} PARAMCASEEND;

		PARAMCASE(Token) {
			PARAMATTRIBS(Token, kSpecialAttrib_TokenAdd, kSpecialAttrib_TokenSet, kSpecialAttrib_TokenClear);
			REQUIREPARAMFIELD(Token, "Token", ppchTokens);
		} PARAMCASEEND;

		PARAMCASE(EffectFilter) {
			PARAMATTRIBS(EffectFilter, kSpecialAttrib_GlobalChanceMod, kSpecialAttrib_PowerChanceMod, kSpecialAttrib_CancelMods);
			if (!param->ppchTags && !param->ps.ppchCategoryNames && !param->ps.ppchPowersetNames && !param->ps.ppchPowerNames) {
				ErrorFilenamef(ppow->pchSourceFile, "Power (%s) must have either a tag or a power in an effect filter.", ppow->pchFullName);
				return false;
			}
			if (!powerspec_Validate(ppow, &param->ps))
				return false;
		} PARAMCASEEND;
	}

	return true;
}

extern StaticDefineInt ModApplicationEnum[];
static void effectgroup_Validate(BasePower *ppow, const EffectGroup **ppEffects)
{
	int ei, i, j;
	for (ei = eaSize(&ppEffects) - 1; ei >= 0; ei--)
	{
		EffectGroup *pEffect = (EffectGroup*)ppEffects[ei];

		for (i = eaSize(&pEffect->ppTemplates) - 1; i >= 0; i--)
		{
			bool broken = false;
			AttribModTemplate *ptemplate = (AttribModTemplate*)pEffect->ppTemplates[i];
			ModApplicationType ppowApplicationType = ptemplate->eApplicationType;
			const char *ppowApplicationTypeString = StaticDefineIntRevLookup(ModApplicationEnum, ppowApplicationType);
			int *tmpAttrib = 0;

			for (j = eaiSize(&ptemplate->piAttrib) - 1; j >= 0; j--) {
				if (eaiSortedFind(&tmpAttrib, ptemplate->piAttrib[j]) != -1) {
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has duplicate attribs in a single attribmod! They will not stack correctly.", ppow->pchFullName);
				}
				eaiSortedPushUnique(&tmpAttrib, ptemplate->piAttrib[j]);
			}
			eaiDestroy(&tmpAttrib);

			if (ppowApplicationType == kModApplicationType_OnActivate
				|| ppowApplicationType == kModApplicationType_OnDeactivate
				|| ppowApplicationType == kModApplicationType_OnExpire
				|| ppowApplicationType == kModApplicationType_OnEnable
				|| ppowApplicationType == kModApplicationType_OnDisable)
			{
				bool doesPowerHaveCasterAutoHit = false;

				if (!stricmp(ppow->psetParent->pcatParent->pchName, "Set_Bonus"))
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod on a set bonus power!", ppow->pchFullName, ppowApplicationTypeString);
				}

				if (ppow->eType == kPowerType_Boost)
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod on a boost!", ppow->pchFullName, ppowApplicationTypeString);
				}
				else if (ppow->eType == kPowerType_GlobalBoost)
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod on a global boost!", ppow->pchFullName, ppowApplicationTypeString);
				}

				if (ptemplate->eTarget != kModTarget_Caster
							&& ptemplate->eTarget != kModTarget_CastersOwnerAndAllPets)
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod that doesn't target the caster!", ppow->pchFullName, ppowApplicationTypeString);
				}

				for (j = 0; j < eaiSize(&(int*)ppow->pAutoHit); j++)
				{
					if (ppow->pAutoHit[j] == kTargetType_Caster)
					{
						doesPowerHaveCasterAutoHit = true;
					}
				}

				if (!doesPowerHaveCasterAutoHit)
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod on a power that doesn't auto-hit the caster!", ppow->pchFullName, ppowApplicationTypeString);
				}
			}

			if (ppowApplicationType == kModApplicationType_OnActivate
				|| ppowApplicationType == kModApplicationType_OnDeactivate
				|| ppowApplicationType == kModApplicationType_OnExpire)
			{
				if (ppow->iTimeToConfirm > 0)
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod on a power that requires confirmation!", ppow->pchFullName, ppowApplicationTypeString);
				}
			}

			if (ppowApplicationType == kModApplicationType_OnDeactivate
				|| ppowApplicationType == kModApplicationType_OnExpire)
			{
				if (ppow->eTargetType == kTargetType_Location)
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod on a TargetType Location power!", ppow->pchFullName, ppowApplicationTypeString);
				}
				else if (ppow->eTargetType == kTargetType_Teleport)
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod on a TargetType Teleport power!", ppow->pchFullName, ppowApplicationTypeString);
				}
				else if (ppow->eTargetType == kTargetType_Position)
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod on a TargetType Position power!", ppow->pchFullName, ppowApplicationTypeString);
				}
			}

			if (ppowApplicationType == kModApplicationType_OnDeactivate
				|| ppowApplicationType == kModApplicationType_OnExpire
				|| ppowApplicationType == kModApplicationType_OnDisable)
			{
				if (ppow->bDoNotSave)
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod on a DoNotSave power!", ppow->pchFullName, ppowApplicationTypeString);
				}
			}

			if (ppowApplicationType == kModApplicationType_OnEnable
				|| ppowApplicationType == kModApplicationType_OnDisable)
			{
				if (ppow->eType == kPowerType_Inspiration)
				{
					ErrorFilenamef(ppow->pchSourceFile, "Power (%s) cannot have a %s attrib mod on an inspiration!", ppow->pchFullName, ppowApplicationTypeString);
				}
			}

			if (!template_Validate(ppow, ptemplate))
				broken = true;

			if (ptemplate->pParams) {
				if (!params_Validate(ppow, ptemplate))
					broken = true;
			}

			if (broken) {
				// Something really important is missing from this template.
				// Remove it, so that we don't have to be paranoid about param
				// pointers and such at runtime.
				eaRemoveConst(&pEffect->ppTemplates, i);
			}
		}

		// Recurse
		if (pEffect->ppEffects)
			effectgroup_Validate(ppow, pEffect->ppEffects);
	}
}

static void effectgroup_Finalize(BasePower *ppow, const EffectGroup **ppEffects, bool bCheckSuppression)
{
	int ei, imod;
	for (ei = eaSize(&ppEffects) - 1; ei >= 0; ei--)
	{
		EffectGroup *pEffect = (EffectGroup*)ppEffects[ei];

		for (imod = eaSize(&ppEffects[ei]->ppTemplates) - 1; imod >= 0; imod--)
		{
			AttribModTemplate *ptemplate = (AttribModTemplate*)pEffect->ppTemplates[imod];

#ifdef SERVER
			// If this is a var lookup, then get the index. If this
			//   lookup fails, then a class table is used instead.
			ptemplate->iVarIndex = basepower_GetVarIdx(ppow, ptemplate->pchTable);

			if (TemplateHasAttrib(ptemplate, kSpecialAttrib_ExecutePower))
			{
				AttribModParam_Power *param = (AttribModParam_Power*)TemplateGetParams(ptemplate, Power);
				int i;

				ppow->bHasExecuteMods = true;

				powerspec_Resolve(&param->ps);
				for (i = eaSize(&param->ps.ppPowers) - 1; i >= 0; i--)
				{
					const BasePower *ref = param->ps.ppPowers[i];
					if (ref->eType != kPowerType_Click)
						ErrorFilenamef(ppow->pchSourceFile,"Power (%s) is a target for ExecutePower, but is not a click power!",ref->pchFullName);
					if (ref->fInterruptTime > 0)
						ErrorFilenamef(ppow->pchSourceFile,"Power (%s) is a target for ExecutePower and is not allowed to have an interrupt time.",ref->pchFullName);
				}
			}

			//
			// Calculate bSuppressMethodAlways
			//
			ptemplate->bSuppressMethodAlways = false;

			// Check BoostTemplate mode
			if(!TemplateHasFlag(ptemplate, Boost))
			{
				ppow->bHasNonBoostTemplates = true;
			}

			if (ptemplate->ppSuppress)
			{
				int iSizeSup = eaSize(&ptemplate->ppSuppress);
				bool bSuppressAlways = false;
				int iSup;

				if (iSizeSup)
				{
					bSuppressAlways = ptemplate->ppSuppress[0]->bAlways;

					for (iSup = 1; iSup < iSizeSup; iSup++)
					{
						if (ptemplate->ppSuppress[iSup]->bAlways != bSuppressAlways)
						{
							// Mismatch! Complain, set always to false, and break
							ErrorFilenamef(ppow->pchSourceFile, "Suppression Always flags of attribmod %d on (%s) are inconsistent.\nSetting SuppressMethodAlways false.\n", imod, ppow->pchSourceName);
							bSuppressAlways = false;
							break;
						}
					}

					for (iSup = 0; iSup < iSizeSup; iSup++)
					{
						int iEvent = ptemplate->ppSuppress[iSup]->idxEvent;
						if (POWEREVENT_INVOKE(iEvent))
							ppow->bHasInvokeSuppression = true;
					}
				}

				ptemplate->bSuppressMethodAlways = bSuppressAlways;
			}

			// Make sure that if this attribmod should be suppressed, that it is
			if (bCheckSuppression
				&& !ptemplate->ppSuppress
				&& ptemplate->eTarget != kModTarget_Caster
				&& IS_MODIFIER(ptemplate->offAspect)
				&& ptemplate->fScale >= 0
				&& TemplateHasAttribFromList(ptemplate, offsetof(CharacterAttributes, fTerrorized),
				   offsetof(CharacterAttributes, fImmobilized),
				   offsetof(CharacterAttributes, fHeld),
				   offsetof(CharacterAttributes, fStunned),
				   offsetof(CharacterAttributes, fSleep))
				)
			{
				bool bHitsPlayers = EffectHasFlag(pEffect, PVPOnly) || !pEffect->ppchRequires;
				if (!bHitsPlayers && !EffectHasFlag(pEffect, PVEOnly))
				{
					// Awful awful hack that needs to die
					int n = eaSize(&pEffect->ppchRequires);
					int i;
					for (i = 0; i < n; i++)
					{
						if (strcmp("player",pEffect->ppchRequires[i]) == 0)
							bHitsPlayers = true;
					}
				}

				if (bHitsPlayers)
				{
					int n = eaiSize(&(int*)ppow->pAffected);
					int i;
					bHitsPlayers = false;
					for (i = 0; i < n; i++)
					{
						if (ppow->pAffected[i] == kTargetType_Foe)
							bHitsPlayers = true;
					}
				}

				if(bHitsPlayers)
				{
					if(TemplateHasFlag(ptemplate, IgnoreSuppressErrors))
					{
						// Suppressed
					}
					else
					{
						ErrorFilenamef(ppow->pchSourceFile, "AttribMod %d of (%s) should probably have supression on it but doesn't.\nIf you are certain that this is correct, you can put \"IgnoreSuppressErrors %s\" on this attribmod to ignore this discrepency.\n",
							imod+1, ppow->pchSourceName,
							getUserName());
					}
				}
				else
				{
					if(TemplateHasFlag(ptemplate, IgnoreSuppressErrors))
					{
						ErrorFilenamef(ppow->pchSourceFile, "AttribMod %d of (%s): used IgnoreSuppressErrors and didn't need to.\n",
							imod+1, ppow->pchSourceName);
					}
				}
			}
#else
			// If this is a var lookup, then get the index. If this
			//   lookup fails, then a class table is used instead.
			ptemplate->iVarIndex = basepower_GetVarIdx(ppow, ptemplate->pchTable);
#endif
		}
		// Recurse
		if (pEffect->ppEffects)
			effectgroup_Finalize(ppow, pEffect->ppEffects, bCheckSuppression);
	}
}

/**********************************************************************func*
* power_Finalize
*
* NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
*
* This does all the per-power finalization, so we can call it seperately on power
* reloads. This must be called after FinalizeDict, as that sets the default dim return
*
*/
void power_Finalize(BasePower *ppow, PowerDictionary *pdict)
{
	int i, ei;
	bool bCheckSuppression = false;
	bool foundRedirectPower = false;

	if (!ppow->psetParent || !ppow->psetParent->pcatParent)
	{
		const BasePower *temp = powerdict_GetBasePowerByFullName(pdict,ppow->pchFullName);
		if (temp && temp != ppow)
		{
			ErrorFilenamef(ppow->pchSourceFile,"Power (%s) is a duplicate of another power with the same name! Delete one of these powers.",ppow->pchFullName);
		}
		else
		{
			ErrorFilenamef(ppow->pchSourceFile,"Power (%s) is not in a power set or category. This probably means the .powers file needs to be removed",ppow->pchFullName);
		}
		return;
	}

	if(ppow->pchChainIntoPowerName)
	{
		const BasePower *temp = powerdict_GetBasePowerByFullName(pdict,ppow->pchChainIntoPowerName);
		if(!temp)
			ErrorFilenamef(ppow->pchSourceFile,"Power (%s) links to non-existent ChainIntoPower (%s)",ppow->pchFullName,ppow->pchChainIntoPowerName);
	}
	if(ppow->pchBoostCatalystConversion)
	{
		const BasePower *temp = powerdict_GetBasePowerByFullName(pdict,ppow->pchBoostCatalystConversion);
		if(!temp)
			ErrorFilenamef(ppow->pchSourceFile,"Power (%s) links to non-existent BoostCatalystConversion (%s)",ppow->pchFullName,ppow->pchBoostCatalystConversion);
	}
	if(ppow->pchRewardFallback)
	{
		const BasePower *temp = powerdict_GetBasePowerByFullName(pdict,ppow->pchRewardFallback);
		if(!temp)
			ErrorFilenamef(ppow->pchSourceFile,"Power (%s) links to non-existent RewardFallback (%s)",ppow->pchFullName,ppow->pchRewardFallback);
	}

#if SERVER
	bCheckSuppression = (stricmp(ppow->psetParent->pcatParent->pchName, "Inherent") == 0
		|| stricmp(ppow->psetParent->pcatParent->pchName, "Temporary_Powers") == 0
		|| stricmp(ppow->psetParent->pcatParent->pchName, "Pool") == 0
		|| stricmp(ppow->psetParent->pcatParent->pchName, "Pets") == 0
		|| stricmp(ppow->psetParent->pcatParent->pchName, "Epic") == 0
		|| strnicmp(ppow->psetParent->pcatParent->pchName, "Tanker",6) == 0
		|| strnicmp(ppow->psetParent->pcatParent->pchName, "Scrapper",8) == 0
		|| strnicmp(ppow->psetParent->pcatParent->pchName, "Blaster",7) == 0
		|| strnicmp(ppow->psetParent->pcatParent->pchName, "Defender",8) == 0
		|| strnicmp(ppow->psetParent->pcatParent->pchName, "Controller",10) == 0
		|| strnicmp(ppow->psetParent->pcatParent->pchName, "Peacebringer",12) == 0
		|| strnicmp(ppow->psetParent->pcatParent->pchName, "Warshade",8) == 0
		|| strnicmp(ppow->psetParent->pcatParent->pchName, "Kheldian",8) == 0);
#endif

	if (ppow->eTargetType != kTargetType_Position
		&& ppow->eTargetTypeSecondary != kTargetType_Position
		&& (ppow->ePositionCenter || ppow->fPositionDistance || ppow->fPositionHeight || ppow->fPositionYaw))
	{
		ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has Position information but does not have a Position target!", ppow->pchFullName);
	}
	else if (ppow->ePositionCenter != kModTarget_Caster && ppow->ePositionCenter != kModTarget_Focus)
	{
		ErrorFilenamef(ppow->pchSourceFile, "Power (%s) has an invalid PositionCenter!", ppow->pchFullName);
	}

	// Validate power redirection
	for (ei = eaSize(&ppow->ppRedirect) - 1; ei >= 0; ei--)
	{
		PowerRedirect *pRedir = (PowerRedirect*)ppow->ppRedirect[ei];
		const BasePower *temp = powerdict_GetBasePowerByFullName(pdict, pRedir->pchName);

		if(!temp)
			ErrorFilenamef(ppow->pchSourceFile,"Power (%s) has PowerRedirect link to non-existent Power (%s)",ppow->pchFullName,pRedir->pchName);
		else
		{
			pRedir->ppowBase = temp;
			foundRedirectPower = true;
			if (isDevelopmentMode())
			{
				if (temp->eType != ppow->eType)
					ErrorFilenamef(ppow->pchSourceFile,"Power (%s) has PowerRedirect attribMod link to a power (%s)that isn't the same type (%i:%i)", ppow->pchFullName,temp->pchFullName, ppow->eType, temp->eType);
				if (temp->bDeletable != ppow->bDeletable)
					ErrorFilenamef(ppow->pchSourceFile,"Power (%s) has PowerRedirect attribMod link to a power (%s)that isn't the same deletable (%i:%i)", ppow->pchFullName,temp->pchFullName, ppow->bDeletable, temp->bDeletable);
				if (temp->bTradeable != ppow->bTradeable)
					ErrorFilenamef(ppow->pchSourceFile,"Power (%s) has PowerRedirect attribMod link to a power (%s)that isn't the same bTradeable (%i:%i)", ppow->pchFullName,temp->pchFullName, ppow->bTradeable, temp->bTradeable);
				if (temp->bDoNotSave != ppow->bDoNotSave)
					ErrorFilenamef(ppow->pchSourceFile,"Power (%s) has PowerRedirect attribMod link to a power (%s)that isn't the same bDoNotSave (%i:%i)", ppow->pchFullName,temp->pchFullName, ppow->bDoNotSave, temp->bDoNotSave);
				if (temp->iNumCharges != ppow->iNumCharges)
					ErrorFilenamef(ppow->pchSourceFile,"Power (%s) has PowerRedirect attribMod link to a power (%s)that isn't the same charge limit (%i:%i)", ppow->pchFullName,temp->pchFullName, ppow->iNumCharges, temp->iNumCharges);
				if (temp->fUsageTime != ppow->fUsageTime)
					ErrorFilenamef(ppow->pchSourceFile,"Power (%s) has PowerRedirect attribMod link to a power (%s)that isn't the same usageTime (%f:%f)", ppow->pchFullName,temp->pchFullName, ppow->fUsageTime, temp->fUsageTime);
				if (temp->bIgnoreToggleMaxDistance != ppow->bIgnoreToggleMaxDistance)
					ErrorFilenamef(ppow->pchSourceFile,"Power (%s) has PowerRedirect attribMod link to a power (%s)that isn't the same bIgnoreToggleMaxDistance (%i:%i)", ppow->pchFullName,temp->pchFullName, ppow->bIgnoreToggleMaxDistance, temp->bIgnoreToggleMaxDistance);
				if (temp->eTargetTypeSecondary != ppow->eTargetTypeSecondary)
					ErrorFilenamef(ppow->pchSourceFile,"Power (%s) has PowerRedirect attribMod link to a power (%s)that isn't the same eTargetTypeSecondary (%i:%i)", ppow->pchFullName,temp->pchFullName, ppow->eTargetTypeSecondary, temp->eTargetTypeSecondary);
			}
		}
	}

	if (ppow->ppRedirect && !foundRedirectPower)
		ErrorFilenamef(ppow->pchSourceFile,"Power (%s) is a PowerRedirector but links to no valid powers",ppow->pchFullName);

	//
	// See if this power has any usage limits
	//

	if(ppow->iNumCharges>0)
	{
		ppow->bHasUseLimit = true;
	}
	if(ppow->fUsageTime>0)
	{
		ppow->bHasUseLimit = true;
	}
	if(ppow->fLifetime>0 || ppow->fLifetimeInGame>0)
	{
		ppow->bHasLifetime = true;
	}

	if (ppow->bStackingUsage && ppow->iNumAllowed > 1)
		ErrorFilenamef(ppow->pchSourceFile,"Power (%s) has StackingUsage with time limits set but allows more than one copy of the power.",ppow->pchFullName);

	//
	// Set up the diminishing returns information for the power
	//
	for(i=0; i<eaSize(&g_DimReturnList.ppSets); i++)
	{
		int j;

		if(pDimSetDefault == g_DimReturnList.ppSets[i])
			continue;

		for(j=0; j<eaiSize(&g_DimReturnList.ppSets[i]->piBoosts); j++)
		{
			int k;
			for(k=0; k<eaiSize(&ppow->pBoostsAllowed); k++)
			{
				if(ppow->pBoostsAllowed[k]==g_DimReturnList.ppSets[i]->piBoosts[j])
				{
					int l;
					for(l=0; l<eaSize(&g_DimReturnList.ppSets[i]->ppReturns); l++)
					{
						int m;
						if(g_DimReturnList.ppSets[i]->ppReturns[l]->bDefault)
						{
							ErrorFilenamef("defs/dim_returns.def", "Illegal AttribReturnSet marked as default. Only one AttribReturnSet can be\nmarked as default, and this set must be in the Default ReturnSet.");
						}

						for(m=0; m<eaiSize(&g_DimReturnList.ppSets[i]->ppReturns[l]->offAttribs); m++)
						{
							int idx = g_DimReturnList.ppSets[i]->ppReturns[l]->offAttribs[m]/sizeof(float);
							if(ppow->apppDimReturns[idx] && ppow->apppDimReturns[idx]!=&g_DimReturnList.ppSets[i]->ppReturns[l]->pReturns)
							{
								int iter;
								bool equivalent = true;

								iter = eaSize(&g_DimReturnList.ppSets[i]->ppReturns[l]->pReturns) - 1;
								if(iter != eaSize(ppow->apppDimReturns[idx]) - 1)
								{
									equivalent = false;
								}
								else
								{
									for( ; iter >= 0; iter--)
									{
										// If all the data for the diminishing returns set is the same, then there is no actual conflict
										const DimReturn *powDimReturn = (*(ppow->apppDimReturns[idx]))[iter];
										const DimReturn *setDimReturn = g_DimReturnList.ppSets[i]->ppReturns[l]->pReturns[iter];
										if(powDimReturn->fStart != setDimReturn->fStart || powDimReturn->fHandicap != setDimReturn->fHandicap ||
											powDimReturn->fBasis != setDimReturn->fBasis)
										{
											equivalent = false;
											break;
										}
									}
								}

								if(!equivalent)
									ErrorFilenamef("defs/dim_returns.def", "Power (%s) has conflicting diminishing return info!", ppow->pchSourceName);
							}
							ppow->apppDimReturns[idx] = &g_DimReturnList.ppSets[i]->ppReturns[l]->pReturns;
						}
					}
				}
			}
		}
	}

	// And now stick in the defaults
	{
		int l;
		AttribDimReturnSet *pDefAttribDimReturnSet = NULL;

		for(l=0; l<eaSize(&pDimSetDefault->ppReturns); l++)
		{
			int m;
			if(pDimSetDefault->ppReturns[l]->bDefault)
			{
				pDefAttribDimReturnSet = pDimSetDefault->ppReturns[l];
				continue;
			}

			for(m=0; m<eaiSize(&pDimSetDefault->ppReturns[l]->offAttribs); m++)
			{
				int idx = pDimSetDefault->ppReturns[l]->offAttribs[m]/sizeof(float);
				if(!ppow->apppDimReturns[idx])
					ppow->apppDimReturns[idx] = &pDimSetDefault->ppReturns[l]->pReturns;
			}
		}


		for(i=0; i<CHAR_ATTRIB_COUNT; i++)
		{
			if(!ppow->apppDimReturns[i])
				ppow->apppDimReturns[i] = &pDefAttribDimReturnSet->pReturns;
		}
	}

	ppow->bHasInvokeSuppression = false; // default, set below if true
	ppow->bHasNonBoostTemplates = false; // ditto
	ppow->bHasExecuteMods = false;
	effectgroup_Finalize(ppow, ppow->ppEffects, bCheckSuppression);
}

/**********************************************************************func*
 * powerdict_Finalize
 *
 * NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
 *
 * This function is done after loading the powers information from the bin
 * file. Anything which is saved to the bin file should be done in
 * powerdict_FixupForBin (above). This function is called after all
 * the information that the client doesn't need has been stripped out.
 * This is called before the data is put in shared memory, so pointers to
 * this data will break.
 *
 * This is all the finalization that cares about power sets
 *
 */
static void powerfx_Finalize(PowerFX *fx)
{
	fx->fTimeToHit = fx->iFramesBeforeHit/30.0f;
	fx->fTimeToSecondaryHit = fx->iFramesBeforeSecondaryHit / 30.0f;
	fx->fInitialTimeToHit = fx->iInitialFramesBeforeHit/30.0f;
	fx->fTimeToBlock = fx->iFramesBeforeBlock/30.0f;
	fx->fInitialTimeToBlock = fx->iInitialFramesBeforeBlock/30.0f;
}

int powerdict_compareCategories(const PowerCategory **left, const PowerCategory **right)
{
	return strcmp((*left)->pchName, (*right)->pchName);
}

void powerdict_Finalize(PowerDictionary *pdict)
{
	int i;
	int icat;
	int iSizeCat;
	pDimSetDefault = NULL;

	assert(pdict!=NULL);

	// Find the default DimReturnSet
	for(i=0; i<eaSize(&g_DimReturnList.ppSets); i++)
	{
		if(g_DimReturnList.ppSets[i]->bDefault)
		{
			pDimSetDefault = g_DimReturnList.ppSets[i];
			break;
		}
	}
	if(!pDimSetDefault)
		ErrorFilenamef("defs/dim_returns.def", "No default ReturnSet defined!");

	eaQSortConst(pdict->ppPowerCategories, powerdict_compareCategories);

	iSizeCat = eaSize(&pdict->ppPowerCategories);
	for(icat=0; icat<iSizeCat; icat++)
	{
		int iset;
		PowerCategory *pcat = (PowerCategory*)pdict->ppPowerCategories[icat];
		int iSizeSets = eaSize(&pcat->ppPowerSets);

		for(iset=0; iset<iSizeSets; iset++)
		{
			int ipow;
			BasePowerSet *pset = (BasePowerSet*)pcat->ppPowerSets[iset];
			int iSizePows = eaSize(&pset->ppPowers);

			pset->pcatParent = pcat;

			for(ipow=0; ipow<iSizePows; ipow++)
			{
				int ifx;
				BasePower *ppow = (BasePower*)pset->ppPowers[ipow];

				ppow->psetParent = pset;

				basepower_SetID(ppow, icat, iset, ipow);

				//Allows artist their own special pfx testing file, loads the tpfx.txt into all art test powers
				{
					extern int artTestPowerLoaded;
					if( isDevelopmentMode() && artTestPowerLoaded && 0 == stricmp( pset->pchName, "Art_Test" ) )
					{
						extern BasePower artTestPower;
						ppow->fx = artTestPower.fx;  //Structure copy
						ppow->fTimeToActivate = ((F32)ppow->fx.iFramesAttack)/30.0; //Because AL said they need to be the same
					}
				}
				//End pfx testing file

				powerfx_Finalize(&ppow->fx);
				for(ifx = eaSize(&ppow->customfx)-1; ifx >= 0; --ifx)
				{
					PowerCustomFX *customfx = cpp_const_cast(PowerCustomFX*)(ppow->customfx[ifx]); 
					powerfx_Finalize(&customfx->fx);
				}
			} // for each power
		} // for each set
	} // for each category

	powerdict_ResetStats(pdict);
}

void powerdict_FindSharedPowersets(const PowerDictionary *pdict)
{
	int icat;
	int iSizeCat;

	iSizeCat = eaSize(&pdict->ppPowerCategories);
	for(icat=0; icat<iSizeCat; icat++)
	{
		int iset;
		PowerCategory *pcat = (PowerCategory*)pdict->ppPowerCategories[icat];
		int iSizeSets = eaSize(&pcat->ppPowerSets);

		for(iset=0; iset<iSizeSets; iset++)
		{
			BasePowerSet *pset = (BasePowerSet*)pcat->ppPowerSets[iset];

			if (pset->bIsShared)
			{
				eaPush(&g_SharedPowersets, pset);
			}
		}
	}

	g_numSharedPowersets = eaSize(&g_SharedPowersets);
}

static void effectgroup_SetBackPointers(BasePower *ppow, EffectGroup *pEffect, EffectGroup *pParent)
{
	int i;
	pEffect->ppowBase = ppow;
	pEffect->peffParent = pParent;

	for (i = eaSize(&pEffect->ppEffects)-1; i >= 0; i--) {
		effectgroup_SetBackPointers(ppow, (EffectGroup*)pEffect->ppEffects[i], pEffect);
	}
	for(i = eaSize(&pEffect->ppTemplates)-1; i >= 0; i--) {
		AttribModTemplate *mod = (AttribModTemplate*)pEffect->ppTemplates[i];
		mod->ppowBase = ppow;
		mod->peffParent = pEffect;
	}
}

/**********************************************************************func*
 * powerdict_SetBackPointers
 *
 * Sets the backpointers on powers. This is called after things are loaded into 
 * shared memory while we can still write to it
 *
 */
void powerdict_SetBackPointers(PowerDictionary *pdict)
{
	int icat;

	assert(pdict!=NULL);

	// Put all the backreferences in the power sets, powers, and templates
	for(icat=eaSize(&pdict->ppPowerCategories)-1; icat>=0; icat--)
	{
		int iset;
		PowerCategory *pcat = (PowerCategory*)pdict->ppPowerCategories[icat];

		for(iset=eaSize(&pcat->ppPowerSets)-1; iset>=0; iset--)
		{
			int ipow;
			BasePowerSet *pset = (BasePowerSet*)pcat->ppPowerSets[iset];

			pset->pcatParent = pcat;	

			for(ipow=eaSize(&pset->ppPowers)-1; ipow>=0; ipow--)
			{
				int ieff;
				BasePower *ppow = (BasePower*)pset->ppPowers[ipow];

				ppow->psetParent = pset;

				for(ieff=eaSize(&ppow->ppEffects)-1; ieff>=0; ieff--)
				{
					effectgroup_SetBackPointers(ppow, (EffectGroup*)ppow->ppEffects[ieff], NULL);
				}
			}
		}
	}
}

/**********************************************************************func*
 * powerdict_FinalizeUnshared
 *
 * This function is called after all the shared memory crap has been done.
 * Unshared globals based on the shared data are set in here.
 *
 */
void powerdict_FinalizeUnshared(const PowerDictionary *pdict)
{
	assert(pdict!=NULL);

	// Get the values for the automatically-set modes.
	{
		const char *pch;

		if((pch=StaticDefineLookup(ParsePowerDefines, "kRaid_Attacker_Mode"))==0)
			ErrorFilenamef("Defs/attrib_names.def", "Unable to find required mode: Raid_Attacker_Mode\n");
		else
			g_eRaidAttackerMode = atoi(pch);

		if((pch=StaticDefineLookup(ParsePowerDefines, "kRaid_Defender_Mode"))==0)
			ErrorFilenamef("Defs/attrib_names.def", "Unable to find required mode: Raid_Defender_Mode\n");
		else
			g_eRaidDefenderMode = atoi(pch);

		if((pch=StaticDefineLookup(ParsePowerDefines, "kCoOpTeam"))==0)
			ErrorFilenamef("Defs/attrib_names.def", "Unable to find required mode: CoOpTeam\n");
		else
			g_eOnCoopTeamMode = atoi(pch);
	}

}

#ifdef HASH_POWERS
/**********************************************************************func*
 * powerdict_CreateHashes
 *
 */
void powerdict_CreateHashes(const PowerDictionary *pdict)
{

	int icat;

	assert(pdict!=NULL);

	pdict->haCategoryNames = stashTableCreateWithStringKeys(2*eaSize(&pdict->ppPowerCategories), StashDeepCopyKeys);

	for(icat=eaSize(&pdict->ppPowerCategories)-1; icat>=0; icat--)
	{
		int iset;
		PowerCategory *pcat = pdict->ppPowerCategories[icat];

		stashAddPointer(pdict->haCategoryNames, pcat->pchName, pcat, false);
		pcat->haSetNames = stashTableCreateWithStringKeys(2*eaSize(&pcat->ppPowerSets), StashDeepCopyKeys);

		for(iset=eaSize(&pcat->ppPowerSets)-1; iset>=0; iset--)
		{
			int ipow;
			BasePowerSet *pset = pcat->ppPowerSets[iset];

			stashAddPointer(pcat->haSetNames, pset->pchName, pset, false)
			pset->haPowerNames = stashTableCreateWithStringKeys(2*eaSize(&pset->ppPowers), StashDeepCopyKeys);

			for(ipow=eaSize(&pset->ppPowers)-1; ipow>=0; ipow--)
			{
				BasePower *ppow = pset->ppPowers[ipow];

				stashAddPointer(pset->haPowerNames, ppow->pchName, ppow, false);

			}
		}
	}

	// --------------------
	// build the recipes inventory

	//powerdictionary_CreateRecipeHashes( pdict );

}
#endif

/**********************************************************************func*
* powerdict_CreateDictHashes
* 
* Create the hashes for the power, set, and category dictionaries
*
*/

bool powerdict_CreateDictHashes(PowerDictionary *pdict, bool shared_memory)
{
	bool ret = true;
	int i;

	assert(!pdict->haCategoryNames);
	pdict->haCategoryNames = stashTableCreateWithStringKeys(stashOptimalSize(eaSize(&pdict->ppPowerCategories)), stashShared(shared_memory));

	for(i=0; i<eaSize(&pdict->ppPowerCategories); i++)
	{
		PowerCategory *pcat = (PowerCategory*)pdict->ppPowerCategories[i];		
		if (!stashAddPointerConst(pdict->haCategoryNames, pcat->pchName, pcat, false))
		{
			ErrorFilenamef(pcat->pchSourceFile,"Duplicate power category name: %s",pcat->pchName);
			ret = false;
		}
	}

	assert(!pdict->haSetNames);
	pdict->haSetNames = stashTableCreateWithStringKeys(stashOptimalSize(eaSize(&pdict->ppPowerSets)), stashShared(shared_memory));

	for(i=0; i<eaSize(&pdict->ppPowerSets); i++)
	{
		BasePowerSet *pset = (BasePowerSet*)pdict->ppPowerSets[i];

		if (!stashAddPointerConst(pdict->haSetNames, pset->pchFullName, pset, false))
		{
			ErrorFilenamef(pset->pchSourceFile,"Duplicate power set name: %s",pset->pchFullName);
			ret = false;
		}
	}

	assert(!pdict->haPowerCRCs);
	pdict->haPowerCRCs = stashTableCreate(stashOptimalSize(eaSize(&pdict->ppPowers)), stashShared(shared_memory), StashKeyTypeInts, sizeof(int));

	for(i=0; i<eaSize(&pdict->ppPowers); i++)
	{
		BasePower *ppow = (BasePower*)pdict->ppPowers[i];
		int CRC = powerdict_GetCRCOfBasePower(ppow);
		if (!stashIntAddPointerConst(pdict->haPowerCRCs, CRC, ppow, false))
		{
			ErrorFilenamef(ppow->pchSourceFile,"Duplicate power name: %s",ppow->pchFullName);
			ret = false;
		}
	}

	return ret;
}

/**********************************************************************func*
* powerdict_DestroyDictHashes
* 
* Destroy the dictionary hashes, which is good if we need to reload things
*
*/

void powerdict_DestroyDictHashes(PowerDictionary *pdict)
{
	if (pdict->haCategoryNames)
	{
		stashTableDestroyConst(pdict->haCategoryNames);
		pdict->haCategoryNames = NULL;
	}
	if (pdict->haSetNames)
	{
		stashTableDestroyConst(pdict->haSetNames);
		pdict->haSetNames = NULL;
	}
	if (pdict->haPowerCRCs)
	{
		stashTableDestroyConst(pdict->haPowerCRCs);
		pdict->haPowerCRCs = NULL;
	}
}

/***************************************************************************
* params_FinalizeShared
* Helper functions
*/

static bool params_FinalizeShared(BasePower *ppow, AttribModTemplate *ptemplate)
{
	switch (*(int*)ptemplate->pParams)
	{
		PARAMCASE(Costume) {
			PARAMGETLO(Costume);
			const NPCDef *npc = npcFindByName(paramlo->pchCostumeName, &param->npcIndex);
			param->npcCostume = npcDefsGetCostume(param->npcIndex, 0);

			if (!param->npcCostume) {
				ErrorFilenamef(ppow->pchSourceFile, "Invalid Costume reference in power (%s): %s", ppow->pchFullName, paramlo->pchCostumeName);
				return false;
			}
		} PARAMCASEEND;

		PARAMCASE(EntCreate) {
			PARAMGETLO(EntCreate);
			if (!TemplateHasFlag(ptemplate, PseudoPet)) {
				if (paramlo->pchEntityDef) {
					if( strstriConst( paramlo->pchEntityDef, DOPPEL_NAME) ) {
						if (s_pPowerdictHandle) {
							// Oh the things we do for shared memory...
							param->pchDoppelganger = sharedMemoryAlloc(s_pPowerdictHandle, strlen(paramlo->pchEntityDef) - strlen(DOPPEL_NAME) + 1);
							strcpy((char*)param->pchDoppelganger, paramlo->pchEntityDef + strlen(DOPPEL_NAME));
						} else
							param->pchDoppelganger = strdup(paramlo->pchEntityDef + strlen(DOPPEL_NAME));
					} else {
						param->pVillain = villainFindByName(paramlo->pchEntityDef);
						if (!param->pVillain) {
							ErrorFilenamef(ppow->pchSourceFile, "Invalid VillainDef reference in power (%s): %s", ppow->pchFullName, paramlo->pchEntityDef);
							return false;
						}
					}
				}
				if (paramlo->pchClass) {
					param->pClass = classes_GetPtrFromName(&g_VillainClasses, paramlo->pchClass);
					if (!param->pClass) {
						ErrorFilenamef(ppow->pchSourceFile, "Invalid Class reference in power (%s): %s", ppow->pchFullName, paramlo->pchClass);
						return false;
					}
				}
			}
			if (paramlo && paramlo->pchCostumeName) {
				const NPCDef *npc = npcFindByName(paramlo->pchCostumeName, &param->npcIndex);
				param->npcCostume = npcDefsGetCostume(param->npcIndex, 0);

				if (!param->npcCostume) {
					ErrorFilenamef(ppow->pchSourceFile, "Invalid Costume reference in power (%s): %s", ppow->pchFullName, paramlo->pchCostumeName);
					return false;
				}
			}
		} PARAMCASEEND;
	}

	return true;
}

static void effectgroup_FinalizeShared(BasePower *ppow, EffectGroup *ppEffect)
{
	int imod;

	for (imod = eaSize(&ppEffect->ppTemplates)-1; imod >= 0; imod--)
	{
		if (ppEffect->ppTemplates[imod]->pParams) {
			if (!params_FinalizeShared(ppow, (AttribModTemplate*)ppEffect->ppTemplates[imod])) {
				// Well, crap. If the params are invalid, just nuke the template
				// because otherwise we might crash trying to follow the pointers.
				eaRemoveConst(&ppEffect->ppTemplates, imod);
			}
		}
	}
}


/**********************************************************************func*
* powerdict_FinalizeShared
*
* If powers were parsed for the first time, pointers need to be set up to
* other structures that also live in shared memory. Those structures depend
* on the powers dictionary being available in order to parse, so we have to
* come back and fix these up after everything else is loaded.
*
*/

void powerdict_FinalizeShared(const PowerDictionary *pdict)
{
	int ipow, ieff;

	// If we're getting our data from shared memory, someone else has already
	// done this.
	if (!s_bNeedSharedFinalization)
		return;

	// Lock the shared memory so we can write to it
	if (s_pPowerdictHandle)
		sharedMemoryLock(s_pPowerdictHandle);

	for (ipow = eaSize(&pdict->ppPowers) - 1; ipow >= 0; ipow--)
	{
		BasePower *ppow = (BasePower*)pdict->ppPowers[ipow];
		for (ieff = eaSize(&ppow->ppEffects) - 1; ieff >= 0; ieff--)
		{
			EffectGroup *peffect = (EffectGroup*)ppow->ppEffects[ieff];
			effectgroup_FinalizeShared(ppow, peffect);
		}
	}

	// Can finally release the load only structures
	ParserFreeLoadOnlyStore(s_PowersLoadOnly);
	s_PowersLoadOnly = 0;

	// Unlock shared memory and set it back to read-only
	if (s_pPowerdictHandle)
		sharedMemoryUnlock(s_pPowerdictHandle);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

StaticDefineInt AIReportEnum[] =
{
	DEFINE_INT
	{ "kAlways",             kAIReport_Always    },
	{ "kNever",              kAIReport_Never     },
	{ "kHitOnly",            kAIReport_HitOnly   },
	{ "kMissOnly",           kAIReport_MissOnly  },
	DEFINE_END
};

StaticDefineInt EffectAreaEnum[] =
{
	DEFINE_INT
	{ "kCharacter",          kEffectArea_Character  },
	{ "kChar",               kEffectArea_Character  },
	{ "kCone",               kEffectArea_Cone       },
	{ "kSphere",             kEffectArea_Sphere     },
	{ "kLocation",           kEffectArea_Location   },
	{ "kChain",              kEffectArea_Chain      },
	{ "kVolume",             kEffectArea_Volume     },
	{ "kNamedVolume",        kEffectArea_NamedVolume},  //Not implemented
	{ "kMap",                kEffectArea_Map        },
	{ "kRoom",               kEffectArea_Room       },
	{ "kTouch",              kEffectArea_Touch      },
	{ "kBox",                kEffectArea_Box        },
	DEFINE_END
};

StaticDefineInt TargetVisibilityEnum[] =
{
	DEFINE_INT
	{ "kLineOfSight",        kTargetVisibility_LineOfSight  },
	{ "kNone",               kTargetVisibility_None         },
	DEFINE_END
};

StaticDefineInt TargetTypeEnum[] =
{
	DEFINE_INT
	{ "kCaster",							kTargetType_Caster								},
	{ "kPlayer",							kTargetType_Player								},
	{ "kPlayerHero",						kTargetType_PlayerHero							},
	{ "kPlayerVillain",						kTargetType_PlayerVillain						},
	{ "kDeadPlayer",						kTargetType_DeadPlayer							},
	{ "kDeadPlayerFriend",					kTargetType_DeadPlayerFriend					},
	{ "kDeadPlayerFoe",						kTargetType_DeadPlayerFoe						},
	{ "kTeammate",							kTargetType_Teammate							},
	{ "kDeadTeammate",						kTargetType_DeadTeammate						},
	{ "kDeadOrAliveTeammate",				kTargetType_DeadOrAliveTeammate					},
	{ "kFriend",							kTargetType_Friend								},
	{ "kEnemy",								kTargetType_Villain								},
	{ "kVillain",							kTargetType_Villain								},
	{ "kDeadVillain",						kTargetType_DeadVillain							},
	{ "kFoe",								kTargetType_Foe									},
	{ "kNPC",								kTargetType_NPC									},
	{ "kLocation",							kTargetType_Location							},
	{ "kTeleport",							kTargetType_Teleport							},
	{ "kDeadFoe",							kTargetType_DeadFoe								},
	{ "kDeadOrAliveFoe",					kTargetType_DeadOrAliveFoe						},
	{ "kDeadFriend",						kTargetType_DeadFriend							},
	{ "kDeadOrAliveFriend",					kTargetType_DeadOrAliveFriend					},
	{ "kMyPet",								kTargetType_MyPet								},
	{ "kDeadMyPet",							kTargetType_DeadMyPet							},
	{ "kDeadOrAliveMyPet",					kTargetType_DeadOrAliveMyPet					},
	{ "kMyOwner",							kTargetType_MyOwner								},
	{ "kMyCreator",							kTargetType_MyCreator							},
	{ "kMyCreation",						kTargetType_MyCreation							},
	{ "kDeadMyCreation",					kTargetType_DeadMyCreation						},
	{ "kDeadOrAliveMyCreation",				kTargetType_DeadOrAliveMyCreation				},
	{ "kLeaguemate",						kTargetType_Leaguemate							},
	{ "kDeadLeaguemate",					kTargetType_DeadLeaguemate						},
	{ "kDeadOrAliveLeaguemate",				kTargetType_DeadOrAliveLeaguemate				},
	{ "kAny",								kTargetType_Any									},
	{ "kPosition",							kTargetType_Position							},
	{ "kNone",								0												},
	DEFINE_END
};

StaticDefineInt PowerTypeEnum[] =
{
	DEFINE_INT
	{ "kClick",			kPowerType_Click		},
	{ "kAuto",			kPowerType_Auto			},
	{ "kToggle",		kPowerType_Toggle		},
	{ "kBoost",			kPowerType_Boost		},
	{ "kInspiration",	kPowerType_Inspiration	},
	{ "kGlobalBoost",	kPowerType_GlobalBoost	},
	DEFINE_END
};

StaticDefineInt BoolEnum[] =
{
	DEFINE_INT
	{ "false",               0  },
	{ "kFalse",              0  },
	{ "true",                1  },
	{ "kTrue",               1  },
	DEFINE_END
};

StaticDefineInt ToggleDroppableEnum[] =
{
	DEFINE_INT
	{ "kSometimes", kToggleDroppable_Sometimes },
	{ "kAlways",    kToggleDroppable_Always    },
	{ "kFirst",     kToggleDroppable_First     },
	{ "kLast",      kToggleDroppable_Last      },
	{ "kNever",     kToggleDroppable_Never     },

	{ "Sometimes",  kToggleDroppable_Sometimes },
	{ "Always",     kToggleDroppable_Always    },
	{ "First",      kToggleDroppable_First     },
	{ "Last",       kToggleDroppable_Last      },
	{ "Never",      kToggleDroppable_Never     },
	DEFINE_END
};

StaticDefineInt ProcAllowedEnum[] =
{
	DEFINE_INT
	{ "kAll",			kProcAllowed_All		},
	{ "kNone",			kProcAllowed_None		},
	{ "kPowerOnly",		kProcAllowed_PowerOnly	},
	{ "kGlobalOnly",	kProcAllowed_GlobalOnly },
	DEFINE_END
};

StaticDefineInt ShowPowerSettingEnum[] =
{
	DEFINE_INT
	// Based on BoolEnum
	{ "false",		kShowPowerSetting_Never		},
	{ "kFalse",		kShowPowerSetting_Never		},
	{ "true",		kShowPowerSetting_Default	},
	{ "kTrue",		kShowPowerSetting_Default	},
	// New strings
	{ "Never",		kShowPowerSetting_Never		},
	{ "kNever",		kShowPowerSetting_Never		},
	{ "Always",		kShowPowerSetting_Always	},
	{ "kAlways",	kShowPowerSetting_Always	},
	{ "Default",	kShowPowerSetting_Default	},
	{ "kDefault",	kShowPowerSetting_Default	},
	{ "IfUsable",	kShowPowerSetting_IfUsable	},
	{ "kIfUsable",	kShowPowerSetting_IfUsable	},
	{ "IfOwned",	kShowPowerSetting_IfOwned	},
	{ "kIfOwned",	kShowPowerSetting_IfOwned	},
	DEFINE_END
};

TokenizerParseInfo ParsePowerVar[] =
{
	{ "Index",              TOK_STRUCTPARAM | TOK_INT(PowerVar, iIndex, 0),  },
	{ "Name",               TOK_STRUCTPARAM | TOK_STRING(PowerVar, pchName, 0)  },
	{ "Min",                TOK_STRUCTPARAM | TOK_F32(PowerVar, fMin, 0)     },
	{ "Max",                TOK_STRUCTPARAM | TOK_F32(PowerVar, fMax, 0)     },
	{ "\n",                 TOK_END, 0 },
	{ "", 0, 0 }
};

// These are all for the visual effects. This supports the old, goofy
// lf_gamer names as well as the new, sensible ones.
#define ParsePowerFX(type, fx) \
	{ "AttackBits",       			TOK_INTARRAY(type, fx.piAttackBits),          ParsePowerDefines }, \
	{ "BlockBits",        			TOK_INTARRAY(type, fx.piBlockBits),           ParsePowerDefines }, \
	{ "WindUpBits",       			TOK_INTARRAY(type, fx.piWindUpBits),          ParsePowerDefines }, \
	{ "HitBits",          			TOK_INTARRAY(type, fx.piHitBits),             ParsePowerDefines }, \
	{ "DeathBits",        			TOK_INTARRAY(type, fx.piDeathBits),           ParsePowerDefines }, \
	{ "ActivationBits",   			TOK_INTARRAY(type, fx.piActivationBits),      ParsePowerDefines }, \
	{ "DeactivationBits", 			TOK_INTARRAY(type, fx.piDeactivationBits),    ParsePowerDefines }, \
	{ "InitialAttackBits",			TOK_INTARRAY(type, fx.piInitialAttackBits),   ParsePowerDefines }, \
	{ "ContinuingBits",   			TOK_INTARRAY(type, fx.piContinuingBits),      ParsePowerDefines }, \
	{ "ConditionalBits",  			TOK_INTARRAY(type, fx.piConditionalBits),     ParsePowerDefines }, \
	{ "ActivationFX",     			TOK_FILENAME(type, fx.pchActivationFX, 0)       }, \
	{ "DeactivationFX",   			TOK_FILENAME(type, fx.pchDeactivationFX, 0)     }, \
	{ "AttackFX",         			TOK_FILENAME(type, fx.pchAttackFX, 0)           }, \
	{ "SecondaryAttackFX",			TOK_FILENAME(type, fx.pchSecondaryAttackFX, 0)  }, \
	{ "HitFX",            			TOK_FILENAME(type, fx.pchHitFX, 0)              }, \
	{ "WindUpFX",         			TOK_FILENAME(type, fx.pchWindUpFX, 0)           }, \
	{ "BlockFX",          			TOK_FILENAME(type, fx.pchBlockFX, 0)            }, \
	{ "DeathFX",          			TOK_FILENAME(type, fx.pchDeathFX, 0)            }, \
	{ "InitialAttackFX",  			TOK_FILENAME(type, fx.pchInitialAttackFX, 0)    }, \
	{ "ContinuingFX",     			TOK_FILENAME(type, fx.pchContinuingFX[0], 0)       }, \
	{ "ContinuingFX1",     			TOK_FILENAME(type, fx.pchContinuingFX[0], 0)       }, \
	{ "ContinuingFX2",     			TOK_FILENAME(type, fx.pchContinuingFX[1], 0)       }, \
	{ "ContinuingFX3",     			TOK_FILENAME(type, fx.pchContinuingFX[2], 0)       }, \
	{ "ContinuingFX4",     			TOK_FILENAME(type, fx.pchContinuingFX[3], 0)       }, \
	{ "ConditionalFX",     			TOK_FILENAME(type, fx.pchConditionalFX[0], 0)       }, \
	{ "ConditionalFX1",     		TOK_FILENAME(type, fx.pchConditionalFX[0], 0)       }, \
	{ "ConditionalFX2",     		TOK_FILENAME(type, fx.pchConditionalFX[1], 0)       }, \
	{ "ConditionalFX3",     		TOK_FILENAME(type, fx.pchConditionalFX[2], 0)       }, \
	{ "ConditionalFX4",     		TOK_FILENAME(type, fx.pchConditionalFX[3], 0)       }, \
	{ "ModeBits",                   TOK_INTARRAY(type, fx.piModeBits), ParsePowerDefines }, \
	{ "FramesBeforeHit",            TOK_INT(type, fx.iFramesBeforeHit, -1)   }, \
	{ "FramesBeforeSecondaryHit",   TOK_INT(type, fx.iFramesBeforeSecondaryHit, 0)   }, \
	{ "SeqBits",                    TOK_REDUNDANTNAME|TOK_INTARRAY(type, fx.piModeBits), ParsePowerDefines }, \
	{ "cast_anim",                  TOK_REDUNDANTNAME|TOK_INTARRAY(type, fx.piAttackBits), ParsePowerDefines }, \
	{ "hit_anim",                   TOK_REDUNDANTNAME|TOK_INTARRAY(type, fx.piHitBits), ParsePowerDefines }, \
	{ "deathanimbits",              TOK_REDUNDANTNAME|TOK_INTARRAY(type, fx.piDeathBits), ParsePowerDefines }, \
	{ "AttachedAnim",               TOK_REDUNDANTNAME|TOK_INTARRAY(type, fx.piActivationBits), ParsePowerDefines }, \
	{ "AttachedFxName",             TOK_REDUNDANTNAME|TOK_FILENAME(type, fx.pchActivationFX, 0)    }, \
	{ "TravellingProjectileEffect", TOK_REDUNDANTNAME|TOK_FILENAME(type, fx.pchAttackFX, 0)        }, \
	{ "AttachedToVictimFxName",     TOK_REDUNDANTNAME|TOK_FILENAME(type, fx.pchHitFX, 0)           }, \
	{ "TimeBeforePunchHits",        TOK_REDUNDANTNAME|TOK_INT(type, fx.iFramesBeforeHit,  -1)   }, \
	{ "TimeBeforeMissileSpawns",    TOK_REDUNDANTNAME|TOK_INT(type, fx.iFramesBeforeHit,  -1)   }, \
	{ "DelayedHit",                 TOK_BOOL(type, fx.bDelayedHit, 0), BoolEnum }, \
	{ "TogglePower",                TOK_IGNORE,                        0 }, \
	\
	{ "AttackFrames",               TOK_INT(type, fx.iFramesAttack, -1)      }, \
	{ "NonInterruptFrames",         TOK_REDUNDANTNAME|TOK_INT(type, fx.iFramesAttack, -1)      }, \
	\
	{ "InitialFramesBeforeHit",     TOK_INT(type, fx.iInitialFramesBeforeHit, -1)    }, \
	{ "InitialAttackFXFrameDelay",	TOK_INT(type, fx.iInitialAttackFXFrameDelay, 0)     }, \
	{ "ProjectileSpeed",            TOK_F32(type, fx.fProjectileSpeed, 0)           }, \
	{ "SecondaryProjectileSpeed",   TOK_F32(type, fx.fSecondaryProjectileSpeed, 0)      }, \
	{ "InitialFramesBeforeBlock",   TOK_INT(type, fx.iInitialFramesBeforeBlock, 0)  }, \
	{ "IgnoreAttackTimeErrors",     TOK_STRING(type, fx.pchIgnoreAttackTimeErrors, 0)  }, \
	{ "FramesBeforeBlock",          TOK_INT(type, fx.iFramesBeforeBlock, 0)         }, \
	{ "FXImportant",                TOK_BOOL(type, fx.bImportant,	0), BoolEnum }, \
	{ "PrimaryTint",				TOK_CONDRGB(type, fx.defaultTint.primary) }, \
	{ "SecondaryTint",				TOK_CONDRGB(type, fx.defaultTint.secondary) },

static TokenizerParseInfo ParseColor[] =
{
	{ "",			TOK_STRUCTPARAM | TOK_FIXED_ARRAY | TOK_F32_X, 0, 3 },
	{ "\n",			TOK_END												},
	{ "", 0, 0 }
};

static TokenizerParseInfo ParseColorPalette[] =
{
	{ "Color", 		TOK_STRUCT(ColorPalette, color, ParseColor)			},
	{ "Name",		TOK_POOL_STRING|TOK_STRING(ColorPalette, name, 0)	},
	{ "End",		TOK_END												},
	{ "EndPalette",	TOK_END												},
	{ "", 0, 0 }
};

TokenizerParseInfo ParsePowerCustomFX[] =
{
	{ "Token",			TOK_STRUCTPARAM|TOK_POOL_STRING|TOK_STRING(PowerCustomFX, pchToken, 0) },
	{ "AltTheme",		TOK_STRINGARRAY(PowerCustomFX, altThemes)					},
	{ "SourceFile",		TOK_CURRENTFILE(PowerCustomFX, fx.pchSourceFile)			},
	{ "Category",		TOK_POOL_STRING|TOK_STRING(PowerCustomFX, pchCategory, 0)	},
	{ "DisplayName",	TOK_STRING(PowerCustomFX, pchDisplayName, 0)				},
	ParsePowerFX(PowerCustomFX, fx) // included flat
	{ "Palette",		TOK_POOL_STRING|TOK_STRING(PowerCustomFX, paletteName, 0)	},
	{ "End",			TOK_END														},
	{ "", 0, 0 }
};

StaticDefineInt DeathCastableSettingEnum[] =
{
	DEFINE_INT
	// old values
	{ "false",				kDeathCastableSetting_AliveOnly		},
	{ "kFalse",				kDeathCastableSetting_AliveOnly		},
	{ "true",				kDeathCastableSetting_DeadOnly		},
	{ "kTrue",				kDeathCastableSetting_DeadOnly		},
	// new values
	{ "AliveOnly",			kDeathCastableSetting_AliveOnly		},
	{ "kAliveOnly",			kDeathCastableSetting_AliveOnly		},
	{ "DeadOnly",			kDeathCastableSetting_DeadOnly		},
	{ "kDeadOnly",			kDeathCastableSetting_DeadOnly		},
	{ "DeadOrAlive",		kDeathCastableSetting_DeadOrAlive	},
	{ "kDeadOrAlive",		kDeathCastableSetting_DeadOrAlive	},
	DEFINE_END
};

extern StaticDefineInt ModTargetEnum[];

TokenizerParseInfo ParsePowerRedirect[] =
{
	{ "{",                TOK_START,    0 },
	{ "Power",            TOK_STRING(PowerRedirect, pchName, 0) },
	{ "Requires",         TOK_STRINGARRAY(PowerRedirect, ppchRequires) },
	{ "ShowInInfo",       TOK_BOOL(PowerRedirect, bShowInInfo, -1) },
	{ "}",                TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseBasePower[] =
{
	{ "{",                  TOK_START,    0                                         },
	{ "FullName",           TOK_STRUCTPARAM | TOK_STRING(BasePower, pchFullName, 0)},
	{ "CRCFullName",		TOK_INT(BasePower, crcFullName, 0) },	// Do NOT add this to .powers files!  Needed here because that's how shared memory works?
	{ "SourceFile",         TOK_CURRENTFILE(BasePower, pchSourceFile) },
	{ "Name",               TOK_STRING(BasePower, pchName, 0)              },
	{ "SourceName",			TOK_STRING(BasePower, pchSourceName, 0)              },
	{ "System",             TOK_INT(BasePower, eSystem, kPowerSystem_Powers), PowerSystemEnum },
	{ "AutoIssue",          TOK_BOOL(BasePower, bAutoIssue, 0), BoolEnum },
	{ "AutoIssueSaveLevel", TOK_BOOL(BasePower, bAutoIssueSaveLevel, 0), BoolEnum },
	{ "Free",               TOK_BOOL(BasePower, bFree, 0), BoolEnum },
	{ "DisplayName",        TOK_STRING(BasePower, pchDisplayName, 0)       },
	{ "DisplayHelp",        TOK_STRING(BasePower, pchDisplayHelp, 0)       },
	{ "DisplayShortHelp",   TOK_STRING(BasePower, pchDisplayShortHelp, 0)  },
	{ "DisplayCasterHelp",     TOK_REDUNDANTNAME|TOK_STRING(BasePower, pchDisplayHelp, 0)       },
	{ "DisplayCasterShortHelp",TOK_REDUNDANTNAME|TOK_STRING(BasePower, pchDisplayShortHelp, 0)  },
	{ "DisplayTargetHelp",     TOK_STRING(BasePower, pchDisplayTargetHelp, 0)       },
	{ "DisplayTargetShortHelp",TOK_STRING(BasePower, pchDisplayTargetShortHelp, 0)  },
	{ "DisplayAttackerAttack", TOK_STRING(BasePower, pchDisplayAttackerAttack, 0)   },
	{ "DisplayAttackerAttackFloater", TOK_STRING(BasePower, pchDisplayAttackerAttackFloater, 0)   },
	{ "DisplayAttackerHit", TOK_STRING(BasePower, pchDisplayAttackerHit, 0)  },
	{ "DisplayVictimHit",   TOK_STRING(BasePower, pchDisplayVictimHit, 0)    },
	{ "DisplayConfirm",     TOK_STRING(BasePower, pchDisplayConfirm, 0)      },
	{ "FloatRewarded",      TOK_STRING(BasePower, pchDisplayFloatRewarded, 0)},
	{ "DisplayPowerDefenseFloat",TOK_STRING(BasePower, pchDisplayDefenseFloat, 0)},
	{ "IconName",           TOK_STRING(BasePower, pchIconName, 0)            },
	{ "FXName",             TOK_IGNORE,   0                                           },
	{ "Type",               TOK_INT(BasePower, eType, kPowerType_Click), PowerTypeEnum },
	{ "NumAllowed",         TOK_INT(BasePower, iNumAllowed, 1)},
	{ "AttackTypes",        TOK_INTARRAY(BasePower, pAttackTypes), ParsePowerDefines },
	{ "BuyRequires",        TOK_STRINGARRAY(BasePower, ppchBuyRequires),     },
	{ "ActivateRequires",   TOK_STRINGARRAY(BasePower, ppchActivateRequires),     },
	{ "SlotRequires",		TOK_STRINGARRAY(BasePower, ppchSlotRequires)       },
	{ "TargetRequires",		TOK_STRINGARRAY(BasePower, ppchTargetRequires)	},
	{ "RewardRequires",		TOK_STRINGARRAY(BasePower, ppchRewardRequires)	},
	{ "AuctionRequires",	TOK_STRINGARRAY(BasePower, ppchAuctionRequires)	},
	{ "RewardFallback",     TOK_STRING(BasePower, pchRewardFallback, 0)            },
	{ "Accuracy",           TOK_F32(BasePower, fAccuracy, 0)            },
	{ "NearGround",         TOK_BOOL(BasePower, bNearGround, 0), BoolEnum },
	{ "TargetNearGround",   TOK_BOOL(BasePower, bTargetNearGround, -1), BoolEnum },
	{ "CastableAfterDeath", TOK_INT(BasePower, eDeathCastableSetting, kDeathCastableSetting_AliveOnly), DeathCastableSettingEnum },
	{ "CastThroughHold",    TOK_BOOL(BasePower, bCastThroughHold, 0), BoolEnum },
	{ "CastThroughSleep",   TOK_BOOL(BasePower, bCastThroughSleep, 0), BoolEnum },
	{ "CastThroughStun",    TOK_BOOL(BasePower, bCastThroughStun, )0, BoolEnum },
	{ "CastThroughTerrorize",  TOK_BOOL(BasePower, bCastThroughTerrorize,0), BoolEnum },
	{ "ToggleIgnoreHold",   TOK_BOOL(BasePower, bToggleIgnoreHold, 0), BoolEnum },
	{ "ToggleIgnoreSleep",  TOK_BOOL(BasePower, bToggleIgnoreSleep, 0), BoolEnum },
	{ "ToggleIgnoreStun",   TOK_BOOL(BasePower, bToggleIgnoreStun, 0), BoolEnum },
	{ "IgnoreLevelBought",  TOK_BOOL(BasePower, bIgnoreLevelBought, 0), BoolEnum },
	{ "ShootThroughUntouchable", TOK_BOOL(BasePower, bShootThroughUntouchable, 0), BoolEnum },
	{ "InterruptLikeSleep", TOK_BOOL(BasePower, bInterruptLikeSleep, 0), BoolEnum },
	{ "AIReport",           TOK_INT(BasePower, eAIReport, kAIReport_Always),      AIReportEnum   },
	{ "EffectArea",         TOK_INT(BasePower, eEffectArea, kEffectArea_Character), EffectAreaEnum },
	{ "MaxTargetsHit",      TOK_INT(BasePower, iMaxTargetsHit, 0)      },
	{ "Radius",             TOK_F32(BasePower, fRadius, 0)              },
	{ "Arc",                TOK_F32(BasePower, fArc, 0)                 },
	{ "ChainDelay",         TOK_F32(BasePower, fChainDelay, 0)          },
	{ "ChainEff",           TOK_STRINGARRAY(BasePower, ppchChainEff)    },
	{ "ChainFork",          TOK_INTARRAY(BasePower, piChainFork)        },
	{ "BoxOffset",          TOK_VEC3(BasePower, vecBoxOffset)         },
	{ "BoxSize",            TOK_VEC3(BasePower, vecBoxSize)           },
	{ "Range",              TOK_F32(BasePower, fRange, 0)               },
	{ "RangeSecondary",     TOK_F32(BasePower, fRangeSecondary, 0)      },
	{ "TimeToActivate",     TOK_F32(BasePower, fTimeToActivate, 0)      },
	{ "RechargeTime",       TOK_F32(BasePower, fRechargeTime, 0)        },
	{ "ActivatePeriod",     TOK_F32(BasePower, fActivatePeriod, 0)      },
	{ "EnduranceCost",      TOK_F32(BasePower, fEnduranceCost, 0)       },
	{ "InsightCost",        TOK_REDUNDANTNAME|TOK_F32(BasePower, fInsightCost, 0)         },
	{ "IdeaCost",           TOK_F32(BasePower, fInsightCost, 0)         },
	{ "TimeToConfirm",      TOK_INT(BasePower, iTimeToConfirm, 0),      },
	{ "SelfConfirm",		TOK_INT(BasePower, iSelfConfirm, 0),		},
	{ "ConfirmRequires",	TOK_STRINGARRAY(BasePower, ppchConfirmRequires)	},
	{ "DestroyOnLimit",     TOK_BOOL(BasePower, bDestroyOnLimit, 1), BoolEnum },
	{ "StackingUsage",      TOK_BOOL(BasePower, bStackingUsage, 0), BoolEnum },
	{ "NumCharges",         TOK_INT(BasePower, iNumCharges, 0),         },
	{ "MaxNumCharges",      TOK_INT(BasePower, iMaxNumCharges, 0),         },
	{ "UsageTime",          TOK_F32(BasePower, fUsageTime, 0),          },
	{ "MaxUsageTime",       TOK_F32(BasePower, fMaxUsageTime, 0),          },
	{ "Lifetime",           TOK_F32(BasePower, fLifetime, 0),           },
	{ "MaxLifetime",        TOK_F32(BasePower, fMaxLifetime, 0),           },
	{ "LifetimeInGame",     TOK_F32(BasePower, fLifetimeInGame, 0),     },
	{ "MaxLifetimeInGame",  TOK_F32(BasePower, fMaxLifetimeInGame, 0),     },
	{ "InterruptTime",      TOK_F32(BasePower, fInterruptTime, 0)       },
	{ "TargetVisibility",   TOK_INT(BasePower, eTargetVisibility, kTargetVisibility_LineOfSight), TargetVisibilityEnum },
	{ "Target",             TOK_INT(BasePower, eTargetType, kTargetType_None), TargetTypeEnum },
	{ "TargetSecondary",    TOK_INT(BasePower, eTargetTypeSecondary, kTargetType_None), TargetTypeEnum },
	{ "EntsAutoHit",        TOK_INTARRAY(BasePower, pAutoHit), TargetTypeEnum },
	{ "EntsAffected",       TOK_INTARRAY(BasePower, pAffected), TargetTypeEnum },
	{ "TargetsThroughVisionPhase",	TOK_BOOL(BasePower, bTargetsThroughVisionPhase, 0), BoolEnum },
	{ "BoostsAllowed",      TOK_INTARRAY(BasePower, pBoostsAllowed), ParsePowerDefines },
	{ "GroupMembership",    TOK_INTARRAY(BasePower, pGroupMembership), ParsePowerDefines },
	{ "ModesRequired",      TOK_INTARRAY(BasePower, pModesRequired), ParsePowerDefines },
	{ "ModesDisallowed",    TOK_INTARRAY(BasePower, pModesDisallowed), ParsePowerDefines },
	{ "AIGroups",           TOK_STRINGARRAY(BasePower, ppchAIGroups),     },
	{ "Redirect",			TOK_STRUCT(BasePower, ppRedirect, ParsePowerRedirect) },
	{ "Effect",             TOK_STRUCT(BasePower, ppEffects, ParseEffectGroup) },
	{ "AttribMod",          TOK_NO_BIN | TOK_LOAD_ONLY | TOK_STRUCT(BasePower_loadonly, ppLegacyTemplates, ParseLegacyAttribModTemplate) },
	{ "IgnoreStrength",     TOK_BOOL(BasePower, bIgnoreStrength, 0), BoolEnum },
	{ "IgnoreStr",          TOK_REDUNDANTNAME|TOK_BOOL(BasePower, bIgnoreStrength, 0), BoolEnum },
	{ "ShowBuffIcon",       TOK_BOOL(BasePower, bShowBuffIcon, -1),		BoolEnum },
	{ "ShowInInventory",    TOK_INT(BasePower, eShowInInventory, kShowPowerSetting_Default),	ShowPowerSettingEnum },
	{ "ShowInManage",       TOK_BOOL(BasePower, bShowInManage, 1),		BoolEnum },
	{ "ShowInInfo",			TOK_BOOL(BasePower, bShowInInfo, 1),		BoolEnum },
	{ "Deletable",			TOK_BOOL(BasePower, bDeletable, 0),			BoolEnum },
	{ "Tradeable",			TOK_BOOL(BasePower, bTradeable,	0),			BoolEnum },
	{ "MaxBoosts",          TOK_INT(BasePower, iMaxBoosts, 6) },
	{ "DoNotSave",          TOK_BOOL(BasePower, bDoNotSave, 0),			BoolEnum },
	{ "DoesNotExpire",      TOK_REDUNDANTNAME|TOK_BOOL(BasePower, bBoostIgnoreEffectiveness, 0), BoolEnum },
	{ "BoostIgnoreEffectiveness", TOK_BOOL(BasePower, bBoostIgnoreEffectiveness, 0), BoolEnum },
	{ "BoostAlwaysCountForSet", TOK_BOOL(BasePower, bBoostAlwaysCountForSet, 0), BoolEnum },
	{ "BoostTradeable",		TOK_BOOL(BasePower, bBoostTradeable, 1), BoolEnum },
	{ "BoostCombinable",    TOK_BOOL(BasePower, bBoostCombinable, 1),	BoolEnum },
	{ "BoostAccountBound",	TOK_BOOL(BasePower, bBoostAccountBound, 0),	BoolEnum },
	{ "BoostBoostable",		TOK_BOOL(BasePower, bBoostBoostable, 0),	BoolEnum },
	{ "BoostUsePlayerLevel",TOK_BOOL(BasePower, bBoostUsePlayerLevel, 0),	BoolEnum },
	{ "BoostCatalystConversion",		TOK_STRING(BasePower, pchBoostCatalystConversion, 0),		},
	{ "StoreProduct",		TOK_STRING(BasePower, pchStoreProduct, 0),		},
	{ "BoostLicenseLevel",  TOK_INT(BasePower, iBoostInventionLicenseRequiredLevel, 999) },
	{ "MinSlotLevel",       TOK_INT(BasePower, iMinSlotLevel, -3) },
	{ "MaxSlotLevel",       TOK_INT(BasePower, iMaxSlotLevel, MAX_PLAYER_SECURITY_LEVEL-1) },
	{ "MaxBoostLevel",      TOK_INT(BasePower, iMaxBoostLevel, MAX_PLAYER_SECURITY_LEVEL) },		// 1 based for designer sanity
	{ "Var",                TOK_STRUCT(BasePower, ppVars, ParsePowerVar) },
	{ "ToggleDroppable",    TOK_INT(BasePower, eToggleDroppable, kToggleDroppable_Sometimes), ToggleDroppableEnum },
	{ "TogglesDroppable",   TOK_REDUNDANTNAME|TOK_INT(BasePower, eToggleDroppable, kToggleDroppable_Sometimes), ToggleDroppableEnum },
	{ "ProcAllowed",		TOK_INT(BasePower, eProcAllowed, kProcAllowed_All), ProcAllowedEnum },
	{ "StrengthsDisallowed",TOK_INTARRAY(BasePower, pStrengthsDisallowed), ParsePowerDefines },
	{ "ProcMainTargetOnly", TOK_BOOL(BasePower, bUseNonBoostTemplatesOnMainTarget, 0),	BoolEnum },
	{ "AnimMainTargetOnly", TOK_BOOL(BasePower, bMainTargetOnly, 0), BoolEnum }, 
	{ "HighlightEval",		TOK_STRINGARRAY(BasePower, ppchHighlightEval),	},
	{ "HighlightIcon",		TOK_STRING(BasePower, pchHighlightIcon, 0),		},
	{ "HighlightRing",		TOK_RGBA(BasePower, rgbaHighlightRing),			},
	{ "TravelSuppression",  TOK_F32(BasePower, fTravelSuppression, 0),		},
	{ "PreferenceMultiplier", TOK_F32(BasePower, fPreferenceMultiplier, 1),	},
	{ "DontSetStance",		TOK_BOOL(BasePower, bDontSetStance, 0),	},
	{ "PointValue",			 TOK_F32(BasePower, fPointVal, 0),		},
	{ "PointMultiplier",	 TOK_F32(BasePower, fPointMultiplier, 0),		},
	{ "ChainIntoPower",		TOK_STRING(BasePower, pchChainIntoPowerName, NULL)},
	{ "InstanceLocked",		TOK_BOOL(BasePower, bInstanceLocked, 0), BoolEnum},
	{ "IsEnvironmentHit",	TOK_BOOL(BasePower, bIsEnvironmentHit, 0), BoolEnum},
	{ "ShuffleTargets",		TOK_BOOL(BasePower, bShuffleTargetList, 0), BoolEnum},
	{ "ForceLevelBought",	TOK_INT(BasePower, iForceLevelBought, -1)},
	{ "RefreshesOnActivePlayerChange",	TOK_BOOL(BasePower, bRefreshesOnActivePlayerChange, 0), BoolEnum},
	{ "Cancelable",			TOK_BOOL(BasePower, bCancelable, 0), BoolEnum},
	{ "IgnoreToggleMaxDistance",	TOK_BOOL(BasePower, bIgnoreToggleMaxDistance, 0), BoolEnum},
	{ "ServerTrayPriority",	TOK_INT(BasePower, iServerTrayPriority, 0) },
	{ "ServerTrayRequires", TOK_STRINGARRAY(BasePower, ppchServerTrayRequires), },
	{ "AbusiveBuff",		TOK_BOOL(BasePower, bAbusiveBuff, true), BoolEnum },
	{ "PositionCenter",		TOK_INT(BasePower, ePositionCenter, kModTarget_Caster), ModTargetEnum },
	{ "PositionDistance",	TOK_F32(BasePower, fPositionDistance, 0.0f) },
	{ "PositionHeight",		TOK_F32(BasePower, fPositionHeight, 0.0f) },
	{ "PositionYaw",		TOK_F32(BasePower, fPositionYaw, 0.0f) },
	{ "FaceTarget",			TOK_BOOL(BasePower, bFaceTarget, true), BoolEnum},
	{ "AttribCache",		TOK_INTARRAY(BasePower, piAttribCache), ParsePowerDefines },
	
	// What .pfx file was included for this power. The rest of the data comes from this pfx file, via an Include
	{ "VisualFX",			TOK_FILENAME(BasePower, fx.pchSourceFile, 0)		},
	ParsePowerFX(BasePower, fx) // included flat
	{ "CustomFX",			TOK_STRUCT(BasePower, customfx, ParsePowerCustomFX)	},
	{ "PowerRedirector",	TOK_IGNORE },	// legacy, removed
	{ "",					TOK_NO_BIN|TOK_AUTOINTEARRAY(BasePower, ppTemplateIdx ) },
	{ "}", TOK_END, 0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseBasePowerSet[] =
{
	{ "{",					TOK_START,       0 },
	{ "SourceFile",			TOK_CURRENTFILE(BasePowerSet, pchSourceFile)       },
	{ "FullName",			TOK_STRUCTPARAM|TOK_STRING(BasePowerSet, pchFullName, 0)},
	{ "Name",				TOK_STRING(BasePowerSet, pchName, 0)             },
	{ "System",				TOK_INT(BasePowerSet, eSystem, kPowerSystem_Powers), PowerSystemEnum },
	{ "Shared",				TOK_BOOL(BasePowerSet, bIsShared, 0), BoolEnum },
	{ "DisplayName",		TOK_STRING(BasePowerSet, pchDisplayName, 0)      },
	{ "DisplayHelp",		TOK_STRING(BasePowerSet, pchDisplayHelp, 0)      },
	{ "DisplayShortHelp",	TOK_STRING(BasePowerSet, pchDisplayShortHelp, 0) },
	{ "IconName",			TOK_STRING(BasePowerSet, pchIconName, 0)         },
	{ "CostumeKeys",		TOK_STRINGARRAY(BasePowerSet, ppchCostumeKeys)   },
	{ "CostumeParts",		TOK_STRINGARRAY(BasePowerSet, ppchCostumeParts)   },
	{ "SetAccountRequires",	TOK_STRING(BasePowerSet, pchAccountRequires, 0)   },
	{ "SetAccountTooltip",	TOK_STRING(BasePowerSet, pchAccountTooltip, 0)      },
	{ "SetAccountProduct",	TOK_STRING(BasePowerSet, pchAccountProduct, 0)      },
	{ "SetBuyRequires",		TOK_STRINGARRAY(BasePowerSet, ppchSetBuyRequires) },
	{ "SetBuyRequiresFailedText",	TOK_STRING(BasePowerSet, pchSetBuyRequiresFailedText, 0)	},
	{ "ShowInInventory",	TOK_INT(BasePowerSet, eShowInInventory, kShowPowerSetting_Default), ShowPowerSettingEnum },
	{ "ShowInManage",		TOK_BOOL(BasePowerSet, bShowInManage, 1), BoolEnum },
	{ "ShowInInfo",			TOK_BOOL(BasePowerSet, bShowInInfo, 1), BoolEnum },
	{ "SpecializeAt",		TOK_INT(BasePowerSet, iSpecializeAt, 0) },
	{ "SpecializeRequires",	TOK_STRINGARRAY(BasePowerSet, ppSpecializeRequires) },
	{ "Powers",				TOK_STRINGARRAY(BasePowerSet, ppPowerNames ) },
	{ "Available",			TOK_INTARRAY(BasePowerSet, piAvailable)         },
	{ "AIMaxLevel",			TOK_INTARRAY(BasePowerSet, piAIMaxLevel)         },
	{ "AIMinRankCon",		TOK_INTARRAY(BasePowerSet, piAIMinRankCon)         },
	{ "AIMaxRankCon",		TOK_INTARRAY(BasePowerSet, piAIMaxRankCon)         },
	{ "MinDifficulty",		TOK_INTARRAY(BasePowerSet, piMinDifficulty)		   },
	{ "MaxDifficulty",		TOK_INTARRAY(BasePowerSet, piMaxDifficulty)		   },
	{ "ForceLevelBought",	TOK_INT(BasePowerSet, iForceLevelBought, -1)},
	{ "",					TOK_NO_BIN|TOK_AUTOINTEARRAY(BasePowerSet, ppPowers ) },
	{ "}",					TOK_END,         0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParsePowerCategory[] =
{
	{ "{",                TOK_START,       0 },
	{ "SourceFile",       TOK_CURRENTFILE(PowerCategory, pchSourceFile)  },
	{ "Name",             TOK_STRUCTPARAM | TOK_STRING(PowerCategory, pchName, 0)        },
	{ "DisplayName",      TOK_STRING(PowerCategory, pchDisplayName, 0) },
	{ "DisplayHelp",      TOK_STRING(PowerCategory, pchDisplayHelp, 0)      },
	{ "DisplayShortHelp", TOK_STRING(PowerCategory, pchDisplayShortHelp, 0) },
	{ "PowerSets",        TOK_STRINGARRAY(PowerCategory, ppPowerSetNames ) },
	{ "",			      TOK_NO_BIN|TOK_AUTOINTEARRAY(PowerCategory, ppPowerSets ) },
	{ "Available",        TOK_IGNORE,      0 },
	{ "}",                TOK_END,         0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParsePowerCatDictionary[] =
{
	{ "PowerCategory",   TOK_STRUCT(PowerDictionary, ppPowerCategories, ParsePowerCategory)},
	{ "", 0, 0 }
};

TokenizerParseInfo ParsePowerSetDictionary[] =
{
	{ "PowerSet",        TOK_STRUCT(PowerDictionary, ppPowerSets, ParseBasePowerSet)},
	{ "", 0, 0 }
};

TokenizerParseInfo ParsePowerDictionary[] =
{
	{ "Power",           TOK_STRUCT(PowerDictionary, ppPowers, ParseBasePower)},
	{ "", 0, 0 }
};

TokenizerParseInfo ParseEntirePowerDictionary[] =
{
	{ "PowerCategory",   TOK_STRUCT(PowerDictionary, ppPowerCategories, ParsePowerCategory)},
	{ "PowerSet",        TOK_STRUCT(PowerDictionary, ppPowerSets, ParseBasePowerSet)},
	{ "Power",           TOK_STRUCT(PowerDictionary, ppPowers, ParseBasePower)},
	{ "", 0, 0 }
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

TokenizerParseInfo ParseDimReturn[] =
{
	{ "Start",    TOK_STRUCTPARAM|TOK_F32(DimReturn, fStart, 0)     },
	{ "Handicap", TOK_STRUCTPARAM|TOK_F32(DimReturn, fHandicap, 0)  },
	{ "Basis",    TOK_STRUCTPARAM|TOK_F32(DimReturn, fBasis, 0)     },
	{ "\n",       TOK_END,    0 },
	{ 0 }
};

TokenizerParseInfo ParseAttribDimReturnSet[] =
{
	{ "{",        TOK_START,      0 },
	{ "Default",  TOK_INT(AttribDimReturnSet, bDefault, 0)  },
	{ "Attrib",   TOK_AUTOINTEARRAY(AttribDimReturnSet, offAttribs), ParsePowerDefines  },
	{ "Return",   TOK_STRUCT(AttribDimReturnSet, pReturns, ParseDimReturn) },
	{ "}",        TOK_END,        0 },
	{ 0 }
};


TokenizerParseInfo ParseDimReturnSet[] =
{
	{ "{",               TOK_START,      0 },
	{ "Default",         TOK_INT(DimReturnSet, bDefault, 0)  },
	{ "Boost",           TOK_INTARRAY(DimReturnSet, piBoosts), ParsePowerDefines  },
	{ "AttribReturnSet", TOK_STRUCT(DimReturnSet, ppReturns, ParseAttribDimReturnSet) },
	{ "}",               TOK_END,        0 },
	{ 0 }
};

TokenizerParseInfo ParseDimReturnList[] =
{
	{ "ReturnSet",		TOK_STRUCT(DimReturnList, ppSets, ParseDimReturnSet) },
	{ "", 0, 0 }
};

ParseTable ParsePowerSpec[] =
{
	{ "{",               TOK_START,      0 },
	ParsePowerSpecInternal(PowerSpec, )			// Ugly but intentional
	{ "}",               TOK_END,        0 },
	{ "", 0, 0 }
};

// The next few functions were copied over from load_def.c

extern StaticDefineInt DurationEnum[];
extern StaticDefineInt ModTypeEnum[];
extern StaticDefineInt CasterStackTypeEnum[];
extern StaticDefineInt StackTypeEnum[];
extern StaticDefineInt ModBoolEnum[];
extern StaticDefineInt PowerEventEnum[];
extern TokenizerParseInfo ParseSuppressPair[];

/**********************************************************************func*
* load_PreprocPowerDictionary
*
*/
bool load_PreprocPowerDictionary(TokenizerParseInfo pti[], PowerDictionary *pdict)
{
	int i;
	int iSizePows = eaSize(&pdict->ppPowers);
	int *crcFullNameList;

	powerdict_CreateDictHashes(pdict, false);

	combateval_StoreToHitInfo(0.0f, 0.0f, false, false);
	combateval_StoreAttribCalcInfo(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	combateval_StoreCustomFXToken(NULL);
	combateval_StoreTargetsHit(0);
	combateval_StoreChance(0.0f, 0.0f);

	for (i = 0; i < eaSize(&pdict->ppPowerSets); i++)
	{
		// must be called after powerdicct_CreateDictHashes()!
		powerset_FixupForBin(pdict->ppPowerSets[i]);
	}

	eaiCreate(&crcFullNameList);
	for (i = 0; i < iSizePows; i++)
	{
		power_FixupForBin((BasePower*)pdict->ppPowers[i], &crcFullNameList);
	}
	eaiDestroy(&crcFullNameList);

	powerdict_DuplicateMissingPowers(pdict);
	powerdict_FixPowerSetErrors(pdict);

	return true;
}

static s_ExtraLoadFlags = 0;

/**********************************************************************func*
* load_PreprocPowerSetDictionary
*
*/
bool load_PreprocPowerSetDictionary(TokenizerParseInfo pti[], PowerDictionary *pdict)
{
#ifndef TEST_CLIENT	
	// We only care about this when we actually parse power sets
	if (!msGetHideTranslationErrors())
	{
		int iSizeCat = eaSize(&pdict->ppPowerCategories);
		int iSizeSets = eaSize(&pdict->ppPowerSets);
		int icat, iset;
		for(icat=0; icat<iSizeCat; icat++)
		{
			const PowerCategory *pcat = pdict->ppPowerCategories[icat];
			CHECK_TRANSLATION_CAT(pcat->pchDisplayHelp);
			CHECK_TRANSLATION_CAT(pcat->pchDisplayName);
			CHECK_TRANSLATION_CAT(pcat->pchDisplayShortHelp);
		}
		
		for(iset=0; iset<iSizeSets; iset++)
		{
			const BasePowerSet *pset = pdict->ppPowerSets[iset];

			if(pset->pchAccountRequires && !pset->pchAccountProduct)
				ErrorFilenamef(pset->pchSourceFile, "Missing SetAccountProduct for powerset %s.\nThis should be a valid product code.", pset->pchFullName);
			
			CHECK_TRANSLATION_SET(pset->pchDisplayHelp);
			CHECK_TRANSLATION_SET(pset->pchDisplayName);
			CHECK_TRANSLATION_SET(pset->pchDisplayShortHelp);
		}
	}

#endif

	// If we rebinned the powersets, we need to rebin powers too, to pick up new duplicates
	s_ExtraLoadFlags = PARSER_FORCEREBUILD;

	return 1;
}


/**********************************************************************func*
* powerDefReload
*
* Reload the powers file pointed to.
*
*/
static void powerDefReload(const char* relpath, int when)
{
	ParserLoadInfo pli;
	PowerDictionary *pdict = cpp_const_cast(PowerDictionary*)(&g_PowerDictionary);
	fileWaitForExclusiveAccess(relpath);
	errorLogFileIsBeingReloaded(relpath);
	if (ParserReloadFile(relpath, PARSER_KEEPLOADONLY, ParsePowerDictionary, sizeof(PowerDictionary), pdict, &pli, NULL, NULL, NULL))
	{
		int i;
		int *crcFullNameList;

		s_PowersLoadOnly = pli.lostore;

		combateval_StoreToHitInfo(0.0f, 0.0f, false, false);
		combateval_StoreAttribCalcInfo(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		combateval_StoreCustomFXToken(NULL);
		combateval_StoreTargetsHit(0);
		combateval_StoreChance(0.0f, 0.0f);

		for (i = 0; i < eaSize(&pdict->ppPowerSets); i++)
		{
			powerset_FixupForBin(pdict->ppPowerSets[i]);
		}

		eaiCreate(&crcFullNameList);
		for (i = 0; i < eaSize(&pdict->ppPowers); i++)
		{
			BasePower *ppow = (BasePower*)pdict->ppPowers[i];			
			if (ppow->pchSourceName && ppow->pchSourceName[0])
			{
				if (strcmp(ppow->pchSourceName,ppow->pchFullName) != 0 &&
					stricmp(ppow->pchSourceFile,relpath) == 0)
				{
					// This is a duplicated power from the reloaded file. We need to redup it
					eaRemoveConst(&pdict->ppPowers,i);
					i--;
				}

				//This doesn't need to be reprocessed
				continue;
			}
			power_FixupForBin(ppow, &crcFullNameList);
		}
		eaiDestroy(&crcFullNameList);

		powerdict_DestroyDictHashes(pdict);
		powerdict_CreateDictHashes(pdict, false);
		powerdict_DuplicateMissingPowers(pdict);
		powerdict_FixPowerSetErrors(pdict);

		powerdict_Finalize(pdict);

		powerdict_SetForwardPointers(pdict);
		powerdict_SetBackPointers(pdict);

		for (i = 0; i < eaSize(&pdict->ppPowers); i++)
		{
			BasePower *ppow = (BasePower*)pdict->ppPowers[i];			
			if (ppow->pchSourceFile && stricmp(ppow->pchSourceFile,relpath) == 0)
			{
				int ieff;

				power_Finalize(ppow,pdict);
				for (ieff = eaSize(&ppow->ppEffects) - 1; ieff >= 0; ieff--)
				{
					EffectGroup *peffect = (EffectGroup*)ppow->ppEffects[ieff];
					effectgroup_FinalizeShared(ppow, peffect);
				}
			}
		}
		
		boostset_DestroyDictHashes((BoostSetDictionary*)&g_BoostSetDict);
		boostset_Finalize((BoostSetDictionary*)&g_BoostSetDict, false, NULL, true);

		ParserFreeLoadOnlyStore(s_PowersLoadOnly);
		s_PowersLoadOnly = 0;
	}
	else
	{
		Errorf("Error reloading villain: %s\n", relpath);
	}
}


/**********************************************************************func*
* powerAnimReload
*
* Given a pfx file name, reload all powers that include that pfx filename
*
*/
static void powerAnimReload(const char* relpath, int when)
{
	// An animation file changed, we need to figure out which power files use that animation
	PowerDictionary *pdict = (PowerDictionary*)&g_PowerDictionary;
	int i;
	int size = eaSize(&pdict->ppPowers);
	char **reloadFiles;
	eaCreate(&reloadFiles);
	fileWaitForExclusiveAccess(relpath);
	errorLogFileIsBeingReloaded(relpath);

	for (i = 0; i < size; i++)
	{
		BasePower *ppow = (BasePower*)pdict->ppPowers[i];
		int reload = ppow->fx.pchSourceFile && stricmp(ppow->fx.pchSourceFile,relpath) == 0; //normalize the path somehow?
		if(!reload)
		{
			int j;
			for(j = eaSize(&ppow->customfx)-1; j >= 0 && !reload; j--)
				reload = ppow->customfx[j]->fx.pchSourceFile && stricmp(ppow->customfx[j]->fx.pchSourceFile,relpath) == 0;
		}
		if(reload)
		{
			int j,found = 0;
			for (j = 0; j < eaSize(&reloadFiles); j++)
			{
				if (stricmp(ppow->pchSourceFile,reloadFiles[j]) == 0)
					found = 1;
			}
			if (found)
				continue; //already in the array
			eaPush(&reloadFiles,strdup(ppow->pchSourceFile));
		}
	}
	size = eaSize(&reloadFiles);
	for (i = 0; i < size; i++)
	{
		powerDefReload(reloadFiles[i],when);
	}
	eaDestroyEx(&reloadFiles,NULL);

}


static void load_PowerDictionary_MoveToShared(PowerDictionary* ppow, SharedMemoryHandle *phandle, int flags)
{
	int i;

	if (eaSize(&(ppow->ppPowerCategories[0])->ppPowerSets) == 0)
	{
		// If we loaded from bins, we need to create the hashes now
		powerdict_CreateDictHashes(ppow, false);
	}

	powerdict_SetForwardPointers(ppow);
	powerdict_Finalize(ppow);

	for (i = 0; i < eaSize(&ppow->ppPowers); i++)
	{
		BasePower *p = (BasePower*)ppow->ppPowers[i];			
		effectgroup_Validate(p, p->ppEffects);
		power_Finalize(p,ppow);
	}

	powerdict_DestroyDictHashes(ppow);
			
	if (phandle)
	{
		ParserMoveToShared(phandle, flags, ParseEntirePowerDictionary, ppow, sizeof(*ppow), NULL);
		// switch to a manual lock
		sharedMemoryCommit(phandle);
		sharedHeapMemoryManagerLock();
	}

	powerdict_CreateDictHashes(ppow, phandle!=0);
	powerdict_SetForwardPointers(ppow);
	powerdict_SetBackPointers(ppow);

	if (phandle)
	{
		ParserCopyToShared(phandle, ppow, sizeof(*ppow));
		sharedHeapMemoryManagerUnlock();
		sharedMemoryUnlock(phandle);
	}
}

/**********************************************************************func*
* load_PowerDictionary
*
*/
int artTestPowerLoaded = 0;
BasePower artTestPower;

void load_PowerDictionary(SHARED_MEMORY_PARAM PowerDictionary *ppow, char *pchFilename, bool bNewAttribs, void (*callback)(SharedMemoryHandle *phandle, bool bNewAttribs))
{
#if SERVER 
	int flags = PARSER_SERVERONLY | (bNewAttribs?PARSER_FORCEREBUILD:0);
#else
	int flags = (bNewAttribs?PARSER_FORCEREBUILD:0);
#endif

	//Code to allow artists their own pfx overrides for testing
	artTestPowerLoaded = 0;
	ZeroStruct( &artTestPower );
	if( isDevelopmentMode() && fileExists(fileMakePath("test.pfx", "debug", "debug", false)) )
	{
		if( ParserLoadFiles( NULL, fileMakePath("test.pfx", "debug", "debug", false), NULL, 0, ParseBasePower, &artTestPower, NULL, NULL, NULL, NULL ) )
			artTestPowerLoaded = 1;
	}
	//End pfx overrides

	if (isDevelopmentMode() && artTestPowerLoaded)
	{
		// Hack for art test powers
		flags |= PARSER_DONTFREE;
	}

	if (bNewAttribs || !ParserLoadFromShared(MakeSharedMemoryName("SM_POWERS"),flags,ParseEntirePowerDictionary,ppow,sizeof(*ppow),NULL,&s_pPowerdictHandle))
	{	
		ParserLoadInfo pli;
		s_ExtraLoadFlags = 0;
		// We didn't succesfully load all from shared memory, so parse them
		ParserLoadFiles(pchFilename,".categories","powercats.bin",flags,
			ParsePowerCatDictionary,(void*)ppow,NULL,NULL,NULL,NULL);
		ParserLoadFiles(pchFilename,".powersets","powersets.bin",flags,
			ParsePowerSetDictionary,(void*)ppow,NULL,NULL,load_PreprocPowerSetDictionary,NULL);
		ParserLoadFiles(pchFilename,".powers","powers.bin",flags|s_ExtraLoadFlags|PARSER_KEEPLOADONLY,
			ParsePowerDictionary,(void*)ppow,NULL,&pli,load_PreprocPowerDictionary,NULL);

		s_PowersLoadOnly = pli.lostore;
		load_PowerDictionary_MoveToShared((PowerDictionary*)ppow, s_pPowerdictHandle, flags);
		s_bNeedSharedFinalization = true;
	}

	powerdict_FindSharedPowersets(ppow);

	powerdict_FinalizeUnshared(ppow);

	TODO(); // Make sure  powerdict_CreateHashes() and powerdict_CreateDictHashes() are equivalent.
	// Old hash tables were created on the individual categories and sets, now the hash tables are global,
	// e.g., one global hash table for all power names instead. powerdict_CreateDictHashes() will throw
	// an error if it comes across duplicate names for the global tables. Current data is not throwing any
	// errors so as long as that remains the case (we should let design know) this should be fine.
	//powerdict_CreateHashes(ppow);

	power_LoadReplacementHash();

	// Handle callbacks that need to modify the power dictionary's shared memory
	if (callback)
		callback(s_pPowerdictHandle, bNewAttribs);

	power_verifyValidMMPowers();
	if (isDevelopmentMode())
	{
		char fullpath[512];
		sprintf(fullpath,"%s*.powers",pchFilename);
		FolderCacheSetCallback(FOLDER_CACHE_CALLBACK_UPDATE, fullpath, powerDefReload);
		FolderCacheSetCallback(FOLDER_CACHE_CALLBACK_UPDATE, "menu/Powers/AnimFX/*.pfx", powerAnimReload);
	}
}

/**********************************************************************func*
* ReloadPowers
*
*/
void ReloadPowers(PowerDictionary *pdictDest, const PowerDictionary *pdictSrc)
{
	int icat;
	int iSizeCats = eaSize(&pdictSrc->ppPowerCategories);

	for(icat=0; icat<iSizeCats; icat++)
	{
		int iset;
		const PowerCategory *pcatNew = pdictSrc->ppPowerCategories[icat];
		PowerCategory *pcatOld = (PowerCategory *)powerdict_GetCategoryByName(pdictDest, pcatNew->pchName);
		int iSizeSets = eaSize(&pcatNew->ppPowerSets);

		if(pcatOld==NULL)
		{
			Errorf("Can't find old category named %s.", pcatNew->pchName);
			continue;
		}

		//free(pcatOld->pchName);
		//free(pcatOld->pchDisplayName);
		//free(pcatOld->pchDisplayHelp);
		//free(pcatOld->pchDisplayShortHelp);

		pcatOld->pchName             = pcatNew->pchName;
		pcatOld->pchDisplayName      = pcatNew->pchDisplayName;
		pcatOld->pchDisplayHelp      = pcatNew->pchDisplayHelp;
		pcatOld->pchDisplayShortHelp = pcatNew->pchDisplayShortHelp;

		for(iset=0; iset<iSizeSets; iset++)
		{
			int ipow;
			const BasePowerSet *psetNew = pcatNew->ppPowerSets[iset];
			BasePowerSet *psetOld = (BasePowerSet*)powerdict_GetBasePowerSetByName(pdictDest, pcatOld->pchName, psetNew->pchName);
			int iSizePows = eaSize(&psetNew->ppPowers);

			if(psetOld==NULL)
			{
				Errorf("Can't find old power set named %s.", psetNew->pchName);
				continue;
			}

			//free(psetOld->pchName);
			//free(psetOld->pchDisplayName);
			//free(psetOld->pchDisplayHelp);
			//free(psetOld->pchDisplayShortHelp);
			//free(psetOld->pchIconName);
			//eaiDestroy(&psetOld->piAvailable);

			psetOld->pchName             = psetNew->pchName;
			psetOld->pchDisplayName      = psetNew->pchDisplayName;
			psetOld->pchDisplayHelp      = psetNew->pchDisplayHelp;
			psetOld->pchDisplayShortHelp = psetNew->pchDisplayShortHelp;
			psetOld->pchIconName         = psetNew->pchIconName;
			psetOld->piAvailable         = psetNew->piAvailable;

			for(ipow=0; ipow<iSizePows; ipow++)
			{
				int ieff;
				const BasePower *ppowNew = psetNew->ppPowers[ipow];
				BasePower *ppowOld = (BasePower*)baseset_GetBasePowerByName(psetOld, ppowNew->pchName);
				int iSizeEffects = eaSize(&ppowNew->ppEffects);

				if(ppowOld==NULL)
				{
					Errorf("Can't find old power named %s.", ppowNew->pchName);
					continue;
				}

				// Not updated
				// BasePowerSet *psetParent


				// Doing this leaks the old FX. But, since I'm leaking
				// everything else right now anyway, who cares?
				ppowOld->fx              = ppowNew->fx;
				//eaDestroy(&ppowOld->customfx);
				ppowOld->customfx        = ppowNew->customfx;

				ppowOld->eType           = ppowNew->eType;
				ppowOld->fAccuracy       = ppowNew->fAccuracy;
				ppowOld->bNearGround     = ppowNew->bNearGround;
				ppowOld->eEffectArea     = ppowNew->eEffectArea;
				ppowOld->fRadius         = ppowNew->fRadius;
				ppowOld->fArc            = ppowNew->fArc;
				ppowOld->fRange          = ppowNew->fRange;
				ppowOld->fTimeToActivate = ppowNew->fTimeToActivate;
				ppowOld->fRechargeTime   = ppowNew->fRechargeTime;
				ppowOld->fInterruptTime  = ppowNew->fInterruptTime;
				ppowOld->fActivatePeriod = ppowNew->fActivatePeriod;
				ppowOld->fEnduranceCost  = ppowNew->fEnduranceCost;

				//free(ppowOld->pchName);
				//free(ppowOld->pchDisplayName);
				//free(ppowOld->pchDisplayHelp);
				//free(ppowOld->pchDisplayShortHelp);
				//free(ppowOld->pchDisplayAttackerHit);
				//free(ppowOld->pchDisplayVictimHit);
				//free(ppowOld->pchIconName);
				//eaiDestroy(&(int *)ppowOld->pAffected);
				//eaiDestroy(&(int *)ppowOld->pAutoHit);
				//eaiDestroy(&(int *)ppowOld->pBoostsAllowed);
				//eaiDestroy(&(int *)ppowOld->pStrengthsDisallowed);

				ppowOld->pchName               = ppowNew->pchName;
				ppowOld->pchDisplayName        = ppowNew->pchDisplayName;
				ppowOld->pchDisplayHelp        = ppowNew->pchDisplayHelp;
				ppowOld->pchDisplayShortHelp   = ppowNew->pchDisplayShortHelp;
				ppowOld->pchDisplayAttackerHit = ppowNew->pchDisplayAttackerHit;
				ppowOld->pchDisplayVictimHit   = ppowNew->pchDisplayVictimHit;
				ppowOld->pchIconName           = ppowNew->pchIconName;
				ppowOld->pAffected             = ppowNew->pAffected;
				ppowOld->pAutoHit              = ppowNew->pAutoHit;
				ppowOld->pBoostsAllowed        = ppowNew->pBoostsAllowed;
				ppowOld->pStrengthsDisallowed  = ppowNew->pStrengthsDisallowed;

				eaDestroyConst(&ppowOld->ppBoostSets);

				if(eaSize(&ppowNew->ppEffects)!=eaSize(&ppowOld->ppEffects))
				{
					Errorf("Unmatched number of effect groups for power %s.", ppowNew->pchName);
					continue;
				}

				for(ieff=0; ieff<iSizeEffects; ieff++)
				{
					int imod;
					const EffectGroup *peffNew = ppowNew->ppEffects[ieff];
					EffectGroup *peffOld = cpp_const_cast(EffectGroup*)(ppowOld->ppEffects[ieff]);
					const BasePower *ppowBaseOld = peffOld->ppowBase;
					int iSizeTemps = eaSize(&peffNew->ppTemplates);

					if(eaSize(&peffNew->ppTemplates)!=eaSize(&peffOld->ppTemplates))
					{
						Errorf("Unmatched number of attrib mods for power %s.", ppowNew->pchName);
						continue;
					}

					peffOld->evalFlags = peffNew->evalFlags;
					peffOld->fChance = peffNew->fChance;
					peffOld->fDelay = peffNew->fDelay;
					peffOld->fProcsPerMinute = peffNew->fProcsPerMinute;
					peffOld->iFlags = peffNew->iFlags;
					peffOld->ppchRequires = peffNew->ppchRequires;

					for(imod=0; imod<iSizeTemps; imod++)
					{
						const AttribModTemplate *pmodNew = peffNew->ppTemplates[imod];
						AttribModTemplate *pmodOld = cpp_const_cast(AttribModTemplate*)(peffOld->ppTemplates[imod]);

						//free(pmodOld->pchName);
						//free(pmodOld->pchDisplayAttackerHit);
						//free(pmodOld->pchDisplayVictimHit);
						//free(pmodOld->pchTable);

						*pmodOld = *pmodNew;
					}

					peffOld->ppowBase = ppowBaseOld;
				}
			}
		}
	}
}

/* End of File */
