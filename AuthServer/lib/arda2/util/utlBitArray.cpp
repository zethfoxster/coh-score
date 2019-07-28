#include "arda2/core/corFirst.h"
#include "arda2/util/utlBitArray.h"

utlBitProxyHelper::utlBitProxyHelper(utlBitArray &array, unsigned pos):
    mArray(array), mPos(pos)
{}

utlBitProxyHelper &utlBitProxyHelper::operator=(bool value)
{
    mArray.Set(mPos, value);
    return *this;
}

utlBitProxyHelper &utlBitProxyHelper::operator=(const utlBitProxyHelper &that)
{
    mArray.Set(mPos, that.mArray.IsBitSet(that.mPos));
    return *this;
}

utlBitProxyHelper::operator bool() const
{
    return mArray.IsBitSet(mPos);
}

bool utlBitProxyHelper::Flip()
{
    mArray.FlipBit(mPos);
    return mArray.IsBitSet(mPos);
}
