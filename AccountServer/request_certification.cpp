#include "request_certification.hpp"
#include "AccountDb.hpp"
#include "AccountServer.hpp"
#include "transaction.h"
#include "utils/log.h"

void RequestCertificationGrant::State_Fulfill(bool timeout)
{
	AccountServerShard *shard = GetShard();
	if (!shard)
		return;

	Packet *pak_out = pktCreateEx(&shard->link, ACCOUNT_SVR_CERTIFICATION_GRANT_ACK);
	pktSendBitsAuto(pak_out, owner->auth_id);
	pktSendBitsAuto2(pak_out, sku_id.u64);
	pktSendBitsAuto2(pak_out, order_id.u64[0]);
	pktSendBitsAuto2(pak_out, order_id.u64[1]);
	pktSendBitsAuto(pak_out, dbid);
	pktSend(&pak_out, &shard->link);
}

void RequestCertificationClaim::State_Fulfill(bool timeout)
{
	AccountServerShard *shard = GetShard();
	if (!shard)
		return;

	Packet *pak_out = pktCreateEx(&shard->link, ACCOUNT_SVR_CERTIFICATION_CLAIM_ACK);
	pktSendBitsAuto(pak_out, owner->auth_id);
	pktSendBitsAuto2(pak_out, sku_id.u64);
	pktSendBitsAuto2(pak_out, order_id.u64[0]);
	pktSendBitsAuto2(pak_out, order_id.u64[1]);
	pktSend(&pak_out, &shard->link);
}

void handleTransactionFinish(AccountServerShard *shard, Packet *pak_in)  // same for charge, claim and refund
{
	U32 auth_id = pktGetBitsAuto(pak_in);
	OrderId order_id = { pktGetBitsAuto2(pak_in), pktGetBitsAuto2(pak_in) };
	bool success = pktGetBool(pak_in);
	AccountRequest *request;

	if (g_accountServerState.accountCertificationTestStage % 100 == 3)
	{
		int flags = g_accountServerState.accountCertificationTestStage / 100;

		if (flags % 10 == 0)
		{
			g_accountServerState.accountCertificationTestStage = 0;
		}

		if (flags / 10 == 1)
		{
			// This function doesn't really fail, so this code will treat this condition
			//   as though the message was never received.
			// You don't want to go into the if (!request) block here, 
			//   because you'll leave the request in the list, so it'll get retried
			//   even though it was completed, which isn't something that can happen normally.
		}

		return;
	}

	request = AccountRequest::Find(order_id);
	if (!request)
	{
		Account *account = AccountDb_LoadOrCreateAccount(auth_id, NULL);

		LOG(LOG_ACCOUNT, LOG_LEVEL_VERBOSE, 0, "recovering %s transaction with order_id %s", success ? "successful" : "failed", orderIdAsString(order_id));

		if (!success && !orderIdIsNull(order_id))
		{
			Transaction_GameRevert(account, order_id);
		}

		AccountServer_NotifyRequestFinished(account, kAccountRequestType_UnknownTransaction, order_id, success, "TransactionRecover");
		return;
	}

	request->Complete(success);
}
