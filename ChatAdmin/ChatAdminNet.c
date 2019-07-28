	#include "ChatAdminNet.h"
#include "UserList.h"
#include "ChannelList.h"
#include "ChatComm.h"
#include "netio.h"
#include "error.h"
#include "comm_backend.h"
#include "netio_core.h"
#include "ChatAdmin.h"
#include "utils.h"
#include "chatdb.h"
#include "LangAdminUtil.h"
#include "earray.h"
#include "timing.h"
#include "ChannelMonitor.h"
#include "CustomDialogs.h"
#include "crypt.h"
#include "Login.h"
#include "resource.h"
#include "WinTabUtil.h"

AdminClient gAdminClient;

void handleChatCmd(Packet * pak_in);

static NetLink chat_link;


typedef void(*ReceiveItemFunc)(Packet * pak, int sortAndFilter);
typedef void(*RemoveAllItemsFunc)();




void receiveUserAdd(Packet * pak, int sortAndFilter)
{
	char	*handle		= pktGetString(pak);
	int		online		= pktGetBits(pak,1);
	int		silenced	= pktGetBitsPack(pak,1);

	CAUserAdd(handle, online, silenced, sortAndFilter);
}

void receiveChannelAdd(Packet * pak, int sortAndFilter)
{
	char	*name = pktGetString(pak);

	CAChannelAdd(name, sortAndFilter);
}

void receiveList(Packet * pak, char * statusMsg, ReceiveItemFunc recvItem, RemoveAllItemsFunc removeAllFunc)
{
	static int s_fullCount;
	static int s_byteCount;
	static int s_itemCount;

	Packet * pak_out=0;
	int i=0;

	int session = pktGetBitsPack(pak,1);

	if(pktGetBits(pak, 1))
	{
		s_fullCount = pktGetBitsPack(pak,1);	// just an estimate
		removeAllFunc();
		setProgressBar(0);
		setStatusBar(1, localizedPrintf(statusMsg));
		loadstart_printf("Receiving list...");
		s_byteCount = 0;
		s_itemCount = 0;
	}

	while(pktGetBits(pak, 1))
	{
		++i;
		recvItem(pak, 0);
	}

	s_itemCount += i;

	setProgressBar((F32)s_itemCount / (F32) s_fullCount);

	printf("\n\t%d items, %d bytes", i,  pak->stream.size);
	s_byteCount += pak->stream.size;

	if(pktGetBits(pak, 1))
	{
		setStatusBar(1, "");
		setProgressBar(0);
		loadend_printf("..done (%d bytes, %.2f bytes each)", s_byteCount, ((F32)s_byteCount) / s_itemCount);
		setStatusBar(1, "");
	}

	// notify server that we've processed this batch
	pak_out = pktCreateEx(&chat_link, SHARDCOMM_ADMIN_RECV_BATCH_OK);
	pktSendBitsPack(pak_out, 1, session);
	pktSend(&pak_out, &chat_link);
}


int chatAdminHandleClientMsg(Packet *pak,int cmd, NetLink *link)
{
	int		ret=1;

	switch(cmd)
	{
		xcase SHARDCOMM_SVR_CMD:
			handleChatCmd(pak);
		xcase SHARDCOMM_SVR_BAD_PROTOCOL:
		{
			char buf[1000];
			U32 protocol=0;

			if (!pktEnd(pak))
				protocol = pktGetBitsPack(pak,1);

			sprintf(buf, "Chatserver protocol: %d\nChatAdmin protocol: %d", protocol, CHATADMIN_PROTOCOL_VERSION);
			MessageBox(NULL, buf, "Protocol Mismatch", MB_ICONERROR);
		}
		xcase SHARDCOMM_SVR_READY_FOR_LOGIN:
		{
			char hashStr[1000];
			gAdminClient.uploadComplete = 0;
			strcpy(hashStr, escapeData((char*)gAdminClient.hash, sizeof(gAdminClient.hash)));
			chatCmdSendf("AdminLogin", "%s %s", escapeString(gAdminClient.handle), hashStr );
		}
		xcase SHARDCOMM_SVR_USER_LIST:
			receiveList(pak, "CADownloadingUserList",  receiveUserAdd, CAUserRemoveAll); 
		xcase SHARDCOMM_SVR_CHANNEL_LIST:
			receiveList(pak, "CADownloadingChannelList",  receiveChannelAdd, CAChannelRemoveAll); 
		xcase SHARDCOMM_SVR_UPLOAD_COMPLETE:
		{
			gAdminClient.uploadComplete = 1;

			setStatusBar(1, "Preparing user list...");
			UserListFilter();
			UserListUpdateCount();
			setStatusBar(1, "Preparing channel list...");
			ChannelListFilter();
			ChannelListUpdateCount();
			setStatusBar(1, "");
			setProgressBar(0);
			EnableControls(NULL, TRUE);
		}
		break;
		default:
			log_printf("CAerrs","Unknown command %d\n",cmd);
			ret = 0;
	}
	return ret;
}

void setLoginInfo(char * server, char * handle, char * password)
{
	// handle & password info
	strncpyt(gAdminClient.server, server, SIZEOF2(AdminClient, server));
	strncpyt(gAdminClient.handle, handle, SIZEOF2(AdminClient, handle));
	cryptMD5Update(password, strlen(password));
	cryptMD5Final(gAdminClient.hash);
}

