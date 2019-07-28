#include <string.h>
#include "users.h"
#include "chatdb.h"
#include "earray.h"
#include "textparser.h"
#include "shardnet.h"
#include "utils.h"
#include "channels.h"
#include "timing.h"
#include "friends.h"
#include "ignore.h"
#include "msgsend.h"
#include "profanity.h"
#include "reserved_names.h"
#include "validate_name.h"
#include "admin.h"
#include "StashTable.h"
#include "AppLocale.h"
#include "chatsqldb.h"

int g_silence_all;
int g_online_count;

int g_gmail_xact_debug = 0;


void userSetGmailXactDebug(User *user, char *debugState )
{
	g_gmail_xact_debug = atoi(debugState);
}

User *userFindByName(char *user_name)
{
	return stashFindPointerReturnPointer(users_by_name, user_name);
}

User *userFindByAuthId(U32 id)
{
	return chatUserFind(id);
}

User * userFindByDbId(User * from, int db_id)
{
	User* pResult;
	if(from && from->link && from->link->userData && stashIntFindPointer(((ClientLink*)from->link->userData)->user_db_ids, db_id, &pResult) )
		return pResult;
	return NULL;
}


User *userFindByNameSafe(User *from,char *user_name)
{
	User	*user = userFindByName(user_name);

	if (!user || user == from)
		return 0;

	return user;
}

int userWatchingCount(User *user)
{
	return (eaSize(&user->watching) + eaSize(&user->reserved));
}


User *userFind(User * from, char * id, int idType)
{
	User * to = 0;

	switch(idType)
	{
		case kChatId_Handle:
			to = userFindByNameSafe(from, id);
		xcase kChatId_DbId:
			to = userFindByDbId(from, atoi(id));
		xcase kChatId_AuthId:
			to = userFindByAuthId((U32)(atoi(id) & 0x7fffffff));
	}

	return to;
}

User *userFindIgnorable(User * from, char * id, int idType)
{
	User * to = userFind(from, id, idType);

	if (!to)
		return 0;
	if (eaiFind(&to->ignore,from->auth_id) >= 0)
	{
		sendSysMsg(from,0,0,"IgnoredBy %s",to->name);
		return 0;
	}
	return to;
}

User *userFindByNameIgnorable(User *from,char *user_name)
{
	User	*to = userFindByNameSafe(from,user_name);

	if (!to)
		return 0;
	if (eaiFind(&to->ignore,from->auth_id) >= 0)
	{
		sendSysMsg(from,0,0,"IgnoredBy %s",user_name);
		return 0;
	}
	return to;
}

int userSilencedTime(User * user)
{
	if (user->silenced <= timerSecondsSince2000())
	{
		user->silenced = 0;
		return 0;
	}
	else
		return (59 + user->silenced - timerSecondsSince2000()) / 60;
}

int userCanTalk(User *user)
{
	int time;
	if (g_silence_all)
	{
		sendSysMsg(user,0,0,"AdminSilenceAll");
		return 0;
	}
	time = userSilencedTime(user);
	if(!time)
		return 1;
	sendSysMsg(user,0,0,"NotifySilenced %d",time);
	return 0;
}



void userLogin(User *user,NetLink *link,int linkType,int access_level,int db_id)
{
	NetLink	*old_link;
	int		i;
	int		refresh = 1;
	bool	from_shard;
	U32 time;

	if (!user)
		return;
	old_link = user->link;
	if (access_level >= 0)
	{
		if(user->access_level != access_level)
		{
			user->access_level = access_level;
			chatUserInsertOrUpdate(user);
		}
	}
	if (access_level > 0)
		user->access |= CHANFLAGS_ADMIN;
	else if (access_level == 0)
		user->access &= ~CHANFLAGS_ADMIN;
	if (access_level < 0 && (user->access & CHANFLAGS_ADMIN))
		user->access_level = 1;
	if (!user->link)
	{
		ClientLink * client = (ClientLink*)link->userData;
		g_online_count++;
		client->online++;
		stashIntAddPointer(client->user_db_ids, db_id, user, false);
		refresh = 0;
	}
	user->db_id		= db_id;
	user->link		= link;
	user->dndmessage = NULL;
	from_shard = (linkType == kChatLink_Shard);
	sendUserMsg(user,"Login %d %s %s",refresh,user->name,from_shard ? getShardName(user->link) : "");
	if(user->access_level)
		sendUserMsg(user, "AccessLevel %d", user->access_level);  // needed by the ChatAdmin tool
	for(i=eaSize(&user->watching)-1;i>=0;i--)
		channelOnline(user->watching[i]->channel,user,1);
	friendStatus(user,"Online");
	friendList(user); 
	ignoreRefreshList(user);
	ignoreListUpdate(user);
	notifyIgnoredBy(user);
	userGetEmail(user);
	friendRequestSendAll(user);

	// update ban timer, or clear spammer flag
 	if(user->chat_ban_expire)
	{
		time = user->chat_ban_expire - timerSecondsSince2000();
		if( time > 0 )
			sendUserMsg(user, "SendSpamPetition %d 0", time );
		else
		{
			user->access &= ~CHANFLAGS_SPAMMER;
			user->chat_ban_expire = 0;
			sendUserMsg(user, "ClearSpam");
		}
	}

	if (!(user->access & CHANFLAGS_RENAMED))
		sendUserMsg(user,"Renameable");

	sendUserMsg(user,"LoginEnd");

	for(i=eaSize(&user->invites)-1;i>=0;i--)
		sendUserMsg(user,"InviteReminder %s",user->invites[i]->name);

	sendAdminMsg("UserOnline %s", user->name);

}

