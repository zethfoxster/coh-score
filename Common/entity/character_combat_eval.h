/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_COMBAT_EVAL_H__
#define CHARACTER_COMBAT_EVAL_H__

typedef struct Entity Entity;

void combateval_Init(void);
void combateval_Validate(const char *pchInfo, const char **ppchExpr, const char *dataFilename);
void combateval_StoreCustomFXToken(const char *token);
void combateval_StoreTargetsHit(int targetsHit);
void combateval_StoreToHitInfo(float fRand, float fToHit, bool bAlwaysHit, bool bForceHit);
void combateval_StoreAttribCalcInfo(float fFinal, float fVal, float fScale, float fEffectiveness, float fStrength);
void combateval_StoreChance(float fChanceFinal, float fChanceMods);
void combateval_StoreChainInfo(int iChainJump, float fChainEff);
float combateval_Eval(Entity *eSrc, Entity *eTarget, const Power *pPow, const char **ppchExpr, const char *dataFilename);
float combateval_EvalFromBasePower(Entity *eSrc, Entity *eTarget, const BasePower *pPowBase, const char **ppchExpr, const char *dataFilename);

#endif /* #ifndef CHARACTER_COMBAT_EVAL_H__ */

/* End of File */

