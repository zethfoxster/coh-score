#include "request_shardxfer.hpp"
#include "AccountDb.hpp"
#include "AccountServer.hpp"
#include "transaction.h"
#include "TurnstileServerCommon.h"
#include "components/earray.h"
#include "utils/file.h"
#include "utils/log.h"
#include "utils/timing.h"

RequestShardTransfer::RequestShardTransfer()
	:dst_shard_id(0), src_confirmed(false), dst_confirmed(false), step(SHARDXFER_FULFILL_COPY_SRC), player_type(0), container_txt(NULL), new_dbid(0)
{
}

bool RequestShardTransfer::Start(const void *extraData)
{
	if (owner->shardXferInProgress)
	{
		Complete(false, "ShardXferPendingOrder");
		return false;
	}

	AccountServerShard *src_shard = GetShard();
	if (!src_shard)
		return false;

	AccountServerShard* dst_shard = AccountServer_GetShardByName((char*)extraData);
	if (!dst_shard)
	{
		Complete(false, "ShardXferInvalid");		
		return false;
	}

	// make sure src and dst are different
	if (isProductionMode() && shard_id == dst_shard->shard_id)
	{
		Complete(false, "ShardXferNoChange");
		return false;
	}

	if (!(flags & ACCOUNTREQUEST_CSR)) {
		// make sure src and dst have not been turned off
		if(src_shard->shardxfer_not_a_src || dst_shard->shardxfer_not_a_dst)
		{
			Complete(false, "ShardXferInvalid");
			return false;
		}

		// make sure dst is valid for src
		int i;
		for (i = eaSize(&src_shard->shardxfer_destinations)-1; i >= 0; --i)
		{
			if(!stricmp(src_shard->shardxfer_destinations[i], dst_shard->name))
				break;
		}
		if (i < 0)
		{
			Complete(false, "ShardXferInvalid");
			return false;
		}
	}

	dst_shard_id = dst_shard->shard_id;
	owner->shardXferInProgress = true;
	return true;
}

RequestShardTransfer::~RequestShardTransfer()
{
	owner->shardXferInProgress = false;
	SAFE_FREE(container_txt);
}

AccountServerShard* RequestShardTransfer::GetDstShard()
{
	AccountServerShard *shard = AccountServer_GetShard(dst_shard_id);
	if (!devassert(shard))
	{
		Complete(false);
		return NULL;
	}

	if (!SAFE_MEMBER(shard, link.connected))
	{
		Complete(false);
		return NULL;
	}

	return shard;
}

void RequestShardTransfer::State_Validate(bool timeout)
{
	if (timeout) {
		AccountRequest::State_Validate(timeout);
		return;
	}

	AccountServerShard *src_shard = GetShard();
	if (!src_shard)
		return;

	AccountServerShard *dst_shard = GetDstShard();
	if (!dst_shard)
		return;

	Packet *pak_out = pktCreateEx(&src_shard->link, ACCOUNT_SVR_SHARDXFER);
	pktSendBitsAuto2(pak_out, order_id.u64[0]);
	pktSendBitsAuto2(pak_out, order_id.u64[1]);
	pktSendBitsAuto(pak_out, SHARDXFER_VALIDATE_SRC);
	pktSendBitsAuto(pak_out, dbid);	// qualifier
	pktSend(&pak_out, &src_shard->link);

	pak_out = pktCreateEx(&dst_shard->link, ACCOUNT_SVR_SHARDXFER);
	pktSendBitsAuto2(pak_out, order_id.u64[0]);
	pktSendBitsAuto2(pak_out, order_id.u64[1]);
	pktSendBitsAuto(pak_out, SHARDXFER_VALIDATE_DST);
	pktSendBitsAuto(pak_out, owner->auth_id);	// qualifier
	pktSendBits(pak_out, 1, flags & ACCOUNTREQUEST_BYVIP ? 1 : 0);
	pktSend(&pak_out, &dst_shard->link);
}

