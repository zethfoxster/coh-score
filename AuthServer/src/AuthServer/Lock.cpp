/**
 * Lock.cpp
 *
 * Lock class definition
 *
 * author: woojeong
 *
 * created: 2002-03-11
 *
**/

#include "GlobalAuth.h"
#include "Lock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLock::CLock(LockType type, DWORD spinCount)
{
    m_type = type;

    if(type == eCustomSpinLock) {
        m_customLock = 0;
    } else if(type == eSystemSpinLock) {
        InitializeCriticalSectionAndSpinCount(&m_critical, spinCount);
    } else {
        InitializeCriticalSection(&m_critical);
    }
}

CLock::~CLock()
{
    if(m_type != eCustomSpinLock) {
        DeleteCriticalSection(&m_critical);
    }
}

bool CLock::TryEnter()
{
    if(m_type == eCustomSpinLock) {
		return InterlockedExchange(&m_customLock, 1) == 0;
    } else {
        return false;
    }
}

void CLock::Enter()
{
    if(m_type == eCustomSpinLock) {
        if(InterlockedExchange(&m_customLock, 1)) {
			Wait();
        }
    } else {
        EnterCriticalSection(&m_critical); 
    }
}

void CLock::Leave()
{
    if(m_type == eCustomSpinLock) {
        InterlockedExchange(&m_customLock, 0);
    } else {
        LeaveCriticalSection(&m_critical);
    }
}

void CLock::Wait()
{
    if(m_type != eCustomSpinLock) {
        return;
    }

	int count = 4000;
	while(--count >= 0) {
        if(InterlockedExchange(&m_customLock, 1) == 0) {
			return;
        }
        // pause
		__asm { rep nop} 
	}
	count = 4000;
	while(--count >= 0) {
		SwitchToThread();
        if(InterlockedExchange(&m_customLock, 1) == 0) {
			return;
        }
	}
	for( ; ; ) {
		Sleep(1000);
        if(InterlockedExchange(&m_customLock, 1) == 0) {
			return;
        }
	}
}


CRWLock::CRWLock()
{
	readerCount = 0;
	dataLock = CreateSemaphore(NULL, 1, 1, NULL);
	_ASSERTE(dataLock);
	InitializeCriticalSection(&mutex);
}

CRWLock::~CRWLock()
{
	CloseHandle(dataLock);
	DeleteCriticalSection(&mutex);
}

void CRWLock::ReadLock()
{
	EnterCriticalSection(&mutex);
	if (++readerCount == 1) {
		if (WaitForSingleObject(dataLock, INFINITE) != WAIT_OBJECT_0) {
			log.AddLog(LOG_ERROR, "ReadLock failed on dataLock");
		}
	}
	LeaveCriticalSection(&mutex);
}

void CRWLock::ReadUnlock()
{
	LONG prevCount;
	EnterCriticalSection(&mutex);
	if (--readerCount == 0) {
		ReleaseSemaphore(dataLock, 1, &prevCount);
	}
	LeaveCriticalSection(&mutex);
}

void CRWLock::WriteLock()
{
	WaitForSingleObject(dataLock, INFINITE);
}

bool CRWLock::WriteTryLock()
{
	return WaitForSingleObject(dataLock, 0) == WAIT_OBJECT_0;
}

void CRWLock::WriteUnlock()
{
	LONG prevCount;
	ReleaseSemaphore(dataLock, 1, &prevCount);
	if (prevCount != 0) {
		log.AddLog(LOG_ERROR, "WriteUnlock semaphore was not locked");
	}
}