int chatAdminConnect()
{
	Packet	*pak;

	if(chatAdminConnected())
		return 1;

	if (!netConnect(&chat_link, gAdminClient.server, DEFAULT_CHATSERVER_PORT, NLT_TCP, 5, NULL)) {
		conTransf(MSG_SYS, "CAUnableToConnectToServer", gAdminClient.server);
		return 0;
	}

//	chat_link.userData = ???;

	netLinkSetMaxBufferSize(&chat_link, BothBuffers, 1*1024*1024); // Set max size to auto-grow to
	netLinkSetBufferSize(&chat_link, BothBuffers, 64*1024);
	
//	chat_link.controlCommandsSeparate = 1;
//	chat_link.compressionAllowed = 1;
//	chat_link.compressionDefault = 1;

	pak = pktCreateEx(&chat_link, SHARDCOMM_ADMIN_CONNECT);
	pktSendBits(pak, 32, CHATADMIN_PROTOCOL_VERSION);
	pktSend(&pak, &chat_link);

	lnkFlush(&chat_link);

	conPrintf(MSG_SYS, localizedPrintf("CAConnectedToServer"), gAdminClient.server);

	return 1;
}


int chatAdminDisconnect()
{
	if (!chatAdminConnected())
		return 0;
	netSendDisconnect(&chat_link,2);

	ChatAdminReset();

	return 1;
}

void chatAdminLoginFail(char * msg)
{
	if(   ! stricmp(msg, "AdminBadHandle")
	   || ! stricmp(msg, "AdminBadAccessLevel")
	   || ! stricmp(msg, "AdminBadPassword")
	   || ! stricmp(msg, "AlreadyConnectedViaChatAdmin")
	   || ! stricmp(msg, "AlreadyConnectedInGame"))
	{
		char title[1000];
		strcpy(title, localizedPrintf("AdminLoginFail"));

		MessageBox(g_hDlgMain, localizedPrintf(msg), title, MB_ICONERROR);
		LoginDlg(g_hDlgMain);
	}
}


void chatCmdAccessLevel(char * access)
{
	gAdminClient.accessLevel = atoi(access);
	EnableControls(NULL, TRUE);
	conTransf(MSG_SYS, "AdminAccessLevel", gAdminClient.accessLevel);
}

void chatAdminBadAccessLevel()
{
	MessageBox(g_hDlgMain, localizedPrintf("AdminBadAccessLevel"), localizedPrintf("AdminInvalidLogin"), MB_ICONERROR);
	LoginDlg(g_hDlgMain);
}

bool chatAdminConnected()
{
	return chat_link.socket > 0;
}

void chatAdminNetTick()
{
	gAdminClient.wasConnected = chatAdminConnected();

	lnkFlushAll();
	netLinkMonitor(&chat_link, 0, chatAdminHandleClientMsg);
	
	if(gAdminClient.wasConnected && !chatAdminConnected())
	{
		conTransf(MSG_SYS, "DisconnectedFromChatserver");
		ChatAdminReset();
	}
}


VOID CALLBACK UpdateNetRateProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	char buf[100],buf2[100],buf3[100];
	setStatusBar(4, localizedPrintf("CAStatusBarBandwidth", 
									printUnit(buf,  pktRate(&chat_link.sendHistory)),
									printUnit(buf2,  pktRate(&chat_link.recvHistory)),
									printUnit(buf3, chat_link.totalBytesRead)));

}


//-------------------------------------------------------------------
// shard comm
//-------------------------------------------------------------------


void chatCmdSendf(char * cmd, char * fmt, ...)
{
	Packet * pak;
	char str[10000];
	char * p;
	va_list ap;

	if (!chatAdminConnected())
		return;

	sprintf(str, "%s ", cmd);

	va_start(ap, fmt);

	for(p=fmt;*p;++p)
	{
		if (p[0] == '%' && p[1] == 's')
		{
			char * s = va_arg(ap,char *);
			if (!s)
				break;

			strcatf(str," \"%s\"",escapeString(s));
			++p;
		}
		else if (p[0] == '%' && p[1] == 'd')
		{
			int i = va_arg(ap,int);

			strcatf(str," \"%d\"",i);
			++p;
		}
		else 
		{
			strcatf(str, "%c", *p);
		}
	}
	va_end(ap);
	str[ARRAY_SIZE(str)-1]='\0';

	if(g_ChatDebug)
		conPrintf(MSG_DEBUG, "SEND--> %s", str);

	pak = pktCreateEx(&chat_link,SHARDCOMM_CMD);  // Note: ChatAdmin clients don't send AuthID
	pktSendString(pak,str);		

	pktSend(&pak, &chat_link);
}




//--------------------------------------------------------
// Chat Server Command Handlers
//--------------------------------------------------------

// Change chat handle/name
void chatCmdName(char * oldHandle, char * newHandle)
{
	CAUser * user = GetUserByHandle(oldHandle);
	if(user)
	{
		CAUserRename(user, newHandle);
		conTransf(MSG_FEEDBACK, "UserHandleHasBeenChangeTo", oldHandle, newHandle);	
	}
}


void chatCmdUserMsgSent(char * handle, char * msg)
{
	conPrintf(MSG_PRIVATE, "-->@%s: %s", handle, msg);
}

// ------------------------------------------------------------------------------
// SysMsg
// ------------------------------------------------------------------------------

void chatCmdFriendsListFull(char * handle, char * friendHandle)
{
	CAUser * user = GetUserByHandle(handle);
	CAUser * target= GetUserByHandle(friendHandle);

	if(user == gAdminClient.user)
		conTransf(MSG_ERROR, "MyGlobalFriendsListFull", friendHandle);
	else if(target == gAdminClient.user)
		conTransf(MSG_ERROR, "TargetGlobalFriendsListFull", handle);
}

