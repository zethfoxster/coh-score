/*\
 *
 *	ArenaUpdates.h/c - Copyright 2004, 2005 Cryptic Studios
 *		All Rights Reserved
 *		Confidential property of Cryptic Studios
 *
 *	Functions to send updates to events as required to mapservers.
 *  Mapservers register dbid's that they care about, and receive
 *  full updates on events those dbid's belong to.
 *
 */

#ifndef ARENAUPDATES_H
#define ARENAUPDATES_H

#include "arenastruct.h"

typedef struct NetLink	NetLink;

// hold on to registered dbid's by client link
typedef struct
{
	NetLink		*link;
	int*		registered_dbids;		// dbid's I need updates for
} ArenaClientLink;

int ArenaClientConnect(NetLink* link);
int ArenaClientDisconnect(NetLink* link);

// ArenaClientList is for accumulating client links for updates.
//   There is actually only one global struct.  Call the ClientList function
//   immediately before use.  Set <add> if you want to build lists iteratively.
typedef struct {
	ArenaClientLink **clients;
} ArenaClientList;

ArenaClientList* ClientListFromClient(ArenaClientLink* client, int add);
ArenaClientList* ClientListFromDbid(int dbid, int add);
ArenaClientList* ClientListFromEvent(ArenaEvent* event, int add);
void DbidsFromEvent(int** dbids, ArenaEvent* event);	// creates dbids earray

// registration, updates
void handleRegisterPlayers(Packet* pak, NetLink* link);
void ClientUpdateParticipant(ArenaClientList* list, int dbid, int fullupdate);
void ClientUpdateEvent(ArenaClientList* list, U32 eventid);
void ClientRequestReadyMessages(ArenaEvent *event);
void ClientSendNotReadyMessage(U32 eventid, char *playername);
void ClientSendNextRoundMessage(U32 eventid, int numseconds);
void ClientUpdatePlayer(int dbid);
void ClientUpdateLeaderList(ArenaClientList* list, ArenaRankingTable leaderlist[ARENA_NUM_RATINGS]);

// TTL list of dbid's - used for logout & disconnect detection
void SetPlayerTTL(U32 dbid, int clear);
void SetTTLForNegotiatingPlayers(void);
void CheckForPlayerDisconnects(void);
void HandlePlayerDisconnect(U32 dbid);


#endif // ARENAUPDATES_H