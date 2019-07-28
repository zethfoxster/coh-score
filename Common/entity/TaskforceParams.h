/***************************************************************************
 *     Copyright (c) 2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef TASKFORCEPARAMS_H
#define TASKFORCEPARAMS_H

#define TFPARAM_TIMELIMIT_BRONZE_DEFAULT	(120 * 60)
#define TFPARAM_TIMELIMIT_SILVER_DEFAULT	(60 * 60)
#define TFPARAM_TIMELIMIT_GOLD_DEFAULT		(30 * 60)

typedef enum TFParamSL
{
	TFPARAM_SL1,
	TFPARAM_SL2,
	TFPARAM_SL25,
	TFPARAM_SL3,
	TFPARAM_SL4,
	TFPARAM_SL5,
	TFPARAM_SL6,
	TFPARAM_SL7,
	TFPARAM_SL8,
	TFPARAM_SL9
} TFParamSL;

typedef enum TaskForceParameter TaskForceParameter;

typedef struct Entity Entity;
typedef struct EvalContext EvalContext;

bool TaskForceIsParameterEnabled(Entity* player, TaskForceParameter param);
U32 TaskForceGetParameter(Entity* player, TaskForceParameter param);
void TaskForceEvalAddDefaultFuncs(EvalContext* context);
TFParamSL TaskForceConvertRangeToStature(int min_level, int max_level);

#if defined(SERVER)
typedef enum TFParamDeaths TFParamDeaths;
typedef enum TFParamPowers TFParamPowers;
typedef enum TFParamTimeLimits TFParamTimeLimits;

typedef struct StoryArc StoryArc;

void TaskForceLoadTimeLimits(void);
U32 TaskForceGetStoryArcTimeLimit(const StoryArc *arc, TFParamTimeLimits limit);
bool TaskForceFindTimeLimits(const StoryArc* arc, U32* bronze_dest,
	U32* silver_dest, U32* gold_dest);
void TaskForceSendTimeLimitsToClient(Entity* player, const StoryArc* arc);
bool TaskForceInitParameters(Entity* player, TFParamDeaths max_deaths,
	U32 time_limit, bool time_counter, bool debuff,
	TFParamPowers disable_powers, bool disable_enhancements, bool buff,
	bool disable_inspirations);
bool TaskForceCheckFailureBit(Entity* player, TaskForceParameter param);
bool TaskForceChalkMemberDeath(Entity* player);
bool TaskForceCheckTimeout(Entity* player);
bool TaskForceCheckBroadcasts(Entity* player);
bool TaskForceShowChallengeFailedMsg(Entity* player);
bool TaskForceShowChallengeSummary(Entity* player);
bool TaskForceSetEffects(Entity* player, bool disable_enhancements,
	TFParamPowers disable_powers, bool disable_inspirations);
bool TaskForceInitDebuffs(Entity* player);
bool TaskForceInitMap(Entity* player);
bool TaskForceResumeEffects(Entity* player);
void TaskForce_HandleExit(Entity* player);
bool TaskForceCheckCompletionParameters(Entity* player, char* arc_leafname,
	bool succeeded);
#endif

#endif