void chatCmdYouInvited(char * channel, char * user)
{
	conTransf(MSG_FEEDBACK, "YouInvited", user, channel);
}

void chatCmdYouInvitedGroup(char * channel, char * count)
{
	conTransf(MSG_FEEDBACK, "YouInvitedGroup", atoi(count));
}

void chatCmdChannelNameNotAllowed(char * channel, char * reason)
{
	if( ! stricmp(reason, "Length"))
		conTransf(MSG_ERROR, "InvalidChannelLength", MAX_CHANNELNAME);
	else if( ! stricmp(reason, "MultipleSymbols"))
		conTransf(MSG_ERROR, "InvalidChannelMultipleSymbols");
	else
		conTransf(MSG_ERROR, "ChannelNameNotAllowed", channel);
}

void chatCmdUserNameNotAllowed(char * handle, char * reason)
{
	if( ! stricmp(reason, "Length"))
		conTransf(MSG_ERROR, "InvalidHandleLength", MAX_PLAYERNAME);
	else if( ! stricmp(reason, "MultipleSymbols"))
		conTransf(MSG_ERROR, "InvalidHandleMultipleSymbols");
	else
		conTransf(MSG_ERROR, "UserNameNotAllowed", handle);
}

void ignoringHelper(char * handle, int print)
{
	CAUser * user = GetUserByHandle(handle);
	if(user)
	{
		if(eaFind(&gAdminClient.ignores, user) < 0)
			eaPush(&gAdminClient.ignores, user);
		if(print)
			conTransf(MSG_FEEDBACK, "Ignoring", handle);
	}
	else
		conPrintf(MSG_DEBUG, "Error: Received 'Ignoring' command for user not in list (%s)", handle);
}

void chatCmdIgnoring(char * handle, char * db_id)
{
	ignoringHelper(handle, 1);
}

void chatCmdIgnoringUpdate(char * handle)	// same as "Ignoring" but don't print message
{
	ignoringHelper(handle, 0);
}

void chatCmdUnignore(char * handle)
{
	int idx;
	CAUser * user = GetUserByHandle(handle);

	if(user)
	{
		if((idx = eaFind(&gAdminClient.ignores, user)) >= 0)
		{
			eaFindAndRemove(&gAdminClient.ignores, user);		
			conTransf(MSG_FEEDBACK, "StoppedIgnoring", user->handle);
			return;
		}
	}

	conPrintf(MSG_DEBUG, "Error: Ignore list might be out of sync... attempted to remove user '%s'", handle);
}

void chatCmdAdminSilenced(char * handle, char * minuteStr)
{	
	int minutes		= atoi(minuteStr);
	CAUser * user	= GetUserByHandle(handle);

	if(!user)
		return;

	user->silenced = timerSecondsSince2000() + (minutes * 60);
	CAUserUpdate(user);
}

void chatCmdSilenced(char * handle, char * minuteStr)
{
	int minutes = atoi(minuteStr);
	int days = minutes / (60 * 24);

	CAUser * user = GetUserByHandle(handle);

	if(!user)
		return;
	
	if(user == gAdminClient.user)
	{
		if(days)
			conTransf(MSG_FEEDBACK, "YouHaveBeenSilencedDays", days);
		else
			conTransf(MSG_FEEDBACK, "YouHaveBeenSilencedMinutes", minutes);
	}
	else
	{
		if(days)
			conTransf(MSG_FEEDBACK, "YouHaveSilencedUserDays", handle, days);
		else
			conTransf(MSG_FEEDBACK, "YouHaveSilencedUserMinutes", handle, minutes);
	}
}

void chatCmdNotifySilenced(char * minStr)
{
	if(gAdminClient.user)
	{
		int min = atoi(minStr);
		int hour = min / 60;
		min =  min % 60;
		conTransf(MSG_FEEDBACK, "NofityGlobalChatSilenced", gAdminClient.user->handle, hour, min);
	}
}


void chatCmdInvalidUser(char * handle)
{
	CAUser * user = GetUserByHandle(handle);

	if(!user)
		return;

	if(user == gAdminClient.user)
		conTransf(MSG_ERROR, "InvalidUserSelf");
	else
		conTransf(MSG_ERROR, "InvalidUser", handle);	
}

void chatCmdUserMsg(char * handle, char * msg)
{
	conPrintf(MSG_PRIVATE, "@%s: %s", handle, msg);
}


void chatCmdStoredMsg(char * secStr, char * handle, char * msg)
{
	char dateStr[200];

	timerMakeDateStringFromSecondsSince2000(dateStr, atoi(secStr));
	conPrintf(MSG_PRIVATE, "%s Message From @%s : %s", dateStr, handle, msg);
}



void addChatChannelSysMsg(char * channelName, char * msg)
{
	ChanMonChannelMsg(channelName, msg, MSG_SYS);
	conPrintf(MSG_SYS, msg);
}