void userLogout(User *user)
{
	int		i;

	if (!user)
		return;
	if (user->link)
	{
		ClientLink * client = (ClientLink*)user->link->userData;
		g_online_count--;
		client->online--;
		if(user->db_id)
			stashIntRemovePointer(client->user_db_ids, user->db_id, NULL);
	}
	user->link		= 0;
	user->db_id		= 0;
	if (user->dndmessage)
		free(user->dndmessage);
	user->dndmessage = 0;
	//printf("Logout: %s\n",user->name);
	for(i=eaSize(&user->watching)-1;i>=0;i--)
		channelOffline(user->watching[i]->channel,user);
	for(i=eaSize(&user->reserved)-1;i>=0;i--)
	{
		Watching * watching = user->reserved[i];
		eaFindAndRemove(&watching->channel->reserved, user);
		channelDeleteIfEmpty(watching->channel);
		ParserFreeString(watching->name);
		ParserFreeStruct(watching);
	}
	eaDestroy(&user->reserved);
	//for(i=eaSize(&user->invites)-1;i>=0;i--)
	//	eaiFindAndRemove(&user->invites[i]->invites, user->auth_id);
	//eaDestroy(&user->invites);
	friendStatus(user,0);
	notifyIgnoredBy(user);
	eaiClear(&user->befriend_reqs);
	user->last_online = timerSecondsSince2000();
	user->publink_id = 0;
	sendAdminMsg("UserOffline %s", user->name);
	chatUserInsertOrUpdate(user);
}

void userLogoutByLink(NetLink *link)
{
	int		i;
	User **users = chatUserGetAll();

	for(i = eaSize(&users)-1; i >= 0; --i)
		if(users[i]->link == link)
			userLogout(users[i]);
}

User *userAdd(U32 auth_id,char *name)
{
	User *user;
	int		i;
	char	namebuf[MAX_PLAYERNAME+1],numbuf[20];

	user = ParserAllocStruct(sizeof(User));
	user->auth_id = auth_id;
	assertmsgf(user, "Double adding authid %d", auth_id);
	strcpy(namebuf,name);
	for(i=1;;i++)
	{
		if(stashAddPointer(users_by_name, namebuf, user, false))
			break;
		strcpy(namebuf,name);
		itoa(i,numbuf,10);
		namebuf[MAX_PLAYERNAME - strlen(namebuf)] = '\0'; // truncate if need be
		strcat(namebuf,numbuf);
	}
	user->name = ParserAllocString(namebuf);
	chatUserInsertOrUpdate(user);
	sendAdminMsg("UserAdd %s %d %d", user->name, user->auth_id, (user->link ? 1 : 0));
	return user;
}

UserGmail *userGmailAdd(U32 auth_id, char *name)
{
	UserGmail *gmuser;
	gmuser = ParserAllocStruct(sizeof(UserGmail));
	gmuser->auth_id = auth_id;
	gmuser->name = ParserAllocString(name);
	chatUserGmailInsertOrUpdate(gmuser);

	return gmuser;
}

