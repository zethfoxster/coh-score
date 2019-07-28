#include "users.h"
#include "friends.h"
#include "earray.h"
#include "shardnet.h"
#include "textparser.h"
#include "msgsend.h"
#include "channels.h"
#include "ignore.h"
#include "chatsqldb.h"
#include "messagefile.h"

static bool sendUserMsgFriend(User *user,User *myfriend,char *status)
{
	return sendUserMsg(user,"Friend %s %d %s %s",myfriend->name,userGetRelativeDbId(user, myfriend), status ? status : "",getShardName(myfriend->link));
}

void friendList(User *user)
{
	int		i,idx=0;
	User	*myfriend;

	for(i=eaiSize(&user->friends)-1;i>=0;i--)
	{
		if (myfriend = userFindByAuthId(user->friends[i]))
		{
			if (myfriend->access & CHANFLAGS_FRIENDHIDE)
				sendUserMsgFriend(user,myfriend,0);
			else
				sendUserMsgFriend(user,myfriend,myfriend->status);
		}
	}
}
static void addFriend(User *user,User *myfriend)
{
	eaiPush(&user->friends,myfriend->auth_id);
	eaPush(&myfriend->befrienders,user);
	sendUserMsgFriend(user,myfriend,myfriend->status);
	chatUserInsertOrUpdate(user);
	chatUserInsertOrUpdate(myfriend);
}

static void removeFriend(User *user,User *myfriend)
{
	if (eaiFindAndRemove(&user->friends,myfriend->auth_id) < 0)
		sendSysMsg(user,0,1,"NotFriendsWith %s",myfriend->name);
	if (eaFindAndRemove(&myfriend->befrienders,user) >= 0)
		sendUserMsg(user,"Unfriend %s",myfriend->name);
	if (eaiFindAndRemove(&user->befriend_reqs,myfriend->auth_id) >= 0)
	{
 		sendSysMsg(myfriend, 0, 0, "UnFriend %s",user->name);
	}
	chatUserInsertOrUpdate(user);
	chatUserInsertOrUpdate(myfriend);
}

void friendRequestSendAll(User *user)
{
	int		i;

	for(i=eaiSize(&user->befriend_reqs)-1;i>=0;i--)
	{
		User	*myfriend = userFindByAuthId(user->befriend_reqs[i]);

		if (myfriend && !isIgnoring(user, myfriend))
			sendUserMsg(user,"Friendreq %s",myfriend->name);
	}
}

void friendRequest(User *user,char *friend_name)
{
	User	*myfriend = userFindByNameIgnorable(user,friend_name),*full=0;
	int		alreadyFriends; // are we already friends?
	int		wasRequested; // was my new friend already requesting me to be friends?
	int		alreadyRequested; // have I already sent a friend request to this friend?

	if (!myfriend)
	{
		myfriend = userFindByNameSafe(user, friend_name);
		if (!myfriend)
		{
			sendSysMsg(user, 0, 0, "OfflineOrDecliningAll");
			return;
		}

		// my friend is ignoring me
		eaiFindAndRemove(&myfriend->befriend_reqs,user->auth_id);
		eaiFindAndRemove(&user->befriend_reqs,myfriend->auth_id);
		return;
	}

	alreadyFriends = eaiFind(&user->friends,myfriend->auth_id) >= 0;
	wasRequested = eaiFindAndRemove(&user->befriend_reqs,myfriend->auth_id) >= 0;
	alreadyRequested = eaiFind(&myfriend->befriend_reqs,user->auth_id) >= 0;

	// just ignore if the player is invisible
	if (!wasRequested && (myfriend->access & CHANFLAGS_FRIENDHIDE) )  
	{
		sendSysMsg(user, 0, 0, "OfflineOrDecliningAll");
		return;
	}

	if (alreadyFriends)
	{
		eaiFindAndRemove(&myfriend->befriend_reqs,user->auth_id);
		sendSysMsg(user,0,1,"AlreadyFriendsWith %s",friend_name);
		return;
	}

	updateCrossShardStats(user, myfriend);

	if (wasRequested)
	{
		eaiFindAndRemove(&myfriend->befriend_reqs,user->auth_id);

		if (eaiSize(&user->friends) >= MAX_GFRIENDS)
			full = user;
		else if (eaiSize(&myfriend->friends) >= MAX_GFRIENDS)
			full = myfriend;
		if (full)
		{
			sendSysMsg(user,0,1,"FriendsListFull %s %s",full->name, myfriend->name);
			sendSysMsg(myfriend,0,1,"FriendsListFull %s %s",full->name, user->name);
		}
		else
		{
			// mutual request: yay, we're friends!
			addFriend(user,myfriend);
			addFriend(myfriend,user);
		}
	}
	else if (!alreadyRequested)
	{
		if (eaiFind(&user->ignore,myfriend->auth_id) >= 0)
		{ //Don't let you try to invite someone you are ignoring
			sendSysMsg(user,0,0,"Ignoring %s %d",myfriend->name, userGetRelativeDbId(user, myfriend));
			return;
		}
		eaiPush(&myfriend->befriend_reqs,user->auth_id);
		sendUserMsg(myfriend,"Friendreq %s",user->name);
		sendSysMsg(user,0,0,"FriendReq %s",myfriend->name);
	}
	else
	{
		sendUserMsg(myfriend,"Friendreq %s",user->name);
		sendSysMsg(user,0,0,"FriendReq %s",myfriend->name);
	}

}

