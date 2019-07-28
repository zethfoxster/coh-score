#ifndef _CONDITION_HPP
#define _CONDITION_HPP

#if _WINNT_WINVER < 0x600
	#error "Condition Variables require Vista or newer"
#endif

#include "criticalsection.hpp"

#if PLATFORMSDK < 0x0600
#error "Condition variables are not supported on a Platform SDK before 6.0A"
#endif

class Condition {
	CONDITION_VARIABLE m_cond;

public:
	Condition() {
		InitializeConditionVariable(&m_cond);
	}

	void wait(CriticalSection * lock) {
		SleepConditionVariableCS(&m_cond, &lock->m_sect, INFINITE);
	}

	void signal() {
		WakeConditionVariable(&m_cond);
	}

	void broadcast() {
		WakeAllConditionVariable(&m_cond);
	}
};

#endif
