/**
 * Lock.h
 *
 * Lock class declaration
 *
 * author: woojeong
 *
 * created: 2002-03-11
 *
**/

#if !defined(AFX_LOCK_H__F2FDB4C3_6D4A_4344_BC16_923008A7A9B3__INCLUDED_)
#define AFX_LOCK_H__F2FDB4C3_6D4A_4344_BC16_923008A7A9B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GlobalAuth.h"

enum LockType
{
    eCriticalSection,
    eSystemSpinLock,
    eCustomSpinLock
};

class CLock  
{
protected:
    CRITICAL_SECTION m_critical;
    long m_customLock;
    LockType m_type;
    void Wait();

public:

	CLock(LockType a = eCriticalSection, DWORD spinCount = 0);
	~CLock();
    void Enter();
    void Leave();
    bool TryEnter();
};

class CRWLock {
public:
	CRWLock();
	~CRWLock();

	void ReadLock();
	void ReadUnlock();
	void WriteLock();
	bool WriteTryLock();
	void WriteUnlock();

private:
	CRITICAL_SECTION mutex;
	HANDLE dataLock;
	int readerCount;
};

#endif // !defined(AFX_LOCK_H__F2FDB4C3_6D4A_4344_BC16_923008A7A9B3__INCLUDED_)
