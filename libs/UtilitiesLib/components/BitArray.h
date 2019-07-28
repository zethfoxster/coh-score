/* File BitArray.h
 *	This file defines a structure with a small memory footprint that can
 *	manage up to 512k bits.
 *
 *
 */

#ifndef BITARRAY_H
#define BITARRAY_H

typedef struct BitArrayImp {
	unsigned int	maxSetByte;
	unsigned int	maxSize;	// The max number of bytes this bit array can manage.
	unsigned char	storage;	// The first byte of the bit array.
} BitArrayImp;

typedef struct BitArrayImp *BitArray;

// BitArray creation/destruction
BitArray createBitArray(unsigned int maxBitNum);
void destroyBitArray(BitArray array);

// Bit setting/clearing
#define BITS_ALTERED	2
int baSetBitValue(BitArray array, unsigned int bitnum, int value);

int baSetBit(BitArray array, unsigned int bitnum);
int baClearBit(BitArray array, unsigned int bitnum);

int baSetBitRange(BitArray array, unsigned int bitnum_start, unsigned int num_bits);
int baClearBitRange(BitArray array, unsigned int bitnum_start, unsigned int num_bits);

int baClearBits(BitArray target, BitArray mask);

void baSetAllBits(BitArray array);
void baClearAllBits(BitArray array);
void baNotAllBits(BitArray array);

// Bit testing.
int baIsSet(BitArray array, unsigned int bitnum);
static INLINEDBG int baIsSetInline(BitArrayImp* array, unsigned int bitnum)
{
	unsigned int baIndex = bitnum >> 3;
	if (baIndex > array->maxSetByte)
		return 0;
	return ((unsigned char*)&array->storage)[baIndex] & 1 << (bitnum & 7);
}

int baGetMaxBits(BitArray array);
int baCountSetBits(BitArray array);
int baOrArray(BitArray array1, BitArray array2, BitArray output);
int baAndArray(BitArray array1, BitArray array2, BitArray output);
int baCopyArray(BitArray input, BitArray output);
int baBitsAreEqual(BitArray array1, BitArray array2);


void testBitArray();

#endif
