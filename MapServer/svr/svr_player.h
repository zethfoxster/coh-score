#ifndef _SVR_PLAYER_H
#define _SVR_PLAYER_H

#include "entserver.h"
#include "dbcomm.h"

typedef struct ClientLink ClientLink;
typedef struct NetLink NetLink;

typedef enum
{
	KICKTYPE_FROMDBSERVER = 0,
	KICKTYPE_KICK,
	KICKTYPE_BAN,
	KICKTYPE_BADDATA,
	KICKTYPE_OLDLINK,
} KickReason;

typedef struct
{
	int		count,max;
	Entity	**ents;
} PlayerEnts;

typedef enum
{
	PLAYER_ENTS_ACTIVE,
	PLAYER_ENTS_CONNECTING,	
	PLAYER_ENTS_OWNED,		// if in this list, database updates may be made on the player - contains
							// most of the ACTIVE and CONNECTING lists
	PLAYER_ENTTYPE_COUNT,
} PlayerEntType;

extern PlayerEnts player_ents[];

extern int player_count;
extern int svr_hero_count;
extern int svr_villain_count;

void svrSendPlayerError(Entity *player,const char *fmt, ...);
void svrForceLogoutLink(NetLink *link,const char *reason,KickReason kicktype);
void svrForceLogoutAll(char *reason);
int svrSendEntListToDb(Entity **ents,int count);
int svrSendAllPlayerEntsToDb(void);
void svrCheckQuitting(void);
void svrBootPlayer(Entity *player);
void svrBootAllPlayers();
Entity* firstActivePlayer(void); // find the first player in the entities list
void svrCheckMissionEmpty(void);
void svrCheckBaseEmpty(void);
void svrCheckUnusedAndEmpty(void);
void playerEntAdd(PlayerEntType type,Entity *e);
void playerEntDelete(PlayerEntType type,Entity *e, int logout);
int playerEntCheckMembership(PlayerEntType type,Entity *e);
void playerCheckFailedConnections(void);
void playerEntCheckDisconnect(void);
NetLink *findLinkById(int id);
Entity *playerCreate(void);
void svrClientLinkInitialize(ClientLink* client, NetLink* link);
void svrClientLinkDeinitialize(ClientLink* client);
int svrClientCallback(NetLink *link);
int svrNetDisconnectCallback(NetLink *link);
void svrResetClientControls(ClientLink* client, int full_reset);
void svrPrintControlQueue(ClientLink* client);
void svrTickUpdatePlayerFromControls(ClientLink *client);
void svrPlayerFindLastGoodPos(Entity *e);
void receiveClientInput(Packet *pak,ClientLink *client);
Entity* svrFindPlayerInTeamup(int teamID, PlayerEntType type);
void svrCompletelyUnloadPlayerAndMaybeLogout(Entity *e, const char * reason, int logout);
void svrPlayerSetDisconnectTimer(Entity *e,int update_val);
int svrAnyOwnedOrActiveEnts(void);
int svrPlayerSendControlState(Packet* pak, Entity* e, NetLink* link, int full_state);

#endif
