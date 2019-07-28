/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_ANIMFX_H__
#define CHARACTER_ANIMFX_H__

#include "Color.h"
#include "character_animfx_common.h"

typedef struct Character	Character;
typedef struct Power		Power;

bool character_StanceIsSame(Power *pA, Power *pB);
	// Returns true if pA and pB have the same stance (same weapon FX).

void character_EnterStance(Character *p, Power *ppow, bool bReplaceStance);
	// Makes the character enter the stance for the given power. This
	//   stance stays current until another stance is given.
	// If NULL is given as the power, the stance is shut off.
	// If bReplaceStance is true, then the given stance is set as the
	//   current one. If false, plays the stance anim and fx, but retains
	//   the previous stance.

void entity_FaceTarget(Entity *entToTurn, Entity *entTarget);
void character_FaceLocation(Character *p, Vec3 vecTarget);
	// Moves the character to face the given target.

void character_Teleport(Character *p, Power *ppow, Vec3 v);
	// Instantly moves the character to the given location.

void character_AddStickyStates(Character *p, const int *piList);
void character_AddStickyState(Character *p, int iState);
	// Adds a sticky anim state to the list of states currently on the
	// character.

void character_SetStickyStates(Character *p);
	// Actually sets the anim stickt anim states on the entity

void character_SetAnimBits(Character *p, const int *piList, float fDelay, int iAttached);
	// Sets the given animation states on the parent entity of the given
	//   character.
	// The state setting is postponed by the number of seconds given by
	//   fDelay.

unsigned int entity_PlayMaintainedFX(Entity *eSrc, Entity *ePrevTarget, Entity *eTarget, const char *pchName, ColorPair uiTint, float fDelay, unsigned int iAttached, float fTimeOut, int iFlags);

unsigned int character_PlayMaintainedFX(Character *pSrc, Character *pPrevTarget, Character *pTarget, const char *pchName, ColorPair uiTint, float fDelay, unsigned int iAttached, float fTimeOut, int iFlags);
unsigned int character_PlayFXLocation(Character *pSrc, Character *pPrevTarget, Character *pTarget, const Vec3 vecTarget, const char *pchName, ColorPair uiTint, float fDelay, unsigned int iAttached, int iFlags);
unsigned int character_PlayFX(Character *pSrc, Character *pPrevTarget, Character *pTarget, const char *pchName, ColorPair uiTint, float fDelay, unsigned int iAttached, int iFlags);
	// Plays the named effect for a character. The "owner" of the effect
	//   is pSrc, the "target" is pTarget.
	// The effect is postponed the number of seconds given by fDelay.
	// Returns the effect ID (net_id) of the created effect.

void entity_KillMaintainedFX(Entity *eSrc, Entity *eTarget, const char *pchName);
void character_KillMaintainedFX(Character *pSrc, Character *pTarget, const char *pchName);
	// Shuts off the named effect.

typedef struct cCostume cCostume;
void character_SetCostume(Character *p, const cCostume *costume, int index, int priority);
void character_SetCostumeByName(Character *p, const char *pchCostumeName, int priority);
	// Set a character costume to a particular NPC's costume.
void character_RestoreCostume(Character *p);
	// Set the costume back to the character's regular costume.


#endif /* #ifndef CHARACTER_ANIMFX_H__ */

/* End of File */