void chatCmdUserModeChange(char * channel, char * admin, char * target, char * mode)
{
	CAUser * user = GetUserByHandle(target);

	if(!user)
		return;

	if(user == gAdminClient.user)
	{
		// action done to you
		if(strstri(mode, "-join"))
			addChatChannelSysMsg(channel, localizedPrintf("YouWereKickedFromChannel", channel));
		if(strstri(mode, "-send"))
			addChatChannelSysMsg(channel, localizedPrintf("YouWereSilencedInChannel", channel));
		if(strstri(mode, "+send"))
			addChatChannelSysMsg(channel, localizedPrintf("YouWereUnilencedInChannel", channel));
		if(strstri(mode, "+operator"))
			addChatChannelSysMsg(channel, localizedPrintf("YouBecameChannelOperator", channel));
	}
	else
	{
		if(strstri(mode, "-join"))
			addChatChannelSysMsg(channel, localizedPrintf("UserKickedFromChannel", target, channel));
		if(strstri(mode, "-send"))
			addChatChannelSysMsg(channel, localizedPrintf("UserSilencedInChannel", target, channel));
		if(strstri(mode, "+send"))
			addChatChannelSysMsg(channel, localizedPrintf("UserUnsilencedInChannel", target, channel));
		if(strstri(mode, "+operator"))
			addChatChannelSysMsg(channel, localizedPrintf("UserBecameChannelOperator", target, channel));
	}
}

void chatCmdNoPermissionToSendInChannel(char * channel)
{
	ChanMonChannelMsg(channel, localizedPrintf("NoPermissionToSendInChannel", channel), MSG_ERROR);
}



int computeFlags(char * flagStr)
{
	int flag = 0;

	if(strstri(flagStr, "o"))
		flag |= CHANFLAGS_OPERATOR;
	if( ! strstri(flagStr, "s"))
		flag |= CHANFLAGS_SEND;
	if(strstri(flagStr, "j"))
		flag |= CHANFLAGS_JOIN;
	if(strstri(flagStr, "a"))
		flag |= CHANFLAGS_ADMIN;

	return flag;
}

// debug only (copied from chatserver\channels.c)
char * flagStr(int flags)
{
	static char	buf[20];
	strcpy(buf, "");

	if (flags & CHANFLAGS_OPERATOR)
		strcat(buf,"O");
	if (! (flags & CHANFLAGS_SEND))
		strcat(buf,"S");
	if (flags & CHANFLAGS_JOIN)
		strcat(buf,"J");
	if (flags & CHANFLAGS_ADMIN)
		strcat(buf,"A");

	return &buf[0];
}

void chatCmdChannel(char * channelName, char * flags)
{
	CAChannel * channel = GetChannelByName(channelName);

	if(channel)
		channel->flags = computeFlags(flags);
}


void chatCmdJoin(char * channelName, char * handle, char * db_id, char * flags, char * refresh)
{
	CAChannel * channel = GetChannelByName(channelName);
	CAUser * user = GetUserByHandle(handle);
	DLGHDR *pHdr;
	int iCount, iSel, retn;

	if(!user || !channel)
		return;

	if(user == gAdminClient.user)
	{
		if( ! GetChanMon(channel))
		{
			conTransf(MSG_FEEDBACK, "JoinedChannelNotification", channelName);
			ChanMonTabCreate(channel);

			// TODO: create new tab for channel HERE

			pHdr = (DLGHDR *) GetWindowLong(g_hDlgMain, GWL_USERDATA); 
			iCount = TabCtrl_GetItemCount( pHdr->hwndTab );
			
			for ( iSel = 0; iSel < iCount; ++iSel )
			{
				retn = stricmp( pHdr->eaTabs[iSel]->title, localizedPrintf( "'%s'", channel->name ) );
				if ( retn == 0 )
				{
					TabCtrl_SetCurSel( pHdr->hwndTab, iSel );
					TabDlgOnSelChanged( g_hDlgMain );
					break;
				}
			}
			//int iSel = TabCtrl_GetCurSel(pHdr->hwndTab);
		}
	}

	ChanMonJoin(channel, user, computeFlags(flags));
}


void chatCmdLeave(char * channelName, char * handle, char *isKicking)
{
	CAChannel * channel = GetChannelByName(channelName);
	CAUser * user = GetUserByHandle(handle);

	if(channel && user)
	{
//		CAChannelRemoveMember(channel, user);

		ChanMonLeave(channel, user);

		if(user == gAdminClient.user)
		{			
			if(!gAdminClient.disableFeedback)
			{
				if (atoi(isKicking))
					conTransf(MSG_FEEDBACK, "KickedFromChannelNotification", channelName);
				else
					conTransf(MSG_FEEDBACK, "LeftChannelNotification", channelName);
			}	
		}
	}
}


void sendChannelInviteAccept(char * channelName)
{
	chatCmdSendf("join", "%s 0", channelName);
	free(channelName);
}

void sendChannelInviteDecline(char * channelName)
{
	chatCmdSendf("leave", "%s", channelName);
	free(channelName);
}

void chatCmdInvite(char * channelName, char * inviter)
{
	CreateDialogYesNo(g_hDlgMain, "Title", localizedPrintf("OfferChannelInvite", inviter, channelName ), (dialogHandler1) sendChannelInviteAccept, (dialogHandler1) sendChannelInviteDecline, strdup(channelName));	
}

void chatCmdInviteReminder(char * channelName)
{
	CreateDialogYesNo(g_hDlgMain, "Title", localizedPrintf("OfferChannelInviteReminder", channelName ), (dialogHandler1) sendChannelInviteAccept, (dialogHandler1) sendChannelInviteDecline, strdup(channelName));	
}

void updateStatusBar()
{
	if(gAdminClient.user)
	{
		if(gAdminClient.invisible)
			setStatusBar(0, localizedPrintf("CAUserStatusInvisible", gAdminClient.user->handle));
		else
			setStatusBar(0, localizedPrintf("CAUserStatusVisible", gAdminClient.user->handle));
	}
}

