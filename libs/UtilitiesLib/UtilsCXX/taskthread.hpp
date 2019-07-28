#ifndef _TASKTHREAD_HPP
#define _TASKTHREAD_HPP

#include <assert.h>
#include "wininclude.h"
#include "utils.h"
#include "ringchain.hpp"
#include "ntapiext.h"

typedef void * (*TaskCallback)(void * data, void * user);

#define FAST_YIELDS_BEFORE_SLOW 40

static INLINEDBG void yield_fast() {
	if (!SwitchToThread()) {
		YieldProcessor();
		YieldProcessor();
		YieldProcessor();
		YieldProcessor();
		YieldProcessor();
		YieldProcessor();
		YieldProcessor();
		YieldProcessor();
	}
}

static INLINEDBG void yield_slow(unsigned & yields) {
	if (yields++ < FAST_YIELDS_BEFORE_SLOW)
		return yield_fast();

	if (!SwitchToThread()) {
		SleepEx(1, FALSE);
	}
}

class TaskPointerRing : public PointerRingChain {
#ifdef EXPERIMENTAL_STUFF_THAT_DID_NOT_WORK_VERY_WELL
	volatile LONG m_count;
	HANDLE m_waiter;
	void signal() {
		NtAlertThread(m_waiter);
	}

public:

	TaskPointerRing() : m_waiter(GetCurrentThread()), m_count(0) {}

	void set_waiter(HANDLE waiter) {
		m_waiter = waiter;
	}

	void * pop() {
		_InterlockedDecrement(&m_count);
		return PointerRingChain::pop();
	}

	void push(void * data) {
		PointerRingChain::push(data);
		if (_InterlockedIncrement(&m_count) == 1)
			signal();
	}
#else
public:

	void set_waiter(HANDLE waiter) {}
#endif
};

class TaskThread {
	HANDLE m_thread;
	volatile unsigned long m_done;
	volatile unsigned long m_freeze;
	HANDLE m_wake;
	HANDLE m_awoke;

#ifdef EXPERIMENTAL_STUFF_THAT_DID_NOT_WORK_VERY_WELL
	TaskThread * m_next;
	static TaskThread * m_head;
#endif

	static unsigned __stdcall run_wrapper(void * data) {
		((TaskThread*)data)->run();
		return 0;
	}

public:
	virtual void run() = 0;

	HANDLE get_thread() { return m_thread; }

	bool done() { return m_done!=0; }

	void mark_done() {
		m_done = true;
	}

	void start() {
		m_freeze = false;
		m_done = false;

		// Enable more higher speed scheduling
		timeBeginPeriod(1);

		m_wake = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(m_wake != INVALID_HANDLE_VALUE);

		m_awoke = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(m_awoke != INVALID_HANDLE_VALUE);

		m_thread = CreateThread(NULL, 0, &run_wrapper, this, 0, NULL);
		assert(m_thread != INVALID_HANDLE_VALUE);
	}

	void stop() {
		mark_done();
		thaw();

		assert(WaitForSingleObject(m_thread, INFINITE) == WAIT_OBJECT_0);
		DWORD retcode;
		assert(GetExitCodeThread(m_thread, &retcode) && retcode == 0);
		assert(CloseHandle(m_thread));
		m_thread = INVALID_HANDLE_VALUE;

		assert(CloseHandle(m_wake));
		m_wake = INVALID_HANDLE_VALUE;

		assert(CloseHandle(m_awoke));
		m_awoke = INVALID_HANDLE_VALUE;
	}

	void sleep() {
		assert(m_freeze);
		WaitForSingleObject(m_wake, INFINITE);
		ResetEvent(m_wake);
		m_freeze = false;
		SetEvent(m_awoke);
	}

	void freeze() {
		m_freeze = true;
	}

	void thaw() {
		if (m_freeze) {
			SignalObjectAndWait(m_wake, m_awoke, INFINITE, FALSE);
		}
	}

	bool frozen() {
		return m_freeze!=0;
	}
};

class StepTaskThread : public TaskThread {
	TaskCallback m_callback;
	void * m_user;

protected:
	void * step(void * data) {
		if (m_callback)
			return m_callback(data, m_user);
		return data;
	}

public:
	StepTaskThread():m_callback(NULL), m_user(NULL) {}

	void set_callback(TaskCallback callback, void * user) {
		m_callback = callback;
		m_user = user;
	}
};

class StepTask : public StepTaskThread {
	TaskPointerRing * m_input;
	TaskPointerRing * m_output;

	void run() {
		while (true) {
			if (void * data = m_input->pop()) {
				void * ret = step(data);
				m_output->push(ret);
			} else if (frozen()) {
				sleep();
			} else if (done()) {
				break;
			} else {
				yield_fast();
			}
		}
	}

public:
	void construct(TaskPointerRing * input, TaskPointerRing * output) {
		assert(input);
		assert(output);

		m_input = input;
		m_output = output;
		
		start();

		m_input->set_waiter(get_thread());
	}

	void destruct() {
		stop();
	}

	void * trypop() {
		return m_output->pop();
	}

	void * pop() {
		unsigned yields = 0;
		void * ret;
		while(!(ret = trypop()))
			yield_slow(yields);
		return ret;
	}

	void push(void * data) {
		m_input->push(data);
	}
};

class EmitTask : public StepTaskThread {
	TaskPointerRing * m_input;
	TaskPointerRing * m_outputs;
	unsigned m_noutputs;
	unsigned m_schedule;

	void run() {
		while (true) {		
			if (void * data = m_input->pop()) {
				void * ret = step(data);

				// Round robin scheduling makes the logic in the gather
				// routine more simple because the input completion
				// order is slightly easier to predict.
				m_schedule = (m_schedule+1) & (m_noutputs-1);
				m_outputs[m_schedule].push(ret);
			} else if (frozen()) {
				sleep();
			} else if (done()) {
				break;
			} else {
				yield_fast();
			}
		}
	}

public:
	void construct(TaskPointerRing * input, TaskPointerRing * outputs, size_t num_outputs) {
		assert(input);
		assert(outputs);
		assert(num_outputs);

		m_input = input;
		m_outputs = outputs;
		m_noutputs = (unsigned)num_outputs;
		m_schedule = (unsigned)-1;

		start();

		m_input->set_waiter(get_thread());
	}

	void destruct() {
		stop();
	}

	void push(void * data) {
		m_input->push(data);
	}
};

class GatherTask : public StepTaskThread {
	TaskPointerRing * m_inputs;
	TaskPointerRing * m_output;
	unsigned m_ninputs;

	void run() {
		while (true) {
			void * data = NULL;
			for(unsigned i=0; i<m_ninputs; i++) {
				if (data = m_inputs[i].pop())
					break;
			}
			if (data) {
				void * ret = step(data);
				m_output->push(ret);
			} else if (frozen()) {
				sleep();
			} else if (done()) {
				break;
			} else {
				yield_fast();
			}
		}
	}

public:
	void construct(TaskPointerRing * inputs, size_t num_inputs, TaskPointerRing * output) {
		assert(inputs);
		assert(num_inputs);
		assert(output);

		m_inputs = inputs;
		m_output = output;
		m_ninputs = (unsigned)num_inputs;

		start();

		for (unsigned i=0; i<m_ninputs; i++) {
			m_inputs[i].set_waiter(get_thread());
		}
	}

	void destruct() {
		stop();
	}

	void * trypop() {
		return m_output->pop();
	}

	void * pop() {
		unsigned yields = 0;
		void * ret;
		while(!(ret = trypop())) {
			yield_slow(yields);
		}
		return ret;
	}
};

#endif

