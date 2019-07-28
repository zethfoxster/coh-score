/*****************************************************************************
	created:	2001/08/09
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Critical section utility classes
*****************************************************************************/

#ifndef   INCLUDED_thrCritical
#define   INCLUDED_thrCritical
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if CORE_SYSTEM_WINAPI
#else
#include <pthread.h>
#endif

//----------------------------------------------------------------------------
// thrCriticalSection... thin object wrapper around OS critical section
//
class thrCriticalSection
{
public:
    thrCriticalSection()
	{
#if CORE_SYSTEM_WINAPI
		InitializeCriticalSection(&m_winCriticalSection);
#else
		// use recursive mutex to emulate the functionalitry of win32 critical sections.
		pthread_mutexattr_init(&m_attr);
		pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&m_mutex, &m_attr);
#endif

		m_valid = true;
	}

	~thrCriticalSection()
	{
#if CORE_SYSTEM_WINAPI
		DeleteCriticalSection(&m_winCriticalSection);
#else
		pthread_mutexattr_destroy(&m_attr);
		pthread_mutex_destroy(&m_mutex);
#endif
		m_valid = false;
	}

	void Enter()
	{
		if ( m_valid )
		{
#if CORE_SYSTEM_WINAPI
			EnterCriticalSection(&m_winCriticalSection);
#else
			pthread_mutex_lock(&m_mutex);
#endif
		}
	}

	void Leave()
	{
		if ( m_valid )
		{
#if CORE_SYSTEM_WINAPI
			LeaveCriticalSection(&m_winCriticalSection);
#else
			pthread_mutex_unlock(&m_mutex);
#endif
		}
	}

private:
    // disable copy constructor and assignment operator
    thrCriticalSection( const thrCriticalSection& );
    thrCriticalSection& operator=( const thrCriticalSection& );

#if CORE_SYSTEM_WINAPI
	CRITICAL_SECTION	m_winCriticalSection;
#else
	pthread_mutex_t         m_mutex;
	pthread_mutexattr_t     m_attr;
#endif
	bool				m_valid;

};


//----------------------------------------------------------------------------
// thrEnterCritical... Enter & automatically leave critical section.
//

class thrEnterCritical
{
public:
	thrEnterCritical( thrCriticalSection &critical ) :
	  m_critical(critical)
	{
		  m_critical.Enter();

	}

	~thrEnterCritical()
	{
		m_critical.Leave();
	}
protected:
	
private:
    // disable copy constructor and assignment operator
    thrEnterCritical( const thrEnterCritical& );
    thrEnterCritical& operator=( const thrEnterCritical& );

	thrCriticalSection	&m_critical;
};


#endif // INCLUDED_thrCritical

