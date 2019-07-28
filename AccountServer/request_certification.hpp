/***************************************************************************
 *     Copyright (c) 2006-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef REQUEST_CERTIFICATION_H
#define REQUEST_CERTIFICATION_H

#include "request.hpp"

class RequestCertificationGrant : public AccountGrantRequest {
	friend void handleTransactionFinish(AccountServerShard *shard, Packet *pak_in);	

	virtual void State_Fulfill(bool timeout);
};

class RequestCertificationClaim : public AccountClaimRequest {
	friend void handleTransactionFinish(AccountServerShard *shard, Packet *pak_in);	

	virtual void State_Fulfill(bool timeout);
};

#endif
