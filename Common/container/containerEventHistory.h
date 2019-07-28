#ifndef CONTAINEREVENTHISTORY_H_
#define CONTAINEREVENTHISTORY_H_

#define KARMA_EVENT_MAX_STAGES		30

typedef enum
{
	KARMA_TABLE_NONE		= 0,
	KARMA_TABLE_BAND		= 1,
	KARMA_TABLE_PERCENTILE	= 2,
} KarmaRewardTableChoice;

typedef struct KarmaEventHistory
{
	U32 timeStamp;
	char *eventName;
	char *authName;
	char *playerName;
	U32 dbid;
	int playerLevel;
	char *playerClass;
	char *playerPrimary;
	char *playerSecondary;
	char *bandTable;
	char *percentileTable;
	KarmaRewardTableChoice whichTable;	// 0=none, 1=band, 2=percentile
	char *rewardChosen;
	int playedStages;
	int avgStagePoints;
	int avgStagePercentile;
	int qualifiedStages;
	U32 pauseDuration;
	U32 pausedStagesCredited;
	int stageScore[KARMA_EVENT_MAX_STAGES];
	int stageThreshold[KARMA_EVENT_MAX_STAGES];
	int stageRank[KARMA_EVENT_MAX_STAGES];
	int stagePercentile[KARMA_EVENT_MAX_STAGES];
	int stageSamples[KARMA_EVENT_MAX_STAGES];
} KarmaEventHistory;

KarmaEventHistory *eventHistory_Create(void);
void eventHistory_Destroy(KarmaEventHistory *event);

#endif
