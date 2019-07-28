#include "users.h"
#include "csr.h"
#include "earray.h"
#include "shardnet.h"
#include "textparser.h"
#include "timing.h"
#include "msgsend.h"
#include "admin.h"
#include "friends.h"
#include "channels.h"
#include "ignore.h"
#include "utils.h"
#include "chatsqldb.h"

void userCsrName(User *user,char *old_name,char *new_name)
{
	User	*target;

	target = userFindByName(old_name);
	if (!target)
	{
		sendSysMsg(user,0,1,"InvalidUser %s",old_name);
		return;
	}
	if (!userChangeName(user,target,new_name))
		return;
	userUpdateName(user,target,old_name,new_name);
}

void userCsrSilence(User *user,char *target_name,char *minutes)
{
	User	*target;
	int		mins;

	target = userFindByNameSafe(user,target_name);
	if (!target)
		return;
	chatUserInsertOrUpdate(target);
	mins = atoi(minutes);
	target->silenced = timerSecondsSince2000() + mins * 60;
	sendSysMsg(user,0,0,"Silenced %s %s",target_name,minutes);
	sendSysMsg(target,0,0,"Silenced %s %s",target_name,minutes);
	sendAdminMsg("SysMsg AdminSilenced %s %s", target_name,minutes);
}

void userCsrRenameable(User *user, char *target_name)
{
	User *target;

	target = userFindByName(target_name);
	if(!target)
	{
		if (user->link)
		{
			sendSysMsg(user, 0, 0, "UserNameSafeFailed");
		}
		return;
	}
	target->access &= ~CHANFLAGS_RENAMED;
	chatUserInsertOrUpdate(target);
	if(target->link)
	{
		updateCrossShardStats(user, target);
		sendSysMsg(target, 0, 0, "YouMayRenameYourChatHandle");
	}
}

void csrSendAll(User *user,char *msg)
{
	int i;
	User **users = chatUserGetAll();
	for(i = eaSize(&users)-1; i >= 0; --i)
	{
		User *target = users[i];
		if (target->link)
		{
			updateCrossShardStats(user, target);
			sendUserMsg(target,"CsrSendAll %s %s",user->name,msg);
		}
	}
}

void csrSysMsgSendAll(User *user,char *msg)
{
	int i;
	User **users = chatUserGetAll();
	for(i = eaSize(&users)-1; i >= 0; --i)
	{
		User *target = users[i];
		if (target->link)
		{
			updateCrossShardStats(user, target);
			sendSysMsg(target, 0, 0, msg);
		}
	}
}

// used for broadcast commands from servermonitor
void csrSendAllAnon(char *msg)
{
	int i;
	User **users = chatUserGetAll();
	for(i = eaSize(&users)-1; i >= 0; --i)
	{
		User *target = users[i];
		if (target->link)
			sendUserMsg(target,"CsrSendAll %s %s","Admin",msg);
	}
}


void csrRenameAll(User *user)
{
	int i;
	User **users = chatUserGetAll();
	for(i = eaSize(&users)-1; i >= 0; --i)
	{
		User *target = users[i];
		target->access &= ~CHANFLAGS_RENAMED;
		chatUserInsertOrUpdate(target);
		if(target->link)
		{
			updateCrossShardStats(user, target);
			sendSysMsg(target, 0, 0, "YouMayRenameYourChatHandle");
		}
	}
}

void csrRemoveAll(User *user)
{
	int i;

	// remove friends
	for(i=eaiSize(&user->friends)-1;i>=0;i--) 
	{
		User *myfriend = userFindByAuthId(user->friends[i]);

		if (myfriend)
			friendRemove(user, myfriend->name);
	}

	// remove ignores
	for(i=eaiSize(&user->ignoredBy)-1;i>=0;i--)
	{
		User *myIgnorer = userFindByAuthId(user->ignoredBy[i]);

		if (myIgnorer)
			unIgnore(myIgnorer, user->name);

	}

	// remove channels
	for(i=eaSize(&user->watching)-1;i>=0;i--)
	{
		Channel	*channel = user->watching[i]->channel;

		if (channel)
			channelLeave(user, channel->name, false);
	}
}

