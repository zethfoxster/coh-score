// TurnstileServerEvent.h
//

#pragma once

typedef struct QueueGroup QueueGroup;

void TurnstileServerEventPrepare(TurnstileMission *mission, MissionInstance *instance, QueueGroup **groups, int numGroup, int numPlayer);
void TurnstileServerEventReleaseInstance(TurnstileMission *mission, MissionInstance *instance);
void TurnstileServerEventStartMap(TurnstileMission *mission, MissionInstance *instance);
void TurnstileServerEventStartEvent(TurnstileMission *mission, MissionInstance *instance);
MissionInstance *TurnstileServerEventFindInstanceByID(int instanceID);
void InstanceInit();