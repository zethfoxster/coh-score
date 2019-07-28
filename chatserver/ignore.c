#include "users.h"
#include "ignore.h"
#include "earray.h"
#include "shardnet.h"
#include "textparser.h"
#include "msgsend.h"
#include "EString.h"
#include "mathutil.h"
#include "timing.h"
#include "chatsqldb.h"

int ignore_spammer_threshold = 100;
int ignore_spammer_multiplier = 5;
int ignore_spammer_duration = 24*60*60;

void setSpammerThreshold(User *user, char *seconds )
{ 
	ignore_spammer_threshold = atoi(seconds);  
	sendSysMsg(user,0,1,"SpamThresholdSet %d", ignore_spammer_threshold); 
}

void setSpammerMultiplier(User *user, char *seconds )
{ 
	ignore_spammer_multiplier = atoi(seconds);
	sendSysMsg(user,0,1,"SpamMultiplierSet %d", ignore_spammer_multiplier);
}

void setSpammerDuration(User *user, char *seconds )

{ 
	ignore_spammer_duration = atoi(seconds);
	sendSysMsg(user,0,1,"SpamDurationSet %d", ignore_spammer_duration);
}


static void ignoreUser( User *user, User * ignore )
{
	if (eaiFind(&user->ignore,ignore->auth_id) < 0)
	{
		if (eaiSize(&user->ignore) >= MAX_IGNORES)
		{
			User *listening = userFindByAuthId(user->ignore[0]);

			if (listening)
			{
				sendSysMsg(user,0,1,"IgnoreListFullListeningTo %s",listening->name);
				eaiFindAndRemove(&listening->ignoredBy, user->auth_id);
			}
			eaiRemove(&user->ignore,0);
		}
		eaiPush(&ignore->ignoredBy, user->auth_id);
		eaiPush(&user->ignore,ignore->auth_id);
	}
	chatUserInsertOrUpdate(user);
	sendSysMsg(user,0,0,"Ignoring %s %d",ignore->name, userGetRelativeDbId(user, ignore));
	ignoreRefreshList(user);
}

static void spamReportUser( User *user, User * spammer )
{
	if(ignore_spammer_threshold && user->reported_spam_count < ignore_spammer_multiplier)
	{
		int multiplier = MAX(ignore_spammer_multiplier - user->reported_spam_count, 1);
		user->reported_spam_count++;

		spammer->spam_ignore_count += multiplier;
		if( !(spammer->access&CHANFLAGS_SPAMMER) && ignore_spammer_threshold && spammer->spam_ignore_count >= ignore_spammer_threshold )
		{
			U32 time = timerSecondsSince2000();
			spammer->access |= CHANFLAGS_SPAMMER;
			spammer->chat_ban_expire = MAX( spammer->chat_ban_expire, time + ignore_spammer_duration );
			sendUserMsg(spammer,"SendSpamPetition %d 1", spammer->chat_ban_expire - time );
		}
	}
}

void unSpamMe( User * user )
{
	user->access &= ~(CHANFLAGS_SPAMMER);
	user->chat_ban_expire = 0;
	user->spam_ignore_count = 0;
	sendUserMsg(user, "ClearSpam" );
}

static void unIgnoreUser(User *user, User *ignore)
{
	eaiFindAndRemove(&ignore->ignoredBy,user->auth_id);
	if (eaiFindAndRemove(&user->ignore,ignore->auth_id) < 0)
		sendSysMsg(user,0,0,"NotIgnore %s",ignore->name);
	 else 
		sendSysMsg(user,0,0,"Unignore %s",ignore->name);
	chatUserInsertOrUpdate(user);
	ignoreRefreshList(user);
}

void ignore(User *user,char *ignore_name)
{
	User *ignore = userFindByNameSafe(user,ignore_name);
	if (!ignore)
	{
		return;
	}
	ignoreUser( user, ignore );
}

void ignoreSpammer(User *user,char *ignore_name)
{
	User *ignore = userFindByNameSafe(user,ignore_name);
	if (!ignore)
	{
		return;
	}

	ignoreUser( user, ignore );
	spamReportUser( user, ignore );
}

void ignoreAuth(User *user, char *authId )
{
	User *ignore = userFindByAuthId(atoi(authId));
	if (!ignore)
		return;
	ignoreUser( user, ignore );
}

void ignoreAuthSpammer(User *user, char *authId )
{
	User *ignore = userFindByAuthId(atoi(authId));
	if (!ignore)
		return;
	ignoreUser( user, ignore );
	spamReportUser( user, ignore );
}

void unIgnore(User *user,char *ignore_name)
{
	User *ignore = userFindByNameSafe(user,ignore_name);
	if (!ignore)
		return;
	unIgnoreUser( user, ignore );
}

void unIgnoreAuth(User *user, char* authId )
{
	User *ignore = 0;

	if( authId )
		ignore = userFindByAuthId(atoi(authId));
	if (!ignore)
		return;
	unIgnoreUser( user, ignore );
}

void ignoreList(User *user)
{
	int		i;
	User	*ignore;

	for(i=eaiSize(&user->ignore)-1;i>=0;i--)
	{
		ignore = userFindByAuthId(user->ignore[i]);
		if (ignore)
			sendSysMsg(user,0,0,"Ignoring %s %d",ignore->name, userGetRelativeDbId(user, ignore));
	}
}

void ignoreListUpdate(User *user)
{
	int		i;
	User	*ignore;

	for(i=eaiSize(&user->ignore)-1;i>=0;i--)
	{
		ignore = userFindByAuthId(user->ignore[i]);
		if (ignore)
			sendSysMsg(user,0,0,"IgnoringUpdate %s %d",ignore->name, userGetRelativeDbId(user, ignore));
	}
}

void notifyIgnoredBy(User * user)
{
	int		i;
	User	*ignoredBy;

	for(i=eaiSize(&user->ignoredBy)-1;i>=0;i--)
	{
		ignoredBy = userFindByAuthId(user->ignoredBy[i]);
		if (ignoredBy)
			sendSysMsg(ignoredBy,0,0,"IgnoringUpdate %s %d",user->name, userGetRelativeDbId(ignoredBy, user));
	}
}

int isIgnoring(User *user, User *ignoree)
{
	return eaiFind(&user->ignore, ignoree->auth_id) >= 0;
}

void ignoreRefreshList(User *user)
{
	char *ignoreList = estrTemp();
	int i;

	estrConcatf( &ignoreList, "IgnoreAuth ");
	for(i=eaiSize(&user->ignore)-1;i>=0;i--)
		estrConcatf( &ignoreList, "%i ", user->ignore[i] );
	estrConcatChar( &ignoreList, '0');
	sendUserMsg(user,ignoreList);

	estrDestroy(&ignoreList);
}
