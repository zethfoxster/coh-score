#include "AuctionDb.h"
#include "AuctionSql.h"
#include "AuctionServer.h"
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

#define SHARD_NAME_COLUMN_LENGTH 50
#define IDENTIFIER_COLUMN_LENGTH 255

/// Stored procedure cache indices
enum aucsql_stored_proc {
	AUCSQL_READ_SHARDS=0,
	AUCSQL_READ_ENTS,
	AUCSQL_READ_HISTORY,
	AUCSQL_INSERT_SHARD,
	AUCSQL_INSERT_OR_UPDATE_ENT,
	AUCSQL_INSERT_OR_UPDATE_HISTORY,
	AUCSQL_DELETE_ENT,
	AUCSQL_STORED_PROCEDURE_MAX
};

/// Stored procedure names
static const char * aucsql_stored_procedure_names[] = {
	"AUCSQL_READ_SHARDS",
	"AUCSQL_READ_ENTS",
	"AUCSQL_READ_HISTORY",
	"AUCSQL_INSERT_SHARD",
	"AUCSQL_INSERT_OR_UPDATE_ENT",
	"AUCSQL_INSERT_OR_UPDATE_HISTORY",
	"AUCSQL_DELETE_ENTS",
};
STATIC_ASSERT(ARRAY_SIZE(aucsql_stored_procedure_names) == AUCSQL_STORED_PROCEDURE_MAX);

struct aucsql_globals {
	/// Stored procedures
	HSTMT proc[AUCSQL_STORED_PROCEDURE_MAX];
} as;

void aucsql_open() {
	assert(sqlConnInit(1));
	sqlConnSetLogin(g_auctionstate.cfg.sqlLogin);

	if (!sqlConnDatabaseConnect(g_auctionstate.cfg.sqlDbName, "AUCTIONSERVER"))
	{
		FatalErrorf("AuctionServer could not connect to SQL database.");
	}

	memset(as.proc, 0, sizeof(as.proc));
}

void aucsql_close() {
	int j;

	for (j=0; j<AUCSQL_STORED_PROCEDURE_MAX; j++) {
		if (as.proc[j]) {
			sqlConnStmtFree(as.proc[j]);
			as.proc[j] = NULL;
		}
	}

	sqlConnShutdown();
}

bool aucsql_read_shards_ver(U32* ver)
{
	return plSqlReadCrc("shards", ver, SQLCONN_FOREGROUND);
}

bool aucsql_read_ents_ver(U32* ver)
{
	return plSqlReadCrc("ents", ver, SQLCONN_FOREGROUND);
}

bool aucsql_read_history_ver(U32* ver)
{
	return plSqlReadCrc("history", ver, SQLCONN_FOREGROUND);
}

bool aucsql_write_shards_ver(U32 ver)
{
	return plSqlWriteCrc("shards", ver, SQLCONN_FOREGROUND);
}

bool aucsql_write_ents_ver(U32 ver)
{
	return plSqlWriteCrc("ents", ver, SQLCONN_FOREGROUND);
}

bool aucsql_write_history_ver(U32 ver)
{
	return plSqlWriteCrc("history", ver, SQLCONN_FOREGROUND);
}

bool aucsql_read_shards(ParseTable pti[], AuctionShard*** hShards)
{
	return plSqlReadData(&as.proc[AUCSQL_READ_SHARDS], "shards", pti, sizeof(AuctionShard), (void***)hShards, SQLCONN_FOREGROUND);
}

bool aucsql_read_ents(ParseTable pti[], AuctionEnt*** hEnts)
{
	return plSqlReadData(&as.proc[AUCSQL_READ_ENTS], "auction_ents", pti, sizeof(AuctionEnt), (void***)hEnts, SQLCONN_FOREGROUND);
}

