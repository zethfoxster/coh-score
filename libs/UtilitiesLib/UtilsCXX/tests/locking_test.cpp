#include "criticalsection.hpp"
#include "keyedcondition.hpp"
#include "taskthread.hpp"
#include "UnitTest++.h"

#if _WINNT_WINVER >= 0x0600
	#include "condition.hpp"
#endif

/// One second timeout
#define LOCKING_TEST_TIMEOUT 1000

#define TEST_LOOPS 10U

static char * s_test[] = {
	"foo", "bar", "widgets",
};

TEST(CriticalSection) {
	UNITTEST_TIME_CONSTRAINT(LOCKING_TEST_TIMEOUT);

	CriticalSection lock;

	for(unsigned i=0; i<TEST_LOOPS; i++) {
		CHECK(lock.trylock());
		lock.unlock();

		lock.lock();
		lock.unlock();
	}
}

template <class MUTEX, class CONDITION>
class ConditionTestFixture : public TaskThread {
	MUTEX lock;
	CONDITION condition;
	bool worked;
	unsigned count;

	virtual void run() {
		for (unsigned i=0; i<TEST_LOOPS; i++) {
			lock.lock();
			if (worked)
				condition.wait(&lock);
			worked = true;
			lock.unlock();
			
			condition.signal();	
		}
	}

protected:
	ConditionTestFixture():worked(false) {}

	void do_test() {
		start();

		for (unsigned i=0; i<TEST_LOOPS; i++) {
			lock.lock();
			if (!worked)
				condition.wait(&lock);
			CHECK_EQUAL(true, worked);
			worked = false;
			lock.unlock();

			condition.broadcast();
		}

		stop();
	}
};

#if _WINNT_WINVER >= 0x0600
typedef ConditionTestFixture<CriticalSection, Condition> ConditionFixture;
TEST_FIXTURE(ConditionFixture, Condition) {
	UNITTEST_TIME_CONSTRAINT(LOCKING_TEST_TIMEOUT);
	do_test();
}
#endif

typedef ConditionTestFixture<CriticalSection, KeyedCondition> KeyedConditionFixture;
TEST_FIXTURE(KeyedConditionFixture, KeyedCondition) {
	UNITTEST_TIME_CONSTRAINT(LOCKING_TEST_TIMEOUT);

	init_ntapiext();
	CHECK(have_keyed_event());
	do_test();
}

