/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SUPERGROUP_EVAL_H__
#define SUPERGROUP_EVAL_H__

typedef struct EvalContext EvalContext;
typedef struct Supergroup Supergroup;

// Helpers
Supergroup *sgeval_SupergroupHelper(EvalContext *pcontext);
void sgeval_SupergroupFetchHelper(EvalContext *pcontext, Supergroup *sg, const char *rhs);
void sgeval_SupergroupTokenOwnedHelper(EvalContext *pcontext, Supergroup *sg, const char *rhs);
void sgeval_SupergroupTokenValHelper(EvalContext *pcontext, Supergroup *sg, const char *rhs);

// EvalFuncs
void sgeval_SupergroupTokenOwned(EvalContext *pcontext);
void sgeval_SupergroupTokenVal(EvalContext *pcontext);
void sgeval_SupergroupOwnsBadge(EvalContext *pcontext);
void sgeval_SupergroupCountBadges(EvalContext *pcontext);
void sgeval_SupergroupFetch(EvalContext *pcontext);

void sgeval_AddDefaultFuncs(EvalContext *pContext);


#endif /* #ifndef SUPERGROUP_EVAL_H__ */

/* End of File */

