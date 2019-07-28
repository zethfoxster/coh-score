/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "net_packet.h"
#include "net_packetutil.h"

#include "entity.h"
#include "entplayer.h"
#include "player.h"

#include "timing.h"

#include "badges.h"
#include "uiBadges.h"

#include "badges_client.h"
#include "character_net.h"

#include "character_base.h"
#include "character_net.h"
#include "character_level.h"
#include "character_inventory.h"

#include "auction.h"
#include "AccountCatalog.h"
#include "dbclient.h"
#include "inventory_client.h"
#include "cmdgame.h"
#include "cmdcommon_enum.h"
#include "cmdaccountserver.h"


//------------------------------------------------------------
// receive salvage updates from the server
// -AB: created :2005 Feb 24 04:36 PM
//----------------------------------------------------------
void entity_ReceiveInvUpdate(Entity *e, Packet *pak)
{
	int num = pktGetBitsPack( pak, 1 );
	int i;
	for( i = 0; i< num ; ++i ) 
	{
		character_inventory_Receive( e->pchar , pak );
	}

	if(num)
	{
		character_SetStoredSalvageInvCurrentCount(e->pchar);
		character_SetSalvageInvCurrentCount(e->pchar);
	}
	
	if( e->pchar->auctionInvUpdated = pktGetBitsAuto(pak) )
		AuctionInventory_Recv(&e->pchar->auctionInv,pak); 
}

#ifndef FINAL
bool inventoryClient_BuyProduct(U32 auth_id, SkuId sku_id, int quantity)
{
	extern NetLink db_comm_link;
	// Pass this on to the AccountServer, adding the user name.
	char buffer[200];
	sprintf_s( buffer, ARRAY_SIZE(buffer), "acc_debug_buyproduct %d %s %d", auth_id, skuIdAsString(sku_id), quantity );
	if ( ! db_comm_link.connected )
	{
		// Send this off to the mapserver
		cmdParse(buffer);
	}
	else
	{
		// In the character creator screen
		Cmd tmpCmd = { 0 };
		tmpCmd.num = ClientAccountCmd_BuyProductFromStore;
		AccountServer_ClientCommand(&tmpCmd,0,auth_id,0,buffer);
	}
	return true;
}

bool inventoryClient_PublishProduct(U32 auth_id, SkuId sku_id, bool bPublish )
{
	extern NetLink db_comm_link;
	// Pass this on to the AccountServer, adding the user name.
	char buffer[200];
	sprintf_s( buffer, ARRAY_SIZE(buffer), "acc_debug_publish_product %s %d", skuIdAsString(sku_id), (bPublish ? 1 : 0) );
	if ( ! db_comm_link.connected )
	{
		// Send this off to the mapserver
		cmdParse(buffer);
	}
	else
	{
		// In the character creator screen
		Cmd tmpCmd = { 0 };
		tmpCmd.num = ClientAccountCmd_PublishProduct;
		AccountServer_ClientCommand(&tmpCmd,0,auth_id,0,buffer);
	}
	return true;
}
#endif

AccountInventorySet* inventoryClient_GetAcctInventorySet( void )
{
	AccountInventorySet * retVal = 0;
	if (getPCCEditingMode() > 0)
	{
		setPCCEditingMode(getPCCEditingMode() * -1);
	}
	{
		Entity* e = playerPtr();
		if (( e ) && ( e->pl ) && ( e->pl->account_inventory.invArr ))
		{
			retVal = &e->pl->account_inventory;
		}
		else
		{
			retVal = &db_info.account_inventory;
		}
	}
	if (getPCCEditingMode() < 0)
	{
		setPCCEditingMode(getPCCEditingMode() * -1);
	}
	return retVal;
}

U32 inventoryClient_GetAcctStatusFlags( void )
{
	U32 retVal = 0;
	if (getPCCEditingMode() > 0)
	{
		setPCCEditingMode(getPCCEditingMode() * -1);
	}
	{
		Entity* e = playerPtr();
		if (( e ) && ( e->pl ) && ( e->pl->account_inventory.invArr ))
		{
			retVal = e->pl->account_inventory.accountStatusFlags;
		}
		else
		{
			retVal = db_info.account_inventory.accountStatusFlags;
		}
	}
	if (getPCCEditingMode() < 0)
	{
		setPCCEditingMode(getPCCEditingMode() * -1);
	}
	return retVal;
}

U32 inventoryClient_GetLoyaltyPointsEarned()
{
	U32 retVal = 0;
	if (getPCCEditingMode() > 0)
	{
		setPCCEditingMode(getPCCEditingMode() * -1);
	}
	{
		Entity* e = playerPtr();
		if (( e ) && ( e->pl ) && ( e->pl->account_inventory.invArr ))
		{
			retVal = e->pl->loyaltyPointsEarned;
		}
		else
		{
			retVal = db_info.loyaltyPointsEarned;
		}
	}
	if (getPCCEditingMode() < 0)
	{
		setPCCEditingMode(getPCCEditingMode() * -1);
	}
	return retVal;
}

AccountServerStatus inventoryClient_GetAcctAuthoritativeState( void )
{
	Entity* e = playerPtr();
	int lastUpdate = 0;

	if (( e ) && ( e->pl ) && ( e->pl->account_inventory.invArr ))
	{
		lastUpdate = e->pl->accountInformationCacheTime;
	}
	else
	{
		lastUpdate = db_info.accountInformationCacheTime;
	}

	if (lastUpdate == 0)
		return ACCOUNT_SERVER_UP;

	lastUpdate = timerSecondsSince2000() - lastUpdate;
	
	if (lastUpdate < 300)
		return ACCOUNT_SERVER_SLOW;
	else
		return ACCOUNT_SERVER_DOWN;

}
