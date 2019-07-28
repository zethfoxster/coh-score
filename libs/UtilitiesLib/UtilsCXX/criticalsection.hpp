#ifndef _CRITICALSECTION_HPP
#define _CRITICALSECTION_HPP

#include "wininclude.h"

class CriticalSection {
	CRITICAL_SECTION m_sect;

	friend class Condition;
public:
	CriticalSection() {
		InitializeCriticalSectionAndSpinCount(&m_sect, 4000);
	}

	~CriticalSection() {
		DeleteCriticalSection(&m_sect);
	}

	bool trylock() {
		return TryEnterCriticalSection(&m_sect) != FALSE;
	}

	void lock() {
		EnterCriticalSection(&m_sect);
	}

	void unlock() {
		LeaveCriticalSection(&m_sect);
	}
};

#endif

