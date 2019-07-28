#include "request_multi.hpp"

#include "AccountCatalog.h"
#include "AccountDb.hpp"
#include "AccountServer.hpp"
#include "transaction.h"

bool RequestMultiTransaction::Start(const void *extraData)
{
	memcpy(&this->extraData, extraData, sizeof(RequestMultiTransactionExtraData));
	return true;
}

bool RequestMultiTransaction::ValidateProduct()
{
	for (unsigned index = 0; index < extraData.count; index++)
	{
		subProducts[index] = accountCatalogGetProduct(extraData.skuIds[index]);
		if (!subProducts[index]) {
			Complete(false);
			return false;
		}
	}

	return true;
}

bool RequestMultiTransaction::ValidateTransaction()
{
	for (unsigned index = 0; index < extraData.count; index++)
	{
		AccountInventory *inv = AccountInventorySet_Find(&owner->invSet, extraData.skuIds[index]);
		int inv_granted = (inv ? inv->granted_total : 0) + (int)(extraData.grantedValues[index]);
		int inv_claimed = (inv ? inv->claimed_total : 0) + (int)(extraData.claimedValues[index]);
		if ((subProducts[index]->grantLimit && !(inv_granted <= subProducts[index]->grantLimit))
			|| (inv_granted < inv_claimed))
		{
			Complete(false);
			return false;
		}
	}

	return true;
}

void RequestMultiTransaction::SubmitTransaction()
{
	order_id = Transaction_MultiGameStartTransaction(owner, shard_id, dbid, extraData.count, subProducts, extraData.grantedValues, extraData.claimedValues, (flags & ACCOUNTREQUEST_CSR) != 0);
}

void RequestMultiTransaction::RevertTransaction()
{
	Transaction_GameRevert(owner, order_id);
}

void RequestMultiTransaction::State_Fulfill(bool timeout)
{
	AccountServerShard *shard = GetShard();
	if (!shard)
		return;

	Packet *pak_out = pktCreateEx(&shard->link, ACCOUNT_SVR_MULTI_GAME_TRANSACTION_ACK);
	pktSendBitsAuto(pak_out, owner->auth_id);
	pktSendString(pak_out, extraData.salvageName);
	pktSendBitsAuto2(pak_out, order_id.u64[0]);
	pktSendBitsAuto2(pak_out, order_id.u64[1]);
	pktSendBitsAuto(pak_out, dbid);
	pktSend(&pak_out, &shard->link);
}

void RequestMultiTransaction::Complete(bool success, const char *msg)
{
	if (!completed && success)
	{
		Packet *pak_out = pktCreateEx(&owner->shard->link, ACCOUNT_SVR_MULTI_GAME_TRANSACTION_COMPLETED);
		pktSendBitsAuto(pak_out, owner->auth_id);
		pktSendString(pak_out, extraData.salvageName);
		pktSendBitsAuto2(pak_out, order_id.u64[0]);
		pktSendBitsAuto2(pak_out, order_id.u64[1]);
		pktSendBitsAuto(pak_out, extraData.count);
		pktSendBitsAuto(pak_out, dbid);

		for (unsigned subIndex = 0; subIndex < extraData.count; subIndex++)
		{
			pktSendBitsAuto2(pak_out, extraData.skuIds[subIndex].u64);
			pktSendBitsAuto(pak_out, extraData.grantedValues[subIndex]);
			pktSendBitsAuto(pak_out, extraData.claimedValues[subIndex]);
			pktSendBitsAuto(pak_out, extraData.rarities[subIndex]);
		}

		pktSend(&pak_out, &owner->shard->link);
	}

	AccountRequest::Complete(success, msg);
}