#ifndef LWC_COMMON_H
#define LWC_COMMON_H
// Header file for Light Weight Client common information
#include "stdtypes.h"

typedef struct Entity Entity;

typedef enum
{
	LWC_STAGE_INVALID	= 0,
	LWC_STAGE_1			= 1,
	LWC_STAGE_2			= 2,
	LWC_STAGE_3			= 3,
	LWC_STAGE_4			= 4,

	LWC_STAGE_LAST		= LWC_STAGE_4,
} LWC_STAGE;

LWC_STAGE LWC_GetRequiredDataStageForMap(const char *map_name);
LWC_STAGE LWC_GetRequiredDataStageForMapID(int map_id);

bool LWC_CheckMapReady(const char *map_name, LWC_STAGE current_stage, LWC_STAGE * out_required_stage);
bool LWC_CheckMapIDReady(int map_id, LWC_STAGE current_stage, LWC_STAGE * out_required_stage);
bool LWC_CheckSGBaseReady(LWC_STAGE current_stage, LWC_STAGE * out_required_stage);

LWC_STAGE LWC_GetCurrentStage(Entity *e);

#if CLIENT
void LWC_SendEntityStageToMapServer(Entity *e);
#endif

#endif

