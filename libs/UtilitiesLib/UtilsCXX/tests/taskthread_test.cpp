#include "taskthread.hpp"
#include "UnitTest++.h"

static char * s_test[] = {
	"fee", "fi", "fo", "fum",
	"I", "smell", "the", "blood",
	"of", "a", "concurrent", "programmer",
	"Lorem", "ipsum", "dolor", "sit"
};

TEST(Step) {
	PointerRingChain::Initialize();

	TaskPointerRing input;
	TaskPointerRing output;

	StepTask queue;
	queue.construct(&input, &output);

	for (int i=0; i<10; i++) {
		for (int j=0; j<32; j++) {
			for(int k=0; k<ARRAY_SIZE(s_test); k++) {
				queue.push(s_test[k]);
			}
		}

		for (int j=0; j<32; j++) {
			for(int k=0; k<ARRAY_SIZE(s_test); k++) {
				CHECK_EQUAL(s_test[k], queue.pop());
			}
		}
	}

	queue.destruct();

	CHECK(!input.pop());
	CHECK(!output.pop());

	PointerRingChain::Shutdown();
}

TEST(StepStep) {
	PointerRingChain::Initialize();

	TaskPointerRing input;
	TaskPointerRing middle;
	TaskPointerRing output;

	StepTask step1;
	StepTask step2;

	step2.construct(&middle, &output);

	for (int i=0; i<10; i++) {
		step1.construct(&input, &middle);
		for (int j=0; j<32; j++) {
			for(int k=0; k<ARRAY_SIZE(s_test); k++) {
				step1.push(s_test[k]);
			}			
		}
		step1.destruct();

		for (int j=0; j<32; j++) {
			for(int k=0; k<ARRAY_SIZE(s_test); k++) {
				CHECK_EQUAL(s_test[k], step2.pop());
			}
		}
	}

	step2.destruct();

	CHECK(!input.pop());
	CHECK(!output.pop());

	PointerRingChain::Shutdown();
}

TEST(EmitGather) {
	PointerRingChain::Initialize();

	TaskPointerRing input;
	TaskPointerRing middle[4];
	TaskPointerRing output;

	EmitTask emit;	
	GatherTask gather;	

	gather.construct(middle, ARRAY_SIZE(middle), &output);

	for (unsigned i=0; i<10; i++) {
		emit.construct(&input, middle, ARRAY_SIZE(middle));
		for (int j=0; j<32; j++) {
			for(int k=0; k<ARRAY_SIZE(s_test); k++) {
				emit.push(s_test[k]);
			}
		}
		emit.destruct();

		int counts[ARRAY_SIZE(s_test)] = {0, };
		for (int j=0; j<32 * ARRAY_SIZE(s_test); j++) {
			void * data = gather.pop();
			for(int k=0; k<ARRAY_SIZE(s_test); k++) {
				if(data == s_test[k]) {
					counts[k]++;
					break;
				}
			}
		}

		CHECK(!input.pop());
		for(int j=0; j<ARRAY_SIZE(middle); j++) {
			CHECK(!middle[j].pop());
		}
		CHECK(!output.pop());
		
		for(int j=0; j<ARRAY_SIZE(s_test); j++) {
			CHECK_EQUAL(32, counts[j]);
		}
	}

	gather.destruct();

	PointerRingChain::Shutdown();
}

TEST(Load) {
	PointerRingChain::Initialize();

	assert(!(ARRAY_SIZE(s_test) & (ARRAY_SIZE(s_test)-1)));

	const int kWorkers = 32;

	TaskPointerRing one;
	TaskPointerRing two[kWorkers];
	TaskPointerRing three[kWorkers];
	TaskPointerRing four;

	const int kMaxQueue = 1024 + 16 * 2 * kWorkers + 1024;

	EmitTask emit;
	StepTask steps[kWorkers];
	GatherTask gather;

	TaskThread * threadList[kWorkers];
	for (int i=0; i<kWorkers; i++) {
		threadList[i] = &steps[i];
	}

	emit.construct(&one, two, ARRAY_SIZE(two));
	gather.construct(three, ARRAY_SIZE(three), &four);

	for (int i=0; i<kWorkers; i++) {
		steps[i].construct(&two[i], &three[i]);
	}

	// Make sure single tasks do not block
	for (int i=0; i<10; i++) {
		for (int j=0; j<ARRAY_SIZE(s_test); j++) {
			emit.push(s_test[j]);
			CHECK_EQUAL(s_test[j], gather.pop());
		}
	}
	
	// Test idling
	emit.freeze();
	CHECK(emit.frozen());
	emit.thaw();
	CHECK(!emit.frozen());

	// Test lots of threads
	for (int i=0; i<10; i++) {
		for (int j=0; j<kMaxQueue; j++) {
			emit.push(s_test[(j+1) & (ARRAY_SIZE(s_test)-1)]);
		}
		for (int j=0; j<kMaxQueue; j++) {
			CHECK(gather.pop());
		}
	}

	// Test random load
	for (int i=0; i<1024; i++) {
		if (i%2) {
			gather.freeze();
		}

		if (i%3) {
			emit.freeze();
		}

		int kRunCount = i % 1024;

		for (int j=0; j<kRunCount; j++) {
			emit.push(s_test[(j+1) & (ARRAY_SIZE(s_test)-1)]);
		}
		
		emit.thaw();
		gather.thaw();

		for (int j=0; j<kRunCount; j++) {
			CHECK(gather.pop());
		}
	}

	emit.destruct();
	for (int i=0; i<kWorkers; i++) {
		steps[i].destruct();
	}
	gather.destruct();

	PointerRingChain::Shutdown();
}
