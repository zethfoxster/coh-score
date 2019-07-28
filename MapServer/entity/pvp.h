#ifndef PVP_H__
#define PVP_H__

#include "stdtypes.h"

typedef struct Entity Entity;

#define DEFEAT_RECORD_TIME	300

void pvp_LogPlayerDeath(Entity *pVictim, Entity *pVictor);

void pvp_RewardVictory(Entity *pVictor, Entity *pVictim);
void pvp_GrantRewardTable(Entity *pVictor, Entity *pVictim);

void pvp_RecordDefeat(Entity *pVictim, Entity *pVictor);
bool pvp_IsOnDefeatList(Entity *pVictim, Entity *pVictor);
void pvp_CleanDefeatList(Entity *pVictim);

void pvp_SetSwitch(Entity *e, bool pvp);
void pvp_ApplyPvPPowers(Entity *e);

void pvp_LogStart(Entity *e);
void pvp_LogStop(Entity *e);
void pvp_LogTick(void);

void pvp_UpdateTargetTracking(float serverTime);

#endif /* #ifndef PVP_H__ */

/* End of File */
