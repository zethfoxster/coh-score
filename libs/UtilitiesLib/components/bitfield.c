// bitfield.c - functions to deal with integer arrays of bits

#include "bitfield.h"
#include <string.h>

#define BITS_PER_ITEM 32

int BitFieldGetNext(U32* bitfield, int size, int from)
{
	int bits = size * BITS_PER_ITEM;

	// iterate through bits to test
	from++;
	while (from < bits)
	{
		int item = from / BITS_PER_ITEM;
		int index = from % BITS_PER_ITEM;

		if (index == 0 && bitfield[item] == 0) from += BITS_PER_ITEM;
		else
		{
			if (BitFieldGet(bitfield, size, from)) return from;
			from++;
		}
	}
	return -1; // couldn't find bit
}

U32 BitFieldGet(U32* bitfield, int size, int bit)
{
	int bits = size * BITS_PER_ITEM;
	int item = bit / BITS_PER_ITEM;
	int index = bit % BITS_PER_ITEM;
	U32 ret;
	if (bit < 0 || bit > bits-1) return 0;
	ret = bitfield[item];
	ret >>= index;
	return ret & 1;
}

void BitFieldSet(U32* bitfield, int size, int bit, U32 value)
{
	int bits = size * BITS_PER_ITEM;
	int item = bit / BITS_PER_ITEM;
	int index = bit % BITS_PER_ITEM;
	U32 ormask = 1 << index;
	U32 andmask = ~ormask;

	if (bit < 0 || bit > bits-1) return;
	if (value) bitfield[item] |= ormask;
	else bitfield[item] &= andmask;
}

void BitFieldClear(U32* bitfield, int size)
{
	memset(bitfield, 0, sizeof(U32)*size);
}

void BitFieldCopy(const U32* src, U32* dest, int size)
{
	memcpy(dest, src, sizeof(U32)*size);
}

// Counts the number of on-bits or off-bits in the given bitfield.
int BitFieldCount(U32* bitfield, int size, int countOnBits)
{
	int bits = size * BITS_PER_ITEM;
	int itemIndex;
	int onBitsCount = 0;

	// Iterator through all bits in the bit field...
	for(itemIndex = 0; itemIndex < size; itemIndex++)
	{
		unsigned int item = bitfield[itemIndex];
		unsigned int bitCursor = 1;

		// Count how many of the bits are set.
		while(bitCursor != 0)
		{
			if(bitCursor & item)
				onBitsCount++;

			bitCursor = bitCursor << 1;
		}
		
	}

	if(countOnBits)
		return onBitsCount;
	else
		return bits - onBitsCount;
}

// Optimized for bitfields with a sparse number of ones
U32 BitFieldCountSparseOn(U32* bitfield, int size)
{
	int word_index=size;
	U32 count = 0;
	while (word_index>0)
	{
		U32 uiWord = bitfield[--word_index];
		while (uiWord)
		{
			++count;
			uiWord &= (uiWord - 1);
		}
	}

	return count;
}

void BitFieldOr(U32* src, U32* dest, int size)
{
	int i;
	for( i = 0; i < size; i++ )
		dest[i] |= src[i];
}


int BitFieldAnd(U32* src, U32* dest, int size)
{
	int i;
	for( i = 0; i < size; i++ )
		dest[i] &= src[i];
	return BitFieldCountSparseOn(dest,size);
}

int BitFieldAnyAnd(U32* left, U32* right, int size)
{
	int i;
	for( i = 0; i < size; i++ )
	{
		if( left[i] & right[i])
			return 1;
	}
	return 0;
}