bool aucsql_read_history(ParseTable pti[], AuctionHistoryItem*** hHistItems)
{
	return plSqlReadData(&as.proc[AUCSQL_READ_HISTORY], "history", pti, sizeof(AuctionHistoryItem), (void***)hHistItems, SQLCONN_FOREGROUND);
}

bool aucsql_insert_shard(const char** estr_shard)
{
	if (!plSqlPrepare(&as.proc[AUCSQL_INSERT_SHARD], "{CALL dbo.SP_insert_shard(@data=?)}", SQL_NTS, SQLCONN_FOREGROUND))
		return false;

	HSTMT stmt = as.proc[AUCSQL_INSERT_SHARD];
	ssize_t shard_length = estrLength(estr_shard);

	sqlConnStmtBindParam(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (void*)*estr_shard, shard_length, &shard_length);

	return plSqlExecute(stmt, aucsql_stored_procedure_names[AUCSQL_INSERT_SHARD], SQLCONN_FOREGROUND);
}

bool aucsql_insert_or_update_ent(int ent_id, const char* shard, const char** estr_ent)
{
	if (!plSqlPrepare(&as.proc[AUCSQL_INSERT_OR_UPDATE_ENT], "{CALL dbo.SP_insert_or_update_ent(@ent_id=?,@shard_name=?,@data=?)}", SQL_NTS, SQLCONN_FOREGROUND))
		return false;

	HSTMT stmt = as.proc[AUCSQL_INSERT_OR_UPDATE_ENT];
	ssize_t shard_length = strlen(shard);
	ssize_t ent_length = estrLength(estr_ent);

	sqlConnStmtBindParam(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &ent_id, sizeof(ent_id), NULL);
	sqlConnStmtBindParam(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, SHARD_NAME_COLUMN_LENGTH, 0, (void*)shard, shard_length, &shard_length);
	sqlConnStmtBindParam(stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (void*)*estr_ent, ent_length, &ent_length);

	return plSqlExecute(stmt, aucsql_stored_procedure_names[AUCSQL_INSERT_OR_UPDATE_ENT], SQLCONN_FOREGROUND);
}

bool aucsql_insert_or_update_history(const char* identifier, const char** estr_history)
{
	if (!plSqlPrepare(&as.proc[AUCSQL_INSERT_OR_UPDATE_HISTORY], "{CALL dbo.SP_insert_or_update_history(@identifier=?,@data=?)}", SQL_NTS, SQLCONN_FOREGROUND))
		return false;

	HSTMT stmt = as.proc[AUCSQL_INSERT_OR_UPDATE_HISTORY];
	ssize_t idenitifer_length = strlen(identifier);
	ssize_t history_length = estrLength(estr_history);

	sqlConnStmtBindParam(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, IDENTIFIER_COLUMN_LENGTH, 0, (void*)identifier, idenitifer_length, &idenitifer_length);
	sqlConnStmtBindParam(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (void*)*estr_history, history_length, &history_length);

	return plSqlExecute(stmt, aucsql_stored_procedure_names[AUCSQL_INSERT_OR_UPDATE_HISTORY], SQLCONN_FOREGROUND);
}

bool aucsql_delete_ent(int ent_id, const char* shard)
{
	if (!plSqlPrepare(&as.proc[AUCSQL_DELETE_ENT], "{CALL dbo.SP_delete_ent(@ent_id=?,@shard_name=?)}", SQL_NTS, SQLCONN_FOREGROUND))
		return false;

	HSTMT stmt = as.proc[AUCSQL_DELETE_ENT];
	ssize_t shard_length = strlen(shard);

	sqlConnStmtBindParam(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &ent_id, sizeof(ent_id), NULL);
	sqlConnStmtBindParam(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, SHARD_NAME_COLUMN_LENGTH, 0, (void*)shard, shard_length, &shard_length);

	return plSqlExecute(stmt, aucsql_stored_procedure_names[AUCSQL_DELETE_ENT], SQLCONN_FOREGROUND);
}
