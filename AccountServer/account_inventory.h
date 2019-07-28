#ifndef ACCOUNT_INVENTORY_H
#define ACCOUNT_INVENTORY_H

#include "account/AccountTypes.h"

C_DECLARATIONS_BEGIN

typedef struct Account Account;
typedef struct asql_inventory asql_inventory;
typedef struct asql_flexible_inventory asql_flexible_inventory;
typedef struct MicroTransaction MicroTransaction;
typedef struct GameTransaction GameTransaction;
typedef struct AccountServerShard AccountServerShard;
typedef struct Packet Packet;

void accountInventory_Init();
void accountInventory_Shutdown();

void accountInventory_LoadFinished(Account *account, asql_inventory *inv_list, int inv_count);
void accountInventory_UpdateInventoryFromSQL(Account *account, asql_inventory *inv);
void accountInventory_UpdateInventoryFromFlexSQL(Account *account, asql_flexible_inventory *flex_inv);

bool accountInventory_PublishProduct(U32 auth_id, SkuId sku_id, bool bPublishIt);

void handleInventoryRequest(AccountServerShard *shard, Packet *packet_in);
void handleInventoryChangeRequest(AccountServerShard *shard, Packet *packet_in);
void handleSetInventoryRequest(AccountServerShard *shard, Packet *packet_in);

void handleCertificationGrant(AccountServerShard *shard, Packet *packet_in);
void handleCertificationClaim(AccountServerShard *shard, Packet *packet_in);
void handleCertificationRefund(AccountServerShard *shard, Packet *packet_in);
void handleMultiGameTransaction(AccountServerShard *shard, Packet *packet_in);

void handleShardXferTokenClaim(AccountServerShard *shard, Packet *packet_in);
void handleGenericClaim(AccountServerShard *shard, Packet *packet_in);

C_DECLARATIONS_END

#endif
