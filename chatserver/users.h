#ifndef _USERS_H
#define _USERS_H

#include "chatdb.h"
#include "netio.h"

extern int g_online_count;

void userSetGmailXactDebug(User *user, char *debugState );

User *userFindByNameSafe(User *from,char *user_name);
User *userFindByNameIgnorable(User *from,char *user_name);
User *userFindByName(char *user_name);
User *userFindByAuthId(U32 id);
User *userFindIgnorable(User * from, char * id, int idType);
User *userFind(User * from, char * id, int idType);
int userCanTalk(User *user);

int userChangeName(User *user,User *target,char *name);
void userUpdateName(User *user,User *target,char *old_name,char *new_name);

Email *emailPush(Email ***email_list,User * to, User *from, char * sender, char *subj, char *msg, char *attach, int delay);
void emailRemove(Email ***email_list,int idx);

Email *gmailPush(U32 to_auth_id, char *to_name, U32 from_auth_id, char *from_name, U32 original_from_auth_id, char *subj, 
				 char *msg, char *attach, int delay);

void gmailRemove(UserGmail *gmUser, int mail_id);

void userLogoutByLink(NetLink *link);
void userLogin(User *user,NetLink *link,int from_shard,int access_level,int db_id);
void userLogout(User *user);

void userGetEmail(User *user);
void userSend(User *user,char *target_name,char *msg);
void userName(User *user,char *name);
User *userAdd(U32 auth_id,char *name);
UserGmail *userGmailAdd(U32 auth_id, char *name);
void userCsrName(User *user,char *old_name,char *new_name);
void userDoNotDisturb(User *user,char *new_dnd);

const char * InvalidName(char * name, bool adminAccess);
int userWatchingCount(User *user);

int userGetRelativeDbId(User * user, User * member);
int userSilencedTime(User * user);

void getGlobal( User *user, char * AuthId );
void getGlobalSilent( User *user, char * AuthId );
void getLocal( User *user, char * name );
void getLocalInvite( User *user, char * name );
void getLocalLeagueInvite( User *user, char * name );
void getLocalTutInvite( User *user, char * name );

void systemSendGmail(User *user, char * senderName, char *subj, char *msg, char* attachment, char * delay );

void userGmailDelete(User *user, char * id);
void userGmailReturn( User * user, char * id);
void bounceGmail( User *user, Email *mail, int allowOriginal );

void userXactRequestGmail(User *user,char *target_name,char *subj, char *msg, char*attachment);
void userCommitRequestGmail(User *user, char *target_name, char *xact_id_str);
void userGmailClaim(User *user, char * id);
void userGmailClaimConfirm(User *user, char * id);
void userConvertToNewGmail(User *starter);

extern int g_silence_all;

#endif

