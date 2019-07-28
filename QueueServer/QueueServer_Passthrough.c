#include "QueueServer_Passthrough.h"
#include "comm_backend.h"
#include "netio.h"
#include "netcomp.h"
#include "..\Common\account\AccountData.h"
#include "..\Common\entity\costume.h"

void clientPassLogin(Packet *pak_in, Packet *pak_out, GameClientLink *client, U32 ipAddress, U32 linkId)
{
	U32			game_checksum[4];

	strcpy(client->account_name, escapeString(pktGetString(pak_in)));
	pktSendString(pak_out, client->account_name);
	pktSendBitsAuto(pak_out, ipAddress);
	pktSendBitsAuto(pak_out, linkId);
	pktSendString(pak_out, client->account_name);	//both queueservercomm and clientservercomm want to consume this
	client->auth_id = pktSendGetBitsPak(pak_out, pak_in, 1);//UID
	pktSendGetBitsPak(pak_out, pak_in, 1);//protocol_version
	pktSendGetBitsArray(pak_out, pak_in, sizeof(client->auth_user_data)*8,client->auth_user_data); // only used if no auth server (for testing)
	pktSendGetBitsPak(pak_out, pak_in, 1);//no_version_check_from_client = dont_check_version
	pktSendGetStringNLength(pak_out, pak_in);//game_version
	pktSendGetBitsArray(pak_out, pak_in,8*sizeof(game_checksum),game_checksum); // game checksum
	pktSendGetBits(pak_out, pak_in, 32);//cookie

	//looks like stuff we don't use anymore, but have in login.  Keeping (for now) for safety.
	
	if (!pktEnd(pak_in))
		pktSendGetBits(pak_out, pak_in, 1);//client->link->notimeout
	if (!pktEnd(pak_in))
		pktSendGetStringNLength(pak_out, pak_in);//patchValue
	if (!pktEnd(pak_in))
		pktSendGetStringNLength(pak_out, pak_in);// No longer used!?
	if (!pktEnd(pak_in)) 
	{
		char	*systemSpecs;
		U32 slen;
		systemSpecs = pktGetZipped(pak_in, &slen);
		pktSendZipped(pak_out,slen,systemSpecs);
		free(systemSpecs);
	}
	if (!pktEnd(pak_in)) 
	{
		U32 keyed_access_level = pktSendGetBitsPak(pak_out, pak_in, 1);//keyed_access_level
		if (keyed_access_level)
			pktSendGetStringNLength(pak_out, pak_in);//issued to
	}
}

void clientPassCanStartStaticMap(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out, pak_in);//start_map_id
}

void clientPassChoosePlayer(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsPak(pak_out, pak_in, 1); // slot_idx
	pktSendGetBitsPak(pak_out, pak_in, 1); // client->local_map_ip
	pktSendGetStringNLength(pak_out, pak_in); // name of character.
	pktSendGetBitsAuto(pak_out, pak_in); // createLocation
}

void clientPassChooseVisitingPlayer(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out, pak_in);//dbid
}

void clientPassMakePlayerOnline(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsPak(pak_out, pak_in, 1); // slot_idx
}

void clientPassDeletePlayer(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsPak(pak_out, pak_in, 1);//slot_id
	pktSendGetStringNLength(pak_out, pak_in);//player_name to
}

void clientPassGetCostume(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsPak(pak_out, pak_in, 1);//slot_id
}

void clientPassGetPowersetName(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsPak(pak_out, pak_in, 1);//slot_id
}

void clientPassCharacterCounts(Packet *pak_in, Packet *pak_out)
{
}

void clientPassCatalog(Packet *pak_in, Packet *pak_out)
{
}

void clientPassLoyaltyBuy(Packet *pak_in, Packet *pak_out)
{
	pktSendGetStringNLength(pak_out, pak_in);	// auth
	pktSendGetBitsAuto(pak_out, pak_in);		// node index
}

void clientPassLoyaltyRefund(Packet *pak_in, Packet *pak_out)
{
	pktSendGetStringNLength(pak_out, pak_in);	// auth
	pktSendGetBitsAuto(pak_out, pak_in);		// node index
}

void clientPassShardXferTokenRedeem(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out, pak_in);//auth_id
	pktSendGetStringNLength(pak_out, pak_in);//name
	pktSendGetBitsAuto(pak_out, pak_in);//slot_idx
	pktSendGetStringNLength(pak_out, pak_in);//destshard
}

void clientPassRename(Packet *pak_in, Packet *pak_out)
{
	pktSendGetStringNLength(pak_out, pak_in);//old_name
	pktSendGetStringNLength(pak_out, pak_in);//new_name
}

