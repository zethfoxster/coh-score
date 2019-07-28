#include "account_inventory.h"
#include "AccountCatalog.h"
#include "AccountDb.hpp"
#include "AccountServer.hpp"
#include "AccountSql.h"
#include "request.hpp"
#include "request_multi.hpp"
#include "transaction.h"
#include "comm_backend.h"
#include "components/earray.h"
#include "network/netio.h"
#include "utils/log.h"
#include "utils/timing.h"
#include "utils/utils.h"

MP_DEFINE(AccountInventory);

void accountInventory_Init()
{
	MP_CREATE(AccountInventory, ACCOUNT_INITIAL_CONTAINER_SIZE);
}

void accountInventory_Shutdown()
{
	MP_DESTROY(AccountInventory);
}

static AccountInventory* accountInventory_FindOrCreateInventory(AccountInventorySet *invSet, SkuId sku_id)
{
	AccountInventory *inventory = AccountInventorySet_Find(invSet, sku_id);
	if (inventory)
		return inventory;

	inventory = MP_ALLOC(AccountInventory);
	inventory->sku_id = sku_id;

	AccountInventorySet_AddAndSort(invSet, inventory);
	return inventory;
}

static void accountInventory_FreeItem(void *item)
{
	MP_FREE(AccountInventory, item);
}

static void accountInventory_Free(AccountInventorySet *invSet)
{
	if (invSet->invArr)
	{
		eaClearEx(&invSet->invArr, accountInventory_FreeItem);
		eaSetSize(&invSet->invArr, 0);
	}
}

static void accountInventory_UpdateInventory(AccountInventory *inventory, asql_inventory * inv)
{
	devassert(skuIdEquals(inventory->sku_id, inv->sku_id));

	inventory->granted_total = inv->granted_total;
	inventory->claimed_total = inv->claimed_total;
	inventory->saved_total = inv->saved_total;
	inventory->expires = timerGetSecondsSince2000FromSQLTimestamp(&inv->expires);
}

void accountInventory_LoadFinished(Account *account, asql_inventory *inv_list, int inv_count)
{
	int i;

	accountInventory_Free(&account->invSet);

	if (!account->invSet.invArr)
		eaCreateWithCapacity(&account->invSet.invArr, inv_count);

	for (i=0; i<inv_count; i++)
	{
		asql_inventory * inv = inv_list + i;

		AccountInventory * inventory = accountInventory_FindOrCreateInventory(&account->invSet, inv->sku_id);
		accountInventory_UpdateInventory(inventory, inv);
	}
}

void accountInventory_UpdateInventoryFromSQL(Account *account, asql_inventory *inv)
{
	if (!devassert(!skuIdIsNull(inv->sku_id)))
		return;

	AccountInventory *inventory = accountInventory_FindOrCreateInventory(&account->invSet, inv->sku_id);
	accountInventory_UpdateInventory(inventory, inv);

	AccountDb_MarkUpdate(account, ACCOUNTDB_UPDATE_NETWORK);
}

void accountInventory_UpdateInventoryFromFlexSQL(Account *account, asql_flexible_inventory *flex_inv)
{
	if (!flex_inv->count)
		return;

	if (flex_inv->list) {
		assert(flex_inv->list_size >= flex_inv->count);

		for (int i = 0; i < flex_inv->count; i++)
			accountInventory_UpdateInventoryFromSQL(account, flex_inv->list + i);
	} else {
		assert(!flex_inv->list_size);
		assert(flex_inv->count == 1);

		accountInventory_UpdateInventoryFromSQL(account, &flex_inv->single);
	}
}

bool accountInventory_PublishProduct(U32 auth_id, SkuId sku_id, bool bPublishIt)
{
	// fpe 7/11/11 merge note:  I assume this function is no longer needed?
	TODO();
#if 0
	const AccountProduct* pProd = accountCatalogGetProduct(sku_id);
	if ( pProd )
	{
		if ( bPublishIt )
		{
			pProd->productFlags &= ~PRODFLAG_NOT_PUBLISHED;
		}
		else
		{
			pProd->productFlags |= PRODFLAG_NOT_PUBLISHED;
		}

		// N.B. - Nothing stored persistently! TBD if we need that. 
		// (Assumption is that this is just for testing...)
		AccountServer_QueueProdForUpdateBroadcast(sku_id);
		return true;
	}
#endif
	return false;
}

