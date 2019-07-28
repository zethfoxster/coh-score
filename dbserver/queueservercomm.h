#pragma once
#include "..\Common\ClientLogin\clientcommLogin.h"

typedef struct Packet Packet;
typedef struct NetLink NetLink;

void queueserver_commInit(void);
int queueserver_secondsSinceUpdate(void);
NetLink* queueserver_getLink(void);


Packet *dbClient_createPacket(int cmd, GameClientLink *client);
void sendClientQuit(char *authname, U32 link_id);	//only authname or id is required.

//stats update
int queueservercomm_getNWaiting();
int queueservercomm_getNLoggingIn();
int queueservercomm_getNConnected();
void queueservercomm_sendUpdateConfig();
//for debugging--make sure we know the correct number of people doing what at all times.
void queueservercomm_updateCount();

//create and destroy the gameclientlink
GameClientLink * queueservercomm_gameclientCreate(U32 ip, U32 linkId);
void queueservercomm_gameclientDestroy(GameClientLink *game);
void setGameClientOnQueueServer(GameClientLink *game);
void destroyGameClientByName(char *authname, U32 linkId);

//receiving g.c.l. from the queue server
GameClientLink *queueservercomm_getLoginClient(Packet *pak_in);


//accessing the gameclientlink from within the dbserver.
GameClientLink *dbClient_getGameClient(U32 auth_id, NetLink *link);
GameClientLink *dbClient_getGameClientByName(char *authname, NetLink *link);

// this is for places where:
// 1) we know we're running a queueserver and 
// 2) we don't have direct access to the incoming queueserver/client link
void getGameClientOnQueueServer(U32 auth_id, GameClientLink **game);
void getGameClientOnQueueServerByName(char *authname, GameClientLink **game);

void queueservercomm_switchgamestatus(GameClientLink *game, GameClientLoginStatus status);






void reconnectQueueserverCommPlayersToAuth();

