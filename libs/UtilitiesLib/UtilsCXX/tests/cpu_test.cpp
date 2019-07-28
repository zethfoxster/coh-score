#include "cpu_count.h"
#include "UnitTest++.h"

TEST(CpuCount) {
	CHECK(getNumRealCpus());
	CHECK(getNumVirtualCpus());
}

TEST(CpuCache) {
	CHECK(EXPECTED_WORST_CACHE_LINE_SIZE >= getCacheLineSize());
}
