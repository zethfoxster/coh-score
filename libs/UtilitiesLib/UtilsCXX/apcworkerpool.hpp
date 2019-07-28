#ifndef _APCWORKERPOOL_HPP
#define _APCWORKERPOOL_HPP

#include "workerpoolbase.hpp"

class APCWorkerPool : public ThreadedWorkerPoolBase {
	struct WorkerThread : public ThreadedWorkerBase {
		PointerRingChain ringIn;
		PointerRingChain ringOut;
		APCWorkerPool * pool;
		bool in_apc;

		WorkerThread() : pool(NULL), in_apc(false) {}

		static void CALLBACK apc_callback(ULONG_PTR dwParam) {
			WorkerThread * worker = (WorkerThread*)dwParam;

			// We were alerted inside of the callback so this APC needs to be ignored
			if (worker->in_apc)
				return;

			worker->in_apc = true;

			while (void * data = worker->ringIn.pop()) {
				void * ret = worker->step(data);
				worker->ringOut.push(ret);
			}

			worker->in_apc = false;
		}

		static void CALLBACK wakeup(ULONG_PTR dwParam) {}

		void mark_done() {
			StepTaskThread::mark_done();
			QueueUserAPC(&wakeup, get_thread(), NULL);
		}

		void run() {
			while (!done()) {
				SleepEx(INFINITE, TRUE);
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
			m_workers[i].pool = this;
			m_workers[i].start();
		}
	}

	void destruct() {
		assert(!m_inFlight);

		// tell the threads to exit
		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].mark_done();
		}
		
		// wait for the threads to exit
		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].stop();
		}
		delete [] m_workers;
	}

	void push_worker(unsigned worker, void * data) {
		m_inFlight++;

		m_workers[worker].ringIn.push(data);
		QueueUserAPC(&WorkerThread::apc_callback, m_workers[worker].get_thread(), (ULONG_PTR)&m_workers[worker]);
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
