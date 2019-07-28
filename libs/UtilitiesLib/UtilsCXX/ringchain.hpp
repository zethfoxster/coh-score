#ifndef _RINGCHAIN_HPP
#define _RINGCHAIN_HPP

#include "barrier.h"
#include "ring.hpp"

#include "wininclude.h"

struct PointerRingPage : public TwoThreadedPointerRingBase {
	PointerRingPage * next;

	void zero_header() {
		m_read = 0;
		m_write = 0;
		m_base = NULL;
		m_size = 0;
		next = NULL;
	}
};

class PointerRingChain {
	__declspec(align(EXPECTED_WORST_CACHE_LINE_SIZE)) PointerRingPage * m_read;
	__declspec(align(EXPECTED_WORST_CACHE_LINE_SIZE)) PointerRingPage * m_write;

	static PointerRingPage * alloc_page();
	static void free_page(PointerRingPage * page);

public:
	static void Initialize();
	static void Shutdown();

	PointerRingChain() {
		m_read = m_write = alloc_page();
	}

	~PointerRingChain() {}

	bool empty() {
		return m_read->empty() && !m_read->next;
	}

	void push(void * data) {
		if (m_write->push(data))
			return;

		PointerRingPage * new_page = alloc_page();
		_WriteBarrier();
		m_write->next = new_page;
		m_write = new_page;
		m_write->push(data);
	}

	void * pop() {
		if (void * ret = m_read->pop())
			return ret;

		if (!m_read->next)
			return NULL;

		// We need to check to make sure the writer did not suddenly wake up
		// and fill an entire page full of data and subsequently update the
		// next pointer while this thread was preemptively swapped off of the CPU.
		_ReadBarrier();
		if (void * ret = m_read->pop())
			return ret;

		// collect the current ring
		PointerRingPage * cur = m_read;
		PointerRingPage * next = cur->next;
		m_read = next;
		_WriteBarrier();
		free_page(cur);
		return m_read->pop();
	}
};

template <class T>
class TypedPointerRingChain : public PointerRingChain {
	STATIC_ASSERT(sizeof(T) <= sizeof(uintptr_t));

public:
	void push(T data) {
		PointerRingChain::push(reinterpret_cast<void*>(data));
	}

	T pop() {
		return reinterpret_cast<T>(PointerRingChain::pop());
	}
};


#endif

