#ifndef _SQLCONN_H
#define _SQLCONN_H

#define SHORT_SQL_STRING 1024
#define LONG_SQL_STRING 16*1024*1024

C_DECLARATIONS_BEGIN

// stolen evilly from sqltypes.h
typedef void* HSTMT;
typedef void* HDBC;

typedef unsigned int SqlConn;
#define SQLCONN_FOREGROUND 0
#define SQLCONN_MAX (64+1)

bool sqlConnInit(U32 connection_count);
void sqlConnShutdown(void);
void sqlConnSetLogin(char *s);

bool sqlConnDatabaseConnect(const char * db_name, const char * app_name);
int sqlConnGetInfo(unsigned info_type, bool ignore_error, SqlConn conn);
void sqlConnGetTimes(int * count, long * avg_usec, long * worst_usec, float * fore_idle_ratio, float * back_idle_ratio);

// statements commands
HSTMT sqlConnStmtAlloc(SqlConn conn);
void sqlConnStmtFree(HSTMT stmt);
void sqlConnStmtPrintBinds(HSTMT stmt);
int sqlConnStmtPrepare(HSTMT stmt, char *cmd, int str_len, SqlConn conn);
int sqlConnStmtExecDirect(HSTMT stmt, char *cmd, int str_len, SqlConn conn, bool utf8);
int sqlConnStmtExecDirectMany(HSTMT stmt, char *cmd, int str_len, SqlConn conn, bool utf8);

// statement commands that do not report errors automatically
int _sqlConnStmtExecute(HSTMT stmt, SqlConn conn);
int _sqlConnStmtFetch(HSTMT stmt, SqlConn conn);

// manual statement error reporting
bool sqlConnStmtPrintErrorAndGetState(HSTMT stmt, const char *original_command, char *sqlstate);
bool sqlConnStmtPrintError(HSTMT stmt, const char *original_command);
bool sqlConnStmtPrintErrorf(HSTMT stmt, char *fmt, ...);

// transactions
bool sqlConnEnableAsynchDBC(bool enable, SqlConn conn);
bool sqlConnEnableAutoTransactions(bool enable, SqlConn conn);
int sqlConnEndTransaction(bool commit, bool wait, SqlConn conn);

// connection
int sqlConnExecDirect(char *cmd, int cmd_len, SqlConn conn, bool utf8);
int sqlConnExecDirectMany(char *cmd, int cmd_len, SqlConn conn, bool utf8);

// parameter binding
int sqlConnStmtBindParamArray(HSTMT stmt, size_t count, size_t row_size);
int sqlConnStmtSetBindOffset(HSTMT stmt, size_t *offset);
int sqlConnStmtBindParam(HSTMT stmt, int param, int paramtype, int ctype, int sqltype, size_t colsize, int decimals, void* data, size_t buflen, ssize_t* strlen);
int sqlConnStmtStartBindParamTableValued(HSTMT stmt, int param, int maxrows, wchar_t *tabletype, ssize_t *numrows);
int sqlConnStmtEndBindParamTableValued(HSTMT stmt);
int sqlConnStmtUnbindParams(HSTMT stmt);

// column binding
int sqlConnStmtBindCol(HSTMT stmt, int col, int ctype, void* data, size_t buflen, ssize_t* strlen);
int sqlConnStmtUnbindCols(HSTMT stmt);

// streaming data
void* sqlConnStmtGetData(HSTMT stmt, int param, int ctype, ssize_t *length, const char *original_command, SqlConn conn);

// cursor
int sqlConnStmtCloseCursor(HSTMT stmt);

C_DECLARATIONS_END

#endif
