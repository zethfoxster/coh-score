
#ifndef ENTAICRITTERPRIVATE_H
#define ENTAICRITTERPRIVATE_H

#include "entaiprivate.h"

//----------------------------------------------------------------
// entaiCritterUtils.c stuff.
//----------------------------------------------------------------

void* aiCritterMsgHandlerDefault(Entity* e, AIMessage msg, AIMessageParams params);

//----------------------------------------------------------------
// entaiCritterActivityMisc.c stuff.
//----------------------------------------------------------------

void aiCritterActivityDoAnim(Entity* e, AIVars* ai);
void aiCritterActivityFollowRoute(Entity* e, AIVars* ai, PatrolRoute* route);
void aiCritterActivityFrozenInPlace(Entity* e, AIVars* ai);
// void aiCritterActivityHideBehindEnt(Entity* e, AIVars* ai);
void aiCritterActivityRunIntoDoor(Entity* e, AIVars* ai);
void aiCritterActivityRunOutOfDoor(Entity* e, AIVars* ai);
void aiCritterActivityStandStill(Entity* e, AIVars* ai);

//----------------------------------------------------------------
// entaiCritterActivityCombat.c stuff.
//----------------------------------------------------------------

void aiCritterActivityCombat(Entity* e, AIVars* ai);
void* aiCritterMsgHandlerCombat(Entity* e, AIMessage msg, AIMessageParams params);
void aiCritterInitPowerGroupFlags(AIVars *ai);

int aiCritterDoDefaultPacedPowers(Entity *e, AIVars* ai);
int aiCritterDoDefaultBuffs(Entity* e, AIVars* ai);
void aiCritterKillToggles(Entity* e, AIVars* ai);
int aiCritterCountMinions(Entity *e, AIVars *ai, int minionBrainType);
int aiCritterDoSummoning(Entity *e, AIVars *ai, int minionBrainType, int minMinionCount);
int aiCritterDoAutomaticSummoning( Entity *e, AIVars *ai );
void aiCritterDoDefault(Entity* e, AIVars* ai, int force_return);
int aiCritterDoToggleBuff(Entity *e, AIVars *ai, U32 *time, AIPowerInfo **curBuff, int groupFlags, U32 minTimeSinceAttacked, U32 maxTimeSinceAttacked);
int aiCritterDoBuff(Entity *e, AIVars *ai, U32 *time, AIPowerInfo **curBuff, int groupFlags, U32 minTimeSinceAttacked, U32 maxTimeSinceAttacked, U32 delayFromTeammates);
Entity* aiCritterFindTeammateInNeed(Entity* e, AIVars *ai, F32 range, AIPowerInfo* info, int targetType);
Entity* aiCritterFindDeadTeammate(Entity* e, AIVars *ai, F32 range, int targetType);
Entity* aiCritterFindNearbyDead(Entity* e, AIVars *ai);
//----------------------------------------------------------------
// Custom Villain behaviors
//----------------------------------------------------------------
void aiCritterDoBanishedPantheon(Entity* e, AIVars* ai);
void aiCritterDoBanishedPantheonSorrow(Entity* e, AIVars* ai);
void aiCritterDoBanishedPantheonLt(Entity* e, AIVars *ai);

void aiCritterDoCotTheHigh(Entity* e, AIVars* ai);
void aiCritterDoCotCaster(Entity* e, AIVars* ai);
void aiCritterDoCotSpectres(Entity* e, AIVars* ai);
void aiCritterDoCotBehemoth(Entity* e, AIVars* ai);

void aiCritterDoDevouringEarthMinion(Entity* e, AIVars* ai);
void aiCritterDoDevouringEarthLt(Entity* e, AIVars* ai);
void aiCritterDoDevouringEarthBoss(Entity* e, AIVars* ai);

void aiCritterDoTsooMinion(Entity* e, AIVars* ai);
void aiCritterDoTsooHealer(Entity* e, AIVars* ai);

void aiCritterDoArachnosMako(Entity* e, AIVars* ai);
void aiCritterDoArachnosLordRecluseSTF(Entity* e, AIVars* ai);
void aiCritterDoArachnosDrAeon(Entity* e, AIVars* ai);
void aiCritterDoArachnosRepairman(Entity* e, AIVars* ai);
void aiCritterDoArachnosWebTower(Entity* e, AIVars* ai);

//----------------------------------------------------------------
// entaiCritterUtils.c stuff.
//----------------------------------------------------------------

void aiCritterUpdateAttackTargetVisibility(Entity* e, AIVars* ai);
int aiCritterDoTeleport(Entity* e, AIVars* ai, AIPowerInfo* telePower);

#endif
