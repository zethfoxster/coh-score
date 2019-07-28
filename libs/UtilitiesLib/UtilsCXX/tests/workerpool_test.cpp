#include "locklessworkerpool.hpp"
#include "apcworkerpool.hpp"
#include "semaphoreworkerpool.hpp"
#include "lockingworkerpool.hpp"
#include "nullworkerpool.hpp"

#include "criticalsection.hpp"
#include "keyedcondition.hpp"

#if _WINNT_WINVER >= 0x600
	#include "condition.hpp"
#endif

#include "UnitTest++.h"

#if _WINNT_WINVER >= 0x0600
#include "condition.hpp"
#endif

void * workerpool_test_task(void * data, void * user) {
	(*(unsigned*)data)++;
	return data;
}

template <class POOL>
void do_workerpool_test(const char * name, unsigned workers) {
	const int size = 1024;

	POOL pool;
	pool.construct(workers, size);
	pool.set_callback(workerpool_test_task, NULL);

	unsigned * numbers = (unsigned*)calloc(size, sizeof(unsigned));

	pool.thaw();

	CHECK(!pool.trypop());

	// spin up	
	for (unsigned i=0; i<size; i++) {
		numbers[i] = i;
		pool.push(&numbers[i]);
	}

	const unsigned n = 16;
	unsigned size_over_n = size/n;

	clock_t start = clock();
	clock_t ticks;

	unsigned loops = 0;
	do {
		loops++;

		for (unsigned i=0; i<size; i++) {
			pool.pop();
		}

		for (unsigned i=0; i<size_over_n; i++) {
			for (unsigned j=0; j<n; j++) {
				pool.push(numbers + i + j * size_over_n);
			}
		}

		ticks = clock()-start;
	} while (ticks < (CLOCKS_PER_SEC/10));

	// spin down
	for (unsigned i=0; i<size; i++) {
		unsigned ret = *(unsigned*)pool.pop();
		CHECK(ret>loops && ret<=(size+loops));
	}

	pool.freeze();

	printf("% 20s(%d): ticks %d / %f per sec\n", name, workers, ticks, (double)loops * (double)size / ((double)ticks / (double)CLOCKS_PER_SEC));

	pool.destruct();
}

TEST(WorkerPools) {
	init_ntapiext();
	CHECK(have_keyed_event());

	PointerRingChain::Initialize();

	//for (unsigned numWorkers=1; numWorkers<=128; numWorkers = numWorkers*2)
	{
		const unsigned numWorkers = 32;

		do_workerpool_test<NullWorkerPool>("Null", numWorkers);
		do_workerpool_test<LocklessQueueChainWorkerPool>("QueueChain", numWorkers);
		do_workerpool_test<LocklessMultiQueueWorkerPool>("MultiQueue", numWorkers);
		do_workerpool_test<APCWorkerPool>("APC", numWorkers);
		do_workerpool_test<SemaphoreWorkerPool<Semaphore>>("Semaphore", numWorkers);
		do_workerpool_test<SemaphoreWorkerPool<KeyedSemaphore>>("KeyedSemaphore", numWorkers);
#if _WINNT_WINVER >= 0x0600
		do_workerpool_test<LockingWorkerPool<CriticalSection, Condition>>("Win32Condition", numWorkers);
#endif
		do_workerpool_test<LockingWorkerPool<CriticalSection, KeyedCondition>>("KeyedCondition", numWorkers);
	}

	PointerRingChain::Shutdown();
}

