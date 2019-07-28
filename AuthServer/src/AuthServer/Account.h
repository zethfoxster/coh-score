// Account.h: interface for the CAccount class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACCOUNT_H__113594CA_4C87_441A_A613_C58591534449__INCLUDED_)
#define AFX_ACCOUNT_H__113594CA_4C87_441A_A613_C58591534449__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_ACCOUNT_LEN 14
#define MAX_REGIONS 10
#define MAX_SSN_LEN 13
#define MAX_PWD_LEN 16
#define ENC_PWD_LEN 128

#include <sql.h>
#include <sqlext.h>

/*
struct tagDATE_STRUCT {
     SQLSMALLINT year; 
     SQLUSMALLINT month; 
     SQLUSMALLINT day; 
} DATE_STRUCT;
*/
class CAccount  
{
public:
	CAccount();
	virtual ~CAccount();
	char Load( const char *name );
	char LoadPassword( const char *name , char *pwd, unsigned char& hash_type, unsigned int& salt );
	char LoadEtc();
	char  CheckPassword( const char *name, char *dbpwdLineage2, char *dbpwdSHA512, int oneTimeKey, bool useMD5 );

	
	int block_flag;
	int block_flag2;
	int warn_flag;
	int pay_stat;
	int uid;
	int login_flag;
	int subscription_flag;
	int remainTime, quotaTime;
	int loyalty, loyaltyLegacy;
	int queueLevel;
	int MakeBlockInfo( char *msg );
	int nSSN,ssn2;
	char age;
	char gender;
	char account[MAX_ACCOUNT_LEN+1];
	char ssn[MAX_SSN_LEN+1];
	ServerId lastworld;
	SQL_TIMESTAMP_STRUCT  block_end_date;
    int regions[MAX_REGIONS];
};

#endif // !defined(AFX_ACCOUNT_H__113594CA_4C87_441A_A613_C58591534449__INCLUDED_)
