#ifndef _THREADEDPOOLS_HPP
#define _THREADEDPOOLS_HPP

#include "ring.hpp"

struct ThreadedFreeList : public UnthreadedPointerRing {
	ThreadedFreeList * next;
	unsigned thread;
};

class ThreadedPoolAllocator {
	MultiThreadedPointerRing m_globalFreeList;
	class CriticalSection * m_lock;
	struct MemoryPoolImp * m_pool;
	bool m_zero;

	ThreadedFreeList * m_threadFreeLists;
	unsigned m_threadFreeListSize;
	unsigned m_threadLocalStorageIndex;

	ThreadedFreeList * new_thread_free_list();
	ThreadedFreeList * get_thread_free_list();
	void empty_global_free_list();
	void empty_free_list(ThreadedFreeList * freeList);

public:
	ThreadedPoolAllocator(unsigned structSize, bool zero, unsigned initialPoolSize, unsigned freeListSize);
	~ThreadedPoolAllocator();

	void * talloc();
	void tfreenozero(void * ptr);
	void tfreezero(void * ptr);

	void tfree(void * ptr) {
		if(m_zero)
			tfreezero(ptr);
		else
			tfreenozero(ptr);
	}

	void empty_thread_free_list();
	void empty_all_free_lists();
};

#endif
