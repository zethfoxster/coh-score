#pragma once
#include "net_link.h"
#include "net_linklist.h"
#include "auth/auth.h"
#ifdef DBSERVER
#include "servercfg.h"	//used for MAX_PLAYER_SLOTS
#endif

void initGameChecksum();
void clientCommLoginInit(NetLinkList *gameclient_links);
void clientCommLoginInitStartListening(NetLinkList *gameclient_links,NetPacketCallback callback, NetLinkDestroyCallback destroyCallback);
int clientCommLoginConnectedCount(NetLinkList *gameclient_links);
bool clientKickLogin(U32 auth_id,int reason, NetLinkList *gameclient_links);
int clientCommLoginVerifyChecksum(U32 checksum[]);
void clientCommLoginClearDeadLinks(Array* links);
char *linkIpStr(NetLink *link);

typedef enum GameClientLoginStatus
{
	GAMELLOGIN_UNINITIALIZED,
	GAMELLOGIN_UNAUTHENTICATED,
	GAMELLOGIN_QUEUED,
	GAMELLOGIN_LOGGINGIN,
	GAMELLOGIN_COUNT,
}GameClientLoginStatus;

typedef struct
{
	U8		valid;
	NetLink	*link;
	U32		auth_id;
	U32		container_id;
	char	account_name[64];
#ifdef DBSERVER
	U32		*players;
	U8		*players_offline;
#endif
	U8		auth_user_data[AUTH_BYTES];
	U32		payStat;
	U32		loyalty;
	U32		loyaltyLegacy;
	U32		local_map_ip;
	int		limit_max_slots;		//support for legacy clients
	int		limit_dual_slots;		//support for legacy clients
	int		char_id;
	int     getting_player;
	char	*systemSpecs;	//this will be freed and zeroed when used, so hopefully shouldn't have to worry about managing.
	U32		nonQueueIP;		//if using the queue server, what is the client's actual IP?
	U32		linkId;			//how to find the queue server link that attaches to the client.
	GameClientLoginStatus		loginStatus;		//player login status
	U32		remove;
	int		vipFlagReady;
	int		vip;
} GameClientLink;

