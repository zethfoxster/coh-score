#ifndef _LOCKLESSWORKERPOOL
#define _LOCKLESSWORKERPOOL

#include "ring.hpp"
#include "taskthread.hpp"

class LocklessQueueChainWorkerPool {
	TaskPointerRing m_source;
	TaskPointerRing * m_inputs;
	TaskPointerRing * m_outputs;
	TaskPointerRing m_sink;

	EmitTask m_emitter;
	StepTask * m_workers;
	GatherTask m_gatherer;
	unsigned m_nworkers;

public:
	void construct(unsigned workers, unsigned size) {
		assert(workers);

		m_nworkers = workers;
		m_inputs = new TaskPointerRing[workers];
		m_outputs = new TaskPointerRing[workers];
		m_workers = new StepTask[workers];

		m_emitter.construct(&m_source, m_inputs, workers);
		for (unsigned i=0; i<m_nworkers; i++)
			m_workers[i].construct(&m_inputs[i], &m_outputs[i]);
		m_gatherer.construct(m_outputs, workers, &m_sink);

		freeze();
	}

	void destruct() {
		thaw();

		m_emitter.destruct();
		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].destruct();
		}
		m_gatherer.destruct();

		delete [] m_inputs;
		m_inputs = NULL;

		delete [] m_outputs;
		m_outputs = NULL;

		delete [] m_workers;
		m_workers = NULL;
	}

	void freeze() {
		m_emitter.freeze();
		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].freeze();
		}
		m_gatherer.freeze();
	}

	void thaw() {
		m_emitter.thaw();
		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].thaw();
		}
		m_gatherer.thaw();
	}

	void push(void * data) {
		m_emitter.push(data);
	}

	void * trypop() {
		return m_gatherer.trypop();
	}

	void * pop() {
		return m_gatherer.pop();
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

class LocklessMultiQueueWorkerPool {
	struct WorkerTask : public StepTask {
		int inFlight;
	};

	TaskPointerRing * m_inputs;
	TaskPointerRing * m_outputs;

	WorkerTask * m_workers;
	unsigned m_nworkers;

	unsigned m_schedule;

public:
	void construct(unsigned workers, unsigned size) {
		assert(workers);

		m_nworkers = workers;
		m_inputs = new TaskPointerRing[workers];
		m_outputs = new TaskPointerRing[workers];
		m_workers = new WorkerTask[workers];

		m_schedule = ~0UL;

		for (unsigned i=0; i<m_nworkers; i++) {
			m_workers[i].inFlight = 0;
			m_workers[i].freeze();
			m_workers[i].construct(&m_inputs[i], &m_outputs[i]);
		}
	}

	void destruct() {
		for (unsigned i=0; i<m_nworkers; i++) {
			assert(!m_workers[i].inFlight);
			m_workers[i].thaw();
			m_workers[i].destruct();
		}

		delete [] m_inputs;
		m_inputs = NULL;

		delete [] m_outputs;
		m_outputs = NULL;

		delete [] m_workers;
		m_workers = NULL;
	}

	void freeze() {
		for (unsigned i=0; i<m_nworkers; i++) {
			if (m_workers[i].inFlight)
				m_workers[i].freeze();
		}
	}

	void thaw() {
		for (unsigned i=0; i<m_nworkers; i++) {
			if (m_workers[i].inFlight)
				m_workers[i].thaw();
		}
	}

	/**
	 * @note Issue the thread thaw before we push the data onto the
	 *       ring to prevent the case where the thread has not yet
	 *       handled the freeze command and the data blocks on
	 *       another worker thread.
	 */
	void push_worker(unsigned worker, void * data) {
		if (!m_workers[worker].inFlight++)
			m_workers[worker].thaw();

		m_workers[worker].push(data);
	}

	void push(void * data) {
		m_schedule = (m_schedule+1)&(m_nworkers-1);
		push_worker(m_schedule, data);
	}

	void * trypop() {
		for (unsigned i=0; i<m_nworkers; i++) {
			if (!m_workers[i].inFlight)
				continue;

			if (void * ret = m_workers[i].trypop()) {
				if (!--m_workers[i].inFlight)
					m_workers[i].freeze();
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
