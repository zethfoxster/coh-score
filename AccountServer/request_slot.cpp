#include "request_slot.hpp"
#include "AccountDb.hpp"
#include "AccountServer.hpp"
#include "transaction.h"
#include "utils/error.h"
#include "utils/log.h"

bool RequestSlot::Start(const void *extraData)
{
	sku_id = SKU("svcslots");
	return true;
}

void RequestSlot::State_Fulfill(bool timeout)
{
	AccountServerShard *shard = GetShard();
	if (!shard)
		return;

	Packet *pak_out = pktCreateEx(&shard->link, ACCOUNT_SVR_SLOT);
	pktSendBitsAuto2(pak_out, order_id.u64[0]);
	pktSendBitsAuto2(pak_out, order_id.u64[1]);
	pktSendBitsAuto(pak_out, owner->auth_id);
	pktSend(&pak_out, &shard->link);
}

void handlePacketClientSlot(Packet *pak_in)
{	
	OrderId order_id = { pktGetBitsAuto2(pak_in), pktGetBitsAuto2(pak_in) };

	AccountRequest *request = AccountRequest::Find(order_id);
	if (!request)
	{
		LOG( LOG_ACCOUNT, LOG_LEVEL_VERBOSE, 0, "got a slot packet with unknown order_id %s", orderIdAsString(order_id));
		return;
	}

	if(!devassert(request->reqType == kAccountRequestType_Slot))
		return;

	if(pktGetBool(pak_in)) // good
	{		
		request->Complete(true);

		TODO(); // Mark the order as saved now even though the dbserver has not serialized the data yet because this currently depends on having an entity loaded
		Transaction_GameSave(request->owner, request->order_id);
	}
	else // bad
	{
		request->Complete(false);
	}
}
