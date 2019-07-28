#include "chatdb.h"
#include "persist/persist_sql.h"
#include "sql/sqlconn.h"
#include "sql/sqlinclude.h"
#include "sql/sqltask.h"
#include "components/earray.h"
#include "components/EString.h"
#include "components/StashTable.h"
#include "components/MemoryPool.h"
#include "utils/error.h"
#include "utils/mathutil.h"
#include "utils/SuperAssert.h"
#include "utils/utils.h"
#include "textparser.h"
#include "shardnet.h"

#define CHANNEL_NAME_COLUMN_LENGTH 32

/// Stored procedure cache indices
enum chatsql_stored_proc {
	CHATSQL_READ_USERS=0,
	CHATSQL_READ_CHANNELS,
	CHATSQL_READ_USER_GMAIL,
	CHATSQL_INSERT_OR_UPDATE_USER,
	CHATSQL_INSERT_OR_UPDATE_CHANNEL,
	CHATSQL_INSERT_OR_UPDATE_USER_GMAIL,
	CHATSQL_DELETE_CHANNEL,
	CHATSQL_STORED_PROCEDURE_MAX
};

/// Stored procedure names
static const char * chatsql_stored_procedure_names[] = {
	"CHATSQL_READ_USERS",
	"CHATSQL_READ_CHANNELS",
	"CHATSQL_READ_USER_GMAIL",
	"CHATSQL_INSERT_OR_UPDATE_USER",
	"CHATSQL_INSERT_OR_UPDATE_CHANNEL",
	"CHATSQL_INSERT_OR_UPDATE_USER_GMAIL",
	"CHATSQL_DELETE_CHANNELS",
};
STATIC_ASSERT(ARRAY_SIZE(chatsql_stored_procedure_names) == CHATSQL_STORED_PROCEDURE_MAX);

struct chatsql_globals {
	/// Stored procedures
	HSTMT proc[CHATSQL_STORED_PROCEDURE_MAX];
} cs;

C_DECLARATIONS_BEGIN

void chatsql_open(void) {
	assert(sqlConnInit(1));
	sqlConnSetLogin(g_chatcfg.sqlLogin);

	if (!sqlConnDatabaseConnect(g_chatcfg.sqlDbName, "CHATSERVER"))
	{
		FatalErrorf("ChatServer could not connect to SQL database.");
	}

	memset(cs.proc, 0, sizeof(cs.proc));
}

void chatsql_close(void) {
	int j;

	for (j=0; j<CHATSQL_STORED_PROCEDURE_MAX; j++) {
		if (cs.proc[j]) {
			sqlConnStmtFree(cs.proc[j]);
			cs.proc[j] = NULL;
		}
	}

	sqlConnShutdown();
}

bool chatsql_read_users_ver(U32* ver)
{
	return plSqlReadCrc("users", ver, SQLCONN_FOREGROUND);
}

bool chatsql_read_channels_ver(U32* ver)
{
	return plSqlReadCrc("channels", ver, SQLCONN_FOREGROUND);
}

bool chatsql_read_user_gmail_ver(U32* ver)
{
	return plSqlReadCrc("user_gmail", ver, SQLCONN_FOREGROUND);
}

bool chatsql_write_users_ver(U32 ver)
{
	return plSqlWriteCrc("users", ver, SQLCONN_FOREGROUND);
}

bool chatsql_write_channels_ver(U32 ver)
{
	return plSqlWriteCrc("channels", ver, SQLCONN_FOREGROUND);
}

bool chatsql_write_user_gmail_ver(U32 ver)
{
	return plSqlWriteCrc("user_gmail", ver, SQLCONN_FOREGROUND);
}

bool chatsql_read_users(ParseTable pti[], User*** hUsers)
{
	return plSqlReadData(&cs.proc[CHATSQL_READ_USERS], "users", pti, sizeof(User), (void***)hUsers, SQLCONN_FOREGROUND);
}

bool chatsql_read_channels(ParseTable pti[], Channel*** hChannels)
{
	return plSqlReadData(&cs.proc[CHATSQL_READ_CHANNELS], "channels", pti, sizeof(Channel), (void***)hChannels, SQLCONN_FOREGROUND);
}

