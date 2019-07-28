/***************************************************************************
 *     Copyright (c) 2007-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * $Workfile: $
 * $Revision: 57515 $
 * $Date: 2010-05-12 12:03:52 -0700 (Wed, 12 May 2010) $
 *
 * Module Description:
 *
 * Revision History:
 *
 ***************************************************************************/
#ifndef ACCOUNTSERVER_H
#define ACCOUNTSERVER_H

#include "net_link.h"
#include "account/AccountTypes.h"
#include "account/AccountData.h"

C_DECLARATIONS_BEGIN

typedef struct StashTableImp* StashTable;
typedef struct StaticDefineInt StaticDefineInt;
typedef struct Account Account;
typedef struct AccountProduct AccountProduct;
typedef struct PostBackRelay PostBackRelay;
class AccountRequest;

typedef struct AccountServerShard
{
	U8 shard_id;
	const char *name;
	char *ip;

	const char **shardxfer_destinations;
	bool shardxfer_not_a_src;
	bool shardxfer_not_a_dst;

	// internal (not read from config)
	NetLink link;
	U32 last_connect;
} AccountServerShard;

typedef struct AccountServerCfg
{
	// Shard configuration
	AccountServerShard **shards;

	// MTX configuration
	char mtxEnvironment[32];
	int mtxIOThreads;
	char mtxSecretKey[256];
	PostBackRelay **relays;
	
	// PlayNC authentication for web clients
	char playNCChallengeRespSecretKey[256];

	// SQL configuration
	char sqlLogin[1024];
	char sqlDbName[1024];

	U32 playSpanServerRetryFreqSecs;
	U32 playSpanServerAckAlarmSecs;					// if we don't get a message from the relay server in this # secs, log an error.
	U32 playSpanServerAckAlarmRepeatFregSecs;		// keep repeating the warning every 'n' seconds
	U32 catalogTimeStampTestOffsetDays;				// for testing; forces the server to report a different time than clock time.
	int playSpanMakeTimeStampedDigest;				// if 1, the auth digest for connecting to PlaySpan uses the current timestamp in its calc.
	U32 playSpanStoreFlags;
	U32 playSpanAuthTimeoutMins;					// playspan digest duration in minutes.  After this time, need a new digest otherwise get "session timed out" message
	U32 playSpanAuthRekeyIntervalMins;				// interval at which we create a new playspan digest with an updated auth timeout.  (At least 2x per timeout in case one gets lost)

	// Web store URL info from account_server.cfg
	char *playSpanDomain;
	char* playSpanCatalog;
	char *playSpanURL_Home;
	char *playSpanURL_CategoryView;
	char *playSpanURL_ItemView;
	char *playSpanURL_ShowCart;
	char *playSpanURL_AddToCart;
	char *playSpanURL_ManageAccount;
	char *playSpanURL_SupportPage;
	char *playSpanURL_SupportPageDE;
	char *playSpanURL_SupportPageFR;
	char *playSpanURL_UpgradeToVIP;
	char *cohURL_NewFeatures;
	char *cohURL_NewFeaturesUpdate;

	// Legacy stuff
	bool archive_merged;
	bool comp_mode;
	bool bFreeShardXfersOnlyEnabled;
    U32 clientAuthTimeout;
	U32 shardXferDayMemory;
	U32 shardXferAllowedInMemory;

} AccountServerCfg;

///////////////////////////////////////////////////////////////////////////////////////////
typedef struct AccountServerState
{
	AccountServerCfg cfg; // Read via TextParser

	char exe_name[MAX_PATH];

	AccountServerShard **shards; // shallow copy of shard config
	int shard_count; // cache of the eaSize()

	AccountServerShard **connectedShardsById; // shard config ordered by shard_id, may contain gaps
	StashTable connectedShardsByName;

	int accountCertificationTestStage;
} AccountServerState;

extern AccountServerState g_accountServerState;

bool AccountServer_DelayPacketUntilLater(AccountServerShard *shard, Packet *pak);

AccountServerShard* AccountServer_GetShard(U8 shard_id);
AccountServerShard* AccountServer_GetShardByName(const char *name);
bool AccountServer_IsCompMode(void);

void AccountServer_NotifyTransactionFinished(Account * account, const AccountProduct * product, OrderId order_id, int granted, int claimed, bool success);
void AccountServer_NotifyRequestFinished(Account * account, AccountRequestType reqType, OrderId order_id, bool success, const char *msg);
void AccountServer_NotifySaved(Account * account, OrderId order_id, bool success);
void AccountServer_NotifyAccountFound(const char *search_string, U32 found_auth_id);

U32	AccountServer_GetClientAuthTimeout(void);

// Add this to the list of products for which we need to send updates to the shards
void AccountServer_QueueProdForUpdateBroadcast(SkuId sku_id);

void takeADump(void);

/// Shamelessly copied from XmppLink.
#define HTTP_MTX_PORT 31416

#define HTTP_MTX_STATE_NONE 0
#define HTTP_MTX_STATE_LISTEN 1
#define HTTP_MTX_STATE_READY 2

#define HTTP_MTX_COMMAND_KEEPALIVE 100
#define HTTP_MTX_COMMAND_APPLY_SKU 101

#define HTTP_MTX_STATUS_OK 100
#define HTTP_MTX_STATUS_OFFLINE 101
#define HTTP_MTX_STATUS_BAD_SKU 102
#define HTTP_MTX_STATUS_BAD_AMOUNT 103
#define HTTP_MTX_STATUS_OVER_CLAIMED 104
#define HTTP_MTX_STATUS_GRANT_LIMIT 105
#define HTTP_MTX_STATUS_FAILED 106

typedef struct HttpMtx
{
	SOCKET socket;
	fd_set fd;
	int state;
	char buffer[1000];
	int size;
	int reconnectTime;
	struct timeval timeout;
} HttpMtx;


C_DECLARATIONS_END

#endif //ACCOUNTSERVER_H
