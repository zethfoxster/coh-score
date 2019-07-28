/***************************************************************************
 *     Copyright (c) 2006-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef ORDER_SHARDXFER_H
#define ORDER_SHARDXFER_H

#include "request.hpp"

class RequestShardTransfer : public AccountClaimRequest {
	friend void handlePacketClientShardTransfer(Packet *pak_in);
	friend int updateShardXferTokenCount(Account *pAccount);

	// context
	U8 dst_shard_id;

	// validation
	bool src_confirmed;
	bool dst_confirmed;

	// fulfillment
	ShardXferStep step;
	bool player_type;
	char *container_txt;
	U32 new_dbid;
	
	AccountServerShard* GetDstShard();

	virtual bool Start(const void *extraData);
	virtual void State_Validate(bool timeout);
	virtual void State_Fulfill(bool timeout);
	virtual bool ValidateTransaction();
	virtual void SubmitTransaction();
	virtual ~RequestShardTransfer();

public:
	RequestShardTransfer();
};

#endif // ORDER_SHARDXFER_H
