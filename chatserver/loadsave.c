#include "chatdb.h"
#include "shardnet.h"
#include "utils.h"
#include "textparser.h"
#include "error.h"
#include "earray.h"
#include "EString.h"
#include "timing.h"
#include "StashTable.h"
#include "channels.h"
#include "loadsave.h"
#include "users.h"
#include "file.h"
#include "textcrcdb.h"
#include "mathutil.h"
#include "log.h"
#include "ChatSql.h"
#include "chatsqldb.h"

StashTable users_by_name;

StaticDefineInt EmailStatusEnum[] =
{
	DEFINE_INT
	{ "READY", EMAIL_STATUS_READY},
	{ "PENDING_COMMIT", EMAIL_STATUS_PENDING_COMMIT},
	{ "PENDING_CLAIM", EMAIL_STATUS_PENDING_CLAIM},
	{ "DELETED", EMAIL_STATUS_DELETED},
	DEFINE_END
};

TokenizerParseInfo parse_email[] = {
	{ "ID",				TOK_INT(Email,id,0)	},
	{ "From",			TOK_INT(Email,from,0)	},
	{ "To",				TOK_INT(Email,to,0)	},
	{ "Sent",			TOK_INT(Email,sent,0)	},
	{ "SenderName",		TOK_STRING(Email,senderName,0)	},
	{ "OriginalSender",	TOK_INT(Email, originalSender,0)	},
	{ "Body",			TOK_STRING(Email,body,0)	},
	{ "Subj",			TOK_STRING(Email,subj,0)	},
	{ "Attachment",		TOK_STRING(Email,attachment,0)	},
	{ "Status",			TOK_INT(Email, status, 0), EmailStatusEnum	},
	{ "ClaimRequest",	TOK_INT(Email,claim_request,0)	},
	{ "{",				TOK_START,		0							},
	{ "}",				TOK_END,			0							},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_sentmail[] = {
	{ "ID",				TOK_INT(SentMail,id,0)		},
	{ "SentTo",			TOK_INT(SentMail, sentTo,0)	},
	{ "Sent",			TOK_INT(SentMail,sent,0)	},
	{ "Attachment",		TOK_STRING(SentMail,attachment,0)	},
	{ "AttachmentClaimTimestamp",	TOK_INT(SentMail,attachmentClaimTimestamp,0)	},
	{ "{",				TOK_START,		0			},
	{ "}",				TOK_END,			0		},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_watching[] = {
	{ "Name",			TOK_STRING(Watching,name,0)	},
	{ "LastRead",		TOK_INT(Watching,last_read,0)	},
	{ "Access",			TOK_INT(Watching,access,0)	},
	{ "{",				TOK_START,		0							},
	{ "}",				TOK_END,			0							},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_user[] = {
	{ "AuthId",			TOK_PRIMARY_KEY|TOK_INT(User,auth_id,0)	},
	{ "Name",			TOK_STRING(User,name,0)	},
	{ "Access",			TOK_INT(User,access,0)	},
	{ "AccessLevel",	TOK_U8(User,access_level,0)	},
	{ "Hash0",			TOK_INT(User,hash0,0)	},
	{ "Hash1",			TOK_INT(User,hash1,0)	},
	{ "Hash2",			TOK_INT(User,hash2,0)	},
	{ "Hash3",			TOK_INT(User,hash3,0)	},
	{ "LastOnline",		TOK_INT(User,last_online,0)	},
	{ "Silenced",		TOK_INT(User,silenced,0)	},
	{ "Watching",		TOK_STRUCT(User,watching,parse_watching) },
	{ "Friends",		TOK_INTARRAY(User,friends)	},
	{ "Ignore",			TOK_INTARRAY(User,ignore)	},
	{ "Email",			TOK_STRUCT(User,email,parse_email) },
	{ "Gmail",			TOK_STRUCT(User,gmail,parse_email) },
	{ "SentMail",		TOK_STRUCT(User,sent_mail,parse_sentmail) },
	{ "MessageHash",	TOK_INT(User,message_hash,0)	},
	{ "ChatBan",		TOK_INT(User,chat_ban_expire,0)	},
	{ "GMailId",		TOK_INT(User,gmail_id,0)	},
	{ "{",				TOK_START,		0						},
	{ "}",				TOK_END,			0						},
	{ "", 0, 0 }
};

TokenizerParseInfo parse_gmailuser[] = {
	{ "AuthId",			TOK_PRIMARY_KEY|TOK_INT(UserGmail,auth_id,0)	},
	{ "Name",			TOK_STRING(UserGmail,name,0)					},
	{ "GMailId",		TOK_INT(UserGmail,gmail_id,0)					},
	{ "Gmail",			TOK_STRUCT(UserGmail,gmail,parse_email)			},
	{ "SentMail",		TOK_STRUCT(UserGmail,sent_mail,parse_sentmail)	},
	{ "{",				TOK_START, 0									},
	{ "}",				TOK_END, 0										},
	{ "", 0, 0 }
};


TokenizerParseInfo parse_channel[] = {
	{ "Name",			TOK_PRIMARY_KEY|TOK_STRING(Channel,name,0)	},
	{ "Description",	TOK_STRING(Channel,description,0)	},
	{ "Access",			TOK_INT(Channel,access,0)	},
	{ "Email",			TOK_STRUCT(Channel,email,parse_email) },
	{ "Invites",		TOK_INTARRAY(Channel,invites)	},
	{ "Timeout",		TOK_INT(Channel, timeout,0) },
	{ "{",				TOK_START,		0						},
	{ "}",				TOK_END,			0						},
	{ "", 0, 0 }
};

static void initUser(User *user, StashTable stash)
{
	int i, dirty = 0;
	int gmdirty = false;
	U32 now = timerSecondsSince2000();
	int converting = false;

	UserGmail *gmuser = chatUserGmailFind(user->auth_id);

	if (gmuser == NULL)
	{
		gmuser = userGmailAdd(user->auth_id, user->name);
		gmuser->gmail_id = user->gmail_id;
	}

	if(!stashAddPointer(stash, user->name, user, false))
	{
		char namebuf[MAX_PLAYERNAME+1], numbuf[20];
		User *user2 = stashFindPointerReturnPointer(stash, user->name);

		printf("\nWARNING: duplicate chat handle found: \"%s\" with auth ids %d and %d.\n", user->name, user2->auth_id, user->auth_id);

		strcpy(namebuf,user->name);
		for(i=1; ; i++)
		{
			strcpy(namebuf,user->name);
			itoa(i,numbuf,10);
			namebuf[MAX_PLAYERNAME - strlen(numbuf)] = '\0';
			strcat(namebuf,numbuf);

			if (stashAddPointer(stash,namebuf,user, false))
				break;
		}

		printf("  Renaming chat handle with auth id %d to \"%s\"\n\n", user->auth_id, user->name);
		StructFreeString(user->name);
		user->name = StructAllocString(namebuf);
		user->access &= ~CHANFLAGS_RENAMED;

		dirty |= 1;
	}

	for(i = eaSize(&user->watching)-1; i >= 0; i--)
	{
		Watching	*watch = user->watching[i];
		Channel		*channel = chatChannelFind(watch->name);
		if (!channel || (channel->timeout && now - user->last_online > channel->timeout * 86400) )
		{
			StructDestroy(parse_watching, eaRemove(&user->watching, i));
			dirty |= 2;
			continue;
		}
		watch->channel = channel;
		eaPush(&channel->members,user);
	}

	for(i = eaiSize(&user->friends)-1; i >= 0; i--)
	{
		User *my_friend = chatUserFind(user->friends[i]);
		if(!my_friend)
		{
			eaiRemove(&user->friends, i);
			dirty |= 4;
			continue;
		}
		eaPush(&my_friend->befrienders,user);
		if(eaiFind(&my_friend->friends, user->auth_id) < 0)
		{
			eaiRemove(&user->friends, i);
			dirty |= 8;
		}
	}

	for(i = eaiSize(&user->ignore)-1; i >= 0; i--)
	{
		User *my_ignore = chatUserFind(user->ignore[i]);
		if(!my_ignore)
		{
			eaiRemove(&user->ignore, i);
			dirty |= 16;
			continue;
		}
		eaiPush(&my_ignore->ignoredBy,user->auth_id);
	}

	for(i = 0; i < eaSize(&user->email); i++)
	{
		if(!user->email[i]->body)
			user->email[i]->body = StructAllocString("");
	}


	// convert old gmail
	if (eaSize(&user->gmail) > 0)
		converting = true;

	for (i = eaSize(&user->gmail) - 1; i >= 0; i-- )
	{
		Email *mail = user->gmail[i];
		Email *email = ParserAllocStruct(sizeof(*email));

		if (!mail || !email)
			continue;

		email->id = mail->id;
		email->sent = mail->sent;
		email->from = mail->from;
		email->to = mail->to;
		if( mail->senderName )
			email->senderName = ParserAllocString(mail->senderName);
		email->body = ParserAllocString(mail->body);
		if( mail->subj )
			email->subj = ParserAllocString(mail->subj);
		if( mail->attachment )	
			email->attachment = ParserAllocString(mail->attachment);
		email->status = mail->status;
		email->claim_request = mail->claim_request;
		eaPush(&gmuser->gmail, email);

		emailRemove(&user->gmail, i);
		gmdirty = true;
		dirty |= 32;
	}

	// convert old sent mail
	if (eaSize(&user->sent_mail) > 0)
		converting = true;

	for (i = eaSize(&user->sent_mail) - 1; i >= 0; i-- )
	{
		SentMail *mail = user->sent_mail[i];
		SentMail *email = ParserAllocStruct(sizeof(*email));

		if (email != NULL)
		{
			email->sentTo = mail->sentTo;
			email->sent = mail->sent;
			email->attachment =  ParserAllocString(mail->attachment);
			email->id = mail->id;
			email->attachmentClaimTimestamp = mail->attachmentClaimTimestamp;

			eaPush(&gmuser->sent_mail, email);

			if(mail->attachment)
				ParserFreeString(mail->attachment);
			ParserFreeStruct(mail);
			eaRemove(&user->sent_mail, i);
		}
		gmdirty = true;
		dirty |= 64;
		converting = true;
	}

	if (dirty)
		chatUserInsertOrUpdate(user);

	if (gmdirty)
		chatUserGmailInsertOrUpdate(gmuser);
}

static void initChannel(Channel *channel)
{
	int i, dirty = 0;

	for(i = eaiSize(&channel->invites)-1; i >= 0; --i)
	{
		User *user = chatUserFind(channel->invites[i]);
		if(user && channelIdx(user, channel) < 0)
		{
			eaPush(&user->invites, channel);
		}
		else
		{
			eaiRemove(&channel->invites, i);
			dirty = 1;
		}
	}

	if(!channel->description)
		channel->description = StructAllocString("");

	for(i = 0; i < eaSize(&channel->email); i++)
	{
		if(!channel->email[i]->body)
			channel->email[i]->body = StructAllocString("");
	}

	if(dirty)
		chatChannelInsertOrUpdate(channel);

	channelDeleteIfEmpty(channel);
}

static void initUsers(void)
{
	int i;
	User **users = chatUserGetAll();
	assert(!users_by_name);
	users_by_name = stashTableCreateWithStringKeys(1000, StashDeepCopyKeys);
	for(i = eaSize(&users)-1; i >= 0; --i)
		initUser(users[i], users_by_name);	
}

static void initChannels(void)
{
	int i;
	Channel **channels = chatChannelGetAll();
	for(i = eaSize(&channels)-1; i >= 0; --i)
		initChannel(channels[i]);
}

void chatDbInit(void)
{
	logSetDir("chatserver");
	chatsql_open();

	loadstart_printf("Loading Users...");
	ChatDb_LoadUsers();
	loadend_printf("");
	
	loadstart_printf("Loading Channels...");
	ChatDb_LoadChannels();
	loadend_printf("");

	loadstart_printf("Loading Gmail...");
	ChatDb_LoadUserGmail();
	loadend_printf("");

	loadstart_printf("Initializing Users...");
	initUsers();
	loadend_printf("");

	loadstart_printf("Initializing Channels...");
	initChannels();
	loadend_printf("");

	LOG(LOG_TEXT_JOURNAL, LOG_LEVEL_VERBOSE, LOG_CONSOLE_ALWAYS, "Loaded %d channels and %d users.", chatChannelGetCount(), chatUserGetCount());

	ChatDb_FlushChanges();
}

void chatDBShutDown(void)
{
	ChatDb_FlushChanges();
	chatsql_close();
}
