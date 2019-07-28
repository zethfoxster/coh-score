#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "AccountTypes.h"
#include "AccountSql.h"

C_DECLARATIONS_BEGIN

typedef struct Account Account;
typedef struct AccountProduct AccountProduct;
typedef struct PostbackMessage PostbackMessage;

typedef struct MicroTransaction {
	asql_micro_transaction mtx;
	asql_inventory inv;
	Account *account;
	const AccountProduct *product;
	PostbackMessage *message;
} MicroTransaction;

typedef struct GameTransaction {
	asql_game_transaction gtx;
	asql_inventory inv;
	Account *account;
	const AccountProduct *product;
} GameTransaction;

typedef struct MultiGameTransaction {
	OrderId order_id;
	ssize_t count;
	asql_game_transaction transactions[MAX_MULTI_GAME_TRANSACTIONS];
	asql_flexible_inventory flex_inv;
	Account *account;
	const AccountProduct *products[MAX_MULTI_GAME_TRANSACTIONS];
} MultiGameTransaction;

void Transaction_Init();
void Transaction_Shutdown();

void Transaction_MicroStartTransaction(Account *account, OrderId order_id, SkuId sku_id, const char * transaction_date, int quantity, int points, PostbackMessage * message);

OrderId Transaction_GameStartTransaction(Account *account, const AccountProduct *product, U8 shard_id, U32 ent_id, int granted, int claimed, bool csr_did_it);
OrderId Transaction_GamePurchase(Account *account, const AccountProduct *product, U8 shard_id, U32 ent_id, int quantity, bool csr_did_it);
OrderId Transaction_GamePurchaseBySkuId(Account *account, SkuId sku_id, U8 shard_id, U32 ent_id, int quantity, bool csr_did_it);
OrderId Transaction_GameClaim(Account *account, const AccountProduct *product, U8 shard_id, U32 ent_id, int quantity, bool csr_did_it);
OrderId Transaction_GameClaimBySkuId(Account *account, SkuId sku_id, U8 shard_id, U32 ent_id, int quantity, bool csr_did_it);
void Transaction_GameRecoverUnsaved(Account *account, U8 shard_id, U32 ent_id);

OrderId Transaction_MultiGameStartTransaction(Account *account, U8 shard_id, U32 ent_id, U32 subtransactionCount, const AccountProduct **products, U32 *grantedValues, U32 *claimedValues, bool csr_did_it);

// Also handles Multi-Game Transactions
void Transaction_GameRevert(Account *account, OrderId order_id);
void Transaction_GameSave(Account *account, OrderId order_id);
// Also handles Multi-Game Transactions

void Transaction_MicroFinished(bool success, MicroTransaction * transaction);
void Transaction_GameFinished(bool success, GameTransaction * transaction);
void Transaction_MultiGameFinished(bool success, MultiGameTransaction * transaction);
void Transaction_GameRecoverUnsavedCallback(bool success, Account *account, asql_game_transaction *gtx_list, int gtx_count);

// This may not be a great place to put this...
void playspan_message(PostbackMessage * message, void * data, size_t size);

C_DECLARATIONS_END

#endif
