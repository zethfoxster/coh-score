#include <UnitTest++.h>
#include "SuperAssert.h"

int main(int, char const *[]) {
	setAssertUnitTesting(true);
	return UnitTest::RunAllTests();
}