void RequestShardTransfer::State_Fulfill(bool timeout)
{
	AccountServerShard *src_shard = GetShard();
	if (!src_shard)
		return;

	AccountServerShard *dst_shard = GetDstShard();
	if (!dst_shard)
		return;

	switch(step)
	{
		xcase SHARDXFER_FULFILL_COPY_SRC:
		{
			Packet *pak_out = pktCreateEx(&src_shard->link, ACCOUNT_SVR_SHARDXFER);
			pktSendBitsAuto2(pak_out, order_id.u64[0]);
			pktSendBitsAuto2(pak_out, order_id.u64[1]);
			pktSendBitsAuto(pak_out, SHARDXFER_FULFILL_COPY_SRC);
			pktSendBitsAuto(pak_out, dbid);
			pktSendBitsAuto(pak_out, XFER_TYPE_PAID);
			pktSendString(pak_out, dst_shard->name);
			pktSend(&pak_out, &src_shard->link);

			if (!devassert(!container_txt))
				SAFE_FREE(container_txt);
		}
		xcase SHARDXFER_FULFILL_COPY_DST:
		{
			// ask the destination shard to create the character
			if (!devassert(container_txt && container_txt[0]))
				return;

			Packet *pak_out = pktCreateEx(&dst_shard->link, ACCOUNT_SVR_SHARDXFER);
			pktSendBitsAuto2(pak_out, order_id.u64[0]);
			pktSendBitsAuto2(pak_out, order_id.u64[1]);
			pktSendBitsAuto(pak_out, SHARDXFER_FULFILL_COPY_DST);
			pktSendString(pak_out, container_txt);
			pktSend(&pak_out, &dst_shard->link);
		}
		xcase SHARDXFER_FULFILL_COMMIT_SRC:
		{
			// ask the source shard to move auction inventory and delete the old character
			if (!devassert(new_dbid))
				return;

			Packet *pak_out = pktCreateEx(&src_shard->link,ACCOUNT_SVR_SHARDXFER);
			pktSendBitsAuto2(pak_out, order_id.u64[0]);
			pktSendBitsAuto2(pak_out, order_id.u64[1]);
			pktSendBitsAuto(pak_out, SHARDXFER_FULFILL_COMMIT_SRC);
			pktSendBitsAuto(pak_out, dbid);
			pktSendString(pak_out, dst_shard->name);
			pktSendBitsAuto(pak_out, new_dbid);
			pktSendBitsAuto(pak_out, XFER_TYPE_PAID);
			pktSend(&pak_out, &src_shard->link);
		}
		xcase SHARDXFER_FULFILL_COMMIT_DST:
		{
			// unlock the character on the destination shard
			Packet *pak_out = pktCreateEx(&dst_shard->link,ACCOUNT_SVR_SHARDXFER);
			pktSendBitsAuto2(pak_out, order_id.u64[0]);
			pktSendBitsAuto2(pak_out, order_id.u64[1]);
			pktSendBitsAuto(pak_out, SHARDXFER_FULFILL_COMMIT_DST);
			pktSendBitsAuto(pak_out, owner->auth_id);
			pktSendBitsAuto(pak_out, new_dbid);
			pktSend(&pak_out, &dst_shard->link);
		}
		xcase SHARDXFER_FULFILL_COMMIT_RECOVERY:
		{
			Packet *pak_out = pktCreateEx(&dst_shard->link,ACCOUNT_SVR_SHARDXFER);
			pktSendBitsAuto2(pak_out, order_id.u64[0]);
			pktSendBitsAuto2(pak_out, order_id.u64[1]);
			pktSendBitsAuto(pak_out, SHARDXFER_FULFILL_COMMIT_RECOVERY);
			pktSendBitsAuto(pak_out, owner->auth_id);
			pktSend(&pak_out, &dst_shard->link);
		}
		xdefault:
		{
			devassertmsg(0, "invalid shardxfer step %d", step);
			Complete(false);
		}
	}
}