int userChangeName(User *user,User *target,char *name)
{
	const char * reason;
	// check invalid name based on invoker's access level
	if( (reason = InvalidName(name, (user->access & CHANFLAGS_ADMIN))) )
	{
		sendSysMsg(user,0,1,"UserNameNotAllowed %s %s",name, reason);
		return 0;
	}
	if (!stashAddPointer(users_by_name, name, target, false))
	{
		if(user == target)
			sendSysMsg(user,0,1,"HandleAlreadyInUse %s",name);
		else
			sendSysMsg(user,0,1,"CsrHandleAlreadyInUse %s", name);
		return 0;
	}
	chatUserInsertOrUpdate(target);
	stashRemovePointer(users_by_name, target->name, NULL);
	ParserFreeString(target->name);
	target->name = ParserAllocString(name);
	return 1;
}

void userUpdateName(User *user,User *target,char *old_name,char *new_name)
{
	int		i,j;

	if(!devassert(target))
		return;

	if (user)
		sendUserMsg(user,"Name %s %s",old_name,new_name);
	if (target != user)
		sendUserMsg(target,"Name %s %s",old_name,new_name);
	for(i=eaiSize(&target->friends)-1;i>=0;i--)
	{
		User	*myfriend = userFindByAuthId(target->friends[i]);

		if(myfriend)
			sendUserMsg(myfriend,"Name %s %s",old_name,new_name);
	}
	for(i=eaSize(&target->watching)-1;i>=0;i--)
	{
		Channel	*channel = target->watching[i]->channel;

		for(j=eaSize(&channel->online)-1;j>=0;j--)
		{
			if (channel->online[j] != target)
				sendUserMsg(channel->online[j],"Name %s %s",old_name,new_name);
		}
	}
	sendAdminMsg("Name %s %s",old_name,new_name);
}

void userDoNotDisturb(User *user,char *new_dnd)
{
	if (user->dndmessage)
		free(user->dndmessage);

	if (strlen(new_dnd) > 0)
		user->dndmessage = strdup(new_dnd);
	else
		user->dndmessage = NULL;
}

void userUpdateGmail(User *user, Email *email)
{
	if( email->sent <= timerSecondsSince2000() && 
		(email->status == EMAIL_STATUS_READY || email->status == EMAIL_STATUS_PENDING_CLAIM) && 
		user->link != NULL)
	{
		if( !email->from )
		{
			sendUserMsg(user, "GMail %s %d %s %s %s %d %d", email->senderName, 0, email->subj, email->body, email->attachment?email->attachment:"", email->sent, email->id );
		}
		else
		{
			User *from = userFindByAuthId(email->from);
			if (from != NULL)
				sendUserMsg(user, "GMail %s %d %s %s %s %d %d", from->name, from->auth_id, email->subj, email->body, email->attachment?email->attachment:"", email->sent, email->id );
		}
	}
}

void userName(User *user,char *name)
{
	char	old_name[256];

	if (user->access & CHANFLAGS_RENAMED)
	{
		sendSysMsg(user,0,1,"AlreadyRenamed");
		return;
	}
	strcpy(old_name,user->name);
	if (!userChangeName(user,user,name))
		return;
	user->access |= CHANFLAGS_RENAMED;
	userUpdateName(0,user,old_name,user->name);
}

Email *emailPush(Email ***email_list, User *To, User *from, char *sender, char *subj, char *msg, char *attach, int delay)
{
	Email *email;

	email = ParserAllocStruct(sizeof(*email));
	if (!email)
		return 0;

	email->sent = timerSecondsSince2000() + delay; // yes, it can be sent from the future
	email->body = ParserAllocString(msg);
	if( from )
		email->from = from->auth_id;
	if( subj )
		email->subj = ParserAllocString(subj);
	if( attach )	
		email->attachment = ParserAllocString(attach);
	if( sender )
		email->senderName = ParserAllocString(sender);

	if(To)
	{
		email->to = To->auth_id;
		To->gmail_id++;
		email->id = To->gmail_id;
	}
	eaPush(email_list,email);

	return email;
}