void clientPassRenameTokenRedeem(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out, pak_in);//auth_id
	pktSendGetStringNLength(pak_out, pak_in);//name
	pktSendGetBitsAuto(pak_out, pak_in);//slot_idx
}

void clientPassResendPlayers(Packet *pak_in, Packet *pak_out)
{
}

void clientPassCheckName(Packet *pak_in, Packet *pak_out)
{
	pktSendGetStringNLength(pak_out, pak_in);//name
	pktSendGetBitsAuto(pak_out, pak_in);//lockname
}

void clientPassRedeemSlot(Packet *pak_in, Packet *pak_out)
{
}

void clientPassUnlockCharacter(Packet *pak_in, Packet *pak_out)
{
	pktSendGetStringNLength(pak_out, pak_in);//name
}

void clientPassAccountCmd(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out,pak_in);	// message_dbid
	pktSendGetBitsAuto(pak_out,pak_in);	// authid
	pktSendGetBitsAuto(pak_out,pak_in);	// dbid
	pktSendGetString(pak_out,pak_in);	// cmd
}

void clientPassQuitClient(Packet *pak_in, Packet *pak_out)
{
}

void clientPassSignNDA(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out,pak_in);		// subtype
	pktSendGetBitsAuto(pak_out,pak_in);		// newValue
}

void clientPassGetPlayNCAuthKey(Packet *pak_in, Packet *pak_out)
{
	// DBGAMECLIENT_GET_PLAYNC_AUTH_KEY
	pktSendGetBitsAuto(pak_out,pak_in);		// request_key
	pktSendGetString(pak_out,pak_in);		// auth_name
	pktSendGetString(pak_out,pak_in);		// key_data
}


////////////////////////////////////////////////////////////////////////////
// Dbserver to Client
////////////////////////////////////////////////////////////////////////////

void passPlayers(Packet *pak_in, Packet *pak_out)
{
	int slots = pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.max_slots
	int i;
	U8 auth_user_data[AUTH_BYTES];
	
	pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.bonus_slots
	pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.vip
	pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.showPremiumSlotLockNag

	for(i=0;i<slots;i++)
	{
		pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.players[i].level
		pktSendGetStringNLength(pak_out, pak_in);//db_info.players[i].name
		pktSendGetStringNLength(pak_out, pak_in);//db_info.players[i].playerslot_class
		pktSendGetStringNLength(pak_out, pak_in);//db_info.players[i].origin
		pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.players[i].db_flags
		pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.players[i].type
		pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.players[i].subtype
		pktSendGetBitsAuto(pak_out, pak_in);//db_info.players[i].typeByLocation
		pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.players[i].praetorian
		pktSendGetStringNLength(pak_out, pak_in);//db_info.players[i].primaryPower
		pktSendGetStringNLength(pak_out, pak_in);//db_info.players[i].secondaryPower
		pktSendGetBitsPak(pak_out, pak_in, 1);//costume_SetBodyType
		pktSendGetF32(pak_out, pak_in);//costume_SetScale
		pktSendGetF32(pak_out, pak_in);//costume_SetBoneScale
		pktSendGetBits(pak_out, pak_in,32);//costume_SetSkinColor (COLOR_CAST)

		pktSendGetStringNLength(pak_out, pak_in);//db_info.players[i].map_name
		pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.players[i].map_seconds
		pktSendGetBits(pak_out, pak_in,32);//seconds_till_offline
		pktSendGetBits(pak_out, pak_in,32);//last_online
		pktSendGetBits(pak_out, pak_in,2);//slot_lock
	}
	// The actual auth data from the auth server, and the overridden auth
	// bits from the DbServer itself.
	pktSendGetBitsArray(pak_out, pak_in, sizeof(auth_user_data) * 8,auth_user_data);
	pktSendGetBitsArray(pak_out, pak_in, sizeof(auth_user_data) * 8,auth_user_data);
}

void passSendPlayers(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out, pak_in);		//shardaccount_con_id
	pktSendGetStringNLength(pak_out, pak_in);	//command_str
	pktSendGetBits(pak_out, pak_in, 1);			//GRNagAndPurchase
	pktSendGetBits(pak_out, pak_in, 1);			//isBetaShard
	pktSendGetBits(pak_out, pak_in, 1);			//isVIPServer
	pktSendGetBitsPak(pak_out, pak_in, 1);		//db_info.base_slots

	passPlayers(pak_in, pak_out);			//NOTE: This does not use the SendGet signature of (out, in)!!!!
}

