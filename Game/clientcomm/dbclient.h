#ifndef _DBCLIENT_H
#define _DBCLIENT_H

#ifndef COSTUME_H__
#include "costume.h"
#endif
#ifndef POWER_CUSTOMIZATION_H__
#include "power_customization.h"
#endif
#include "auth.h"
#include "AccountData.h"

#define DB_CONNECTION_TIMEOUT 40 // warn user if havent heard from db in this time

typedef struct
{
	int		idx;
	int		type;
	int		subtype;
	int		typeByLocation;
	int		praetorian;
	char	name[64];
	char	playerslot_class[64];
	char	origin[64];
	U32		db_flags;
	int		level;
	int		db_id;
	char	map_name[256];
	int		map_seconds;
	U32		seconds_till_offline;
	U32		last_online;
	SlotLock slot_lock;
	Costume *costume;
	U32		invalid:1;
	U32		hidden:1;
	char	primaryPower[64];
	char	secondaryPower[64];
	const BasePowerSet *primarySet;
	const BasePowerSet *secondarySet;
} PlayerSlot;


typedef struct
{
	U32		ip;
	U32		port;
	U32		cookie;
	U32		user_id;
} MapServerConn;

typedef struct AccountInventory AccountInventory;

typedef struct
{
	PlayerSlot						*players;
	int								player_count;
	int								base_slots;
	int								bonus_slots;
	int								vip;
	int								showPremiumSlotLockNag;
	int								shardXferTokens;
	int								shardXferFreeOnly;
	int								max_slots;
	MapServerConn					mapserver;
	char							error_msg[1000];
	U8								auth_user_data[AUTH_BYTES];
	char							address[100];
	U32								time_recvd; // when this info was sent
	//U32								time_remaining;
	//U32								reservations;
	char							shardname[100];
	AccountInventorySet				account_inventory;
	U8								loyaltyStatus[LOYALTY_BITS/8];
	U32								loyaltyPointsUnspent;
	U32								loyaltyPointsEarned;
	U32								virtualCurrencyBal;
	U32								lastEmailTime;
	U32								lastNumEmailsSent;
	U32								accountInformationAuthoritative;
	U32								accountInformationCacheTime;
} DbInfo;

extern DbInfo db_info;

// Generated by mkproto
void dbOrMapConnect(Packet *pak);
int dbCheckCanStartStaticMap(int createLocation, F32 timeout);
void dbMakePlayerOnline(int slot_number);
int dbChoosePlayer(int slot_number,U32 local_map_ip, int createLocation, F32 timeout);
int dbChooseVisitingPlayer(int dbid, F32 timeout);
int dbGetCostume(int slot_number);
int dbGetCharacterCountsAsync(void);
int dbGetAccountServerCatalogAsync(void);
int dbLoyaltyRewardBuyItem(char *nodeName);
int dbLoyaltyRewardRefundItem(char *nodeName);

void handleSendCharacterCounts(Packet *pak);
void handleSendAccountServerCatalog(Packet *pak);

void handleAccountPlayNCAuthKeyUpdate(Packet *pak);

void handleClientAuth(Packet *pak);

int dbRedeemShardXferToken(U32 auth_id, char *name, U32 slot_idx, char *destshard);
int dbRenameCharacter(char *old_name, char *new_name);
int dbRedeemRenameToken(U32 auth_id, char *name, U32 slot_idx);
int dbCheckName(char *name, int fromLoginScreen, int tempLockName);
int dbRedeemSlot(void);
int dbUnlockCharacter(char *name);
int dbSignNDA(U32 NDAVersion, U32 versionSubtype);

int dbGetPlayNCAuthKey(int request_key, char* auth_name, char* digest_data); // returns 1=success, 0=not connected to dbserver directly

int dbResendPlayers(void);

int dbConnect(char *server,int port,int user_id,int cookie,char *auth_name,int no_version_check,F32 timeout,int no_timeout);
int dbDeletePlayer(int player_slot, F32 timeout);
void dbDisconnect();
char *dbGetError();
int dbhasError();
void dbPingIfConnected();

int dbclient_getQueueStatus(int *position, int *count);
void dbSendQuit();
void dbclient_resetQueueStatus();

int dbclient_getTimeSinceMessage(U32 currentTime);

// waits until dbRedeemSlot finishes
int waitForRedeemSlot(F32 timeout);
// End mkproto

int AccountIsReallyVIPAfterServerSelected(void);
int getVIPAdjustedBaseSlots(void);
int getTotalServerSlots(void);
int getSlotUnlockedPlayerCount(void);
int AvailableUnallocatedCharSlots(void);
int CanAllocateCharSlots(void);
#endif