Email *gmailPush(U32 to_auth_id, char *to_name, U32 from_auth_id, char *from_name, U32 original_from_auth_id, char *subj, 
				 char *msg, char *attach, int delay)
{
	Email *email;
	UserGmail *FromGmuser = NULL;
	UserGmail *ToGmuser = NULL;
	int i;

	if (from_auth_id > 0)
	{
		FromGmuser = chatUserGmailFind(from_auth_id);
		if (FromGmuser == NULL)
		{
			FromGmuser = userGmailAdd(from_auth_id, from_name);
		}
	}

	ToGmuser = chatUserGmailFind(to_auth_id);
	if (ToGmuser == NULL)
	{
		ToGmuser = userGmailAdd(to_auth_id, to_name);
	}

	// Check to see if there are outstanding pending xacts on the recipient from the sender.  If there are, they can 
	//	safely be removed.
	for (i = eaSize(&ToGmuser->gmail) - 1; i >= 0; i--)
	{
		email = ToGmuser->gmail[i];

		if (email->from == from_auth_id && 
			(email->status == EMAIL_STATUS_PENDING_COMMIT))
		{
			gmailRemove(ToGmuser, email->id);
		}
	}

	email = ParserAllocStruct(sizeof(*email));
	if (!email)
		return 0;

	email->sent = timerSecondsSince2000() + delay; // yes, it can be sent from the future
	email->body = ParserAllocString(msg);
	email->status = EMAIL_STATUS_PENDING_COMMIT;
	if( from_auth_id != 0 )
		email->from = from_auth_id;
	email->originalSender = original_from_auth_id;
	if( subj )
		email->subj = ParserAllocString(subj);
	if( attach )	
		email->attachment = ParserAllocString(attach);
	if( from_name )
		email->senderName = ParserAllocString(from_name);

	if (ToGmuser)
	{
		email->to = to_auth_id;
		ToGmuser->gmail_id++;
		email->id = ToGmuser->gmail_id;
	}

	eaPush(&ToGmuser->gmail, email);

	return email;
}

/*
void sentMailPush( User *To, User *from,  int id, char * attachment )
{
	SentMail *sentmail;

	sentmail = ParserAllocStruct(sizeof(*sentmail));
	if (!sentmail)
		return;
	
	sentmail->sentTo = To->auth_id;
	sentmail->sent = timerSecondsSince2000();
	sentmail->attachment =  ParserAllocString(attachment);
	sentmail->id = id;
	sentmail->attachmentClaimTimestamp = 0;

	eaPush(&from->sent_mail, sentmail);
}
*/

void emailRemove(Email ***email_list,int idx)
{
	Email	*email;

	if(!EAINRANGE(idx, *email_list))
		return;
	email = (*email_list)[idx];
	if (email)
	{
		if (email->body)
			ParserFreeString(email->body);
		if(email->subj)
			ParserFreeString(email->subj);
		if(email->attachment)
			ParserFreeString(email->attachment);
		ParserFreeStruct(email);
	}
	eaRemove(email_list,idx);
}

void gmailRemove(UserGmail *gmUser, int mail_id)
{
	int i;
	Email *mail;

	for(i = eaSize(&gmUser->gmail)-1; i>=0; i-- )
	{
		if( mail_id == gmUser->gmail[i]->id )
		{
			mail = gmUser->gmail[i];
			eaRemove(&gmUser->gmail, i);
			if (mail->body)
				ParserFreeString(mail->body);
			if(mail->subj)
				ParserFreeString(mail->subj);
			if(mail->attachment)
				ParserFreeString(mail->attachment);
			ParserFreeStruct(mail);
			return;
		}
	}	
}

// This email is being returned to sender, we will allow them to exceed the 
// stored email limit
// Converted to new system
void bounceGmail( User *user, Email *mail, int allowOriginal )
{
	UserGmail *gmUser;
	int i;
	User *To, *From;

	if (user == NULL || mail == NULL || user->auth_id != mail->to)
		return;

	gmUser = chatUserGmailFind(user->auth_id);

	To = userFindByAuthId(mail->to);
	From = userFindByAuthId(mail->from);

	if( To && From  && mail->to != mail->from &&					// cannot bounce mail to self 
		(allowOriginal || user->auth_id != mail->originalSender))	// and most cases you can't auto-bounce mail you originally sent
	{
		Email *newmail = gmailPush(mail->from, From->name, mail->to, To->name, mail->originalSender, mail->subj, mail->body, mail->attachment, 0);
		newmail->status = EMAIL_STATUS_READY;		// no need to confirm since there is no transfer of items between processes

		if (newmail->attachment && *newmail->attachment && newmail->from > 0)
		{
			SentMail *sentmail;
			UserGmail *NewFromGmuser = chatUserGmailFind(newmail->from);
			UserGmail *FromGmuser;

			if (NewFromGmuser != NULL)
			{
				sentmail = ParserAllocStruct(sizeof(*sentmail));
				if (sentmail != NULL)
				{
					sentmail->sentTo = newmail->to;
					sentmail->sent = timerSecondsSince2000();
					sentmail->attachment =  ParserAllocString(newmail->attachment);
					sentmail->id = newmail->id;
					sentmail->attachmentClaimTimestamp = 0;

					eaPush(&NewFromGmuser->sent_mail, sentmail);
				}

				chatUserGmailInsertOrUpdate(NewFromGmuser);
			}

			// mark it as claimed on old email & sent record
			FromGmuser = chatUserGmailFind(mail->from);
			for (i = 0; i < eaSize(&FromGmuser->sent_mail); i++)
			{
				if (mail->id == FromGmuser->sent_mail[i]->id)
				{
					FromGmuser->sent_mail[i]->attachmentClaimTimestamp = timerSecondsSince2000();
				}
			}

			mail->claim_request = timerSecondsSince2000();
			if( mail->attachment )
			{
				ParserFreeString(mail->attachment);
				mail->attachment = 0;
			}
			mail->status = EMAIL_STATUS_READY;

			chatUserGmailInsertOrUpdate(FromGmuser);
		}

		userUpdateGmail(From, newmail);
	}
	// otherwise its a system mail, which can't be bounced.

	// clean up
	if (gmUser)
	{
		gmailRemove(gmUser, mail->id);
		chatUserGmailInsertOrUpdate(gmUser);
	}
}