// Login
// SYNTAX: <name>
void chatCmdLogin(char * refresh, char * handle, char * shard)
{
	CAUser * user = GetUserByHandle(handle);

	if(user)
	{
		gAdminClient.user = user;
		gAdminClient.disableFeedback = atoi(refresh);

		// flush all client data
		//conPrintf(MSG_DEBUG, "Chatserver login: %s", handle);

		eaSetSize(&gAdminClient.ignores, 0);
		eaSetSize(&gAdminClient.friends, 0);

		// get rid of outstanding invitations / prompts
		// TODO...

		gAdminClient.canChangeHandle = 0;

		// print chat handle
		if( ! gAdminClient.disableFeedback)
			conTransf(MSG_FEEDBACK, "UsingChatHandleMessage", user->handle);

		updateStatusBar();
	}
}


void chatCmdLoginEnd()
{
	gAdminClient.disableFeedback = 0;
}

void chatCmdRename()
{
	gAdminClient.canChangeHandle = 1;
}



void chatCmdChanMsg(char * channelName,  char * userName, char * msg)
{
	CAChannel * channel = GetChannelByName(channelName);

	if(channel)
	{
		char * str = malloc(strlen(channelName) + 3 + strlen(userName) + strlen(msg) + 3);
		sprintf(str, "%s: %s", userName, msg);
		ChanMonChannelMsg(channelName, str, MSG_CHANNEL);
		free(str);
	}
}

void chatCmdChanMotd(char *secStr, char * channelName, char * postingUserName, char *msg)
{
	CAChannel * channel = GetChannelByName(channelName);

	if(channel)
	{
		char dateStr[200];
		char * str;
		
		timerMakeDateStringFromSecondsSince2000(dateStr, atoi(secStr));
		str = malloc(strlen(channelName) + 12 + strlen(dateStr) + strlen(msg) + 3);
		sprintf(str, "[%s] MOTD (%s): %s", channelName, dateStr, msg);

		ChanMonChannelMsg(channelName, str, MSG_CHANNEL);

		free(str);
	}
}

void chatCmdChanMember(char * channelName, char * userName)
{
	CAChannel * channel = GetChannelByName(channelName);

	if(channel)
	{
		char * str = malloc(strlen(channelName) + 5 + strlen(userName) + 3);
		sprintf(str, "[%s] <%s>", channelName, userName);

		ChanMonChannelMsg(channelName, str, MSG_CHANNEL);

		free(str);
	}
}

void chatCmdChanDesc(char * channelName, char *description)
{
	CAChannel * channel = GetChannelByName(channelName);

	if(channel)
	{
		char * str = malloc(strlen(channelName) + 5 + strlen(description) + 3);
		sprintf(str, "[%s] : %s", channelName, description);
		ChanMonChannelMsg(channelName, str, MSG_CHANNEL);
		free(str);
	}
}

void chatCmdWatching(char * channelName)
{
	CAChannel * channel = GetChannelByName(channelName);
	if(channel)
	{
		int i;
		int count = eaSize(&channel->members);
		conTransf(MSG_FEEDBACK, "WatchingChannelMembers", channelName, count);
		for(i=0;i<count;i++)
		{
			conPrintf(MSG_FEEDBACK, "\t\t--> %s", channel->members[i]->user->handle);
		}
	}
}

void chatCmdChannelKill(char * channelName, char * handle)
{
	conTransf(MSG_FEEDBACK, "ChannelClosedByAdmin", channelName);	
}


void chatCmdCsrSendAll(char * handle, char * msg)
{
	conTransf(MSG_ADMIN, "@%s: %s", handle, msg);	
}



void chatCmdInvisible()
{
	conTransf(MSG_FEEDBACK, "YouAreInvisibleMessage");
	gAdminClient.invisible = 1;
	updateStatusBar();
	EasyCheckMenuItem(IDM_STATUS_INVISIBLE, gAdminClient.invisible);
}

void chatCmdVisible()
{
	// only display message on state change, since this is the normal status
	if(gAdminClient.invisible)
		conTransf(MSG_FEEDBACK, "YouAreVisibleMessage");

	gAdminClient.invisible = 0;

	updateStatusBar();
	EasyCheckMenuItem(IDM_STATUS_INVISIBLE, gAdminClient.invisible);
}


// --------------------------------------------
// FRIEND STUFF
// --------------------------------------------




//SYNTAX: friend <STATUS> <SHARD>
void chatCmdFriend(char * handle, char * db_id, char * status, char * shard)
{
	// for now, just keep a list

	CAUser * user = GetUserByHandle(handle);
	if(user)
	{
		if(eaFind(&gAdminClient.friends, user) < 0)
		{
			eaPush(&gAdminClient.friends, user);
			if(!gAdminClient.disableFeedback)
				conTransf(MSG_FEEDBACK, "GlobalFriendAdd", handle);
		}
	}
}


void chatCmdCsrStatus(char * handle, char * shard, char * status, char * silencedMin, char * auth_id, char * channels)
{
	char buf[MAX_FRIENDSTATUS];
	char * args[100];
	int count;

	char buf2[1024];
	char * channel_args[100];
	int channel_count;

	// get rest of status info
	strcpy(buf, unescapeString(status));
	count = tokenize_line_safe(buf,args,ARRAY_SIZE(args),0);	// skip the first double-quote

	strcpy(buf2, unescapeString(channels));
	channel_count = tokenize_line_safe(buf2,channel_args,ARRAY_SIZE(channel_args),0);
	
	UserListStatusUpdate(handle, auth_id, shard, silencedMin, args, count, channel_args, channel_count);
}

