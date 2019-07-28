#ifndef _SLEEPLOCK_HPP
#define _SLEEPLOCK_HPP

#include "criticalsection.hpp"
#include "condition.hpp"

class SleepLock {
	int m_counter;
	CriticalSection m_lock;
	Condition m_cond;

public:
	SleepLock():m_counter(0) {}

	void sleep(int count = 1) {
		m_lock.lock();
		while (m_counter < count)
			m_cond.wait(&m_lock);
		m_counter -= count;
		m_lock.unlock();
	}

	void wakeone() {
		m_lock.lock();
		m_counter++;
		m_lock.unlock();
		m_cond.signal();
	}

	void wakemany(int count) {
		m_lock.lock();
		m_counter += count;
		m_lock.unlock();
		m_cond.broadcast();
	}

};

#endif
