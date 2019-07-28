#ifndef _CHATSQLDB_H
#define _CHATSQLDB_H

typedef struct User User;
typedef struct Channel Channel;
typedef struct UserGmail UserGmail;

void ChatDb_LoadUsers(void);
void ChatDb_LoadChannels(void);
void ChatDb_LoadUserGmail(void);

void ChatDb_FlushChanges(void);

User *chatUserFind(U32 auth_id);
void chatUserInsertOrUpdate(User *user);
User **chatUserGetAll(void);
U32 chatUserGetCount(void);

Channel *chatChannelFind(const char *name);
void chatChannelInsertOrUpdate(Channel *channel);
void chatChannelDelete(Channel *channel);
Channel **chatChannelGetAll(void);
U32 chatChannelGetCount(void);

UserGmail *chatUserGmailFind(U32 auth_id);
void chatUserGmailInsertOrUpdate(UserGmail *gmuser);

#endif