// ACCOUNT_CLIENT_INVENTORY_REQUEST
void handleInventoryRequest(AccountServerShard *shard, Packet *packet_in)
{
	U32 auth_id = pktGetBitsAuto(packet_in);

	Account *account = AccountDb_GetAccount(auth_id);
	if (!devassert(account))
		return;
	if (!devassertmsg((account->shard == shard),
			"Inventory update for account %d (%s) arrived from shard %d (%s) but account is marked as being in shard %d (%s).",
			account->auth_id,
			account->auth_name,
			(( shard ) ? shard->shard_id : -1 ),
			(( shard ) ? shard->name : "unknown" ),
			(( account->shard ) ? account->shard->shard_id : -1 ),
			(( account->shard ) ? account->shard->name : "unknown" ))) {
		return;
	}

	AccountDb_MarkUpdate(account, ACCOUNTDB_UPDATE_NETWORK);
}

// ACCOUNT_CLIENT_INVENTORY_CHANGE
void handleInventoryChangeRequest(AccountServerShard *shard, Packet *packet_in)
{
	U32 auth_id;
	SkuId sku_id;
	int delta;
	Account *pAccount;
	const AccountProduct * pProduct;

	auth_id = pktGetBitsAuto(packet_in);
	sku_id.u64 = pktGetBitsAuto2(packet_in);
	delta = pktGetBitsAuto(packet_in);

	pAccount = AccountDb_GetAccount(auth_id);
	if (!devassert(pAccount))
		return;
	if (!devassertmsg((pAccount->shard == shard),
			"Inventory change for account %d (%s) arrived from shard %d (%s) but account is marked as being in shard %d (%s).",
			pAccount->auth_id,
			pAccount->auth_name,
			(( shard ) ? shard->shard_id : -1 ),
			(( shard ) ? shard->name : "unknown" ),
			(( pAccount->shard ) ? pAccount->shard->shard_id : -1 ),
			(( pAccount->shard ) ? pAccount->shard->name : "unknown" ))) {
		return;
	}

	pProduct = accountCatalogGetProduct(sku_id);
	if (!pProduct) {
		LOG(LOG_ACCOUNT, LOG_LEVEL_ALERT, LOG_CONSOLE_ALWAYS, "{\"reason\":\"dropping inventory change due to invalid product\", \"auth_id\":%d, \"sku_id\":\"%.8s\"}",
			pAccount->auth_id, sku_id.c);
		return;
	}

	Transaction_GamePurchase(pAccount, pProduct, shard->shard_id, 0, 1, false);
}

// ACCOUNT_CLIENT_SET_INVENTORY
// NOTE: Most of this logic is from handleInventoryChangeRequest()
void handleSetInventoryRequest(AccountServerShard *shard, Packet *packet_in)
{
	U32 auth_id;
	SkuId sku_id;
	int newValue;
	Account *pAccount;
	AccountInventory *pInv;
	const AccountProduct * pProduct;

	auth_id = pktGetBitsAuto(packet_in);
	sku_id.u64 = pktGetBitsAuto2(packet_in);
	newValue = pktGetBitsAuto(packet_in);

	pAccount = AccountDb_GetAccount(auth_id);
	if (!devassert(pAccount))
		return;
	if (!devassertmsg((pAccount->shard == shard),
			"Inventory 'set' request for account %d (%s) arrived from shard %d (%s) but account is marked as being in shard %d (%s).",
			pAccount->auth_id,
			pAccount->auth_name,
			(( shard ) ? shard->shard_id : -1 ),
			(( shard ) ? shard->name : "unknown" ),
			(( pAccount->shard ) ? pAccount->shard->shard_id : -1 ),
			(( pAccount->shard ) ? pAccount->shard->name : "unknown" ))) {
		return;
	}

	pProduct = accountCatalogGetProduct(sku_id);
	if (!pProduct) {
		LOG(LOG_ACCOUNT, LOG_LEVEL_ALERT, LOG_CONSOLE_ALWAYS, "{\"reason\":\"dropping inventory change due to invalid product\", \"auth_id\":%d, \"sku_id\":\"%.8s\"}",
			pAccount->auth_id, sku_id.c);
		return;
	}

	// get the old value so we can calculate the delta
	pInv = accountInventory_FindOrCreateInventory(&pAccount->invSet, sku_id);
	Transaction_GamePurchase(pAccount, pProduct, shard->shard_id, 0, newValue - pInv->granted_total, false);
}

