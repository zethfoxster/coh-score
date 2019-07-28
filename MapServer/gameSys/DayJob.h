/***************************************************************************
 *     Copyright (c) 2008-2008, NC NorCal
 *     All Rights Reserved
 *     Confidential Property of NC NorCal
 ***************************************************************************/
#ifndef DAYJOB_H__
#define DAYJOB_H__

#include "stdtypes.h"

typedef struct Entity Entity;
typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

typedef enum DayJobPowerTypeEnum
{
	kDayJobPowerType_TimeInGame,
	kDayJobPowerType_TimeUsed,
	kDayJobPowerType_Activation,
	kDayJobPowerType_Count
} DayJobPowerTypeEnum;


typedef struct DayJobPower {
	char					*pchPower;
		// Name of power to be granted (uses xpath notation)
	char					*pchSalvage;
		// Name of salvage to be granted
	char					**ppchRequires;
		// Requires that must be true in order for this power block to be applied
	float					fFactor;
		// scalar factor that is multiplied by the time offline (in sec) to give the amount of time/uses/salvage that this block will award
	int						iMax;
		
	DayJobPowerTypeEnum		eType;

	char					*pchRemainderToken;

} DayJobPower;

typedef struct DayJobDetail {
	char *pchName;
		// internal name

	char *pchSourceFile;

	char *pchDisplayName;

	char **ppchVolumeNames;
	char *pchZoneName;

	char *pchStat;
		// stat to be incremented when the player completes their service in this day job

	DayJobPower **ppPowers;
		// list of powers that can be applied

} DayJobDetail;


typedef struct DayJobDetailDict
{
	const DayJobDetail **ppJobs;
	cStashTable haVolumeNames;
	cStashTable haZoneNames;

	int minTime;
		// minimum amount of time (in minutes) that a player must be logged out in order to start gaining 
		//		Day Job credit

	float fPatrolXPMultiplier;
		// Multiplier to be applied when player has patrol xp remaining

	float fPatrolScalar;
		// Amount of patrol xp to be awarded for each second a character is offline


} DayJobDetailDict;


// ------------------------------------------------------------
// globals

extern SHARED_MEMORY DayJobDetailDict g_DayJobDetailDictDict;

// ------------------------------------------------------------
// functions

void dayjobdetail_Load(void);

void dayjob_Tick(Entity *pPlayer);

float dayjob_GetPatrolXPMultiplier(void);

int dayjob_IsPlayerInAnyDayjobVolume(Entity *pPlayer);

void awardRestExperience( Entity * e, F32 fNewPercent );

#endif /* #ifndef DAYJOB_H__ */

/* End of File */