void chatCmdUnfriend(char * handle)
{
	CAUser * user = GetUserByHandle(handle);
	if(user)
	{
		if(eaFindAndRemove(&gAdminClient.friends, user) >= 0)
			conTransf(MSG_FEEDBACK, "GlobalFriendRemove", handle);
	}
}

void sendGlobalFriendDecline(char * handle)
{
	chatCmdSendf("unfriend",  "%s", handle);
	free(handle);
}

void sendGlobalFriendAccept(char * handle)
{
	chatCmdSendf("friend",  "%s", handle);
	free(handle);
}

void chatCmdFriendReq(char * handle)
{
	CreateDialogYesNo(g_hDlgMain, "Title", localizedPrintf("OfferGlobalFriend", handle ), (dialogHandler1) sendGlobalFriendAccept, (dialogHandler1) sendGlobalFriendDecline, strdup(handle));	
}



void chatCmdChannelAdd(char * channelName)
{
	CAChannel * channel = GetChannelByName(channelName);
	if(! channel)
		CAChannelAdd(channelName, 1);
}


void chatCmdChannelRemove(char * channelName)
{
	CAChannel * channel = GetChannelByName(channelName);
	if(channel)
		CAChannelRemove(channel);
}


void chatCmdUserAdd(char * handle)
{
	CAUser * user = GetUserByHandle(handle);
	if(! user)
		CAUserAdd(handle, 0, 0, 1);
}

void chatCmdUserOnline(char * handle)
{
	CAUser * user = GetUserByHandle(handle);
	if(user)
	{
		user->online = 1;
		CAUserUpdate(user);
	}
}

void chatCmdUserOffline(char * handle)
{
	CAUser * user = GetUserByHandle(handle);
	if(user)
	{
		user->online = 0;
		CAUserUpdate(user);
	}
}





typedef void (*handler0)();
typedef void (*handler1)(char *cmd1);
typedef void (*handler2)(char *cmd1,char *cmd2);
typedef void (*handler3)(char *cmd1,char *cmd2,char *cmd3);
typedef void (*handler4)(char *cmd1,char *cmd2,char *cmd3, char *cmd4);
typedef void (*handler5)(char *cmd1,char *cmd2,char *cmd3, char *cmd4, char * cmd5);
typedef void (*handler6)(char *cmd1,char *cmd2,char *cmd3, char *cmd4, char * cmd5, char * cmd6);

typedef struct PrimaryChatCmd PrimaryChatCmd;
typedef struct{
	
	char				*cmdname;
	void				(*handler)();
	int					msgType;
	U32					cmd_sizes[6];
	char				*altMsg;
	PrimaryChatCmd		*sub_cmd;
	int					num_args;

} ChatCmd;

#define MAX_CHAT_CMDS (200)

typedef struct PrimaryChatCmd{
	
	ChatCmd cmds[MAX_CHAT_CMDS];
	StashTable names;

	bool flipFirstArgs;	// hack to get around the fact that channel system messages send 'channel name'	before message 'type'

}PrimaryChatCmd;

static PrimaryChatCmd channelCmds = 
{
	{
		{	"NoPermissionToSendInChannel",		chatCmdNoPermissionToSendInChannel, 0, {MAX_CHANNELNAME}	},
		{	"UserModeChange",					chatCmdUserModeChange,				0, {MAX_CHANNELNAME, MAX_PLAYERNAME, MAX_PLAYERNAME, 200}	},

		{ 0 },
	}
};