// ACCOUNT_CLIENT_CERTIFICATION_GRANT
void handleCertificationGrant(AccountServerShard *shard, Packet *packet_in)
{
	U32 auth_id;
	SkuId sku_id;
	U32 dbid;
	int count;
	Account *pAccount;

	auth_id = pktGetBitsAuto(packet_in);
	sku_id.u64 = pktGetBitsAuto2(packet_in);
	dbid = pktGetBitsAuto(packet_in);
	count = pktGetBitsAuto(packet_in);

	if (g_accountServerState.accountCertificationTestStage % 100 == 3)
	{
		int flags = g_accountServerState.accountCertificationTestStage / 100;

		if (flags % 10 == 0)
		{
			g_accountServerState.accountCertificationTestStage = 0;
		}

		if (flags / 10 == 1)
		{
			// the rest of this function just silently fails, so this code shall follow.
		}

		return;
	}

	pAccount = AccountDb_GetAccount(auth_id);
	if (!devassert(pAccount))
		return;
	if (!devassertmsg((pAccount->shard == shard),
			"Certification grant for account %d (%s) arrived from shard %d (%s) but account is marked as being in shard %d (%s).",
			pAccount->auth_id,
			pAccount->auth_name,
			(( shard ) ? shard->shard_id : -1 ),
			(( shard ) ? shard->name : "unknown" ),
			(( pAccount->shard ) ? pAccount->shard->shard_id : -1 ),
			(( pAccount->shard ) ? pAccount->shard->name : "unknown" ))) {
		return;
	}

	AccountRequest::New(pAccount, kAccountRequestType_CertificationGrant, 0, shard->shard_id, dbid, sku_id, count, NULL);
}

#if 0
void certificationRefundAck(AccountServerShard *shard, Account *pAccount, SkuId sku_id, bool success, char * msg, OrderId order_id)
{
	NetLink *link = NULL;
	Packet *pak_out;

	// send back account inventory information
	pak_out = pktCreateEx(&shard->link, ACCOUNT_SVR_CERTIFICATION_REFUND_ACK);
	pktSendBitsAuto(pak_out, pAccount->auth_id);
	pktSendBitsAuto2(pak_out, sku_id.u64);
	pktSendBitsAuto(pak_out, success);
	pktSendString(pak_out, msg?msg:"");
	pktSendBitsArray(pak_out, ORDER_ID_BITS, order_id.c);
	pktSend(&pak_out, &shard->link);
}
#endif

void handleCertificationRefund(AccountServerShard *shard, Packet *packet_in)
{
	U32 auth_id;
	U32 dbid;
	SkuId sku_id;
	Account *pAccount;

	auth_id = pktGetBitsAuto(packet_in);
	sku_id.u64 = pktGetBitsAuto2(packet_in);
	dbid = pktGetBitsAuto(packet_in);

	if (g_accountServerState.accountCertificationTestStage % 100 == 3)
	{
		int flags = g_accountServerState.accountCertificationTestStage / 100;

		if (flags % 10 == 0)
		{
			g_accountServerState.accountCertificationTestStage = 0;
		}

		if (flags / 10 == 1)
		{
			// the rest of this function just silently fails, so this code shall follow.
		}

		return;
	}

	pAccount = AccountDb_GetAccount(auth_id);
	if (!devassert(pAccount))
		return;
	if (!devassertmsg((pAccount->shard == shard),
			"Certification refund for account %d (%s) arrived from shard %d (%s) but account is marked as being in shard %d (%s).",
			pAccount->auth_id,
			pAccount->auth_name,
			(( shard ) ? shard->shard_id : -1 ),
			(( shard ) ? shard->name : "unknown" ),
			(( pAccount->shard ) ? pAccount->shard->shard_id : -1 ),
			(( pAccount->shard ) ? pAccount->shard->name : "unknown" ))) {
		return;
	}

	devassertmsg(0, "handleCertificationRefund is not currently supported");
	//AccountRequest::New(pAccount, kAcountRequestType_CertificationRefund, 0, shard->shard_id, dbid, sku_id, count, NULL);
}