// Converted to new system
void userGetEmail(User *user)
{
	int		i,count;
	Email	*email;
	User	*from;
	char	datestr[100];
	int now = timerSecondsSince2000();
	UserGmail *gmUser;

	count = eaSize(&user->email);
	for(i=0;i<count;i++)
	{
		email = user->email[0];
		if (!email)
			break;

		from = userFindByAuthId(email->from);
		if (!from || !from->name || !email->body)
		{
			emailRemove(&user->email,0);
			continue;
		}
		itoa(email->sent,datestr,10);
		sendUserMsg(user,"storedmsg %s %s %s",datestr,from->name,email->body);
		emailRemove(&user->email,0);
	}

	gmUser = chatUserGmailFind(user->auth_id);
	if (gmUser != NULL)
	{
		count = eaSize(&gmUser->gmail);

		for(i=count-1;i>=0;i--)
		{
			email = gmUser->gmail[i];
			if (!email)
				break;

			from = userFindByAuthId(email->from);
			if( email->from )
			{
				if (!from || !from->name)
				{
					email->from = 0;
					email->senderName = StructAllocString("OriginalSenderDeleted");
					chatUserGmailInsertOrUpdate(gmUser);
				}
			}

			userUpdateGmail(user, email);
		}
	}

	chatUserInsertOrUpdate(user);

}

void userSend(User *user,char *target_name,char *msg)
{
	User	*target;

	if (!userCanTalk(user))
		return;

	target = userFindByNameIgnorable(user,target_name);
	if (!target)
		return;
	
 	updateCrossShardStats(user, target); 
	if (target->link && target->dndmessage) {
		// sendUserMsg won't loop, but if they're DND they don't need to be notified
		if (!user->dndmessage)
			sendUserMsg(user,"UserMsg %s %s",target_name,target->dndmessage);
	}
	else if (target->link )
	{
		sendUserMsg(target,"UserMsg %s %s",user->name,msg);
		if( target->access&CHANFLAGS_TELLHIDE )
			sendSysMsg(user,0,0,"PlayerNotOnlineMessageQueued %s",target_name);
		else
			sendSysMsg(user,0,1,"UserMsgSent %s %s",target_name,msg);
	}
	else
	{
		if (eaSize(&target->email) >= MAX_STOREDMSG)
			sendSysMsg(user,0,0,"PlayerNotOnlineMailboxFull %s",target_name);
		else
		{
			sendSysMsg(user,0,0,"PlayerNotOnlineMessageQueued %s",target_name);
			emailPush(&target->email, 0, user, 0, 0, msg, 0, 0);
			chatUserInsertOrUpdate(target);
		}
	}
}



const char * InvalidName(char * name, bool adminAccess)
{
	char * reason = NULL;

	int len = strlen(name);

	if(   len > MAX_PLAYERNAME
	   || len < 2)
	{
		return "Length";
	}

	if(g_chatLocale ==  LOCALE_ID_KOREAN)
	{
		if (NonAsciiChars(name)&& NonKoreanChars(name))
		{
			return "NonKorean";
		}
	}
	else if (NonAsciiChars(name))
	{
		return "NonAscii";
	}

	if(multipleNonAciiCharacters(name))
		return "MultipleSymbols";

	if(    IsAnyWordProfane(name) || (!adminAccess && IsReservedName(name, 0)))
	{
		return "NotAllowed"; // prevents confusion with system channel abbreviations
	}

	if (BeginsOrEndsWithWhiteSpace(name))
	{
		return "NotAllowedWhiteSpace";
	}

	return NULL;	// name is valid
}