void friendRemove(User *user,char *friend_name)
{
	User	*myfriend = userFindByNameSafe(user,friend_name);

	if (!myfriend)
	{
		// TODO? attempt to clean up users list just in case something bad happened
		return;
	}

	removeFriend(user,myfriend);
	removeFriend(myfriend,user);
	updateCrossShardStats(user, myfriend);
}

static void sendFriendStatus(User *user,char *status)
{
	int		i,resend=0;
	User	*myfriend;

	for(i=eaSize(&user->befrienders)-1;i>=0;i--)
	{
		myfriend = user->befrienders[i];
		if (myfriend->link)
		{
			if (resend)
				resendMsg(myfriend);
			else
				resend = sendUserMsgFriend(myfriend,user,status);
		}
	}
}

void friendStatus(User *user,char *status)
{
	if (user->status)
		ParserFreeString(user->status);
	user->status = 0;
	if (status)
		user->status = ParserAllocString(status);
	if (user->access & CHANFLAGS_FRIENDHIDE)
		sendFriendStatus(user,0);
	else
		sendFriendStatus(user,user->status);
}

void userInvisible(User *user)
{
	int i, j;

	user->access |= CHANFLAGS_INVISIBLE;
	chatUserInsertOrUpdate(user);
	sendFriendStatus(user,0);
	sendUserMsg(user,"Invisible");

	for (i = eaSize(&user->watching) - 1; i >= 0; i--)
	{
		int resend = 0;
		Channel *cc = user->watching[i]->channel;

		for (j = eaSize(&cc->online) - 1; j >= 0; j--)
		{
			if (cc->online[j] == user)
				continue;

			if (resend)
				resendMsg(cc->online[j]);
			else
				resend = sendUserMsg(cc->online[j], "Leave %s %s 0", cc->name, user->name);
			updateCrossShardStats(user, cc->online[j]);
		}
	}
}

void userFriendHide(User *user)
{
	user->access |= CHANFLAGS_FRIENDHIDE;
	chatUserInsertOrUpdate(user);
	sendFriendStatus(user,0);
	sendUserMsg(user,"FriendHide");
}

void userTellHide(User *user)
{
	user->access |= CHANFLAGS_TELLHIDE;
	chatUserInsertOrUpdate(user);
	sendUserMsg(user,"TellHide");
}

void userVisible(User *user)
{
	int		i, j;

	user->access &= ~CHANFLAGS_INVISIBLE;
	chatUserInsertOrUpdate(user);
	sendUserMsg(user,"Visible");

	for (i = eaSize(&user->watching) - 1; i >= 0; i--)
	{
		Channel *cc = user->watching[i]->channel;

		for (j = eaSize(&cc->online) - 1; j >= 0; j--)
		{
			if (cc->online[j] == user)
				continue;

			sendUserMsgJoin(cc->online[j],user,cc,1);
			updateCrossShardStats(user, cc->online[j]);
		}
	}
}


void userFriendUnHide(User *user)
{
	user->access &= ~CHANFLAGS_FRIENDHIDE;
	chatUserInsertOrUpdate(user);
	sendFriendStatus(user,user->status);
	sendUserMsg(user,"FriendUnHide");
}

void userTellUnHide(User *user)
{
	user->access &= ~CHANFLAGS_TELLHIDE;
	chatUserInsertOrUpdate(user);
	sendUserMsg(user,"TellUnHide");
}


void userGmailFriendOnlySet(User *user)
{
	user->access |= CHANFLAGS_GMAILFRIENDONLY;
	chatUserInsertOrUpdate(user);
}

void userGmailFriendOnlyUnset(User *user)
{
	user->access &= ~CHANFLAGS_GMAILFRIENDONLY;
	chatUserInsertOrUpdate(user);
}

void userClearMessageHash(User *user)
{
	user->message_hash = 0;
	messageCheckUser(user);			// go ahead and re-send it
}
