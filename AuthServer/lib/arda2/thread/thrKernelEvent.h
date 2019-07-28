/*****************************************************************************
	created:	2001/08/13
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Thin class wrapper for win32 Event Objects
*****************************************************************************/

#ifndef   INCLUDED_thrKernelEvent
#define   INCLUDED_thrKernelEvent
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/thread/thrKernelObject.h"

#if CORE_SYSTEM_WINAPI
#else
#include <sys/time.h>
#include <pthread.h>
#endif


class thrKernelEvent: public thrKernelObject
{
public:
    // creates event, passes handle to inherited
    explicit thrKernelEvent( bool bManualReset, bool bInitialState = false, char *szName = NULL );

    virtual ~thrKernelEvent();

    // checks to see if this object is signaled (asserts if handle invalid)
    bool IsSignaled(void) const;

    // signals the event
    errResult Set();

    // resets the event (only necessary of manualReset TRUE in ctor)
    errResult Reset(void);

    // signals the event, lets all blocking threads through, then resets it
    errResult Pulse(void);

#if CORE_SYSTEM_WINAPI
    // Wait() is implemented in corKernelObject on win32
#else
    bool Wait(uint32 milliseconds = INFINITE);
#endif

protected:


#if CORE_SYSTEM_WINAPI
#else
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;
    bool            m_state;
#endif
    bool            m_manualReset;


private:

    thrKernelEvent(); // hide default constructor

    // disable copy constructor and assignment operator
    thrKernelEvent( const thrKernelEvent& );
    thrKernelEvent& operator=( const thrKernelEvent& );
};


#endif // INCLUDED_thrKernelEvent
