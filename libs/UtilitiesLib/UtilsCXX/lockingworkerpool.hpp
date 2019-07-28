#ifndef _LOCKINGWORKERPOOL_HPP
#define _LOCKINGWORKERPOOL_HPP

#include "workerpoolbase.hpp"
#include "ring.hpp"

template <class MUTEX, class CONDITION>
class LockingWorkerPool : public ThreadedWorkerPoolBase {
	struct WorkerThread : public ThreadedWorkerBase {
		LockingWorkerPool<MUTEX, CONDITION> * pool;

		void run() {
			while (!done()) {
				pool->internal_push(step(pool->internal_pop()));
			}
		}
	};

	unsigned m_nworkers;
	WorkerThread * m_workers;

	UnthreadedPointerRing m_ringIn;
	MUTEX m_lockIn;
	CONDITION m_condIn;
	CONDITION m_condRoomIn;

	UnthreadedPointerRing m_ringOut;
	MUTEX m_lockOut;
	CONDITION m_condOut;
	CONDITION m_condRoomOut;

	unsigned m_inFlight;

	void * internal_pop() {
		m_lockIn.lock();
		void * data;
		while(!(data = m_ringIn.pop())) {
			m_condIn.wait(&m_lockIn);
		}
		m_lockIn.unlock();
        m_condRoomIn.signal();
		return data;
	}

	void internal_push(void * data) {
		m_lockOut.lock();
		while (!m_ringOut.push(data)) {
			m_condRoomOut.wait(&m_lockOut);
		}
		m_lockOut.unlock();
		m_condOut.signal();
	}
		
public:
	void construct(unsigned num_workers, size_t count) {
		m_ringIn.construct(count);
		m_ringOut.construct(count);

		m_inFlight = 0;

		m_nworkers = num_workers;
		m_workers = new WorkerThread[m_nworkers];

		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].pool = this;
			m_workers[i].start();
		}
	}

	void destruct() {
		assert(!m_inFlight);

		// setup the threads to exit
		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].set_callback(NULL, NULL);
			m_workers[i].mark_done();
		}

		// push a fake job for each worker
		void * fake = (void*)0xf007ball;
		for (unsigned i=0; i<m_nworkers; i++) {
			push(fake);
			assert(pop() == fake);
		}

		// wait for the threads to exit
		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].stop();
		}
		delete [] m_workers;

		m_ringIn.destruct();
		m_ringOut.destruct();
	}

	void push(void * data) {
		m_lockIn.lock();
		while (!m_ringIn.push(data)) {
			m_condRoomIn.wait(&m_lockIn);
		}
		m_lockIn.unlock();
		m_condIn.signal();
		m_inFlight++;
	}

	void * trypop() {
		m_lockOut.lock();
		void * ret = m_ringOut.pop();
		m_lockOut.unlock();
		if (ret) {
			m_condRoomOut.signal();
			m_inFlight--;
		}
		return ret;
	}

	void * pop() {
		m_lockOut.lock();
		void * ret;
		while (!(ret = m_ringOut.pop())) {
			m_condOut.wait(&m_lockOut);
		}
		m_lockOut.unlock();
		m_condRoomOut.signal();
		m_inFlight--;
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
