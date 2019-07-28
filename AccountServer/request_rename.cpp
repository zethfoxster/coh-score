#include "request_rename.hpp"
#include "AccountServer.hpp"
#include "transaction.h"
#include "utils/log.h"

bool RequestRename::Start(const void *extraData)
{
	sku_id = SKU("svrename");
	return true;
}

void RequestRename::State_Validate(bool timeout)
{
	if (timeout) {
		AccountRequest::State_Validate(timeout);
		return;
	}

	AccountServerShard *shard = GetShard();
	if (!shard)
		return;

	Packet *pak_out = pktCreateEx(&shard->link, ACCOUNT_SVR_RENAME);
	pktSendBitsAuto2(pak_out, order_id.u64[0]);
	pktSendBitsAuto2(pak_out, order_id.u64[1]);
	pktSendBool(pak_out, true);					// validating
	pktSendBitsAuto(pak_out, dbid);	// qualifier
	pktSend(&pak_out, &shard->link);
}

void RequestRename::State_Fulfill(bool timeout)
{
	AccountServerShard *shard = GetShard();
	if (!shard)
		return;

	Packet *pak_out = pktCreateEx(&shard->link, ACCOUNT_SVR_RENAME);
	pktSendBitsAuto2(pak_out, order_id.u64[0]);
	pktSendBitsAuto2(pak_out, order_id.u64[1]);
	pktSendBool(pak_out, false);				// fulfilling
	pktSendBitsAuto(pak_out, dbid);	// qualifier
	pktSend(&pak_out, &shard->link);
}

void handlePacketClientRename(Packet *pak_in)
{
	OrderId order_id = { pktGetBitsAuto2(pak_in), pktGetBitsAuto2(pak_in) };

	AccountRequest *request = AccountRequest::Find(order_id);
	if (!request)
	{
		LOG( LOG_ACCOUNT, LOG_LEVEL_VERBOSE, 0, "got a rename packet with unknown order_id %s", orderIdAsString(order_id));
		return;
	}

	if(!devassert(request->reqType == kAccountRequestType_Rename))
		return;

	if(pktGetBool(pak_in)) // validating
	{
		if(!devassert(request->reqState == kAccountRequestState_Validate))
			return;

		if(pktGetBool(pak_in)) // good
			request->ChangeState(kAccountRequestState_Fulfill);
		else // bad
			request->Complete(false, "RenameInvalidCharacter");
	}
	else // fulfilling
	{
		if(!devassert(request->reqState == kAccountRequestState_Fulfill))
			return;

		if(pktGetBool(pak_in)) // good
			request->Complete(true);
		else // bad
			request->Complete(false, "RenameInvalidCharacter");
	}
}
