#include "memorypool.h"
#include "threadedpoolallocator.hpp"
#include "criticalsection.hpp"
#include "taskthread.hpp"
#include "UnitTest++.h"

#define NUM_LOOPS 128
#define NUM_ALLOCS 16
#define NUM_THREADS 32

static void test_loop(ThreadedPoolAllocator * allocator) {
	int * allocs[NUM_ALLOCS];

	// test zeroed free
	for (unsigned i=0; i<NUM_LOOPS; i++) {
		for (int j=0; j<ARRAY_SIZE(allocs); j++) {
			allocs[j] = (int*)allocator->talloc();
			CHECK_EQUAL(0, *allocs[j]);
			*allocs[j] = j;
		}

		for (int j=0; j<ARRAY_SIZE(allocs); j++) {
			CHECK_EQUAL(j, *allocs[j]);
			*allocs[j] = 0;
			allocator->tfreenozero(allocs[j]);
		}
	}

	// tested unzeroed free
	for (unsigned i=0; i<NUM_LOOPS; i++) {
		for (int j=0; j<ARRAY_SIZE(allocs); j++) {
			allocs[j] = (int*)allocator->talloc();
			CHECK_EQUAL(0, *allocs[j]);
			*allocs[j] = j;
		}

		for (int j=0; j<ARRAY_SIZE(allocs); j++) {
			CHECK_EQUAL(j, *allocs[j]);
			allocator->tfree(allocs[j]);
		}
	}
}

TEST(SingleThreadAlloc) {
	ThreadedPoolAllocator allocator(sizeof(int), true, 2, 2);
	test_loop(&allocator);	
}

struct AllocThreadedTester {
	ThreadedPoolAllocator * allocator;

	struct AllocThreadedWorker : public TaskThread {
		AllocThreadedTester * parent;

		virtual void run() {
			test_loop(parent->allocator);
		}
	};

	AllocThreadedWorker threads[NUM_THREADS];

	void do_test (const char * name) {
		allocator = new ThreadedPoolAllocator(sizeof(int), true, NUM_ALLOCS * NUM_THREADS / 4, NUM_ALLOCS / 4);

		// spin up
		for (unsigned i=0; i<NUM_THREADS; i++) {
			threads[i].parent = this;
			threads[i].start();
		}

		// spin down
		for (unsigned i=0; i<NUM_THREADS; i++)
			threads[i].stop();

		delete allocator;
	}
};

TEST(MultiThreadAlloc) {
	AllocThreadedTester test;
	test.do_test("CriticalSection");
}
