#ifndef SCRIPTEDZONEEVENTKARMA_H
#define SCRIPTEDZONEEVENTKARMA_H

#define KARMA_BUBBLE_POLLING_INTERVAL	5

typedef struct Entity Entity;
typedef enum ScriptKarmaType
{
	SKT_SORTED_STAT,
	SKT_MEDIAN,
	SKT_AVERAGE,
	SKT_NUM,
	SKT_COUNT,
}ScriptKarmaType;
typedef enum ScriptKarmaResetType
{
	SKRT_STAGE_KARMA,
	SKRT_ALL_KARMA,
	SKRT_COUNT,
}ScriptKarmaResetType;
typedef struct ScriptKarmaArray
{
	int *karma;
}ScriptKarmaArray;
typedef struct ScriptKarmaBucket
{
	ScriptKarmaArray karmaArrays[SKT_COUNT];
	int db_id;
	int timeUpdated;
	U32 usedPardonTime;
	int pardonReturnGrace;
}ScriptKarmaBucket;
typedef struct ScriptKarmaBucketList
{
	ScriptKarmaBucket **scriptKarmaBuckets;
	int scriptId;
}ScriptKarmaBucketList;

typedef struct KarmaEventHistory KarmaEventHistory;
typedef struct PlayerKarmaData
{
	int points;
	int stages;
	Entity *ePtr;
	KarmaEventHistory *hPtr;
}PlayerKarmaData;

typedef struct ScriptReward
{
	char *rewardName;
	int deprecated;
	int pointsNeeded;
	float pointsNeededByPercentile;	//	is local member that is set by code
	F32 percentileNeeded;
	int numStagesRequired;
	const char *rewardTable;
	U8 deprecatedChar;
} ScriptReward;

ScriptKarmaBucket *ScriptGetBucketForPlayer(Entity *e, int scriptId);
int ScriptKarmaGetAverage(Entity *e, int scriptId, int reason);
int ScriptKarmaGetMedian(Entity *e, int scriptId, int reason);
ScriptKarmaBucket *ScriptAddBucket(Entity *e, int scriptId);
void ScriptAddStatToPlayer(Entity *e, int scriptId, int reason);
void ScriptAddKarmaToTeam(const char *player, int reason, int amount, int lifespan);
void ScriptAddKarmaToAllPlayers(int reason, int amount, int lifespan);
void ScriptAddStatToAllActivePlayers(int reason);
void ScriptAddStatToAllPlayers(int reason);
void ScriptDestroyKarmaBucketsForAllPlayers(int method);
int ScriptKarmaGetActivePlayers(int scriptId);
int ScriptKarmaIsPlayerActive(int db_id);
void ScriptUpdateBubbles(Entity *e);
void ScriptKarmaPoll(int scriptId, int currentTime);

typedef const char* ENTITY;
typedef int NUMBER;
// Extracted from ScriptedZoneEvent internals for exposing to Lua
void ScriptSetKarmaContainer(ENTITY player);
void ScriptClearKarmaContainer(ENTITY player);
void ScriptGlowieBubbleActivate(ENTITY activator, ENTITY player);
void ScriptEnableKarma();
void ScriptDisableKarma();
void ScriptResetKarma();
bool IsKarmaEnabled();
void ScriptKarmaAdvanceStage();
void ScriptUpdateKarma(bool pause);
void ScriptAddObjectiveKarma(ENTITY player, NUMBER teamKarmaValue, NUMBER teamKarmaLife, NUMBER allKarmaValue, NUMBER allKarmaLife);
void HandleRewardGroup(ScriptReward **rewards, int* stageThreshs, int ZERewardGroup, const char* scriptName);
#endif