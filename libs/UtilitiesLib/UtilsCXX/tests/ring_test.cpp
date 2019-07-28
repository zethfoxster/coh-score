#include "ring.hpp"
#include "ringchain.hpp"
#include "UnitTest++.h"

static char s_test[] = "abcdefghijklmnopqrstuvwxyz01234";

TEST(PointerRing) {
	UnthreadedPointerRing ring;
	ring.construct(8 * ARRAY_SIZE(s_test));

	CHECK(ring.empty());
	CHECK(!ring.pop());

	for (int i=0; i<10; i++) {
		for (int j=0; j<8; j++) {
			for(int k=0; k<ARRAY_SIZE(s_test); k++) {
				CHECK(ring.push(&s_test[k]));
			}
		}
		CHECK(ring.full());
		CHECK(!ring.push("fail"));

		for (int j=0; j<8; j++) {
			for(int k=0; k<ARRAY_SIZE(s_test); k++) {
				CHECK_EQUAL(&s_test[k], ring.pop());
			}
		}
		CHECK(ring.empty());
		CHECK(!ring.pop());
	}

	ring.destruct();

	ring.construct(2);
	for(int i=0; i<ARRAY_SIZE(s_test); i++) {
		CHECK(ring.push(&s_test[i]));
		CHECK_EQUAL(&s_test[i], ring.pop());
	}
	ring.destruct();
}

TEST(PointerRingChain) {
	PointerRingChain::Initialize();

	PointerRingChain ringChain;

	CHECK(ringChain.empty());
	CHECK(!ringChain.pop());

	for (int i=0; i<10; i++) {
		for (int j=0; j<8; j++) {
			for(int k=0; k<ARRAY_SIZE(s_test); k++) {
				ringChain.push(&s_test[k]);
			}
		}

		for (int j=0; j<8; j++) {
			for(int k=0; k<ARRAY_SIZE(s_test); k++) {
				CHECK_EQUAL(&s_test[k], ringChain.pop());
			}
		}
		CHECK(ringChain.empty());
		CHECK(!ringChain.pop());
	}

	for(int i=0; i<ARRAY_SIZE(s_test); i++) {
		ringChain.push(&s_test[i]);
		CHECK_EQUAL(&s_test[i], ringChain.pop());
	}

	PointerRingChain::Shutdown();
}