void passSendCostume(Packet *pak_in, Packet *pak_out)
{

	int idx = pktSendGetBitsPak(pak_out, pak_in, 1);//idx
	int numparts =pktSendGetBitsPak(pak_out, pak_in, 1);//numparts
	int i, j;
	int num2DScales, num3Dscales;
	int present = 0;

	for(i=0; i<numparts; i++)
	{
		pktSendGetBits(pak_out, pak_in, 32);//subId
		pktSendGetStringNLength(pak_out, pak_in);//pchGeom
		pktSendGetStringNLength(pak_out, pak_in);//pchTex1
		pktSendGetStringNLength(pak_out, pak_in);//pchTex2
		pktSendGetStringNLength(pak_out, pak_in);//pchName
		pktSendGetStringNLength(pak_out, pak_in);//pchFxName

		for (j=0; j<4; j++)	//4 is the size of color.
			pktSendGetBits(pak_out, pak_in, 32);//part.color[j].integer
	}

	present = pktGetBits(pak_in, 1);
	pktSendBits(pak_out, 1, present);
	if (present)
	{
		pktSendGetBitsPak(pak_out, pak_in, 1);		//	bodytype
		pktSendGetBits(pak_out, pak_in, 32);	//	skincolor

		// get appearance (bone scale) info
		num2DScales = pktSendGetBitsPak(pak_out, pak_in, 1);//num2DScales
		if(!num2DScales)	//this means that we've got a bad costume and will be leaving early, thanks.
		{
			pktSendGetBits(pak_out, pak_in,1);
			return;
		}
		num3Dscales = pktSendGetBitsPak(pak_out, pak_in, 1);//num3Dscales
		if(num2DScales && num3Dscales)
		{
			for(i=0;i<NUM_2D_BODY_SCALES && i<num2DScales;i++)
			{
				pktSendGetF32(pak_out, pak_in);//db_info.players[idx].costume->appearance.fScales[i]
			}	
			pktSendGetBits(pak_out, pak_in, 1);//db_info.players[idx].costume->appearance.convertedScale

			for(i=0;i<NUM_3D_BODY_SCALES && i<num3Dscales;i++)
			{
				pktSendGetBitsPak(pak_out, pak_in, 1);//db_info.players[idx].costume->appearance.compressedScales[i]
			}
		}
	}
}

void passSendPowersetNames(Packet *pak_in, Packet *pak_out)
{

	pktSendGetBitsPak(pak_out, pak_in, 1);		//idx

	pktSendGetStringNLength(pak_out, pak_in);	//primaryPowerset
	pktSendGetStringNLength(pak_out, pak_in);	//secondaryPowerset
}

void passSendCharacterCounts(Packet *pak_in, Packet *pak_out)
{
	int shards = pktSendGetBitsAuto(pak_out,pak_in);//shards
	if(shards)
	{
		int i;

		for(i = 0; i < shards; ++i)
		{
			pktSendGetStringNLength(pak_out, pak_in);//shard
			pktSendGetBitsAuto(pak_out,pak_in);//unlocked character count
			pktSendGetBitsAuto(pak_out,pak_in);//total character count
			pktSendGetBitsAuto(pak_out,pak_in);//account slot count, excludes VIP slots
			pktSendGetBitsAuto(pak_out,pak_in);//server slot count
			pktSendGetBitsAuto(pak_out,pak_in);//vip-only status
		}
	}
	else
		pktSendGetStringNLength(pak_out, pak_in);//error;

}

// TBD - Should use accountCatalog_RelayServerCatalogPacket() to keep all of this
// knowledge in one place. But we'd need to drag in AccountCatalog.c into the
// QueueSever. Refactor the packet code out of AccountCatalog.c into a simpler
// module that can be shared more easily? LSTL, 07/05/11.
void passSendAccountServerCatalog(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out,pak_in);			// Account Catalog time stamp
	pktSendGetBitsAuto(pak_out,pak_in);			//auth_timeout
	pktSendString(pak_out,pktGetString(pak_in));//MTX environment

	pktSendString(pak_out,pktGetString(pak_in));//playSpanDomain
	pktSendString(pak_out,pktGetString(pak_in));//playSpanCatalog
	pktSendString(pak_out,pktGetString(pak_in));//playSpanURL_Home
	pktSendString(pak_out,pktGetString(pak_in));//playSpanURL_CategoryView
	pktSendString(pak_out,pktGetString(pak_in));//playSpanURL_ItemView
	pktSendString(pak_out,pktGetString(pak_in));//playSpanURL_ShowCart
	pktSendString(pak_out,pktGetString(pak_in));//playSpanURL_AddToCart
	pktSendString(pak_out,pktGetString(pak_in));//playSpanURL_ManageAccount
	pktSendString(pak_out,pktGetString(pak_in));//playSpanURL_SupportPage
	pktSendString(pak_out,pktGetString(pak_in));//playSpanURL_SupportPageDE
	pktSendString(pak_out,pktGetString(pak_in));//playSpanURL_SupportPageFR
	pktSendString(pak_out,pktGetString(pak_in));//playSpanURL_UpgradeToVIP
	pktSendString(pak_out,pktGetString(pak_in));//cohURL_NewFeatures
	pktSendString(pak_out,pktGetString(pak_in));//cohURL_NewFeaturesUpdate
	pktSendGetBitsAuto(pak_out,pak_in);//playSpanStoreFlags
}