void handleCertificationClaim(AccountServerShard *shard, Packet *packet_in)
{
	static const SkuId respec_sku_id = SKU("svrespec");
	U32 auth_id;
	U32 dbid;
	SkuId sku_id;
	Account *pAccount;
	const AccountProduct * pProduct;
	AccountRequestType reqType = kAccountRequestType_CertificationClaim;
	int quantity = 1;
	
	auth_id = pktGetBitsAuto(packet_in);
	sku_id.u64 = pktGetBitsAuto2(packet_in);
	dbid = pktGetBitsAuto(packet_in);

	if (g_accountServerState.accountCertificationTestStage % 100 == 3)
	{
		int flags = g_accountServerState.accountCertificationTestStage / 100;

		if (flags % 10 == 0)
		{
			g_accountServerState.accountCertificationTestStage = 0;
		}

		if (flags / 10 == 1)
		{
			// the rest of this function just silently fails, so this code shall follow.
		}

		return;
	}

	pAccount = AccountDb_GetAccount(auth_id);
	if (!devassert(pAccount))
		return;
	if (!devassertmsg(( pAccount->shard == shard ),
			"Certification claim for account %d (%s) arrived from shard %d (%s) but account is marked as being in shard %d (%s).",
			pAccount->auth_id,
			pAccount->auth_name,
			(( shard ) ? shard->shard_id : -1 ),
			(( shard ) ? shard->name : "unknown" ),
			(( pAccount->shard ) ? pAccount->shard->shard_id : -1 ),
			(( pAccount->shard ) ? pAccount->shard->name : "unknown" ))) {
		return;
	}

	pProduct = accountCatalogGetProduct(sku_id);
	if (!devassert(pProduct))
		return;

	if (pProduct->invType == kAccountInventoryType_Certification)
		quantity = 0;
	else if (!devassert(pProduct->invType == kAccountInventoryType_Voucher))
		return;

	if (skuIdEquals(sku_id, respec_sku_id))
		reqType = kAccountRequestType_Respec;		

	AccountRequest::New(pAccount, reqType, 0, shard->shard_id, dbid, sku_id, quantity, NULL);
}

// ACCOUNT_CLIENT_MULTI_GAME_TRANSACTION
void handleMultiGameTransaction(AccountServerShard *shard, Packet *packet_in)
{
	U32 auth_id;
	U32 dbid;
	Account *pAccount;
	RequestMultiTransactionExtraData extraData;

	auth_id = pktGetBitsAuto(packet_in);
	dbid = pktGetBitsAuto(packet_in);
	extraData.count = pktGetBitsAuto(packet_in);
	if (pktGetBits(packet_in, 1))
	{
		strncpy(extraData.salvageName, pktGetString(packet_in), sizeof(extraData.salvageName));
	}
	else
	{
		extraData.salvageName[0] = '\0';
	}

	if (g_accountServerState.accountCertificationTestStage % 100 == 3)
	{
		int flags = g_accountServerState.accountCertificationTestStage / 100;

		if (flags % 10 == 0)
		{
			g_accountServerState.accountCertificationTestStage = 0;
		}

		if (flags / 10 == 1)
		{
			// the rest of this function just silently fails, so this code shall follow.
		}

		return;
	}

	pAccount = AccountDb_GetAccount(auth_id);
	if (!devassert(pAccount))
		return;
	if (!devassertmsg((pAccount->shard == shard),
			"Multi-game transaction for account %d (%s) arrived from shard %d (%s) but account is marked as being in shard %d (%s).",
			pAccount->auth_id,
			pAccount->auth_name,
			(( shard ) ? shard->shard_id : -1 ),
			(( shard ) ? shard->name : "unknown" ),
			(( pAccount->shard ) ? pAccount->shard->shard_id : -1 ),
			(( pAccount->shard ) ? pAccount->shard->name : "unknown" ))) {
		return;
	}

	for (unsigned subtransactionIndex = 0; subtransactionIndex < extraData.count; subtransactionIndex++)
	{
		extraData.skuIds[subtransactionIndex].u64 = pktGetBitsAuto2(packet_in);
		extraData.grantedValues[subtransactionIndex] = pktGetBitsAuto(packet_in);
		extraData.claimedValues[subtransactionIndex] = pktGetBitsAuto(packet_in);
		extraData.rarities[subtransactionIndex] = pktGetBitsAuto(packet_in);
	}

	AccountRequest::New(pAccount, kAccountRequestType_MultiTransaction, 0, shard->shard_id, dbid, kSkuIdInvalid, 0, &extraData);
}

