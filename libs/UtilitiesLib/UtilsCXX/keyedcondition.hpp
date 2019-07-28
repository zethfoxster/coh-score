#ifndef _KEYEDCONDITION_HPP
#define _KEYEDCONDITION_HPP

#include "criticalsection.hpp"
#include "ntapiext.h"

class KeyedCondition {
	CriticalSection m_lock;
	unsigned long m_waiters;

public:
	KeyedCondition() {
		init_ntapiext();

		m_waiters = 0;
	}

	void wait(CriticalSection * futex) {
		m_lock.lock();
		futex->unlock();
		m_waiters++;
		m_lock.unlock();
		
		NtWaitForKeyedEvent(g_keyed_event_handle, &m_waiters, 0, NULL);

		futex->lock();
	}

	void signal() {
		m_lock.lock();
		if (m_waiters)
		{
			m_waiters--;
			NtReleaseKeyedEvent(g_keyed_event_handle, &m_waiters, 0, NULL);
		}
		m_lock.unlock();
	}

	void broadcast() {
		m_lock.lock();
		while (m_waiters)
		{
			m_waiters--;
			NtReleaseKeyedEvent(g_keyed_event_handle, &m_waiters, 0, NULL);
		}
		m_lock.unlock();
	}
};

#endif

