/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef INVENTORY_SERVER_H
#define INVENTORY_SERVER_H

typedef struct Packet Packet;
typedef struct Entity Entity;
void entity_SendInvUpdate( Entity *e, Packet *pak );

bool inventoryServer_BuyProduct( U32 auth_id, SkuId sku_id, int quantity );
bool inventoryServer_PublishProduct( U32 auth_id, SkuId sku_id, bool bPublish );

#endif //INVENTORY_SERVER_H