void csrSilenceAll(User *user)
{
	g_silence_all = 1;
	csrSysMsgSendAll(user,"AdminSilenceAll");
}

void csrUnsilenceAll(User *user)
{
	g_silence_all = 0;
	csrSysMsgSendAll(user,"AdminUnsilenceAll");
}


void csrStatus(User * user, char *target_name)
{
	int num_watching, i, total_chars = 0;
	char buf[1024] = "";

	User * target = userFindByName(target_name);
	int time;

	if (!target)
	{
		sendUserMsg(user,"InvalidUser %s",target_name);
		return;
	}

	time = userSilencedTime(target);

	num_watching = eaSize(&target->watching);
	for (i = 0; i < num_watching; i++)
	{
		char *s;
		const char *es;
		int len;

		es = escapeString(target->watching[i]->name);
		len = strlen(es) + 3;
		if (total_chars + len >= ARRAY_SIZE(buf))
			break;

		s = &buf[total_chars];
		total_chars += sprintf_s(s, ARRAY_SIZE(buf)-total_chars, "\"%s\" ", es);
	}

	sendUserMsg(user, "CsrStatus %s %s %s %d %d %s", 
							target->name, 
							getShardName(target->link), 
							(target->status ? target->status : ""),
							time,
							target->auth_id,
							buf);
}




void csrCheckMailSent(User * user, char *target_name)
{
	User * target = userFindByName(target_name);
	UserGmail *targetGM;
	int i,j;

	if (!target)
	{
		sendUserMsg(user,"InvalidUser %s",target_name);
		return;
	}

	targetGM = chatUserGmailFind(target->auth_id);
	if (!targetGM)
	{
		sendUserMsg(user, "InvalidUser %s", target_name);
		return;
	}

  	sendUserMsg(user, "CsrMailTotal %s %d %d", target->name, eaSize(&targetGM->sent_mail), eaSize(&targetGM->gmail) );

	for( i = 0; i < eaSize(&targetGM->sent_mail); i++ ) 
	{
		SentMail *sentmail = targetGM->sent_mail[i];
		User * owner = userFindByAuthId(sentmail->sentTo);
		UserGmail *ownerGM;
		Email * mail = 0;

		if(!sentmail->attachment)
			 continue;

		if( !owner )
		{
			sendUserMsg(user, "CsrMailSent %d %d %s %s %s", sentmail->id, sentmail->sent, sentmail->attachment, "N/A", "Recipient No Longer Exists" );
			continue;
		}

		ownerGM = chatUserGmailFind(owner->auth_id);
		if( !ownerGM )
		{
			sendUserMsg(user, "CsrMailSent %d %d %s %s %s", sentmail->id, sentmail->sent, sentmail->attachment, "N/A", "Recipient No Longer Exists" );
			continue;
		}

		for(j = 0; j < eaSize(&ownerGM->gmail); j++)
		{
			if( ownerGM->gmail[j]->id == sentmail->id )
				mail = ownerGM->gmail[j];
		}

		if( !mail )
		{
			if (sentmail->attachmentClaimTimestamp > 0)
			{
				char temp[128];
				char temp2[64];

				timerMakeDateStringFromSecondsSince2000(temp2, sentmail->attachmentClaimTimestamp);
				sprintf(temp, "Attachment claimed by Recipient at %s, Deleted by Recipient afterwards", temp2);

				sendUserMsg(user, "CsrMailSent %d %d %s %s %s", sentmail->id, sentmail->sent, sentmail->attachment, owner->name, temp);
			} else {
				sendUserMsg(user, "CsrMailSent %d %d %s %s %s", sentmail->id, sentmail->sent, sentmail->attachment, owner->name, "Deleted by Recipient" );
			}
			continue;
		}
		else if( mail->attachment && *mail->attachment )
		{
			sendUserMsg(user, "CsrMailSent %d %d %s %s %s", sentmail->id, sentmail->sent, sentmail->attachment, owner->name, "Attachment Unclaimed" );
			continue;
		}
		else
		{
			char temp[128];
			char temp2[64];

			timerMakeDateStringFromSecondsSince2000(temp2, sentmail->attachmentClaimTimestamp);
			sprintf(temp, "Attachment claimed by Recipient at %s", temp2);

			sendUserMsg(user, "CsrMailSent %d %d %s %s %s", sentmail->id, sentmail->sent, sentmail->attachment, owner->name, temp);
			continue;
		}
	}
}

