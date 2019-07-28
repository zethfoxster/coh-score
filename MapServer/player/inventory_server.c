/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "character_base.h"
#include "Auction.h"

#include "assert.h"
#include "utils.h"
#include "entPlayer.h"
#include "entity.h"
#include "entserver.h"
#include "earray.h"
#include "netio.h"
#include "timing.h"
#include "character_net.h"
#include "inventory_server.h"
#include "AccountData.h"
#include "AccountCatalog.h"
#include "cmdoldparse.h"
#include "cmdaccountserver.h"
#include "cmdcommon_enum.h"


//------------------------------------------------------------
// update the inventory by type, idx pairs
//----------------------------------------------------------
void entity_SendInvUpdate(Entity *player_ent, Packet *pak)
{
	if( verify( player_ent && player_ent->pchar && pak ))
	{		
		int *rarities = player_ent->pchar->invStatusChange.type;
		int num = eaiSize( &rarities );
		int i;
		assert( num == eaiSize( &player_ent->pchar->invStatusChange.idx ));
		
		pktSendBitsPack(pak, 1, num );
		for( i = 0; i < num; ++i ) 
		{
			int idx = player_ent->pchar->invStatusChange.idx[i];
			int type = player_ent->pchar->invStatusChange.type[i];
			character_inventory_Send(player_ent->pchar, pak, type, idx);
		}
		
		pktSendBitsAuto( pak, player_ent->pchar->auctionInvUpdated);
		if(player_ent->pchar->auctionInvUpdated)
		{
			AuctionInventory_Send(player_ent->pchar->auctionInv,pak);
		}
		player_ent->pchar->auctionInvUpdated = 0;
		
		// clear the inventory
		eaiSetSize( &player_ent->pchar->invStatusChange.idx, 0 );
		eaiSetSize( &player_ent->pchar->invStatusChange.type, 0 );
	}
}

bool inventoryServer_BuyProduct( U32 auth_id, SkuId sku_id, int quantity )
{
	// Pass this on to the AccountServer
	char buffer[200];
	Cmd tmpCmd = { 0 };

	sprintf_s( buffer, ARRAY_SIZE(buffer), "acc_debug_buyproduct %d %s %d", auth_id, skuIdAsString(sku_id), quantity );
	// In the character creator screen
	tmpCmd.num = ClientAccountCmd_BuyProductFromStore;
	AccountServer_ClientCommand(&tmpCmd,0,auth_id,0,buffer);
	return true;
}

bool inventoryServer_PublishProduct( U32 auth_id, SkuId sku_id, bool bPublish )
{
	// Pass this on to the AccountServer
	char buffer[200];
	Cmd tmpCmd = { 0 };

	sprintf_s( buffer, ARRAY_SIZE(buffer), "acc_debug_publish_product %s %d", skuIdAsString(sku_id), ( bPublish ? 1 : 0 ) );
	// In the character creator screen
	tmpCmd.num = ClientAccountCmd_PublishProduct;
	AccountServer_ClientCommand(&tmpCmd,0,auth_id,0,buffer);

	return true;
}
