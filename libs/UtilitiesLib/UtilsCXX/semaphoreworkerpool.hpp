#ifndef _SEMAPHOREWORKERPOOL_HPP
#define _SEMAPHOREWORKERPOOL_HPP

#include "workerpoolbase.hpp"

class Semaphore {
	HANDLE m_sema;

public:
	Semaphore() {
		m_sema = CreateSemaphoreA(NULL, 0, LONG_MAX, NULL);
	}

	~Semaphore() {
		CloseHandle(m_sema);
	}

	void acquire() {
		WaitForSingleObject(m_sema, INFINITE);
	}

	void release() {
		ReleaseSemaphore(m_sema, 1, NULL);
	}
};

class KeyedSemaphore {
	volatile LONG m_count;

public:
	KeyedSemaphore() : m_count(0) {}

	void acquire() {
		if (_InterlockedDecrement(&m_count) < 0)
			NtWaitForKeyedEvent(g_keyed_event_handle, (void*)&m_count, 0, NULL);
	}

	void release() {
		if (_InterlockedIncrement(&m_count) <= 0)
			NtReleaseKeyedEvent(g_keyed_event_handle, (void*)&m_count, 0, NULL);						
	}
};

template <class SEMAPHORE>
class SemaphoreWorkerPool : public ThreadedWorkerPoolBase {
	struct WorkerThread : public ThreadedWorkerBase {
		PointerRingChain ringIn;
		PointerRingChain ringOut;
		SEMAPHORE semaphore;

		void push(void * data) {
			ringIn.push(data);
			semaphore.release();
		}

		void run() {
			while (true) {
				semaphore.acquire();
				if (done())
					break;

				void * ret = step(ringIn.pop());
				ringOut.push(ret);
			}
		}
	};

	WorkerThread * m_workers;
	unsigned m_nworkers;

	unsigned m_inFlight;
	unsigned m_schedule;

public:
	void construct(unsigned num_workers, size_t size) {
		m_inFlight = 0;
		m_schedule = -1;

		m_nworkers = num_workers;
		m_workers = new WorkerThread[m_nworkers];
		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].start();
		}
	}

	void destruct() {
		assert(!m_inFlight);

		// tell the threads to exit
		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].mark_done();
			m_workers[i].semaphore.release();
		}

		// wait for the threads to exit
		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].stop();
		}
		delete [] m_workers;
	}

	void push_worker(unsigned worker, void * data) {
		m_workers[worker].push(data);
		m_inFlight++;
	}

	void push(void * data) {
		m_schedule = (m_schedule+1)&(m_nworkers-1);
		push_worker(m_schedule, data);
	}

	void * trypop() {
		for (unsigned i=0; i<m_nworkers; i++) {
			if(void * ret = m_workers[i].ringOut.pop()) {
				m_inFlight--;
				return ret;
			}
		}
		return NULL;
	}

	void * pop() {
		unsigned yields = 0;
		void * ret;
		while (!(ret = trypop()))
			yield_slow(yields);
		return ret;
	}

	void set_worker_callback(unsigned worker, TaskCallback callback, void * user) {
		m_workers[worker].set_callback(callback, user);
	}

	void set_callback(TaskCallback callback, void * user) {
		for (unsigned i=0; i<m_nworkers; i++) {
			set_worker_callback(i, callback, user);
		}
	}
};

#endif
