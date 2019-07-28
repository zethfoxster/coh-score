//
//	process_entry.h
//

#ifndef PROCESS_ENTRY_H
#define PROCESS_ENTRY_H

#include "common.h"

typedef BOOL (*EntryProcessingFunc) (int time, char * mapName, char * entityName, int teamID, const char * desc);

const char * GetActionName(EntryProcessingFunc action);
EntryProcessingFunc GetActionFunc(char *name);

typedef struct PlayerStats PlayerStats;

BOOL RecordInfluenceReward(PlayerStats * pPlayer, int influence, int time);
void SetLastRewardAction(PlayerStats * pPlayer, E_LastRewardAction action);

const char * GetLastRewardActionString(E_LastRewardAction action);

void InitEntryMap();

int IsSuperGroupAction(E_Action_Type action);
int IsTaskForceAction(E_Action_Type action);


#define DECLARE_PROCESSOR(PROCESSOR_NAME) BOOL PROCESSOR_NAME(int time, char * mapName, char * entityName, int teamID, const char * desc);


int ProcessEntry(const char * entry);

DECLARE_PROCESSOR(ProcessMapserverConnect);
DECLARE_PROCESSOR(ProcessMapserverQuit);

DECLARE_PROCESSOR(ProcessChatserver);

DECLARE_PROCESSOR(ProcessConnectionEntry);

DECLARE_PROCESSOR(ProcessEntityResumeCharacter);
DECLARE_PROCESSOR(ProcessEntityMapMove);

DECLARE_PROCESSOR(ProcessEntityCreateVillain);
DECLARE_PROCESSOR(ProcessEntityCreatePet);

DECLARE_PROCESSOR(ProcessEntityRecordKillPlayer);
DECLARE_PROCESSOR(ProcessEntityRecordKill);
DECLARE_PROCESSOR(ProcessEntityDiePet);
DECLARE_PROCESSOR(ProcessEntityDie);

DECLARE_PROCESSOR(ProcessEntityEffect);
DECLARE_PROCESSOR(ProcessEntityPeriodicInfo);

DECLARE_PROCESSOR(ProcessPowerToggle);
DECLARE_PROCESSOR(ProcessPowerUseInspiration);
DECLARE_PROCESSOR(ProcessPowerQueue);
DECLARE_PROCESSOR(ProcessPowerActivate);
DECLARE_PROCESSOR(ProcessPowerCancelled);
DECLARE_PROCESSOR(ProcessPowerHit);
DECLARE_PROCESSOR(ProcessPowerMiss);
DECLARE_PROCESSOR(ProcessPowerEffect);
DECLARE_PROCESSOR(ProcessPowerWindup);
DECLARE_PROCESSOR(ProcessPowerInterrupted);

DECLARE_PROCESSOR(ProcessEnhancements);

DECLARE_PROCESSOR(ProcessRewardLevelUp);
DECLARE_PROCESSOR(ProcessRewardPoints);
DECLARE_PROCESSOR(ProcessCharacterAddDebt);
DECLARE_PROCESSOR(ProcessRewardAddInspiration);
DECLARE_PROCESSOR(ProcessRewardAddSpecialization);
DECLARE_PROCESSOR(ProcessRewardTempPower);
//DECLARE_PROCESSOR(ProcessRewardInspiration);
//DECLARE_PROCESSOR(ProcessRewardEnhancement);
DECLARE_PROCESSOR(ProcessRewardStoryReward);
DECLARE_PROCESSOR(ProcessRewardMissionReward);



DECLARE_PROCESSOR(ProcessTrade);

DECLARE_PROCESSOR(ProcessMissionAttach);
DECLARE_PROCESSOR(ProcessMissionComplete);

DECLARE_PROCESSOR(ProcessTaskComplete);

DECLARE_PROCESSOR(ProcessInitialPowerSet);
DECLARE_PROCESSOR(ProcessInitialPower);
DECLARE_PROCESSOR(ProcessBuyPowerSet);
DECLARE_PROCESSOR(ProcessBuyPower);
DECLARE_PROCESSOR(ProcessAssignSlots);

DECLARE_PROCESSOR(ProcessCostumeTailor);


DECLARE_PROCESSOR(ProcessEncounter);


DECLARE_PROCESSOR(ProcessCharacterCreate);

DECLARE_PROCESSOR(ProcessTeamLeader);
DECLARE_PROCESSOR(ProcessTeamKick);


DECLARE_PROCESSOR(ProcessCmdParse);
DECLARE_PROCESSOR(ProcessChat);

DECLARE_PROCESSOR(ProcessStorePurchase);
DECLARE_PROCESSOR(ProcessStoreSell);

DECLARE_PROCESSOR(ProcessTeamCreate);
DECLARE_PROCESSOR(ProcessTeamDelete);
DECLARE_PROCESSOR(ProcessTeamAddPlayer);
DECLARE_PROCESSOR(ProcessTeamRemovePlayer);
DECLARE_PROCESSOR(ProcessTeamQuit);

DECLARE_PROCESSOR(ProcessTaskForceCreate);
DECLARE_PROCESSOR(ProcessTaskForceDelete);
DECLARE_PROCESSOR(ProcessTaskForceAddPlayer);
DECLARE_PROCESSOR(ProcessTaskForceRemovePlayer);
DECLARE_PROCESSOR(ProcessTaskForceQuit);

DECLARE_PROCESSOR(ProcessContactAdd);

void DisconnectActivePlayers();
void CleanupActivePlayers();

void CleanupActiveZoneStats();		// sets # of active players & encounters to '0'



#endif  // PROCESS_ENTRY_H