void csrBounceMailSent(User * user, char *target_name, char * id )
{
	User * target = userFindByName(target_name);
	int i, idx = atoi(id);
	SentMail * sentmail= 0;
	UserGmail *targetGM;

	if (!target)
	{
		sendUserMsg(user,"InvalidUser %s",target_name);
		return;
	}

	targetGM = chatUserGmailFind(target->auth_id);
	if (!targetGM)
	{
		sendUserMsg(user, "InvalidUser %s", target_name);
		return;
	}

	for(i = 0; i < eaSize(&targetGM->sent_mail); i++)
	{
		if( idx == targetGM->sent_mail[i]->id )
			sentmail = targetGM->sent_mail[i];
	}

	if( sentmail )
	{
		User * owner = userFindByAuthId(sentmail->sentTo);
		UserGmail *OwnerGM;
		Email * mail = 0;

		if( !owner )
		{
			sendUserMsg(user, "CsrBounceMailResult 0" );
			return;
		}

		OwnerGM = chatUserGmailFind(owner->auth_id);
		if (!targetGM)
		{
			sendUserMsg(user, "CsrBounceMailResult 0" );
			return;
		}

		for(i = 0; i < eaSize(&OwnerGM->gmail); i++)
		{
			if( OwnerGM->gmail[i]->id == sentmail->id )
				mail = OwnerGM->gmail[i];
		}

		if( mail && mail->to != mail->from)
		{
			bounceGmail(owner, mail, true);
			sendUserMsg(user, "CsrBounceMailResult 1" );
			return;
		}
	}

	sendUserMsg(user, "CsrBounceMailResult 0" );
}


void csrCheckMailReceived(User * user, char *target_name)
{
	User * target = userFindByName(target_name);
	UserGmail *targetGM;
	int i;

	if (!target)
	{
		sendUserMsg(user,"InvalidUser %s",target_name);
		return;
	}

	targetGM = chatUserGmailFind(target->auth_id);
	if (!targetGM)
	{
		sendUserMsg(user, "InvalidUser %s", target_name);
		return;
	}

	sendUserMsg(user, "CsrMailTotal %s %d %d",	target->name, eaSize(&targetGM->sent_mail), eaSize(&targetGM->gmail) );
	
	for( i = 0; i < eaSize(&targetGM->gmail); i++ )
	{
		Email *mail = targetGM->gmail[i];
		User * from = userFindByAuthId(mail->from);
		char temp[128];

		temp[0] = '\0';

		if (mail->attachment)
		{
			strncpyt(temp, mail->attachment, 128);
		}
		else if (mail->claim_request)
		{
			char temp2[64];
			timerMakeDateStringFromSecondsSince2000(temp2, mail->claim_request);
			sprintf(temp, "Attachment claimed by Recipient at %s", temp2);
		}

		sendUserMsg(user, "CsrMailInbox %d %s %s %s %d", mail->id, mail->subj, temp, (mail->from && from)?from->name:"System Mail", mail->sent );
	}


}

void csrBounceMailReceived(User * user, char *target_name, char * id )
{
	User * target = userFindByName(target_name);
	int i, email_id = atoi(id);
	UserGmail *targetGM;

	if (!target)
	{
		sendUserMsg(user,"InvalidUser %s",target_name);
		return;
	}

	targetGM = chatUserGmailFind(target->auth_id);
	if (!targetGM)
	{
		sendUserMsg(user, "InvalidUser %s", target_name);
		return;
	}

	for( i = 0; i < eaSize(&targetGM->gmail); i++ )
	{
		Email *mail = targetGM->gmail[i];

		if( mail->id == email_id && mail->to != mail->from) 
		{
			bounceGmail(target, mail, true);
			sendUserMsg(user, "CsrBounceMailResult 1" );
			return;
		}
	}

	sendUserMsg(user, "CsrBounceMailResult 0" );
}