void passAccountServerUnavailable(Packet *pak_in, Packet *pak_out)
{
}

void passAccountServerInventoryDB(Packet *pak_in, Packet *pak_out)
{
	U8 loyaltyStatus[LOYALTY_BITS/8];
	int count, i;
	pktSendGetBits(pak_out, pak_in, 1);		//	authoritative information?
	pktSendGetBits(pak_out, pak_in, 1);		//	shardxfer free only?
	pktSendGetBitsAuto(pak_out, pak_in);	//	shardxfer tokens
	pktSendGetStringNLength(pak_out, pak_in);//shardname
	count = pktSendGetBitsAuto(pak_out,pak_in);//count

	for (i = 0; i < count; i++)
	{
		AccountInventorySendReceive(pak_in, pak_out);
	}

	pktSendGetBitsArray(pak_out, pak_in, LOYALTY_BITS, &loyaltyStatus);
	pktSendGetBitsAuto(pak_out,pak_in);		//loyaltyPointsUnspent
	pktSendGetBitsAuto(pak_out,pak_in);		//loyaltyPointsEarned
	pktSendGetBitsAuto(pak_out,pak_in);		//virtualCurrencyBal
	pktSendGetBitsAuto(pak_out,pak_in);		//lastEmailTime
	pktSendGetBitsAuto(pak_out,pak_in);		//lastNumEmailsSent
	pktSendGetBitsAuto(pak_out,pak_in);		//accountStatusFlags
	pktSendGetBitsAuto(pak_out,pak_in);		//bonus_slots
}

void passdbOrMapConnect(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsPak(pak_out, pak_in, 1); // link_id
	pktSendGetBitsPak(pak_out, pak_in, 1); //map_id
	pktSendGetBitsPak(pak_out, pak_in, 1); //ip_list[0]
	pktSendGetBitsPak(pak_out, pak_in, 1); //ip_list[1]
	pktSendGetBitsPak(pak_out, pak_in, 1); //udp_port
	pktSendGetBitsPak(pak_out, pak_in, 1); //tcp_port
	pktSendGetBitsPak(pak_out, pak_in, 1); //login_cookie
}

void passCanStartStaticMapResponse(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsPak(pak_out, pak_in, 1); // result
}

void passAccountServerClientAuth(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out,pak_in);		//timestamp
	pktSendGetString(pak_out, pak_in);//digest
}

void passAccountServerPlayNCAuthKey(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out, pak_in); // request_key
	pktSendGetString(pak_out, pak_in);//auth_key
}

void passAccountServerNotifyRequest(Packet *pak_in, Packet *pak_out)
{
	pktSendGetBitsAuto(pak_out, pak_in); // request type
	pktSendGetBitsAuto2(pak_out, pak_in); // order_id
	pktSendGetBitsAuto2(pak_out, pak_in);
	pktSendGetBool(pak_out, pak_in); // success/failure
	pktSendGetStringNLength(pak_out, pak_in); //msg
}

void passAccountServerInventory(Packet *pak_in, Packet *pak_out)
{
	int size, i;
	pktSendGetBitsAuto(pak_out,pak_in);//db_id
	pktSendGetStringNLength(pak_out, pak_in);//shardName
	size = pktSendGetBitsAuto(pak_out,pak_in);//size

	for (i = 0; i < size; i++)
	{
		pktSendGetBitsAuto(pak_out,pak_in);//type
		pktSendGetBitsAuto(pak_out,pak_in);//subtype
		pktSendGetBitsAuto(pak_out,pak_in);//count
		pktSendGetBitsAuto(pak_out,pak_in);//date
	}
}
