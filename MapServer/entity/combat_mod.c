/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "earray.h"

#include "combat_mod.h"
#include "entity.h"
#include "character_base.h"
#include "character_pet.h"
#include "character_target.h"
#include "character_level.h"
#include "teamCommon.h"


SHARED_MEMORY CombatModsTable g_CombatModsPlayer;
SHARED_MEMORY CombatModsTable g_CombatModsVillain;


/**********************************************************************func*
 * combatmod_Ignored
 *
 */
bool combatmod_Ignored(Character *pSrc, Character *pTarget)
{
	if(pSrc==pTarget || pSrc->bIgnoreCombatMods || pTarget->bIgnoreCombatMods)
	{
		return true;
	}
	else
	{
		Entity *eSrcOwner = erGetEnt(pSrc->entParent->erOwner);
		Entity *eTargetOwner = erGetEnt(pTarget->entParent->erOwner);

		if(eSrcOwner==NULL)
		{
			eSrcOwner = pSrc->entParent;
		}
		if(eTargetOwner==NULL)
		{
			eTargetOwner = pTarget->entParent;
		}

		if((ENTTYPE(eSrcOwner)==ENTTYPE_PLAYER && ENTTYPE(eTargetOwner)==ENTTYPE_PLAYER)
			|| character_TargetMatchesType(pSrc, pTarget, kTargetType_DeadOrAliveTeammate, false)
			|| character_TargetMatchesType(pSrc, pTarget, kTargetType_DeadOrAliveFriend, false))
		{
			return true;
		}
	}

	return false;
}

/**********************************************************************func*
 * combatmod_Get
 *
 */
void combatmod_Get(Character *pSrc, int iSelfLevel, int iTargetLevel, const CombatMod **ppcm, int *piDelta)
{
	int i, n;
	int iTeamSize = 1;
	const CombatModsTable *ptable;

	// TODO: What about pets?
	if(ENTTYPE(pSrc->entParent)==ENTTYPE_PLAYER)
	{
		ptable = &g_CombatModsPlayer;
		if(pSrc->entParent->teamup)
		{
			iTeamSize = pSrc->entParent->teamup->members.count;
		}
	}
	else
	{
		Character *pOwner = character_GetOwner(pSrc);
		if(ENTTYPE(pOwner->entParent)==ENTTYPE_PLAYER)
		{
			ptable = &g_CombatModsPlayer;
			if(pOwner->entParent->teamup)
			{
				iTeamSize = pOwner->entParent->teamup->members.count;
			}
		}
		else
		{
			ptable = &g_CombatModsVillain;
		}
	}

	n = eaSize(&ptable->ppMods);
	for(i=0; i<n; i++)
	{
		if(iTeamSize >= ptable->ppMods[i]->iMinSize
			&& iTeamSize <= ptable->ppMods[i]->iMaxSize)
		{
			if(iSelfLevel>iTargetLevel)
			{
				*piDelta = iSelfLevel-iTargetLevel;
				*ppcm = &ptable->ppMods[i]->Lower;
				return;
			}
			else
			{
				*piDelta = iTargetLevel-iSelfLevel;
				*ppcm = &ptable->ppMods[i]->Higher;
				return;
			}
		}
	}

	*piDelta = iTargetLevel-iSelfLevel;
	*ppcm = &ptable->ppMods[0]->Higher;
}

/**********************************************************************func*
 * combatmod_ToHit
 *
 */
float combatmod_ToHit(CombatMod *pcm, int iDelta)
{
	if(iDelta>=0 && iDelta<eafSize(&pcm->pfToHit))
	{
		return pcm->pfToHit[iDelta];
	}

	return 1.0f;
}

/**********************************************************************func*
 * combatmod_Duration
 *
 */
float combatmod_Duration(CombatMod *pcm, int iDelta)
{
	if(iDelta>=0 && iDelta<eafSize(&pcm->pfDuration))
	{
		return pcm->pfDuration[iDelta];
	}

	return 1.0f;
}

/**********************************************************************func*
 * combatmod_Magnitude
 *
 */
float combatmod_Magnitude(CombatMod *pcm, int iDelta)
{
	if(iDelta>=0 && iDelta<eafSize(&pcm->pfMagnitude))
	{
		return pcm->pfMagnitude[iDelta];
	}

	return 1.0f;
}

/**********************************************************************func*
 * combatmod_Accuracy
 *
 */
float combatmod_Accuracy(CombatMod *pcm, int iDelta)
{
	if(iDelta>=0 && iDelta<eafSize(&pcm->pfAccuracy))
	{
		return pcm->pfAccuracy[iDelta];
	}

	return 1.0f;
}

/**********************************************************************func*
 * combatmod_PvPToHitMod
 *
 */
float combatmod_PvPToHitMod(Character *pSrc, Character *pTarget)
{
	// If the target and source are players or pets of players and they're
	// foes, modify 
	if((ENTTYPE(pSrc->entParent)==ENTTYPE_PLAYER || erGetDbId(pSrc->entParent->erOwner)!=0)
		&& (ENTTYPE(pTarget->entParent)==ENTTYPE_PLAYER || erGetDbId(pTarget->entParent->erOwner)!=0))
	{
		if(character_TargetMatchesType(pSrc, pTarget, kTargetType_DeadOrAliveFoe, false))
		{
			return g_CombatModsPlayer.fPvPToHitMod;
		}
	}

	return 0.0f;
}

/**********************************************************************func*
* combatmod_LevelToHitMod
*
*/
float combatmod_LevelToHitMod(Character *pSrc)
{
	if (!pSrc)
		return 0.0f;

	if(ENTTYPE(pSrc->entParent)==ENTTYPE_PLAYER)
	{
		if (g_CombatModsPlayer.pfToHitLevelMods)
			return g_CombatModsPlayer.pfToHitLevelMods[character_GetCombatLevelExtern(pSrc) - 1];
	} else {
		Character *pOwner = character_GetOwner(pSrc);
		if(ENTTYPE(pOwner->entParent)==ENTTYPE_PLAYER)
		{
			if (g_CombatModsPlayer.pfToHitLevelMods)
				return g_CombatModsPlayer.pfToHitLevelMods[character_GetCombatLevelExtern(pSrc) - 1];
		} else {
			if (g_CombatModsVillain.pfToHitLevelMods)
				return g_CombatModsVillain.pfToHitLevelMods[character_GetCombatLevelExtern(pSrc) - 1];
		}
	}

	return 0.0f;
}

/**********************************************************************func*
* combatmod_PvPElusivityMod
*
*/
float combatmod_PvPElusivityMod(Character *pSrc)
{
	return g_CombatModsPlayer.fPvPElusivityMod;
}

/* End of File */
