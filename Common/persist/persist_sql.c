#include "persist_sql.h"
#include "sql/sqlconn.h"
#include "components/earray.h"
#include "components/EString.h"
#include "utils/error.h"
#include "utils/log.h"
#include "utils/mathutil.h"

bool plSqlPrepare(HSTMT* stmt, char* cmd, int cmd_len, SqlConn conn)
{
	int rc;

	// one time bind
	if (*stmt) {
		sqlConnStmtUnbindParams(*stmt);
		sqlConnStmtCloseCursor(*stmt);
		return true;
	}

	*stmt = sqlConnStmtAlloc(0);
	rc = sqlConnStmtPrepare(*stmt, cmd, cmd_len, conn);
	return SQL_SUCCEEDED(rc) != 0;
}

bool plSqlExecute(HSTMT stmt, const char* original_command, SqlConn conn)
{
	SQLRETURN ret = _sqlConnStmtExecute(stmt, conn);
	if (SQL_SHOW_ERROR(ret))
		sqlConnStmtPrintError(stmt, original_command);
	return SQL_SUCCEEDED(ret);
}

bool plSqlReadCrc(const char* table, U32* ver, SqlConn conn)
{
	SQLRETURN ret;
	ssize_t actual_bytes = sizeof(*ver);
	char* cmd = estrTemp();
	HSTMT stmt = sqlConnStmtAlloc(conn);

	estrPrintf(&cmd, "SELECT %s_crc from dbo.parse_versions;", table);

	sqlConnStmtBindCol(stmt, 1, SQL_C_LONG, ver, sizeof(*ver), &actual_bytes);

	ret = sqlConnStmtExecDirect(stmt, cmd, estrLength(&cmd), conn, false);
	if (SQL_SHOW_ERROR(ret))
		sqlConnStmtPrintError(stmt, cmd);

	if (SQL_SUCCEEDED(ret))
	{
		ret = _sqlConnStmtFetch(stmt, 0);
		if (ret == SQL_NO_DATA)
		{
			*ver = 0;
			return true;
		}
		if (SQL_SHOW_ERROR(ret))
			sqlConnStmtPrintError(stmt, cmd);
	}

	estrDestroy(&cmd);
	sqlConnStmtFree(stmt);
	return (SQL_SUCCEEDED(ret) != 0) && (actual_bytes == sizeof(*ver));
}

bool plSqlWriteCrc(const char* table, U32 ver, SqlConn conn)
{
	SQLRETURN ret;
	char* cmd = estrTemp();
	HSTMT stmt = sqlConnStmtAlloc(conn);

	estrPrintf(&cmd, "{CALL dbo.SP_write_%s_crc(@crc=?)}", table);

	sqlConnStmtBindParam(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &ver, sizeof(ver), NULL);

	ret = sqlConnStmtExecDirect(stmt, cmd, estrLength(&cmd), conn, false);
	if (SQL_SHOW_ERROR(ret))
		sqlConnStmtPrintError(stmt, cmd);

	estrDestroy(&cmd);
	sqlConnStmtFree(stmt);
	return (SQL_SUCCEEDED(ret) != 0);
}

bool plSqlReadData(HSTMT* stmt, const char* table, ParseTable pti[], size_t structsize, void*** earray, SqlConn conn)
{
	bool ok;
	SQLRETURN ret;
	char* cmd = estrTemp();

	estrPrintf(&cmd, "SELECT data FROM dbo.%s;", table);
	ok = plSqlPrepare(stmt, cmd, estrLength(&cmd), conn);

	if (ok)
		ok = plSqlExecute(*stmt, cmd, conn);

	if (!ok)
	{
		estrDestroy(&cmd);
		return false;
	}

	while ((ret = _sqlConnStmtFetch(*stmt, 0)) != SQL_NO_DATA)
	{
		ssize_t actual_bytes;
		void* data;
		void* mem;

		if (ret != SQL_SUCCESS)
			sqlConnStmtPrintError(*stmt, cmd);

		if (!SQL_SUCCEEDED(ret))
			return false;

		data = sqlConnStmtGetData(*stmt, 1, SQL_C_CHAR, &actual_bytes, cmd, conn);
		if (!data || actual_bytes<1)
			FatalErrorf("Data could not be read or was empty: %d", actual_bytes);

		mem = StructAllocRaw(structsize);
		if (!ParserReadText((char*)data, actual_bytes, pti, mem))
			FatalErrorf("Data could not be parsed: %s", data);

		eaPush(earray, mem);
	}

	return true;
}
