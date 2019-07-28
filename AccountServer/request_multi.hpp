#ifndef REQUEST_MULTI_H
#define REQUEST_MULTI_H

#include "request.hpp"

struct RequestMultiTransactionExtraData {
	char salvageName[32]; // the name of the salvage that you need to decrement on the map when this multi-transaction succeeds.
	U32 count;
	SkuId skuIds[MAX_MULTI_GAME_TRANSACTIONS];
	U32	grantedValues[MAX_MULTI_GAME_TRANSACTIONS];
	U32 claimedValues[MAX_MULTI_GAME_TRANSACTIONS];
	U32 rarities[MAX_MULTI_GAME_TRANSACTIONS];
};

class RequestMultiTransaction : public AccountRequest {
	friend void handleTransactionFinish(AccountServerShard *shard, Packet *pak_in);	

protected:
	virtual bool Start(const void *extraData);
	virtual bool ValidateProduct();
	virtual bool ValidateTransaction();
	virtual void SubmitTransaction();
	virtual void RevertTransaction();
	virtual void State_Fulfill(bool timeout);
	virtual void Complete(bool success, const char *msg = NULL);

protected:
	RequestMultiTransactionExtraData extraData;
	const AccountProduct *subProducts[MAX_MULTI_GAME_TRANSACTIONS];
};

#endif