// if 'member' is on same shard as 'user', return valid db_id. Else, return -1.
int userGetRelativeDbId(User * user, User * member)
{
	if(      user
		&&   member
		&& ! user->publink_id
		&&   member->link
		&&   user->link == member->link)
	{
		return member->db_id;
	}
	else
		return 0;
}

void getGlobal( User *user, char * AuthId )
{
	User *whois = userFindByAuthId(atoi(AuthId));
	if( whois && whois->name && whois->status && !(whois->access&CHANFLAGS_INVISIBLE) )
	{
		sendUserMsg(user,"WhoGlobal %s %s", whois->name, whois->status );
	}
	else if( whois && whois->name )
		sendUserMsg(user,"InvalidUser %s", whois->name );
}

void getGlobalSilent( User *user, char * AuthId )
{
	User *whois = userFindByAuthId(atoi(AuthId));
	if( whois && whois->name && whois->status )
	{
		sendUserMsg(user,"WhoGlobalHidden %s %s", whois->name, whois->status );
	}
}

void getLocal( User *user, char * name )
{
	User *whois = userFindByName(name);
	if( whois && whois->name && whois->status && !(whois->access&CHANFLAGS_INVISIBLE) )
	{
		sendUserMsg(user,"WhoLocal %s %s", whois->name, whois->status );
	}
	else
		sendUserMsg(user,"InvalidUser %s", name );
}

void getLocalInvite( User *user, char * name )
{
	User *whois = userFindByName(name);
	if( whois && whois->name && whois->status && !(whois->access&CHANFLAGS_INVISIBLE) )
	{
		sendUserMsg(user,"WhoLocalInvite %s %s", whois->name, whois->status );
	}
}

void getLocalLeagueInvite( User *user, char * name )
{
	User *whois = userFindByName(name);
	if( whois && whois->name && whois->status && !(whois->access&CHANFLAGS_INVISIBLE) )
	{
		sendUserMsg(user,"WhoLocalLeagueInvite %s %s", whois->name, whois->status );
	}
}
void systemSendGmail(User *user, char *senderName, char *subj, char *msg, char* attachment, char * delay )
{
	Email *mail;
	UserGmail *gmuser;

	mail = gmailPush(user->auth_id, user->name, 0, senderName, 0, subj, msg, attachment, atoi(delay));
	mail->status = EMAIL_STATUS_READY;

	gmuser = chatUserGmailFind(user->auth_id);
	if (devassert(gmuser))
	{
		chatUserGmailInsertOrUpdate(gmuser);
	}

	userUpdateGmail(user, mail);
}

int userCanSendGmail(User *user,char *target_name, int selfValid)
{
	User	*target;
	int i, count = 0;
	UserGmail *gmuser;

	if (!userCanTalk(user))
	{
		return false;
	}

	target = userFindByNameIgnorable(user,target_name);
	if (!target)
	{
		if (selfValid)
		{
			target = userFindByName(target_name);
			if (target != user)
				return false;
		} else {
			return false;
		}
	}

	if( target != user && target->access & CHANFLAGS_GMAILFRIENDONLY && eaiFind(&target->friends, user->auth_id) < 0 ) // see if sender is friend of target
	{
		return false;
	}

	gmuser = chatUserGmailFind(target->auth_id);
	if (gmuser == NULL)
		gmuser = userGmailAdd(target->auth_id, target->name);
		

	// Check inbox sizes - target
	for( i = 0; i < eaSize(&gmuser->gmail); i++ )
	{
		if( gmuser->gmail[i]->sent < timerSecondsSince2000() ) // don't count delayed emails 
			count++;
	}
	if (count >= MAX_GMAIL)
	{
		return false;
	}

	// Check inbox sizes - user
	if (user->auth_id != target->auth_id)   // only need to check if they are actually different
	{
		gmuser = chatUserGmailFind(user->auth_id);
		if (gmuser == NULL)
			gmuser = userGmailAdd(user->auth_id, user->name);

		count = 0;
		for( i = 0; i < eaSize(&gmuser->gmail); i++ )
		{
			if( gmuser->gmail[i]->sent < timerSecondsSince2000() ) // don't count delayed emails 
				count++;
		}
		if (count >= MAX_GMAIL)
		{
			return false;
		}
	}
	return true;
}

