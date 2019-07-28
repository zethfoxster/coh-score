#include "AccountTypes.h"
#include "structDefines.h"
#include "components/HashFunctions.h"
#include "components/StashTable.h"
#include "utils/endian.h"
#include "utils/SuperAssert.h"

const SkuId kSkuIdInvalid = {0};
const OrderId kOrderIdInvalid = {0,0};

StaticDefineInt AccountTypes_RarityEnum[] =
{
	DEFINE_INT
	{ "Common", 1},
	{ "Uncommon", 2},
	{ "Rare", 3},
	{ "VeryRare", 4},
	DEFINE_END
};

STATIC_ASSERT(ORDER_ID_BITS == sizeof(OrderId)*8);

/**
* Convert a SKU from a 8 character NULL terminated string to a @ref SkuId.
*/
SkuId skuIdFromString(const char * sku_string)
{
	SkuId ret = {0};
	unsigned i;
	bool valid_sku_id = true;

	for (i=0; i<sizeof(SkuId); i++) {
		valid_sku_id &= (sku_string[i] != 0);
		ret.c[i] = toupper(sku_string[i]);
	}
	valid_sku_id &= (sku_string[i] == 0);

	if (!devassertmsg(valid_sku_id, "Invalid SkuId '%s'", sku_string))
		return kSkuIdInvalid;
	return ret;
}

/**
* Build a 8 character NULL terminated printable string for the @ref SkuId.
* @return Stack allocated temporary string.
*/
char * skuIdAsString(SkuId sku_id)
{
	static char buf[9];
	memcpy(buf, sku_id.c, sizeof(SkuId));
	buf[8] = 0;
	return buf;
}

/**
* Compare two @ref SkuId values in order to sort them.
*/
int skuIdCmp(SkuId a, SkuId b)
{
	if (a.u64 == b.u64)
		return 0;
	return a.u64 > b.u64 ? 1 : -1;
}

/**
* Test two @ref SkuId values for equality.
*/
bool skuIdEquals(SkuId a, SkuId b)
{
	return a.u64 == b.u64;
}

/**
* Check if a @ref SkuId is null.
*/
bool skuIdIsNull(SkuId sku_id)
{
	return sku_id.u64 == 0;
}

/**
* Endian swap half of an order id depending on the platform endianess.
*/
static INLINEDBG OrderId orderIdEndianSwap(OrderId order_id)
{
	if (isBigEndian())
	{
		order_id.u32[0] = endianSwapU32(order_id.u32[0]);
		order_id.u16[2] = endianSwapU16(order_id.u16[2]);
		order_id.u16[3] = endianSwapU16(order_id.u16[3]);
	}
	else
	{
		order_id.u16[4] = endianSwapU16(order_id.u16[4]);
		order_id.u16[5] = endianSwapU16(order_id.u16[5]);
		order_id.u32[3] = endianSwapU32(order_id.u32[3]);
	}
	return order_id;
}

/**
* Convert an UUID formatted string into an @ref OrderId.
* @param order_string 00000000-0000-0000-0000-000000000000.
*/
OrderId orderIdFromString(const char * order_string)
{
	OrderId order_id = kOrderIdInvalid;
	int ate = 0;
	int got = sscanf(order_string, "%08x-%04hx-%04hx-%04hx-%04hx%08x%n", &order_id.u32[0], &order_id.u16[2], &order_id.u16[3], &order_id.u16[4], &order_id.u16[5], &order_id.u32[3], &ate);
	if (!devassertmsg(got == 6 && ate == 36 && order_string[ate]==0, "Invalid OrderId '%s'", order_string))
		return kOrderIdInvalid;
	return orderIdEndianSwap(order_id);
}

/**
* Build a UUID formatted NULL terminated printable string for the @ref OrderId.
* @return Stack allocated temporary string in the form
*         00000000-0000-0000-0000-000000000000.
*/
char * orderIdAsString(OrderId order_id)
{
	static char buf[ORDER_ID_STRING_LENGTH];
	order_id = orderIdEndianSwap(order_id);
	snprintf(buf, sizeof(buf), "%08x-%04hx-%04hx-%04hx-%04hx%08x", order_id.u32[0], order_id.u16[2], order_id.u16[3], order_id.u16[4], order_id.u16[5], order_id.u32[3]);
	return buf;
}

/**
* Compare two @ref OrderId values in order to sort them.
*/
int orderIdCmp(OrderId a, OrderId b)
{
	if (a.u64[0] != b.u64[0])
		return a.u64[0] > b.u64[0] ? 1 : -1;
	if (a.u64[1] == b.u64[1])
		return 0;
	return a.u64[1] > b.u64[1] ? 1 : -1;
}

/**
* Test two @ref OrderId values for equality.
*/
bool orderIdEquals(OrderId a, OrderId b)
{
	return a.u64[0] == b.u64[0] && a.u64[1] == b.u64[1];
}

/**
* Check if a @ref OrderId is null.
*/
bool orderIdIsNull(OrderId order_id)
{
	return order_id.u64[0] == 0 && order_id.u64[1] == 0;
}

/// Custom @ref StashTable hash function for @ref SkuId keys
static unsigned int skuIndexHash(void* userData, const void* key, int hashSeed) {
	return burtlehash2(reinterpret_cast<const ub4*>(key), sizeof(SkuId)/sizeof(ub4), hashSeed);
}

/// Custom @ref StashTable comparison function for @ref SkuId keys.
static int skuIndexCmp(void* userData, const void* key1, const void* key2) {
	return skuIdCmp(*reinterpret_cast<const SkuId*>(key1), *reinterpret_cast<const SkuId*>(key2));
}

/**
* Create a @ref StashTable with @ref SkuId keys.
*/
StashTable stashTableCreateSkuIndexEx(U32 uiInitialSize, const char* pcFile, U32 uiLine)
{
	return stashTableCreateExternalFunctionsEx(uiInitialSize, StashDefault, skuIndexHash, skuIndexCmp, NULL, pcFile, uiLine);
}
