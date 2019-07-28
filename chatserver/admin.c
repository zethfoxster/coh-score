#include "admin.h"
#include "ChatComm.h"
#include "error.h"
#include "comm_backend.h"
#include "net_linklist.h"
#include "chatdb.h"
#include "earray.h"
#include "users.h"
#include "mathutil.h"
#include "timing.h"
#include "chatsqldb.h"

ClientLink ** admin_clients;	// list of all connected ChatAdmins
ClientLink * admin_upload;		// ChatAdmin that is currently being sent full channel/user info


bool adminUserOnline(User * user)
{
	int i, size = eaSize(&admin_clients);
	for(i=0;i<size;++i)
	{
		if(admin_clients[i]->adminUser == user)
			return 1;
	}
	return 0;
}



int adminHandleClientMsg(Packet *pak,int cmd, NetLink *link)
{
	switch(cmd)
	{
	xcase SHARDCOMM_ADMIN_CONNECT:
		{
			Packet * pkt2;
			int protocol = pktGetBits(pak, 32);
			if(protocol == CHATADMIN_PROTOCOL_VERSION)
			{
				ClientLink * client = (ClientLink*)link->userData;
				client->linkType = kChatLink_Admin;
				pak = pktCreateEx(link, SHARDCOMM_SVR_READY_FOR_LOGIN);
				pktSend(&pak, link);
			}
			else
			{
				pkt2 = pktCreateEx(link, SHARDCOMM_SVR_BAD_PROTOCOL);
				pktSendBits(pkt2, 32, CHATADMIN_PROTOCOL_VERSION);
				pktSend(&pkt2, link);

				lnkBatchSend(link);
				netRemoveLink(link);
			}
		}
	xcase SHARDCOMM_ADMIN_RECV_BATCH_OK:
		{
			if(   admin_upload 
			   && admin_upload->link == link 
			   && (admin_upload->uploadStatus & UPLOAD_WAITING_FOR_ACK))
			{
				int session = pktGetBitsPack(pak, 1);
				if(session == admin_upload->uploadPending)
					admin_upload->uploadStatus &= ~(UPLOAD_WAITING_FOR_ACK);
			}
		}
		break;
	default:
		return 0;
	}

	return 1;
}

void adminClearSent()
{
	int i;
	User **users = chatUserGetAll();
	Channel **channels = chatChannelGetAll();

	for(i = eaSize(&users)-1; i >= 0; --i)
		users[i]->adminSent = 0;

	for(i = eaSize(&channels)-1; i >= 0; --i)
		channels[i]->adminSent = 0;
}


void adminSendUser(Packet * pak, User * user)
{
	pktSendString(pak, user->name);
	pktSendBits(pak, 1, (user->link ? 1 : 0));
	pktSendBitsPack(pak, 1, userSilencedTime(user)); 
	user->adminSent = 1;
}

void adminSendChannel(Packet * pak, Channel * channel)
{
	pktSendString(pak, channel->name);
	channel->adminSent = 1;
}

bool shouldSendUser(User * user)
{
	return ( ! user->adminSent);
}

bool shouldSendChannel(Channel * channel)
{
	return ( ! channel->adminSent);
}

typedef bool(*ShouldSendFunc)(void * item);
typedef void(*SendItemFunc)(Packet * pak, void * item);

int gUploadTimer = 0;

