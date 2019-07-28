/*\
 *
 *	arenamapserver.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Client code on the mapserver side for talking to arenaserver.
 *
 */

#ifndef ARENAMAPSERVER_H
#define ARENAMAPSERVER_H

#include "netio.h"

typedef struct ArenaEvent ArenaEvent;
typedef struct ArenaEventList ArenaEventList;
typedef struct Entity Entity;
typedef struct StuffBuff	StuffBuff;

ArenaEvent* FindEventDetail(U32 eventid, int add, int* index);

//filter for ArenaKioskSendFiltered
void EventIgnoreFilter(ArenaEvent * event,Entity * e);
void EventSupergroupFilter(ArenaEvent * event,Entity * e);

// networking
extern NetLink arena_comm_link;
int ArenaAsyncConnect(void);
int ArenaSyncConnect(void);
void ArenaKioskSubscribe(int on);	// if turned on, get periodic kiosk refreshes from arenaserver
void ArenaClientTick(void);
void ArenaSendKioskFail(int dbid);
void ArenaRequestKioskInfo(void);
void handleSendNotReadyMessage(Packet *pak);
void handleSendNextRoundMessage(Packet *pak);
void handleReportKillAck(Packet *pak);
void handleArenaKiosk(Packet* pak);
void handleArenaAddress(Packet* pak);
void handleEventUpdate(Packet* pak);
void handleRequestReady(Packet *pak);
void handleUpdateParticipant(Packet* pak);
void handleUpdatePlayer(Packet* pak);
void handleArenaResult(Packet* pak);
void handleClientRequestResults(Packet* pak);
void handleUpdateLeaderList(Packet* pak);
void handlePlayerStats(Packet* pak);
void SendActiveEvent(Entity* e, ArenaEvent* event);
void ArenaClearSupergroupRatings(Entity* e);

void ArenaRegisterPlayer(Entity* e, int add, int logout);	// should be called on ENTS_OWNED change
void ArenaPlayerLeavingMap(Entity* e, int logout);
void ArenaRegisterAllPlayers(void);
void ArenaPlayerAttributeUpdate(Entity* e);		// supergroup, sg_rank, or level change

void ArenaKioskSendIncremental(Entity* e);		// should be called on ent update

// commands
void ArenaSendKioskInfo(Entity * e,int radioButtonFilter,int tabFilter);	// now sends to arena server to avoid caching
void ArenaSendDebugInfo(int dbid);
void ArenaRequestPlayerStats(int dbid);
void ArenaReportKill(ArenaEvent *event, int partid);
int ArenaReportMapResults(ArenaEvent* event, int seat, int side, U32 time);

void ArenaRequestCreate(Entity* e);
void ArenaRequestResults(Entity* e, int eventid, int uniqueid);
void ArenaRequestCurrentResults(Entity *e);
void ArenaRequestAllCurrentResults();
void ArenaNotifyFinish(Entity *e);
void ArenaRequestObserve(Entity* e, int eventid, int uniqueid);
void ArenaRequestJoin(Entity* e, int eventid, int uniqueid, int invite, int ignore_active);
void ArenaDropPlayer(int dbid, int eventid, int uniqueid);
void ArenaDrop(Entity *e, int eventid, int uniqueid, int kicked_dbid );
void ArenaRequestUpdatePlayer(int dbidSource,int dbidTarget,int tabWindowRequest);

int ArenaCreateEvent(Entity* e, char* eventname);
int ArenaDestroyEvent(Entity* e, int eventid, int uniqueid);
void ArenaSyncRequestEvent(int eventid);
void ArenaSyncUpdateEvent(ArenaEvent* event);
int ArenaSetSide(Entity* e, int eventid, int uniqueid, int side);
void ArenaSetInsideLobby(Entity* e, int inside);
int ArenaSetMap(Entity* e, int eventid, int uniqueid, char* newmap);
int ArenaEnter(Entity* e, int eventid, int uniqueid);

void ArenaReceiveCreatorUpdate( Entity * e, Packet * pak );
void ArenaReceivePlayerUpdate( Entity * e, Packet * pak );
void ArenaReceiveFullPlayerUpdate( Entity * e, Packet * pak );
void ArenaFeePayment(int* dbids, int eventid, int uniqueid);
void ArenaConfirmReward(int dbid);
void ArenaReceiveCamera(Entity* e, Packet* pak);
void ArenaReceiveRespawnReq(Entity* e, Packet* pak);

void ArenaStartClientCountdown(Entity *e, char *str, int count);
void ArenaSetClientCompassString(Entity* e, char* str, U32 timeUntilMatchEnds, U32 matchLength, U32 respawnPeriod);
void ArenaSetAllClientsCompassString(ArenaEvent* event, char* str, int timeUntilMatchEnds);
void ArenaSendVictoryInformation(Entity* e, int numlives, int numkills, int infinite_resurrect);

void SendServerRunEventWindow(ArenaEvent* event, Entity* e);
void ArenaSendScheduledMessages(Entity* e);

void ArenaRequestLeaderBoard(Entity* e);
void DumpArenaLeaders(Entity *e,StuffBuff* psb, int iNumPlaces);

// active event and kiosk tracking
void ArenaPlayerLeftKiosk(Entity* e);		// called when player leaves area of kiosk


#endif // ARENAMAPSERVER_H