static PrimaryChatCmd systemCmds = 
{
	{
		{	"Ignoring",							chatCmdIgnoring,				0,	{MAX_CHANNELNAME}	},
		{	"IgnoringUpdate",					chatCmdIgnoringUpdate,			0,	{MAX_CHANNELNAME}	},
		{	"StoppedIgnoring",					chatCmdUnignore,				0,	{MAX_PLAYERNAME}	},

		{	"UserNameNotAllowed",				chatCmdUserNameNotAllowed,		0,	{MAX_PLAYERNAME, 64}	},
		{	"ChannelNameNotAllowed",			chatCmdChannelNameNotAllowed,	0,	{MAX_PLAYERNAME, 64}	},

		{	"YouInvited",						chatCmdYouInvited,				0,	{MAX_CHANNELNAME, MAX_PLAYERNAME}},
		{	"YouInvitedGroup",					chatCmdYouInvitedGroup,			0,	{MAX_CHANNELNAME, 64}	},
		
		{	"FriendsListFull",					chatCmdFriendsListFull,			0,	{MAX_PLAYERNAME, MAX_PLAYERNAME}},
		{	"Silenced",							chatCmdSilenced,				0,	{MAX_PLAYERNAME, 64}},
		{	"AdminSilenced",					chatCmdAdminSilenced,			0,	{MAX_PLAYERNAME, 64}},
		{	"NotifySilenced",					chatCmdNotifySilenced,			0,	{64}},
		
		{	"PlayerNotOnlineMessageQueued",		0,	MSG_ERROR,		{MAX_PLAYERNAME}},
		{	"UnableToJoinChannelFull",			0,	MSG_ERROR,		{MAX_CHANNELNAME}},
		{	"UnableToJoinPrivateChannel",		0,	MSG_ERROR,		{MAX_CHANNELNAME}},
		{	"CreateChannelAlreadyExists",		0,	MSG_ERROR,		{MAX_CHANNELNAME}},
		{	"ChannelDoesNotExist",				0,	MSG_ERROR,		{MAX_CHANNELNAME}},
		{	"YouAreNotFriendsWith",				0,	MSG_ERROR,		{MAX_PLAYERNAME}},
		{	"YouAreAlreadyFriendsWith",			0,	MSG_ERROR,		{MAX_PLAYERNAME}},
		{	"NotifyGlobalFriendDecline",		0,	MSG_FEEDBACK,	{MAX_PLAYERNAME}},
		{	"SentGlobalFriendReqest",			0,	MSG_FEEDBACK,	{MAX_PLAYERNAME}},
		{	"CannotInviteMaxInvites",			0,	MSG_ERROR,		{MAX_PLAYERNAME, MAX_CHANNELNAME}},
		{	"CannotInviteAlreadyInvited",		0,	MSG_ERROR,		{MAX_PLAYERNAME, MAX_CHANNELNAME}},
		{	"CannotInviteAlreadyMember",		0,	MSG_ERROR,		{MAX_PLAYERNAME, MAX_CHANNELNAME}},
		{	"PlayerNotOnlineMailboxFull",		0,	MSG_ERROR,		{MAX_PLAYERNAME}},
		{	"WatchingTooManyChannels",			0,	MSG_ERROR,		{MAX_CHANNELNAME}},
		{	"NotInChannel",						0,	MSG_ERROR,		{MAX_CHANNELNAME}},	
		{	"InvalidMode",						0,	MSG_ERROR,		{128}},
		{	"HandleAlreadyInUse",				0,	MSG_ERROR,		{MAX_PLAYERNAME},	"CsrHandleAlreadyInUse"},
		{	"CsrHandleAlreadyInUse",			0,	MSG_ERROR,		{MAX_PLAYERNAME}},
		{	"UnignoredBy",						0,	MSG_FEEDBACK,	{MAX_PLAYERNAME}},
		{	"IgnoredBy",						0,	MSG_FEEDBACK,	{MAX_PLAYERNAME}},
		{	"IgnoreListFullListeningTo",		0,	MSG_FEEDBACK,	{MAX_PLAYERNAME}},
		{	"CannotInviteMaxChannels",			0,	MSG_FEEDBACK,	{MAX_PLAYERNAME}},
		{	"InvalidCommand",					0,	MSG_ERROR,		{200},				"BadCommand"},
		{	"WrongNumberOfParams",				0,	MSG_ERROR,		{200},				"WrongParams"},
		{	"ParameterTooLong",					0,	MSG_ERROR,		{200},				"ParamTooLong"},
		{	"CannotInviteMaxMembers",			0,	MSG_ERROR,		{MAX_CHANNELNAME}},
		{	"AlreadyRenamed",					0,	MSG_ERROR,		{200},				"ChatHandleAlreadyRenamed"},
		{	"NotAllowed",						0,	MSG_ERROR,		{200},				"ChatActionNotAllowed"},

		{	"CannotCsrInviteAlreadyMember",		0,	MSG_ERROR,		},

		{	"AdminSilenceAll",					0,	MSG_FEEDBACK,	},
		{	"AdminUnsilenceAll",				0,	MSG_FEEDBACK,	},

		{ 0 },
	}
};

static PrimaryChatCmd mainCmds =
{
	{
		{	"SysMsg",			0, 0, {0}, 0, &systemCmds		},
		{	"ChanSysMsg",		0, 0, {0}, 0, &channelCmds		},

		{	"Disconnect",		chatAdminDisconnect,	},
		{	"AdminLoginFail",	chatAdminLoginFail,		0,	100},		
		{	"KickedByGameLogin",				 0,		MSG_SYS	},		

		{	"Name",				chatCmdName,			0, {MAX_PLAYERNAME, MAX_PLAYERNAME}},	
		{	"Login",			chatCmdLogin,			0, {32, MAX_PLAYERNAME, MAX_PLAYERNAME}},	
		{	"LoginEnd",			chatCmdLoginEnd,		},	
		{	"Renameable",		chatCmdRename			},	
		{	"AccessLevel",		chatCmdAccessLevel,		0,	64},

		{	"UserMsg",			chatCmdUserMsg,			0, {MAX_PLAYERNAME, MAX_MESSAGE}},
		{	"StoredMsg",		chatCmdStoredMsg,		0, {64, MAX_PLAYERNAME, MAX_MESSAGE}},


		{	"Channel",			chatCmdChannel,			0, {MAX_CHANNELNAME, 64}},
		{	"Join",				chatCmdJoin,			0, {MAX_CHANNELNAME, MAX_PLAYERNAME, 64, 64, 1}},
	//	{	"Reserve",			chatCmdReserve,			0, {MAX_CHANNELNAME}},
		{	"Leave",			chatCmdLeave,			0, {MAX_CHANNELNAME, MAX_PLAYERNAME, 10}},
		{	"Invite",			chatCmdInvite,			0, {MAX_CHANNELNAME, MAX_PLAYERNAME}},
		{	"InviteReminder",	chatCmdInviteReminder,	0, {MAX_CHANNELNAME}},
		{	"ChanMsg",			chatCmdChanMsg,			0, {MAX_CHANNELNAME, MAX_PLAYERNAME,MAX_MESSAGE}},
		{	"ChanMotd",			chatCmdChanMotd,		0, {100, MAX_CHANNELNAME, MAX_PLAYERNAME, MAX_MESSAGE}},
		{	"ChanDesc",			chatCmdChanDesc,		0, {MAX_CHANNELNAME, MAX_MESSAGE}},
		{	"chanmember",		chatCmdChanMember,		0, {MAX_CHANNELNAME, MAX_PLAYERNAME}},

		{	"ChanKill",			chatCmdChannelKill,		0, {MAX_CHANNELNAME, MAX_PLAYERNAME}},
		{	"CsrSendAll",		chatCmdCsrSendAll,		0, {MAX_MESSAGE, MAX_MESSAGE}},
		{	"CsrStatus",		chatCmdCsrStatus,		0, {MAX_PLAYERNAME, 64, MAX_FRIENDSTATUS, 64, 64, 1024}},
		{	"Watching",			chatCmdWatching,		{MAX_CHANNELNAME}},

		{	"Invisible",		chatCmdInvisible		},
		{	"Visible",			chatCmdVisible			},

		{	"FriendReq",		chatCmdFriendReq,		0, {MAX_PLAYERNAME}},
		{	"Friend",			chatCmdFriend,			0, {MAX_PLAYERNAME, 64, MAX_FRIENDSTATUS, MAX_FRIENDSTATUS}},
		{	"UnFriend",			chatCmdUnfriend,		0, {MAX_PLAYERNAME}},

		{	"InvalidUser",		chatCmdInvalidUser,		0, {MAX_PLAYERNAME}},


		// ChatAdmin-specific commands
		{	"ChanAdd",			chatCmdChannelAdd,		0, {MAX_CHANNELNAME}},
		{	"ChanRemove",		chatCmdChannelRemove,	0, {MAX_CHANNELNAME}},
		{	"UserAdd",			chatCmdUserAdd,			0, {MAX_PLAYERNAME}},
		{	"UserOnline",		chatCmdUserOnline,		0, {MAX_PLAYERNAME}},
		{	"UserOffline",		chatCmdUserOffline,		0, {MAX_PLAYERNAME}},
	
		{ 0 },
	}
};

