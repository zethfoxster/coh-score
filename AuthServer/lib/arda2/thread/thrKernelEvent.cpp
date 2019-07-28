#include "arda2/core/corFirst.h"
#include "arda2/thread/thrKernelEvent.h"


thrKernelEvent::thrKernelEvent( bool bManualReset, bool bInitialState, char *szName ) :
#if CORE_SYSTEM_WINAPI
#else
    m_state(bInitialState),
#endif
    m_manualReset(bManualReset)

{
#if CORE_SYSTEM_WINAPI
    SetHandle(::CreateEvent(NULL, bManualReset, bInitialState, szName));
#else
    UNREF(szName);
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);
#endif
}

 thrKernelEvent::~thrKernelEvent()
{
#if CORE_SYSTEM_WINAPI
#else
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
#endif
}

// checks to see if this object is signaled (asserts if handle invalid)
bool thrKernelEvent::IsSignaled(void) const
{
#if CORE_SYSTEM_WINAPI
    return Wait(0);
#else
    return m_state;
#endif
}

// signals the event
errResult thrKernelEvent::Set()
{
#if CORE_SYSTEM_WINAPI
    errAssert(IsValid());
    if (::SetEvent(GetHandle()) )
	return ER_Success;
    return ER_Failure;
#else
    if (pthread_mutex_lock(&m_mutex) == 0)
    {
	m_state = true;
	pthread_mutex_unlock(&m_mutex);
	pthread_cond_signal(&m_cond);
	return ER_Success;
    }
    else
    {
	return ER_Failure;
    }

#endif
}

// resets the event (only necessary of manualReset TRUE in ctor)
errResult thrKernelEvent::Reset(void)
{
#if CORE_SYSTEM_WINAPI
    errAssert(IsValid());
    if (::ResetEvent(GetHandle()) )
	return ER_Success;
    return ER_Failure;
#else
    if (pthread_mutex_lock(&m_mutex) == 0)
    {
	m_state = false;
	pthread_mutex_unlock(&m_mutex);
	return ER_Success;
    }
    else
    {
	return ER_Failure;
    }
#endif
}

// signals the event, lets all blocking threads through, then resets it
errResult thrKernelEvent::Pulse(void)
{
#if CORE_SYSTEM_WINAPI
    errAssert(IsValid());
    if (::PulseEvent(GetHandle()) )
	return ER_Success;
    return ER_Failure;
#else
    ERR_RETURN_UNIMPLEMENTED();
#endif
}

#if CORE_SYSTEM_WINAPI
#else
/**
 * Wait for the specified number of milliseconds.
 * This will reset m_state if manual reset is off.
**/
bool thrKernelEvent::Wait(uint32 milliseconds)
{
    if (pthread_mutex_lock(&m_mutex) == 0)
    {
	if (milliseconds == INFINITE)
  	{
	    if (pthread_cond_wait(&m_cond, &m_mutex) == 0)
	    {
		// the event was triggered
		pthread_mutex_unlock(&m_mutex);
		return true;
	    }
	}
	else
	{
	    
	    struct timeval tval;
	    struct timezone tzone;
	    gettimeofday(&tval, &tzone);
	    
	    tval.tv_usec += (milliseconds*1000);
	    int overflow = tval.tv_usec - 1000000;
	    if (overflow > 0)
	    {
		tval.tv_sec += (overflow / 1000000);
	    }
	    struct timespec tspec;
	    tspec.tv_sec = tval.tv_sec;
	    tspec.tv_nsec = tval.tv_usec * 1000;
	    
	    if (pthread_cond_timedwait(&m_cond, &m_mutex, &tspec) == 0)
	    {
		// the event was triggered
		if (m_manualReset)
		{
		    // dont reset the state variable
		}
		else
		{
		    // reset the state variable automatically
		    m_state = false;
		}
		pthread_mutex_unlock(&m_mutex);
		return true;
	    }
	    
	}
    }
    pthread_mutex_unlock(&m_mutex);
    return false;
}

#endif
