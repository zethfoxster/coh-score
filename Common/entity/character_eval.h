/***************************************************************************
 *     Copyright (c) 2004-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef CHARACTER_EVAL_H__
#define CHARACTER_EVAL_H__

typedef struct EvalContext EvalContext;
typedef struct Supergroup Supergroup;
typedef struct Character Character;
typedef struct Entity Entity;

void chareval_Init(void);
int chareval_Eval(Character *p, const char * const *ppchExpr, const char *dataFilename);
void chareval_JustEval(Character *p, const char * const *ppchExpr);
void chareval_Validate(const char **ppchExpr, const char *dataFilename);
void chareval_AddDefaultFuncs(EvalContext *pContext);

#if SERVER
#include "storyarcprivate.h"
extern int g_scriptCharEvalResult;

int chareval_EvalWithTask(Character *p, const char **ppchExpr, StoryTaskInfo *pTask, const char *dataFilename);
void chareval_ValidateWithTask(const char **ppchExpr, const char *dataFilename);
void scripteval_CharEval(EvalContext *pcontext);
#endif

EvalContext *chareval_Context(void);

// Helpers
void chareval_EntityFetchHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityHasTagHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityIsToggleActiveHelper(EvalContext *pcontext, Entity *e, const char *powerFullName);
void chareval_EntityHasSouvenirClueHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityEventCountHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityEventTimeHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_MapTeamAreaHelper(EvalContext *pcontext, Entity *e);
void chareval_EntityOwnsBadgeHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityTokenOwnedHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityTokenValHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityTokenTimeHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityInModeHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_TeamSizeHelper(EvalContext *pcontext, Entity *e, float fRadius);
void chareval_AuthFetchHelper(EvalContext *pcontext, U32 *auth_user_data, const char *rhs);
void chareval_EntityLoyaltyOwnedHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityLoyaltyTierOwnedHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityLoyaltyLevelOwnedHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityProductOwnedHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityProductAvailableHelper(EvalContext *pcontext, Entity *e, const char *rhs);
void chareval_EntityLoyaltyPointsEarnedHelper(EvalContext *pcontext, Entity *e);
void chareval_EntityIsVIPHelper(EvalContext *pcontext, Entity *e);
void chareval_EntityIsOutsideHelper(EvalContext *pcontext, Entity *e);
void chareval_EntityIsAccountServerAvailableHelper(EvalContext *pcontext, Entity *e);
void chareval_IsAccountInventoryLoadedHelper(EvalContext *pcontext, Entity *e);

// EvalFuncs
void chareval_Now(EvalContext *pcontext);
void chareval_IsPvPMap(EvalContext *pcontext);
void chareval_IsMissionMap(EvalContext *pcontext);
void chareval_IsArchitectMap(EvalContext *pcontext);
void chareval_IsHeroOnlyMap(EvalContext *pcontext);
void chareval_IsVillainOnlyMap(EvalContext *pcontext);
void chareval_IsHeroAndVillainMap(EvalContext *pcontext);
void chareval_SystemFetch(EvalContext *pcontext);
void chareval_OwnsPower(EvalContext *pcontext, const char *entString);
void chareval_OwnsPowerNum(EvalContext *pcontext, const char *entString);
void chareval_OwnsPowerCharges(EvalContext *pcontext, const char *entString);
void chareval_EntityIsInVolume(EvalContext *pcontext, const char *ent);
void chareval_EntityOwnsPower(EvalContext *pcontext);
void chareval_EntityFetch(EvalContext *pcontext);
void chareval_EntityHasTag(EvalContext *pcontext);
void chareval_EntityHasSouvenirClue(EvalContext *pcontext);
void chareval_EntityEventCount(EvalContext *pcontext);
void chareval_EntityEventTime(EvalContext *pcontext);
void chareval_EntityTokenOwned(EvalContext *pcontext);
void chareval_EntityBadgeStatFetch(EvalContext *pcontext);
void chareval_EntityOwnsBadge(EvalContext *pcontext);
void chareval_EntityCountBadges(EvalContext *pcontext);
void chareval_EntityCountSouvenirs(EvalContext *pcontext);
void chareval_EntityInMode(EvalContext *pcontext);
void chareval_EntityHasPowerset(EvalContext *pcontext);
void chareval_EntityArchitectTokenOwned(EvalContext *pcontext);
void chareval_EntityHasEnabledPower(EvalContext *pcontext);
void chareval_MapName(EvalContext *pcontext);
void chareval_ScriptFetch(EvalContext *pcontext);
void chareval_ProductAvailable(EvalContext *pcontext);

int chareval_requires(Character *pchar, char *requires, const char *dataFilename);
void chareval_requiresValidate(char *requires, const char *dataFilename);

#endif /* #ifndef CHARACTER_EVAL_H__ */

/* End of File */

