#include "../sqltask.h"
#include "UnitTest++.h"

#define NUM_LOOPS 4
#define NUM_THREADS 64

static int s_test[16384];

TEST(SingleSqlTask) {
	sql_task_init(1, ARRAY_SIZE(s_test));
	for (unsigned i=0; i<NUM_LOOPS; i++) {
		for (unsigned j=0; j<ARRAY_SIZE(s_test); j++) {
			sql_task_push(0, s_test + j);
			CHECK_EQUAL(s_test + j, sql_task_pop());
		}

		if (i&1)
			sql_task_freeze();

		for (unsigned j=0; j<ARRAY_SIZE(s_test); j++) {
			sql_task_push(0, s_test + j);
		}

		sql_task_thaw();

		for (unsigned j=0; j<ARRAY_SIZE(s_test); j++) {
			CHECK(sql_task_pop());
		}
	}
	sql_task_shutdown();
}

TEST(MultiSqlTask) {
	sql_task_init(NUM_THREADS, ARRAY_SIZE(s_test));

	// spin up
	for (unsigned i=0; i<ARRAY_SIZE(s_test); i++) {
		s_test[i]++;
		sql_task_push(0, s_test + i);
	}

	clock_t start = clock();
	clock_t ticks;

	unsigned loops = 0;
	do {
		loops++;

		for (unsigned j=0; j<ARRAY_SIZE(s_test); j++) {
			s_test[j]++;
			sql_task_push(0, s_test + j);
			(*(int*)sql_task_pop())--;
		}

		ticks = clock()-start;
	} while (ticks < (CLOCKS_PER_SEC/4));

	// spin down
	for (unsigned i=0; i<ARRAY_SIZE(s_test); i++) {
		(*(int*)sql_task_pop())--;
	}

	// check for errors
	for (unsigned i=0; i<ARRAY_SIZE(s_test); i++) {
		CHECK_EQUAL(0, s_test[i]);
	}

	printf("% 20s(%d): ticks %d / %f per sec\n", __FUNCTION__, NUM_THREADS, ticks, (double)loops * (double)ARRAY_SIZE(s_test) / ((double)ticks / (double)CLOCKS_PER_SEC));

	sql_task_shutdown();
}
