/*****************************************************************************
	created:	2002/02/06
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Ryan M. Prescott
	
	purpose:	threadage
*****************************************************************************/

#ifndef _THR_THREAD_H_
#define _THR_THREAD_H_

#if CORE_SYSTEM_WINAPI
// win32 implementation for windows
#define OSTHREADHANDLE	HANDLE
#define TIDTYPE		DWORD
#else
// Pthread Implementation for Linux
#include "pthread.h"
#define OSTHREADHANDLE  pthread_t
#define TIDTYPE         int
#endif

class thrThread {
public:
    virtual	~thrThread();
    void Run();
    void Create(bool CreateSuspended);
    void Suspend();
    bool IsRunning();

    static const TIDTYPE GetCurrentThreadId();

protected:
    thrThread();
    virtual void ThreadMain() = 0;

    const OSTHREADHANDLE &GetThreadHandle() { return m_hThread; }
    const TIDTYPE &GetThreadId() { return m_tid; }

private:
    TIDTYPE m_tid;
    OSTHREADHANDLE m_hThread;

#if CORE_SYSTEM_WINAPI
    static DWORD WINAPI Dispatch(LPVOID ThreadObj);
#else
    static void *Dispatch(void *ThreadObj);
    bool             m_running;
#endif
};



#endif //_THR_THREAD_H_