void handleGenericClaim(AccountServerShard *shard, Packet *packet_in)
{
	U32 auth_id;
	U32 dbid;
	U32 reqType;
	Account *pAccount;

	auth_id = pktGetBitsAuto(packet_in);
	reqType = pktGetBitsAuto(packet_in);
	dbid = pktGetBitsAuto(packet_in);

	pAccount = AccountDb_GetAccount(auth_id);
	if (!devassert(pAccount))
		return;
	if (!devassertmsg((pAccount->shard == shard),
			"Generic claim for account %d (%s) arrived from shard %d (%s) but account is marked as being in shard %d (%s).",
			pAccount->auth_id,
			pAccount->auth_name,
			(( shard ) ? shard->shard_id : -1 ),
			(( shard ) ? shard->name : "unknown" ),
			(( pAccount->shard ) ? pAccount->shard->shard_id : -1 ),
			(( pAccount->shard ) ? pAccount->shard->name : "unknown" ))) {
		return;
	}

	if (!devassert(reqType >= 0 && reqType < kAccountRequestType_Count))
		return;

	AccountRequest::New(pAccount, (AccountRequestType)reqType, 0, shard->shard_id, dbid, kSkuIdInvalid, 1, NULL);
}

void handleShardXferTokenClaim(AccountServerShard *shard, Packet *packet_in)
{
	U32 auth_id;
	U32 dbid;
	char *pDestShard;
	Account *pAccount;
	int isVIP = 0;

	auth_id = pktGetBitsAuto(packet_in);
	dbid = pktGetBitsAuto(packet_in);
	strdup_alloca(pDestShard, pktGetString(packet_in));
	isVIP = pktGetBits(packet_in, 1);

	pAccount = AccountDb_GetAccount(auth_id);
	if (!devassert(pAccount))
		return;
	if (!devassertmsg((pAccount->shard == shard),
			"Shard transfer token claim for account %d (%s) arrived from shard %d (%s) but account is marked as being in shard %d (%s).",
			pAccount->auth_id,
			pAccount->auth_name,
			(( shard ) ? shard->shard_id : -1 ),
			(( shard ) ? shard->name : "unknown" ),
			(( pAccount->shard ) ? pAccount->shard->shard_id : -1 ),
			(( pAccount->shard ) ? pAccount->shard->name : "unknown" ))) {
		return;
	}

	if (!devassert(pDestShard && pDestShard[0]))
		return;

	if (pAccount->shardXferInProgress)
	{
		// delay this packet until the ACK is sent
		if (!AccountServer_DelayPacketUntilLater(shard, packet_in))
			LOG(LOG_ACCOUNT, LOG_LEVEL_ALERT, LOG_CONSOLE_ALWAYS, "{\"reason\":\"dropping certification refund due to pending shard transfer\", \"auth_id\":%d}", pAccount->auth_id);
		return;
	}

	AccountRequest::New(pAccount, kAccountRequestType_ShardTransfer, isVIP ? ACCOUNTREQUEST_BYVIP : 0, shard->shard_id, dbid, SKU("svsvtran"), 1, pDestShard);
}