void userXactRequestGmail(User *user, char *target_name, char *subj, char *msg, char *attachment)
{
	int success;
	User	*target = userFindByName(target_name);
	Email *mail;

	// debug failure state
	if (g_gmail_xact_debug == 1)
		return;

	success = userCanSendGmail(user, target_name, true);

	if (success && target && g_gmail_xact_debug != 5)
	{
		// Catch case of just a player type.  Not really an attachment
		if (strlen(attachment) == 1)
			attachment = NULL;

		mail = gmailPush(target->auth_id, target->name, user->auth_id, user->name, user->auth_id, subj, msg, attachment, 0 );
		chatUserGmailInsertOrUpdate(chatUserGmailFind(target->auth_id));

		// debug failure state
		if (g_gmail_xact_debug == 2)
			return;

		sendUserMsg(user, "GMailXactRequestAck %d %d", 1, mail->id);
	} else {

		// debug failure state
		if (g_gmail_xact_debug == 2)
			return;

		sendUserMsg(user, "GMailXactRequestAck %d %d", 0, 0);
	}
}


void userCommitRequestGmail(User *user, char *target_name, char *xact_id_str)
{
	User *to = NULL;
	UserGmail *gmuser = NULL;
	int xact_id = atoi(xact_id_str);
	int i;
	Email *mail = NULL;
	Email *found = NULL;

	// debug failure state
	if (g_gmail_xact_debug == 3)
		return;

	to = userFindByName(target_name);
	if (to == NULL)
	{
		if (g_gmail_xact_debug == 4)
			return;

		sendUserMsg(user, "GMailCommitRequestAck %d %d", xact_id);
		return;
	}

	gmuser = chatUserGmailFind(to->auth_id);
	if (gmuser)
	{
		for (i = 0; i < eaSize(&gmuser->gmail); i++)
		{
			mail = gmuser->gmail[i];

			if (mail->id == xact_id)
			{
				if (mail->status == EMAIL_STATUS_PENDING_COMMIT)
				{
					mail->status = EMAIL_STATUS_READY;
					chatUserGmailInsertOrUpdate(gmuser);
				}
				found = mail;
				break;
			}
		}
	}

	if (found != NULL && found->attachment && *found->attachment && found->from > 0)
	{
		SentMail *sentmail;
		UserGmail *FromGmuser = chatUserGmailFind(found->from);

		if (FromGmuser != NULL)
		{
			sentmail = ParserAllocStruct(sizeof(*sentmail));
			if (sentmail != NULL)
			{
				sentmail->sentTo = found->to;
				sentmail->sent = timerSecondsSince2000();
				sentmail->attachment =  ParserAllocString(found->attachment);
				sentmail->id = found->id;
				sentmail->attachmentClaimTimestamp = 0;

				eaPush(&FromGmuser->sent_mail, sentmail);
			}

			chatUserGmailInsertOrUpdate(FromGmuser);
		}
	}

	if (g_gmail_xact_debug == 4)
		return;

	sendUserMsg(user, "GMailCommitRequestAck %d %d", xact_id);

	// if the mail is confirmed and the player receiving is online, let them know about the new mail
	if (found != NULL)
	{
		User *To = userFindByAuthId(found->to);
		userUpdateGmail(To, found);
	}
}


// Converted to new system
void userGmailClaim(User *user, char * id)
{
	int i, email_id = atoi(id);
	UserGmail *gmuser = chatUserGmailFind(user->auth_id);

	if (gmuser == NULL || g_gmail_xact_debug == 6)
		return;

	for( i = 0; i < eaSize(&gmuser->gmail); i++)
	{
		Email *mail = gmuser->gmail[i];

		if( mail->id == email_id && (mail->status == EMAIL_STATUS_READY || mail->status == EMAIL_STATUS_PENDING_CLAIM))
		{
			if( !mail->attachment || mail->attachment[0] == '\0' )
			{
				sendSysMsg(user,0,1,"AttachmentAlreadyClaimed");
				sendUserMsg(user,"GMailNotFound %d", mail->id);
			}
			else
			{
				mail->status = EMAIL_STATUS_PENDING_CLAIM;
				mail->claim_request = timerSecondsSince2000();
				chatUserGmailInsertOrUpdate(gmuser);

				if (g_gmail_xact_debug == 7)
					return;

				sendUserMsg(user,"GMailClaimAck %d %s", email_id, mail->attachment );
			}
			return;
		}
	}

	sendUserMsg(user,"GMailNotFound %i", email_id);
}


