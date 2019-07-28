#include "AccountTypes.h"
#include "components/StashTable.h"
#include "utils/endian.h"
#include "UnitTest++.h"

TEST(SkuIdString) {
	CHECK(skuIdIsNull(kSkuIdInvalid));
	CHECK(skuIdEquals(kSkuIdInvalid, kSkuIdInvalid));

	const SkuId one = skuIdFromString("abcd1234");
	CHECK(kSkuIdInvalid.u64 != one.u64);
	CHECK(!skuIdEquals(kSkuIdInvalid, one));

	char string_two[9];
	strcpy(string_two, skuIdAsString(one));
	CHECK_EQUAL("ABCD1234", string_two);

	const SkuId two = skuIdFromString(string_two);
	CHECK_EQUAL(one.u64, two.u64);
	CHECK(skuIdEquals(one, two));
}

TEST(OrderIdString) {
	CHECK(orderIdIsNull(kOrderIdInvalid));
	CHECK(orderIdEquals(kOrderIdInvalid, kOrderIdInvalid));

	const OrderId test = {endianSwapU64(0x67452301AB89EFCDULL), endianSwapU64(0x0123456789ABCDEFULL)};

	const OrderId one = orderIdFromString("01234567-89AB-CDEF-0123-456789ABCDEF");
	CHECK_ARRAY_EQUAL(test.c, one.c, sizeof(OrderId));
	CHECK(!orderIdEquals(kOrderIdInvalid, one));

	char string_two[37];
	strcpy(string_two, orderIdAsString(one));
	CHECK_EQUAL("01234567-89ab-cdef-0123-456789abcdef", string_two);

	const OrderId two = orderIdFromString(string_two);
	CHECK_ARRAY_EQUAL(one.c, two.c, sizeof(OrderId));
	CHECK(orderIdEquals(one, two));
}

TEST(SkuIdStashTable) {
	const SkuId a = skuIdFromString("aaaaaaaa");
	const SkuId b = skuIdFromString("bbbbbbbb");
	const SkuId c = skuIdFromString("cccccccc");
	const SkuId d = skuIdFromString("dddddddd");

	StashTable stash = stashTableCreateSkuIndex(0);	
	
	CHECK(stashAddInt(stash, &a, 1, false));
	CHECK(!stashAddInt(stash, &a, 2, false));
	CHECK(stashAddInt(stash, &b, 3, false));
	CHECK(!stashAddInt(stash, &b, 4, false));

	CHECK(stashAddInt(stash, &c, 5, false));
	CHECK(stashAddInt(stash, &c, 6, true));
	CHECK(stashAddInt(stash, &d, 7, false));
	CHECK(stashAddInt(stash, &d, 8, true));

	int value = 0;

	CHECK(stashFindInt(stash, &a, &value));
	CHECK_EQUAL(1, value);
	CHECK(stashFindInt(stash, &b, &value));
	CHECK_EQUAL(3, value);

	CHECK(stashFindInt(stash, &c, &value));
	CHECK_EQUAL(6, value);
	CHECK(stashFindInt(stash, &d, &value));
	CHECK_EQUAL(8, value);

	stashTableDestroy(stash);
}