bool RequestShardTransfer::ValidateTransaction() {
	if (g_accountServerState.cfg.bFreeShardXfersOnlyEnabled)
	{
		return updateShardXferTokenCount(owner) > 0;
	}
	else
	{
		return AccountClaimRequest::ValidateTransaction();
	}
}
void RequestShardTransfer::SubmitTransaction() {
	if (g_accountServerState.cfg.bFreeShardXfersOnlyEnabled)
	{
		eaiPush(&owner->shardXferTokens, timerSecondsSince2000());
		quantity = 0;
	}

	AccountClaimRequest::SubmitTransaction();
}
void handlePacketClientShardTransfer(Packet *pak_in)
{
	OrderId order_id = { pktGetBitsAuto2(pak_in), pktGetBitsAuto2(pak_in) };
	ShardXferStep step = (ShardXferStep)pktGetBitsAuto(pak_in);

	AccountRequest *request = AccountRequest::Find(order_id);
	if (!request)
	{
		LOG( LOG_ACCOUNT, LOG_LEVEL_VERBOSE, 0, "got a shardxfer packet with unknown order_id %s on step %d", orderIdAsString(order_id), step);
		return;
	}

	if(!devassert(request->reqType == kAccountRequestType_ShardTransfer))
		return;

	RequestShardTransfer *xfer = reinterpret_cast<RequestShardTransfer*>(request);

	if(step == SHARDXFER_VALIDATE_SRC || step == SHARDXFER_VALIDATE_DST)
	{
		if(!devassert(xfer->reqState == kAccountRequestState_Validate))
			return;
	}
	else
	{
		if(!devassert(xfer->reqState == kAccountRequestState_Fulfill))
			return;
		if(!devassert(xfer->step == step))
			return;
	}

	switch(step)
	{
		xcase SHARDXFER_VALIDATE_SRC:
		{
			if (pktGetBool(pak_in)) // good
			{
				xfer->src_confirmed = true;
				if (xfer->dst_confirmed)
					xfer->ChangeState(kAccountRequestState_Fulfill);
			}
			else // bad
				xfer->Complete(false, "ShardXferInvalidCharacter");
		}
		xcase SHARDXFER_VALIDATE_DST:
		{
			int unlockedUsedCount = pktGetBitsAuto(pak_in);
			int totalUsedCount = pktGetBitsAuto(pak_in);
			int accountMaxslots = pktGetBitsAuto(pak_in); // includes VIP slots
			int serverMaxSlots = pktGetBitsAuto(pak_in);
			int vipOnlyStatus = pktGetBitsAuto(pak_in);

			if (unlockedUsedCount < accountMaxslots && totalUsedCount < serverMaxSlots &&
				(!vipOnlyStatus || xfer->flags & ACCOUNTREQUEST_BYVIP))
			{
				xfer->dst_confirmed = true;
				if (xfer->src_confirmed)
					xfer->ChangeState(kAccountRequestState_Fulfill);
			}
			else // bad
				xfer->Complete(false, "ShardXferNoFreeSlots");
		}
		xcase SHARDXFER_FULFILL_COPY_SRC:
		{
			devassert(!xfer->container_txt);
			SAFE_FREE(xfer->container_txt);

			int success = pktGetBool(pak_in);
			if (!success)
			{
				xfer->Complete(false, NULL);
				break;
			}

			xfer->player_type = pktGetBool(pak_in);
			xfer->container_txt = strdup(pktGetString(pak_in));

			if (xfer->container_txt[0])
				xfer->step = SHARDXFER_FULFILL_COPY_DST;
			else // the character does not exist on src (we may be recovering from COMMIT_DST)
				xfer->step = SHARDXFER_FULFILL_COMMIT_RECOVERY;

			// run the new step
			xfer->ChangeState(kAccountRequestState_Fulfill);
		}
		xcase SHARDXFER_FULFILL_COPY_DST:
		{
			xfer->new_dbid = pktGetBitsAuto(pak_in);
			xfer->step = SHARDXFER_FULFILL_COMMIT_SRC;

			// run the new step
			xfer->ChangeState(kAccountRequestState_Fulfill);
		}
		xcase SHARDXFER_FULFILL_COMMIT_SRC:
		{
			xfer->step = SHARDXFER_FULFILL_COMMIT_DST;

			// run the new step
			xfer->ChangeState(kAccountRequestState_Fulfill);
		}
		xcase SHARDXFER_FULFILL_COMMIT_DST:
		{
			TODO(); // visiter xfer disabled
#if 0
			if (waiting->order->type == kAccountOrderType_VisitorXferOutbound || 
				waiting->order->type == kAccountOrderType_VisitorXferReturn)
			{
				AccountServerShard *src = AccountServer_GetShard(waiting->order->shardname);

				if(SAFE_MEMBER(src,link.connected))
				{
					Packet *pak_out = pktCreateEx(&src->link,ACCOUNT_SVR_SHARDXFER);
					pktSendBitsAuto2(pak_out, waiting->order->order_id.u64[0]);
					pktSendBitsAuto2(pak_out, waiting->order->order_id.u64[1]);
					pktSendBitsAuto(pak_out,SHARDXFER_FULFILL_XFER_JUMP);
					pktSendBitsAuto(pak_out,waiting->order->dbid);
					pktSend(&pak_out,&src->link);
				}
			}
#endif

			xfer->Complete(true);
		}
		xcase SHARDXFER_FULFILL_COMMIT_RECOVERY:
		{
			// there's a tiny window here where a character can be created and cleaned up on dst,
			// but the account server is never notified.  in this case, the order will be marked
			// unfulfillable. csr should verify whether or not the character transfered properly.

			bool success = pktGetBool(pak_in);
			if(success)
				xfer->Complete(true);
			else
				xfer->Complete(false, "ShardXferInvalidCharacter");
		}
		xdefault:
		{
			devassertmsg(0, "invalid shardxfer step %d", xfer->step);
			xfer->Complete(false);
		}
	}
}

/**
* accountServerInventoryUpdateShardXferTokenCount checks the history of the
* tokens and if if its past the memory time, it removes them, and returns the
* count of available transfers.
*/	
int updateShardXferTokenCount(Account *account)
{
	int count = 0;
	if (g_accountServerState.cfg.bFreeShardXfersOnlyEnabled)
	{
		int i;
		U32 currentDate = timerSecondsSince2000();
		//	loop through backwards
		for (i = eaiSize(&account->shardXferTokens)-1; i >= 0; i--)
		{
			//	clear out all the ones past the window
			if ((currentDate - account->shardXferTokens[i]) > (g_accountServerState.cfg.shardXferDayMemory*SECONDS_PER_DAY))
			{
				eaiRemove(&account->shardXferTokens, i);
			}
		}
		//	return the remaining xfers count, or 0 if negative
		count = (g_accountServerState.cfg.shardXferAllowedInMemory - eaiSize(&account->shardXferTokens));
		return max(0, count);
	}
	return 0;
}
