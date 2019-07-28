/*\
 *
 *	ArenaEvent.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Server logic for arena events
 *
 */

#ifndef ARENAEVENT_H
#define ARENAEVENT_H

#include "containerArena.h"
#include "netio.h"

// global list of events
extern ArenaEventList g_events;

typedef struct EventHistoryEntry EventHistoryEntry;

ArenaEvent* EventAdd(char* name);				// returns event
ArenaEvent* EventGet(U32 eventid);
int EventGetId(ArenaEvent *event);
ArenaEvent* EventGetByDbid(U32 dbid);
ArenaEvent* EventGetUnique(U32 eventid, U32 uniqueid);
ArenaEvent* EventGetAdd(U32 eventid, int initialLoad);	// return existing record or create new one
void EventDelete(U32 eventid);
void EventSave(U32 eventid);			// propagate to dbserver
int EventTotal(void);
void EventIter(int(*func)(ArenaEvent*));
void EventAddParticipant(ArenaEvent* event, NetLink* link, U32 dbid, const char* archetype, int level, U32 sgid, int sgleader);
void EventRemoveParticipant(ArenaEvent* event, U32 dbid);
ArenaParticipant* EventGetParticipant(ArenaEvent* event, U32 dbid);
void EventLog(ArenaEvent* event, char* logfile);

void arenaLoadMaps();

// client requests
void handleRequestKiosk(Packet* pak,NetLink* link);
void handleJoinEvent(Packet* pak, NetLink* link);
void handleDropEvent(Packet* pak, NetLink* link);
void handleCreateEvent(Packet* pak, NetLink* link);
void handleDestroyEvent(Packet* pak, NetLink* link);
void handleRequestEvent(Packet* pak, NetLink* link);
void handleSetSide(Packet* pak, NetLink* link);
void handleSetReady(Packet* pak, NetLink* link);
void handleSetMap(Packet* pak, NetLink* link);
void handleRequestPlayerUpdate(Packet* pak, NetLink* link);
void handleCreatorUpdate(Packet* pak, NetLink* link);
void handlePlayerUpdate(Packet* pak, NetLink* link);
void handleFullPlayerUpdate(Packet* pak, NetLink* link);
void handleReportKill(Packet *pak, NetLink *link);
void handleMapResults(Packet* pak, NetLink* link);
void handleFeePayment(Packet* pak, NetLink* link);
void handleConfirmReward(Packet* pak, NetLink* link);
void handleRequestResults(Packet* pak, NetLink* link);
void handlePlayerAttributeUpdate(Packet* pak, NetLink* link);
void handleClearPlayerSGRatings(Packet* pak, NetLink* link);
void handleRequestLeaderUpdate(Packet* pak, NetLink* link);
void handleRequestPlayerStats(Packet* pak, NetLink* link);
void handleReadyAck(Packet *pak, NetLink *link);

// server processing
void EventProcess(void);				// main loop over events
void EventPlayerLoggedOut(U32 dbid);	// player just got disconnected
void EventListInit(void);				// event list just got loaded

extern int g_disableEventLog;							// disable writing events to log file as they are destroyed

#endif // ARENAEVENT_H