// Converted to new system
void userGmailClaimConfirm(User *user, char * id)
{
	int email_id = atoi(id);
	int i;
	Email *mail = NULL;
	UserGmail *gmuser = chatUserGmailFind(user->auth_id);

	if (gmuser == NULL || g_gmail_xact_debug == 8)
		return;

	for (i = 0; i < eaSize(&gmuser->gmail); i++)
	{
		if (gmuser->gmail[i]->id == email_id)
		{
			mail = gmuser->gmail[i];
			break;
		}
	}

	if (!mail || !(mail->status == EMAIL_STATUS_PENDING_CLAIM || mail->status == EMAIL_STATUS_READY))
	{
		// couldn't find requested mail
		sendUserMsg(user,"GMailNotFound %i", email_id);
		return;
	}

	//logging claim on sentmail
	if (mail->from > 0)
	{
		UserGmail *from = chatUserGmailFind(mail->from);
		int sentMailIndex = 0;
		for (sentMailIndex = 0; sentMailIndex < eaSize(&from->sent_mail); sentMailIndex++)
		{
			if( email_id == from->sent_mail[sentMailIndex]->id )
			{
				from->sent_mail[sentMailIndex]->attachmentClaimTimestamp = timerSecondsSince2000();
				break;
			}
		}

		chatUserGmailInsertOrUpdate(from);
	}

	if( mail->attachment )
	{
		ParserFreeString(mail->attachment);
		mail->attachment = 0;
	}

	mail->status = EMAIL_STATUS_READY;

	chatUserGmailInsertOrUpdate(gmuser);

	if (g_gmail_xact_debug == 9)
		return;

	sendUserMsg(user,"GMailClaimCommitAck %i", email_id);
	userUpdateGmail(user, mail);
}

// NOT Converted to new system
void userGmailReturn( User * user, char * id)
{
	int i, email_id = atoi(id);
	UserGmail *gmuser = chatUserGmailFind(user->auth_id);

	for( i = eaSize(&gmuser->gmail)-1; i>=0; i--)
	{
		Email *mail = gmuser->gmail[i];
		if( mail->id == email_id && mail->status == EMAIL_STATUS_READY )
		{
			if( mail->from && mail->from != user->auth_id ) // don't allow user to return their own email, it could delete attachments
			{
				bounceGmail(user, mail, true);
				return;
			}
			else // not deleted, re-transmit
			{
				if( !mail->from ) // system mail
					sendUserMsg(user,"GMail %s %d %s %s %s %d %d", mail->senderName, 0, mail->subj, mail->body,mail->attachment?mail->attachment:"", mail->sent, mail->id );
				else
					sendUserMsg(user,"GMail %s %d %s %s %s %d %d", user->name, user->auth_id, mail->subj, mail->body, mail->attachment?mail->attachment:"", mail->sent, mail->id );
			}
		}
	}
}


// Converted to new system
void userGmailDelete(User *user, char * id)
{
	int i, email_id = atoi(id);
	UserGmail *gmUser = chatUserGmailFind(user->auth_id);

	if (!gmUser)
		return;

	for( i = eaSize(&gmUser->gmail)-1; i>=0; i--)
	{
		Email *mail = gmUser->gmail[i];
		if( mail->id == email_id && mail->status == EMAIL_STATUS_READY)
		{
			if( mail->attachment && *mail->attachment )  // cannot delete mail with attachment, re-send
			{
				User * from = userFindByAuthId(mail->from);
				if( from )	
					sendUserMsg(user,"GMail %s %d %s %s %s %d %d", from->name, from->auth_id, mail->subj, mail->body, mail->attachment, mail->sent, mail->id );
				else
				{	
					if( mail->from )
					{
						mail->from = 0;
						mail->senderName = StructAllocString("OriginalSenderDeleted");
						chatUserGmailInsertOrUpdate(gmUser);
					}
					sendUserMsg(user,"GMail %s %d %s %s %s %d %d", mail->senderName, 0, mail->subj, mail->body, mail->attachment, mail->sent, mail->id );
					
				}
				sendSysMsg(user, 0, 1, "CannotDeleteEmailWithAttachment" );
			}
			else
			{
				gmailRemove(gmUser, mail->id);
				chatUserGmailInsertOrUpdate(gmUser);
				return;
			}
		}
	}
}
