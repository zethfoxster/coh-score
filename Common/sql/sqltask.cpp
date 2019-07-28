#include "sqltask.h"
#include "criticalsection.hpp"

/// Enable to support pre-Vista operating systems by using slower synchronization primitives
#define ENABLE_COMPAT_CONDITION

// Sorted by most throughput to least
//#define LOCKLESS_WORKERPOOL // Lockless workers with nearly ideal performance under heavy load, but consume more CPU when idle and fight with the windows scheduler
#define SEMAPHORE_WORKERPOOL // Semaphore driven workers that are easy on the scheduler and faster than APCs
#define APC_WORKERPOOL // QueueUserAPC based workers that rely on the kernel to schedule tasks but are fairly slow

#ifdef ENABLE_COMPAT_CONDITION
	#include "keyedcondition.hpp"
	typedef KeyedCondition SqlTaskCondition;
#else
	#include "condition.hpp"
	typedef Condition SqlTaskCondition;
#endif

#if defined(LOCKLESS_WORKERPOOL)
	#include "locklessworkerpool.hpp"
	typedef LocklessMultiQueueWorkerPool SqlTaskWorkerPool;
#elif defined(SEMAPHORE_WORKERPOOL)
	#include "semaphoreworkerpool.hpp"
	typedef SemaphoreWorkerPool<Semaphore> SqlTaskWorkerPool;
#elif defined(APC_WORKERPOOL)
	#include "apcworkerpool.hpp"
	typedef APCWorkerPool SqlTaskWorkerPool;
#else
	#include "nullworkerpool.hpp"
	#define NO_BARRIERS
	typedef NullWorkerPool SqlTaskWorkerPool;
#endif

template <class MUTEX, class CONDITION>
struct sqlTaskGlobals {
	SqlTaskWorkerPool pool;
	MUTEX barrier_lock;
	CONDITION barrier_condition;
	unsigned barrier_count;
	unsigned num_workers;
};

sqlTaskGlobals<CriticalSection, SqlTaskCondition> sqltask;

C_DECLARATIONS_BEGIN

void sql_task_init(unsigned workers, unsigned count) {
	assert(workers);
	assert(count);

	assert(!sqltask.num_workers);

	PointerRingChain::Initialize();
	sqltask.pool.construct(workers, count);
	sqltask.barrier_count = sqltask.num_workers = workers;
}

void sql_task_shutdown(void) {
	if (!sqltask.num_workers)
		return;

	sqltask.pool.destruct();
	PointerRingChain::Shutdown();
	sqltask.num_workers = 0;
}

void sql_task_set_callback(unsigned worker, TaskCallback callback, void * user) {
	sqltask.pool.set_worker_callback(worker, callback, user);
}

void sql_task_push(unsigned worker, void * data) {
	sqltask.pool.push_worker(worker, data);
}

void * sql_task_trypop(void) {
	return sqltask.pool.trypop();
}

void * sql_task_pop(void) {
	return sqltask.pool.pop();
}

void sql_task_freeze(void) {
	sqltask.pool.freeze();
}

void sql_task_thaw(void) {
	sqltask.pool.thaw();
}

void sql_task_worker_barrier(void) {
#ifdef NO_BARRIERS
	return;
#endif

	// decrement the barrier count until the count is zero and then wake everyone back up
	sqltask.barrier_lock.lock();
	if(--sqltask.barrier_count) {
		sqltask.barrier_condition.wait(&sqltask.barrier_lock);
	} else {
		sqltask.barrier_count = sqltask.num_workers;
		sqltask.barrier_condition.broadcast();
	}
	sqltask.barrier_lock.unlock();
}

C_DECLARATIONS_END