static StashTable gChatCmds;

void chatCmdInit(PrimaryChatCmd * cmd)
{
	int		i,j;
	ChatCmd * cmds = cmd->cmds;

	cmd->names = stashTableCreateWithStringKeys(100,StashDeepCopyKeys);
	for(i=0;cmds[i].cmdname;i++)
	{
		for(j=0;cmds[i].cmd_sizes[j];j++)
			cmds[i].num_args++;

		stashAddPointer(cmd->names,cmds[i].cmdname,&cmds[i], false);
	}
}

void ChatAdminInitNet()
{
	chatCmdInit(&mainCmds);
	chatCmdInit(&channelCmds);
	chatCmdInit(&systemCmds);

	channelCmds.flipFirstArgs = 1;
}


void processShardCmd(PrimaryChatCmd * pCmds, char **args,int count)
{
	ChatCmd	*cmd;
	int	i;

	if(pCmds->flipFirstArgs && count >= 2)	// hack to get around fact that chan sys msgs send channel before 'type'
	{
		// swap first & second args
		char * t = args[0];
		args[0] = args[1];
		args[1] = t;
	}

	if (!stashFindPointer(pCmds->names, args[0], &cmd))
	{
		conTransf(MSG_ERROR, "BadCommand",args[0]);
		return;
	}
	count--;
	if(cmd->sub_cmd)
	{
		args++;
		processShardCmd(cmd->sub_cmd, args, count);
	}
	else
	{
		if (cmd->num_args != count)
		{
			conTransf(MSG_ERROR, "WrongParamsLong",args[0], count, cmd->num_args);
			return;
		}
		args++;
		for(i=0;i<count;i++)
		{
			if (strlen(args[i]) > cmd->cmd_sizes[i])
			{
				conTransf(MSG_ERROR, "ParamTooLong",args[i]);
				return;
			}
		}
		if(cmd->handler)
		{
			switch(cmd->num_args)
			{
				xcase 0:
					((handler0)cmd->handler)();
				xcase 1:
					((handler1)cmd->handler)(args[0]);
				xcase 2:
					((handler2)cmd->handler)(args[0],args[1]);
				xcase 3:
					((handler3)cmd->handler)(args[0],args[1],args[2]);
				xcase 4:
					((handler4)cmd->handler)(args[0],args[1],args[2],args[3]);
				xcase 5:
					((handler5)cmd->handler)(args[0],args[1],args[2],args[3], args[4]);
				xcase 6:
					((handler6)cmd->handler)(args[0],args[1],args[2],args[3], args[4], args[5]);
			}
		}
		else
		{
			switch(cmd->num_args)
			{
			case 0:
				conTransf(cmd->msgType, cmd->cmdname);
			xcase 1:
				conTransf(cmd->msgType, cmd->cmdname, args[0]);
			xcase 2:
				conTransf(cmd->msgType, cmd->cmdname, args[0], args[1]);
			xcase 3:
				conTransf(cmd->msgType, cmd->cmdname, args[0], args[1], args[2]);
			xcase 4:
				conTransf(cmd->msgType, cmd->cmdname, args[0], args[1], args[2], args[3]);
			break;
			default:
				printf("Not handled!!!");

			}
		}
	}
}



void handleChatCmd(Packet * pak)
{
	static int count = 0;
	static char *args[100];
	int i;

	while(!pktEnd(pak))
	{
		int auth_id = pktGetBitsPack(pak,31);
		int repeat	= pktGetBits(pak,1);
		if (!repeat)
		{
			char	*str = pktGetString(pak);

			count = tokenize_line(str,args,0);
			for(i=0;i<count;i++)
				strcpy(args[i],unescapeString(args[i]));
		}


		if(count>0)
		{
			if(g_ChatDebug)
			{	
				char buf[10000] = "";
				for(i=0;i<count;i++)
					strcatf(buf,"%s ",args[i]);

				conPrintf(MSG_DEBUG, "RECV--> %s",buf);
			}

			processShardCmd(&mainCmds, args, count);
		}
	}
}
