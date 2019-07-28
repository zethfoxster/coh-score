#ifndef CHATSQL_H
#define CHATSQL_H

#include "sql/sqlconn.h"
#include "sql/sqlinclude.h"

typedef struct User User;
typedef struct Channel Channel;
typedef struct UserGmail UserGmail;

C_DECLARATIONS_BEGIN

void chatsql_open(void);
void chatsql_close(void);

bool chatsql_read_users_ver(U32* ver);
bool chatsql_read_channels_ver(U32* ver);
bool chatsql_read_user_gmail_ver(U32* ver);

bool chatsql_write_users_ver(U32 ver);
bool chatsql_write_channels_ver(U32 ver);
bool chatsql_write_user_gmail_ver(U32 ver);

bool chatsql_read_users(ParseTable pti[], User*** hUsers);
bool chatsql_read_channels(ParseTable pti[], Channel*** hChannels);
bool chatsql_read_user_gmail(ParseTable pti[], UserGmail*** hUserGmail);

bool chatsql_insert_or_update_user(U32 auth_id, const char** estr_user);
bool chatsql_insert_or_update_channel(const char *name, const char** estr_channel);
bool chatsql_insert_or_update_user_gmail(U32 auth_id, const char** estr_user_gmail);

bool chatsql_delete_channel(const char* name);

C_DECLARATIONS_END

#endif
