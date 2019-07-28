/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef INVENTORY_CLIENT_H
#define INVENTORY_CLIENT_H

#include "AccountData.h"

typedef enum AccountServerStatus
{
	ACCOUNT_SERVER_UP = 0,
	ACCOUNT_SERVER_SLOW,
	ACCOUNT_SERVER_DOWN,

} AccountServerStatus;

typedef struct Packet Packet;
typedef struct Entity Entity;
void entity_ReceiveInvUpdate(Entity *e, Packet *pak);

#ifndef FINAL
bool inventoryClient_BuyProduct(U32 auth_id, SkuId sku_id, int quantity );
bool inventoryClient_PublishProduct(U32 auth_id, SkuId sku_id, bool bPublish );
#endif

AccountInventorySet*   inventoryClient_GetAcctInventorySet();
U32                    inventoryClient_GetAcctStatusFlags();
AccountServerStatus    inventoryClient_GetAcctAuthoritativeState();
U32                    inventoryClient_GetLoyaltyPointsEarned();



#endif //INVENTORY_CLIENT_H