bool chatsql_read_user_gmail(ParseTable pti[], UserGmail*** hUserGmail)
{
	return plSqlReadData(&cs.proc[CHATSQL_READ_USER_GMAIL], "user_gmail", pti, sizeof(UserGmail), (void***)hUserGmail, SQLCONN_FOREGROUND);
}

bool chatsql_insert_or_update_user(U32 auth_id, const char** estr_user)
{
	if (!plSqlPrepare(&cs.proc[CHATSQL_INSERT_OR_UPDATE_USER], "{CALL dbo.SP_insert_or_update_user(@user_id=?,@data=?)}", SQL_NTS, SQLCONN_FOREGROUND))
		return false;

	HSTMT stmt = cs.proc[CHATSQL_INSERT_OR_UPDATE_USER];
	ssize_t user_length = estrLength(estr_user);

	sqlConnStmtBindParam(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &auth_id, sizeof(auth_id), NULL);
	sqlConnStmtBindParam(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (void*)*estr_user, user_length, &user_length);

	return plSqlExecute(stmt, chatsql_stored_procedure_names[CHATSQL_INSERT_OR_UPDATE_USER], SQLCONN_FOREGROUND);
}

bool chatsql_insert_or_update_channel(const char *name, const char** estr_channel)
{
	if (!plSqlPrepare(&cs.proc[CHATSQL_INSERT_OR_UPDATE_CHANNEL], "{CALL dbo.SP_insert_or_update_channel(@name=?,@data=?)}", SQL_NTS, SQLCONN_FOREGROUND))
		return false;

	HSTMT stmt = cs.proc[CHATSQL_INSERT_OR_UPDATE_CHANNEL];
	ssize_t name_length = strlen(name);
	ssize_t channel_length = estrLength(estr_channel);

	sqlConnStmtBindParam(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, CHANNEL_NAME_COLUMN_LENGTH, 0, (void*)name, name_length, &name_length);
	sqlConnStmtBindParam(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (void*)*estr_channel, channel_length, &channel_length);

	return plSqlExecute(stmt, chatsql_stored_procedure_names[CHATSQL_INSERT_OR_UPDATE_CHANNEL], SQLCONN_FOREGROUND);
}

bool chatsql_insert_or_update_user_gmail(U32 auth_id, const char** estr_user_gmail)
{
	if (!plSqlPrepare(&cs.proc[CHATSQL_INSERT_OR_UPDATE_USER_GMAIL], "{CALL dbo.SP_insert_or_update_user_gmail(@user_gmail_id=?,@data=?)}", SQL_NTS, SQLCONN_FOREGROUND))
		return false;

	HSTMT stmt = cs.proc[CHATSQL_INSERT_OR_UPDATE_USER_GMAIL];
	ssize_t user_gmail_length = estrLength(estr_user_gmail);

	sqlConnStmtBindParam(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &auth_id, sizeof(auth_id), NULL);
	sqlConnStmtBindParam(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (void*)*estr_user_gmail, user_gmail_length, &user_gmail_length);

	return plSqlExecute(stmt, chatsql_stored_procedure_names[CHATSQL_INSERT_OR_UPDATE_USER_GMAIL], SQLCONN_FOREGROUND);
}

bool chatsql_delete_channel(const char* name)
{
	if (!plSqlPrepare(&cs.proc[CHATSQL_DELETE_CHANNEL], "{CALL dbo.SP_delete_channel(@name=?)}", SQL_NTS, SQLCONN_FOREGROUND))
		return false;

	HSTMT stmt = cs.proc[CHATSQL_DELETE_CHANNEL];

	ssize_t name_length = strlen(name);
	sqlConnStmtBindParam(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, CHANNEL_NAME_COLUMN_LENGTH, 0, (void*)name, name_length, &name_length);

	return plSqlExecute(stmt, chatsql_stored_procedure_names[CHATSQL_DELETE_CHANNEL], SQLCONN_FOREGROUND);
}

C_DECLARATIONS_END
