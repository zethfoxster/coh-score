// TurnstileServerMsg.h
//

#pragma once

typedef struct Packet Packet;
typedef struct NetLink NetLink;
typedef struct QueueGroup QueueGroup;

void turnstileServer_handleQueueForEvents(Packet *pak, NetLink *link);
void turnstileServer_handleRemoveFromQueue(Packet *pak, NetLink *link);
void turnstileServer_handleEventReadyAck(Packet *pak);
void turnstileServer_handleEventResponse(Packet *pak);
void turnstileServer_handleMapID(Packet *pak);
void turnstileServer_handleShardXferOut(Packet *pak, NetLink *link);
void turnstileServer_handleShardXferBack(Packet *pak, NetLink *link);
void turnstileServer_handleCookieRequest(Packet *pak);
void turnstileServer_handleCookieReply(Packet *pak);
void turnstileServer_handleGroupUpdate(Packet *pak);
void turnstileServer_handleRequestQueueStatus(Packet *pak, NetLink *link);
void turnstileServer_handleCloseInstance(Packet *pak);
void turnstileServer_handleRejoinRequest(Packet *pak, NetLink *link);
void turnstileServer_handlePlayerLeave(Packet *pak);
void turnstileServer_handleIncarnateTrialComplete(Packet *pak);
void turnstileServer_handleCrashedMap(Packet *pak);
void turnstileServer_handleQueueForSpecificMissionInstance(Packet *pak, NetLink *link);
void turnstileServer_handleAddBanID(Packet *pak);

void turnstileServer_generateEventWaitTimes();
void turnstileServer_generateQueueStatus(QueueGroup *group, int dbid, int inQueue, NetLink *link);
void turnstileServer_generateEventReady(QueueGroup *group, TurnstileMission *mission, MissionInstance *instance, int playerIndex, int inProgress);
void turnstileServer_generateEventReadyAccept(QueueGroup *group, MissionInstance *instance, int playerIndex);
void turnstileServer_generateEventFailedStart(QueueGroup *group, int playerIndex);
void turnstileServer_generateMapStart(QueueGroup *group, TurnstileMission *mission, MissionInstance *instance, int playerIndex);
void turnstileServer_generateEventStart(QueueGroup *group, TurnstileMission *mission, MissionInstance *instance);
void turnstileServer_generateShardXfer(U32 dbid, char *shardName, NetLink *link, U32 guid);