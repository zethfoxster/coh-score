/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_TARGET_H__
#define CHARACTER_TARGET_H__

#ifndef POWERS_H__
#include "powers.h" // for TargetType
#endif

typedef struct Character Character;
typedef struct PNPCDef PNPCDef;

typedef enum CalcTeleportTargetResult_t
{
	TELEPORT_TARGET_OK = 0,
	TELEPORT_TARGET_FEET,
	TELEPORT_TARGET_HEAD
} CalcTeleportTargetResult;

void setGangTestFunction(int (*_GangTestFunction)(int, int));
bool character_TargetIsFriend(Character *pSrc, Character *pTarget);
bool character_TargetIsFoe(Character *pSrc, Character *pTarget);
bool character_TargetIsInCombatPhase(Character *pSrc, Character *pTarget);
bool entity_TargetIsInVisionPhase(Entity *pSrc, Entity *pTarget);
bool entity_PNPCIsInVisionPhase(Entity *pSrc, const PNPCDef *pNPC);
bool character_PowerAffectsTarget(Character *pSrc, Character *pTarget, const Power *ppow);
bool character_PowerAlwaysHits(Character *pSrc, Character *pTarget, const Power *ppow); // belongs in character_combat
bool character_TargetMatchesType(Character *pSrc, Character *pTarget, TargetType eType, bool targetsThroughVisionPhase);
bool character_TargetMatchesTypeEx(Character *pSrc, Character *pTarget, TargetType eType, bool targetsThroughVisionPhase, int *wouldHaveIfInPhase);

bool power_AffectsType(Power *ppow, TargetType type);

bool MatchesTypeList(Character *pSrc, Character *pTarget, const TargetType *pTypes, bool targetsThroughVisionPhase);

bool IsTargetOutOfBounds(Vec3 vTarget);
bool IsTeleportTargetValid(Vec3 vecTarget);
bool ExtraTeleportChecks(Vec3 vecTarget, Vec3 playerLoc, Vec3 camera);
CalcTeleportTargetResult CalcTeleportTarget(Mat4 matLocation, Vec3 vecTargetCalc, Vec3 probe);
void CalcLocationTarget(Mat4 matLocation, Vec3 vecTargetCalc);

#endif /* #ifndef CHARACTER_TARGET_H__ */

/* End of File */

