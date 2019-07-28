#ifndef _NULLWORKERPOOL_HPP
#define _NULLWORKERPOOL_HPP

#include "workerpoolbase.hpp"

class NullWorkerPool : public ThreadedWorkerPoolBase {
	struct WorkerThread : public ThreadedWorkerBase {
		void * do_step(void * data) {
			return step(data);
		}
		void run() {}
	};

	PointerRingChain * m_ringOut;

	WorkerThread * m_workers;
	unsigned m_nworkers;

	unsigned m_inFlight;
	unsigned m_schedule;

public:
	void construct(unsigned num_workers, size_t size) {
		m_inFlight = 0;
		m_schedule = -1;

		m_ringOut = new PointerRingChain;

		m_nworkers = num_workers;
		m_workers = new WorkerThread[m_nworkers];
	}

	void destruct() {
		assert(!m_inFlight);

		delete [] m_workers;
		delete m_ringOut;
	}

	void push_worker(unsigned worker, void * data) {
		m_inFlight++;

		void * ret = m_workers[worker].do_step(data);
		m_ringOut->push(ret);
	}

	void push(void * data) {
		m_schedule = (m_schedule+1)&(m_nworkers-1);
		push_worker(m_schedule, data);
	}

	void * trypop() {
		if(void * ret = m_ringOut->pop()) {
			m_inFlight--;
			return ret;
		}
		return NULL;
	}

	void * pop() {
		void * ret;
		ret = trypop();
		assert(ret);
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
