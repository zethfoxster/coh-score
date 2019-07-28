#ifndef LWC_H
#define LWC_H
// Header file for Light Weight Client
#include "stdtypes.h"
#include "LWC_common.h"

typedef enum
{
	LWC_DOWNLOAD_NORMAL		= 0,
	LWC_DOWNLOAD_FASTEST	= 1,
	LWC_DOWNLOAD_PAUSED		= 2,
} LWC_DOWNLOAD_RATE;

void LWC_Init();
void LWC_Enable();

void LWC_Tick();

// Last complete stage.  Should always be at least LWC_STAGE_1
LWC_STAGE LWC_GetLastCompletedDataStage(void);

// LWC is considered active if the current stage is less than the final stage (ie not all data is loaded)
bool LWC_IsActive();

char *LWC_GetStageString();

bool LWC_CheckStageAndShowMessageIfNotReady(LWC_STAGE stage);

// seconds remaining until the next stage is ready
U32 LWC_GetRemainingSecondsForNextStage();
F32 LWC_GetPercentDoneNextStage();		// 0.0f to 1.0f

// seconds remaining until the last stage is ready (all data is ready)
U32 LWC_GetRemainingSecondsTotal();
F32 LWC_GetPercentDoneTotal();		// 0.0f to 1.0f

void LWCDebug_SetMode(int);
void LWC_SetCurrentDataStage(LWC_STAGE stage);

void LWC_SetDownloadRate(LWC_DOWNLOAD_RATE newRate);
LWC_DOWNLOAD_RATE LWC_GetDownloadRate();

// for convenience:
void LWC_SetThrottled(bool bThrottled);
#endif

