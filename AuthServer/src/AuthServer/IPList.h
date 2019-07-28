// IPList.h: interface for the CIPList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IPLIST_H__D54C2521_4012_489C_A37E_A84B3AFBF20E__INCLUDED_)
#define AFX_IPLIST_H__D54C2521_4012_489C_A37E_A84B3AFBF20E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GlobalAuth.h"

typedef struct {
	UINT ip;
	BYTE conn_num;
} IPACCESS;

typedef std::map< unsigned long, INT > ACCESSMAP;

class CIPAccessLimit
{
private:
	CRITICAL_SECTION lock[16];
	int GetIPLockValue( in_addr ip );
	ACCESSMAP accessmap[16];
public:
	bool SetAccessIP( in_addr ip );
	bool DelAccessIP( in_addr ip );
	CIPAccessLimit();
	virtual ~CIPAccessLimit();
};

class IPRecord
{
public:
	unsigned begin;
	unsigned end;
};

bool  inline operator < (const IPRecord &x, const IPRecord &y) 
{ return x.end < y.end; }
bool  inline operator < (const IPRecord &x, unsigned y) { return x.end < y; }
bool  inline operator < (unsigned x, const IPRecord &y) { return x < y.end; } 

class CIPList
{
protected:
	std::vector<IPRecord> table;
	CRWLock lock;

public:
	CIPList();

	BOOL Load(char *fileName);
	BOOL IpExists(in_addr ipAddr);
};

extern CIPAccessLimit IPaccessLimit;
extern CIPList forbiddenIPList;
extern CIPList allowedIPList;

#endif // !defined(AFX_IPLIST_H__D54C2521_4012_489C_A37E_A84B3AFBF20E__INCLUDED_)