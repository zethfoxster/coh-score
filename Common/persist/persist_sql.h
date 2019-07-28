#pragma once

C_DECLARATIONS_BEGIN

#include "sql/sqlconn.h"
#include "sql/sqlinclude.h"
#include "utils/textparser.h"

bool plSqlPrepare(HSTMT* stmt, char* cmd, int cmd_len, SqlConn conn);
bool plSqlExecute(HSTMT stmt, const char* original_command, SqlConn conn);
bool plSqlReadCrc(const char* table, U32* ver, SqlConn conn);
bool plSqlWriteCrc(const char* table, U32 ver, SqlConn conn);
bool plSqlReadData(HSTMT* stmt, const char* table, ParseTable pti[], size_t structsize, void*** earray, SqlConn conn);

C_DECLARATIONS_END