static void sendList(void ***list, int cmd, int inProgressFlag, int completeFlag, ShouldSendFunc shouldSendFunc, SendItemFunc sendFunc)
{
	int size = eaSizeUnsafe(list);
	int idx = MIN(admin_upload->uploadHint, MAX(0, (size-1)));
	int orig = idx;	
	int loop = 0;
	int count = 0;
	Packet * pak;
	static int s_timer;

	if(!s_timer)
		s_timer = timerAlloc();

	if(!gUploadTimer)
		gUploadTimer = timerAlloc();

	timerStart(s_timer);

	admin_upload->link->compressionAllowed = 1;
	admin_upload->link->compressionDefault = 1;

	pak = pktCreateEx(admin_upload->link, cmd);

	pktSendBitsPack(pak, 1, ++admin_upload->uploadPending);

	if( ! (admin_upload->uploadStatus & inProgressFlag))
	{
		pktSendBits(pak, 1, 1);
		pktSendBitsPack(pak, 1, size);	
	}
	else
		pktSendBits(pak, 1, 0);

	while(timerElapsed(s_timer) < 0.01 || count < 100)
	{
		void * item;

		if(idx < 0 || idx >= size)
		{
			idx		= 0;
			loop	= 1;
		}
		
		if(loop && idx >= orig)
		{
			admin_upload->uploadStatus |= completeFlag;
			admin_upload->uploadHint = 0;
			break;	// we're done!!
		}

		item = (*list)[idx];
		if( shouldSendFunc(item))
		{
			pktSendBits(pak, 1, 1);
			sendFunc(pak, item);
			++count;

			if(pak->stream.size > 500000)
			{
				pktSendBits(pak, 1, 0);	
				pktSendBits(pak, 1, 0); // "not finished yet"
				pktSend(&pak, admin_upload->link);
				pak = pktCreateEx(admin_upload->link, cmd);
				admin_upload->link->compressionAllowed = 1;
				admin_upload->link->compressionDefault = 1;
				pktSendBits(pak, 1, 0);
			}
		}

		++idx;
	}

	pktSendBits(pak, 1, 0);
	pktSendBits(pak, 1, (0 != (admin_upload->uploadStatus & completeFlag)));

	pktSend(&pak, admin_upload->link);

	admin_upload->uploadHint	 = idx;
	admin_upload->uploadStatus	|= inProgressFlag;
	admin_upload->uploadStatus	|= UPLOAD_WAITING_FOR_ACK;

	timerStart(gUploadTimer);
}




void adminTick()
{
	if(!admin_upload)
	{
		int i;
		for(i=eaSize(&admin_clients)-1;i>=0;--i)
		{
			if( admin_clients[i]->uploadStatus == UPLOAD_READY)
			{
				admin_upload = admin_clients[i];
				adminClearSent();
				break;
			}
		}
		return;  // start sending on next tick
	}
	else 
	{
		// send portion of the necessary uploads

		if(admin_upload->uploadStatus & UPLOAD_WAITING_FOR_ACK)
		{	
			// waiting for client to acknowledge they received last batch of users/channels

			// timeout if they've taken too long...
			if(timerElapsed(gUploadTimer) > 30)
			{
				// terminate the connection
				netSendDisconnect(admin_upload->link, 2);
				return;	
			}
		}
		else if( ! (admin_upload->uploadStatus & UPLOAD_USERLIST_COMPLETE))
		{
			User **users = chatUserGetAll();
			sendList(	&users, 
						SHARDCOMM_SVR_USER_LIST, 
						UPLOAD_USERLIST_INPROGRESS,
						UPLOAD_USERLIST_COMPLETE,
						shouldSendUser,
						adminSendUser);
		}
		else if( ! (admin_upload->uploadStatus & UPLOAD_CHANNELLIST_COMPLETE))
		{
			Channel **channels = chatChannelGetAll();
			sendList(	&channels,
						SHARDCOMM_SVR_CHANNEL_LIST, 
						UPLOAD_CHANNELLIST_INPROGRESS,
						UPLOAD_CHANNELLIST_COMPLETE,
						shouldSendChannel,
						adminSendChannel);
		}
		else
		{
			Packet * pak = pktCreateEx(admin_upload->link, SHARDCOMM_SVR_UPLOAD_COMPLETE);
			pktSend(&pak, admin_upload->link);

			admin_upload->uploadStatus = UPLOAD_COMPLETE;			
			adminLoginFinal(admin_upload);	// user is ready to start receiving regular messages

			admin_upload = 0;

		}
	}
}