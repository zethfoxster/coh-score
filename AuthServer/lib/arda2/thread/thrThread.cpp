/*****************************************************************************
	created:	2002/08/05
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Ryan Prescott
	
	purpose:	
*****************************************************************************/

#include "arda2/core/corFirst.h"
#include "arda2/thread/thrThread.h"
#include "arda2/timer/timTimer.h"


thrThread::thrThread() : m_tid(0), m_hThread(0)
{
#if CORE_SYSTEM_WINAPI
#else
    m_running = false;
#endif
}

thrThread::~thrThread()
{
    if(m_hThread)
    {
#if CORE_SYSTEM_WINAPI
        CloseHandle(m_hThread); 
#else
        m_running = false;
#endif
        m_tid = 0;
        m_hThread = 0;
    }
}

void thrThread::Create(bool CreateSuspended)
{
    UNREF(CreateSuspended);
    // should we worry about mutexing m_tid and the thread creation process??
    // seems like we should be doing some heavy duty threading to worry about this. . .
    if(!m_hThread)
    {
#if CORE_SYSTEM_WINAPI
        m_hThread = CreateThread(NULL, 0, thrThread::Dispatch, this, CreateSuspended?CREATE_SUSPENDED:0, &m_tid);
        if (m_hThread == NULL)
	{
            ERR_REPORTV(ES_Error, ("Error creating thread <%d>", GetLastError()) );
        }
#else
        int value = pthread_create(&m_hThread, NULL, thrThread::Dispatch, this);
        if (value != 0)
        {       
            ERR_REPORTV(ES_Error, ("Error creating thread <%d>", value));
        }
	m_tid = (int)m_hThread;
#endif
    }
    else
    {
        ERR_REPORTV(ES_Error, ("Thread already created!"));
    }
}

void thrThread::Run()
{
    if(!m_hThread)
    {
        // Create an OS thread, using the static callback
        Create(FALSE);
    }

#if CORE_SYSTEM_WINAPI
    ResumeThread(m_hThread);
#endif

}

void thrThread::Suspend()
{
    if(m_hThread)
    {
#if CORE_SYSTEM_WINAPI
        SuspendThread(m_hThread);
#else
	ERR_UNIMPLEMENTED();
#endif
    }
}

/**
 * static dispatch function for thread creation. This is invoked in the 
 * newly created thread with the thread object as a parameter.
 *
 **/
#if CORE_SYSTEM_WINAPI
DWORD thrThread::Dispatch(LPVOID ThreadObj)
{
    // Call the actual OO thread code
    corSleep(50);  // wait for a bit to startup
    errAssert(ThreadObj);

    ((thrThread*)ThreadObj)->ThreadMain();

    return 1;
}
#else
void *thrThread::Dispatch(void *ThreadObj)
{
    corSleep(50);
    errAssert(ThreadObj);
    thrThread *thread = (thrThread*)ThreadObj;

    // setup the running flag
    thread->m_running = true;

    // actually run the thread
    thread->ThreadMain();

    // reset the running flag
    thread->m_running = false;
    return NULL;
}
#endif

bool thrThread::IsRunning()
{
#if CORE_SYSTEM_WINAPI
    DWORD Status;
    GetExitCodeThread(m_hThread, &Status);
    return (Status == STILL_ACTIVE)?1:0;
#else
    return m_running;
#endif
}


const TIDTYPE thrThread::GetCurrentThreadId()
{
    TIDTYPE id;
#if CORE_SYSTEM_WINAPI
    id = ::GetCurrentThreadId();
#else
    id = (int)pthread_self();
#endif
    return id;
}
