#ifndef _SQLINCLUDE_H
#define _SQLINCLUDE_H

#include "wininclude.h"
#include <sqlext.h>

#if 1
#define SQLNCLI_NO_BCP
#include "mssql/sqlncli.h"
#else
#include <odbcss.h>
#endif

#define SQLAllocConnect ODBC_deprecated
#define SQLAllocEnv ODBC_deprecated
#define SQLAllocConnect ODBC_deprecated
#define SQLAllocEnv ODBC_deprecated
#define SQLAllocStmt ODBC_deprecated
#define SQLBindParam ODBC_deprecated
#define SQLColAttributes ODBC_deprecated
#define SQLError ODBC_deprecated
#define SQLFreeConnect ODBC_deprecated
#define SQLFreeEnv ODBC_deprecated
//#define SQLFreeStmt ODBC_deprecated
#define SQLGetConnectOption ODBC_deprecated
#define SQLGetStmtOption ODBC_deprecated
#define SQLParamOptions ODBC_deprecated
#define SQLSetConnectOption ODBC_deprecated
#define SQLSetParam ODBC_deprecated
#define SQLSetScrollOption ODBC_deprecated
#define SQLSetStmtOption ODBC_deprecated
#define SQLTransact  ODBC_deprecated

/// test for not SQL_SUCCESS or SQL_NO_DATA
#define SQL_SHOW_ERROR(rc) (((rc)&(~SQL_NO_DATA))!=0)

/// test if an exception occured
/// @note SQL_STILL_EXECUTING, SQL_NO_DATA, SQL_NEED_DATA do not count as failures here
#define SQL_NEEDS_RETRY(rc) (rc == SQL_ERROR)

/// skip "Class Code 23 Constraint Violation" as they will not succeed in future attempts either
#define SQLSTATE_SKIP_RETRY(state) (state[0] == '2' && state[1] == '3')

#define MSSQL_DATETIME_PRECISION (sizeof("yyyy-mm-dd hh:mm:ss.fff")-1)
#define MSSQL_DATETIME_DECIMALDIGITS 3
#define MSSQL_SMALLDATETIME_PRECISION (sizeof("yyyy-mm-dd hh:mm:ss")-1)
#define MSSQL_SMALLDATETIME_DECIMALDIGITS 0

#endif
