/* File BitArray
 *	This file provides an implemenation of BitArray.
 *
 *	Structure layout
 *		Since the very purpose of a BitArray is to be able track a large
 *		quanity of boolean values using little memory, this implementation 
 *		of BitArray attempts to minize the memory overhead associated 
 *		with dynamic memory allocation.
 *	
 *		The trick here is that BitArray::storage is used to mark where the
 *		storage area of the BitArray begins.  It is safe to index pass the
 *		address of BitArray::storage up to BitArray::maxSize-1.
 *
 *		The main purpose of the trick is to save the amount of storage
 *		needed per BitArray.  Compared to calling malloc for a small storage
 *		array, this implemenation avoids the one pointer (4 bytes) needed 
 *		in the BitArray structure and any additional memory required by 
 *		the malloc call (at least 4 bytes).
 *
 *		This is the same trick used in StringTable.
 *
 *	Limitations
 *		This implementation of BitArray can only manage 512k bits.  This is just a 
 *		arbitrary limitation due to the fact that maxSize is tracked using a 16 bit
 *		value.  Since maxSize tracks the number of bytes that the Bitarray can track,
 *		this allows 64k * 8 = 512k bits to be tracked.  It seems unlikely for a BitArray 
 *		to hold in excess of 512k bits.  If this should become the case, change 
 *		BitArrayImp::maxSize to unsigned int instead.
 *
 *		Note that normally, a 16 bit value can represents values 0 ~ 64k-1.  However,
 *		since it does not make sense to have a BitArray that can track 0 bits.  The
 *		logical value range for BitArrayImp::maxSize has been mapped to is 1 ~ 64k.  
 *		This explains the various +1 and -1 operations done on BitArrayImp::maxSize
 *		in this module.
 *
 *  Changes
 *		CD: I changed the type of maxSize from unsigned short to unsigned int to get bigger BitArrays.
 *
 */

#include <wininclude.h>
#include "BitArray.h"

#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


BitArray createBitArray(unsigned int maxBitNum){
	unsigned int storageByteSize;
	BitArrayImp* array;

	// Calculate the number of bytes required to hold the requested number of bits.
	storageByteSize = (maxBitNum + 7) >> 3;

	// Can the BitArray manage the requested number of bytes?
	// If not, then BitArray creation has failed.
	if (0 == maxBitNum)
		return 0;

	// Allocate enough memory to store BitArrayImp::maxSize and
	// maxBitNum bits.
	array = (BitArrayImp*)calloc(1, sizeof(array->maxSetByte) + sizeof(array->maxSize) + storageByteSize);

	array->maxSetByte = storageByteSize - 1;
	array->maxSize = storageByteSize - 1;
	return (BitArray)array;
}

void destroyBitArray(BitArrayImp* array){
	free(array);
}

int baSetBitValue(BitArrayImp* array, unsigned int bitnum, int value){
	if(value)
		return baSetBit(array, bitnum);
	else
		return baClearBit(array, bitnum);
}


int baSetBitRange(BitArray array, unsigned int bitnum_start, unsigned int num_bits){
	unsigned int baIndex, baIndexEnd, i;
	unsigned int bitnum_end = bitnum_start + num_bits - 1;
	int bitAltered = 0;
	unsigned char* byte;
	int byteMask;

	assert(array);

	if (!num_bits)
		return 0;

	// Divide the given bitnum by 8 to find out which byte index
	// the given bit can be found.
	baIndex = bitnum_start >> 3;
	baIndexEnd = bitnum_end >> 3;

	// Make sure that the bitnum can be stored in the given BitArray.
	if (baIndexEnd > array->maxSize)
		return 0;

	if (baIndex == baIndexEnd)
	{
		byteMask = 0;
		for (i = 0; i < num_bits; i++)
			byteMask |= 1 << ((bitnum_start+i) & 7);
		byte = &array->storage + baIndex;
		bitAltered = !(*byte & byteMask);
		*byte |= byteMask;
	}
	else
	{
		unsigned int startBits = 8 - (bitnum_start & 7);
		unsigned int endBits = 1 + (bitnum_end & 7);

		// start byte
		byteMask = 0;
		for (i = 0; i < startBits; i++)
			byteMask |= 1 << ((bitnum_start+i) & 7);
		byte = &array->storage + baIndex;
		bitAltered = !(*byte & byteMask);
		*byte |= byteMask;

		// middle bytes
		for (i = baIndex + 1; i < baIndexEnd; i++)
		{
			byte = &array->storage + i;
			bitAltered = bitAltered || !(*byte & 0xff);
			*byte |= 0xff;
		}

		// end byte
		byteMask = 0;
		for (i = 0; i < endBits; i++)
			byteMask |= 1 << i;
		byte = &array->storage + baIndexEnd;
		bitAltered = bitAltered || !(*byte & byteMask);
		*byte |= byteMask;
	}

	if (baIndexEnd > array->maxSetByte)
		array->maxSetByte = baIndexEnd;

	return 1 + bitAltered;
}

int baClearBitRange(BitArray array, unsigned int bitnum_start, unsigned int num_bits){
	unsigned int baIndex, baIndexEnd, i;
	unsigned int bitnum_end = bitnum_start + num_bits - 1;
	int bitAltered = 0;
	unsigned char* byte;
	int byteMask;

	assert(array);

	if (!num_bits)
		return 0;

	// Divide the given bitnum by 8 to find out which byte index
	// the given bit can be found.
	baIndex = bitnum_start >> 3;
	baIndexEnd = bitnum_end >> 3;

	// Make sure that the bitnum can be stored in the given BitArray.
	if (baIndexEnd > array->maxSize)
		return 0;

	if (baIndex == baIndexEnd)
	{
		byteMask = 0;
		for (i = 0; i < num_bits; i++)
			byteMask |= 1 << ((bitnum_start+i) & 7);
		byte = &array->storage + baIndex;
		bitAltered = (*byte & byteMask);
		*byte &= ~byteMask;
	}
	else
	{
		unsigned int startBits = 8 - (bitnum_start & 7);
		unsigned int endBits = 1 + (bitnum_end & 7);

		// start byte
		byteMask = 0;
		for (i = 0; i < startBits; i++)
			byteMask |= 1 << ((bitnum_start+i) & 7);
		byte = &array->storage + baIndex;
		bitAltered = (*byte & byteMask);
		*byte &= ~byteMask;

		// middle bytes
		for (i = baIndex + 1; i < baIndexEnd; i++)
		{
			byte = &array->storage + i;
			bitAltered = bitAltered || (*byte & 0xff);
			*byte = 0;
		}

		// end byte
		byteMask = 0;
		for (i = 0; i < endBits; i++)
			byteMask |= 1 << i;
		byte = &array->storage + baIndexEnd;
		bitAltered = bitAltered || (*byte & byteMask);
		*byte &= ~byteMask;
	}

	if (baIndexEnd > array->maxSetByte)
		array->maxSetByte = baIndexEnd;

	return 1 + bitAltered;
}

/* Function baSetBit()
 *	This function sets the specified bit in the given BitArray.
 *
 *	Returns:
 *		0 - Unable to set bit because bitnum is invalid.
 *		1 - Bit set.
 *		2 - Bit set and is altered.
 */
int baSetBit(BitArrayImp* array, unsigned int bitnum){
	unsigned int baIndex;		// Given the bitnum, which byte should be examined?
	int bitAltered = 0;
	unsigned char* byte;
	int byteMask;

	assert(array);

	// Divide the given bitnum by 8 to find out which byte index
	// the given bit can be found.
	baIndex = bitnum >> 3;

	// Make sure that the bitnum can be stored in the given BitArray.
	if(baIndex > array->maxSize)
		return 0;

	// Mask out anything that cannot be represented by a single byte then shift
	// the bit into the correct position to produce a bitmask.
	byteMask = 1 << (bitnum & 7);
	byte = &array->storage + baIndex;

	// Find out if the bit will be altered or not.
	bitAltered = !(*byte & byteMask);

	*byte |= byteMask;

	if (baIndex > array->maxSetByte)
		array->maxSetByte = baIndex;

	return 1 + bitAltered;
}

/* Function baClearBit()
 *	This function clears the specified bit in the given BitArray.
 *
 *	Returns:
 *		0 - Unable to clear bit because bitnum is invalid.
 *		1 - Bit cleared.
 *		2 - Bit cleared and is altered.
 */
int baClearBit(BitArrayImp* array, unsigned int bitnum){
	unsigned int baIndex;		// Given the bitnum, which byte should be examined?
	int bitAltered = 0;
	unsigned char* byte;
	int byteMask;

	assert(array);

	// Divide the given bitnum by 8 to find out which byte index
	// the given bit can be found.
	baIndex = bitnum >> 3;

	// Make sure that the bitnum can be stored in the given BitArray.
	if(baIndex > array->maxSize)
		return 0;

	// Mask out anything that cannot be represented by a single byte then shift
	// the bit into the correct position to produce a bitmask.
	byteMask = 1 << (bitnum & 7);
	byte = &array->storage + baIndex;

	// Find out if the bit will be altered or not.
	bitAltered = *byte & byteMask;

	*byte &= ~byteMask;
	
	if (baIndex == array->maxSetByte)
	{
		while (*byte == 0 && array->maxSetByte)
		{
			array->maxSetByte--;
			byte = &array->storage + array->maxSetByte;
		}
	}

	return 1 + bitAltered;
}

/* Function baClearBits()
 *	This function takes the given mask and clears the specified bits in the
 *	target BitArray.
 *
 *	Returns:
 *		1 - Bits cleared
 *		2 - Bits cleared and altered.
 */
int baClearBits(BitArrayImp* target, BitArrayImp* mask){
	int indexLimit;
	int bitsAltered = 0;

	assert(target && mask);

	// Find out how many byte should be CLEARED'd.
	indexLimit = MIN(target->maxSize, mask->maxSize);

	// CLEAR the allowed number of bytes.
	{
		int i;
		for(i = 0; i <= indexLimit; i++){
			if(!bitsAltered && ((unsigned char*)&target->storage)[i] & ((unsigned char*)&mask->storage)[i])
				bitsAltered = 1;

			((unsigned char*)&target->storage)[i] = 
				((unsigned char*)&target->storage)[i] & ~((unsigned char*)&mask->storage)[i];
		}
	}

	return 1 + bitsAltered;
}

/* Function baSetAllBits()
 */
void baSetAllBits(BitArrayImp* array)
{
	assert(array);

	memset(&array->storage, 0xffffffff, array->maxSize + 1);

	array->maxSetByte = array->maxSize;

	//for(i = 0; i <= array->maxSize; i++){
	//	((unsigned char*)&array->storage)[i] = 0xff;
	//}
}

/* Function baClearAllBits()
 */
void baClearAllBits(BitArrayImp* array)
{
	assert(array);

	memset(&array->storage, 0, array->maxSetByte + 1);

	array->maxSetByte = 0;

	//for(i = 0; i <= array->maxSize; i++){
	//	((unsigned char*)&array->storage)[i] = 0;
	//}
}

/* Function baClearAllBits()
 */
void baNotAllBits(BitArrayImp* array){
	unsigned int i;

	assert(array);

	for(i = 0; i <= array->maxSize; i++){
		((unsigned char*)&array->storage)[i] = ~((unsigned char*)&array->storage)[i];
	}

	array->maxSetByte = array->maxSize;
}

/* Function baIsSet()
 *	This function tests the specified bit in the given BitArray.
 *
 *	Returns:
 *		0 - Bit is not set.
 *		1 - Bit is set.
 */
int baIsSet(BitArrayImp* array, unsigned int bitnum){
	unsigned int baIndex;		// Given the bitnum, which byte should be examined?
	assert(array);

	// Divide the given bitnum by 8 to find out which byte index
	// the given bit can be found.
	baIndex = bitnum >> 3;

	// Make sure that the bitnum can be stored in the given BitArray.
	if (baIndex > array->maxSetByte)
		return 0;

	return ((unsigned char*)&array->storage)[baIndex] & 1 << (bitnum & 7);
}

/* Function baCountSetBits()
 *	This function counts the bits that are set in the given BitArray.
 *
 *	Returns:
 *		0 - Bit is not set.
 *		1 - Bit is set.
 */
int baCountSetBits(BitArrayImp* array){
	unsigned int byteIndex;
	int setBitCount = 0;

	assert(array);

	// For every byte in the BitArray...
	for(byteIndex = 0; byteIndex <= array->maxSetByte; byteIndex++){
		unsigned char byte = ((unsigned char*)&array->storage)[byteIndex];

		// For every bit in the byte...
		while(byte){

			// If the lowest bit is set, count it.
			if(1 & byte){
				setBitCount++;
			}

			byte >>= 1;
		}
	}

	return setBitCount;
}

int baGetMaxBits(BitArrayImp* array){
	assert(array);

	return (array->maxSize + 1) << 3;
}

int baOrArray(BitArrayImp* array1, BitArrayImp* array2, BitArrayImp* output){
	unsigned int indexLimit;

	assert(array1 && array2 && output);

	// Find out how many byte should be OR'd.
	indexLimit = MAX(array1->maxSetByte, array2->maxSetByte);
	indexLimit = MIN(indexLimit, array1->maxSize);
	indexLimit = MIN(indexLimit, array2->maxSize);
	indexLimit = MIN(indexLimit, output->maxSize);

	// OR the allowed number of bytes.
	{
		unsigned int i;
		for(i = 0; i <= indexLimit; i++){
			((unsigned char*)&output->storage)[i] = 
				((unsigned char*)&array1->storage)[i] | ((unsigned char*)&array2->storage)[i];
		}
	}

	output->maxSetByte = indexLimit;
	return 1;
}

int baAndArray(BitArrayImp* array1, BitArrayImp* array2, BitArrayImp* output){
	unsigned int indexLimit;

	assert(array1 && array2 && output);

	// Find out how many byte should be AND'd.
	indexLimit = MIN(array1->maxSetByte, array2->maxSetByte);
	indexLimit = MIN(indexLimit, output->maxSize);

	// AND the allowed number of bytes.
	{
		unsigned int i;
		for(i = 0; i <= indexLimit; i++){
			((unsigned char*)&output->storage)[i] = 
				((unsigned char*)&array1->storage)[i] & ((unsigned char*)&array2->storage)[i];
		}
	}

	output->maxSetByte = indexLimit;
	return 1;
}

int baCopyArray(BitArrayImp* input, BitArrayImp* output){
	int indexLimit;

	assert(input && output);

	indexLimit = 1 + MIN(input->maxSize, output->maxSize);

	memcpy(&output->storage, &input->storage, indexLimit * sizeof(unsigned char));

	//for(i = 0; i < indexLimit; i++){
	//	((unsigned char*)&output->storage)[i] = ((unsigned char*)&input->storage)[i];
	//}

	output->maxSetByte = input->maxSetByte;
	return 1;
}

int baBitsAreEqual(BitArrayImp* array1, BitArrayImp* array2){
	int indexLimit;
	int i;

	assert(array1 && array2);

	indexLimit = MIN(array1->maxSize, array2->maxSize);

	for(i = 0; i <= indexLimit; i++){
		if(((unsigned char*)&array1->storage)[i] != ((unsigned char*)&array2->storage)[i])
			return 0;
	}

	return 1;
}

#include <stdio.h>
/* Function testBitArrayAllBits()
 *	This function turns all bits in a bit array on, then off.  The target bit is 
 *	tested after each operation to make sure that the bits have been altered in 
 *	the correct ways.
 *
 */
void testBitArrayAllBits(unsigned int bitnum){
	BitArray array;
	int maxBits;

	printf("Testing all bits in a %i bit BitArray\n", bitnum);
	printf("Creating BitArray to hold at least %i bits\n", bitnum);
	array = createBitArray(bitnum);

	if(!array){
		printf("Could not create BitArray\n");
		return;
	}

	maxBits = baGetMaxBits(array);
	printf("The BitArray can hold a maximum of %i bits\n", maxBits);

	// Test all the bits in the array.
	{
		int i;
		int testFailed = 0;

		for(i = 0; i < maxBits; i++){
			// Check the current status of the bit under examination.
			// Since the BitArray was just created, the bit should be cleared.
			if(baIsSet(array, i)){
				printf("Bit %i was set right after creation\n", i);
				testFailed = 1;
				continue;
			}

			// Try to set the bit.
			// Check if the operation succeeded.
			if(!baSetBit(array, i)){
				printf("Bit %i could not be set\n", i);
				testFailed = 1;
				continue;
			}
			
			// Was the bit realy set?
			if(!baIsSet(array, i)){
				printf("Bit %i set operation succeeded but tested as \"cleared\"\n", i);
				testFailed = 1;
				continue;
			}

			// Try to clear the bit now.
			if(!baClearBit(array, i)){
				printf("Bit %i could not be cleared\n", i);
				testFailed = 1;
				continue;
			}

			if(baIsSet(array, i)){
				printf("Bit %i clear operation succeeded but tested as \"set\"\n", i);
				testFailed = 1;
				continue;
			}
		}

		if(testFailed){
			printf("Test failed\n");	
		}else{
			printf("Test succeeded\n");
		}
	}
	destroyBitArray(array);
}

void testBitArrayCountBits(int bitnum){
	BitArray array;
	int count;
	int testFailed = 0;

	printf("Testing baCountSetBits in a %i bit BitArray\n", bitnum);

	array = createBitArray(bitnum);
	
	count = baCountSetBits(array);
	printf("The current count is %i\n", count);
	if(0 != count){
		printf("The baCountSetBits function counted %i set bits right after creation\n", count);
		testFailed = 1;
		goto exit;
	}

	{
		int i;

		// Set all bits in the array.
		for(i = 0; i < baGetMaxBits(array); i++){
			baSetBit(array, i);
		}
	}

	count = baCountSetBits(array);
	printf("After setting all bits, the set bit count is %i\n", count);
	if(count != baGetMaxBits(array)){
		printf("The baCountSetBits function counted %i set bits when it should have counted %i bits\n", count, baGetMaxBits(array));
		testFailed = 1;
		goto exit;
	}

exit:
	if(testFailed)
		printf("Test failed\n");
	else
		printf("Test succeeded\n");

	destroyBitArray(array);
}

/* Function testBitArraySetSpeed()
 *	This function turns all bits on and measures the average elapse time
 *	over the given number of test runs.
 *
 */

void testBitArraySetSpeed(unsigned int bitnum, unsigned int testRuns){
	LARGE_INTEGER beginTime;
	LARGE_INTEGER endTime;
	LARGE_INTEGER frequency;
	LONGLONG totalElapsedTime = 0;
	BitArray array;
	unsigned int curTestRun;
	int maxArrayBits;

	printf("Testing set speed in a %i bit BitArray\n", bitnum);

	QueryPerformanceFrequency(&frequency);
	
	for(curTestRun = 0; curTestRun < testRuns; curTestRun++){
		int i;
		array = createBitArray(bitnum);
		maxArrayBits = baGetMaxBits(array);
		
		QueryPerformanceCounter(&beginTime);
		for(i = 0; i < maxArrayBits; i++){
			baSetBit(array, i);
		}
		QueryPerformanceCounter(&endTime);

		destroyBitArray(array);

		totalElapsedTime += endTime.QuadPart - beginTime.QuadPart;
	}

	printf("In %i test runs, it took %f seconds on average to set %i bits\n", 
		testRuns, 
		(float)totalElapsedTime / frequency.QuadPart / testRuns,
		bitnum);
}

void testBitArray(){


	printf("----------\tBitArray Test Begin\t----------\n");

	// Run a test where the amount of storage needed is below the byte boundry.
	testBitArrayAllBits(5);
	printf("\n");
	
	// Run a test where the amount of storage needed is right on the byte boundry.
	testBitArrayAllBits(8);
	printf("\n");

	// Run a test where the amount of storage needed is over the byte boundry.
	testBitArrayAllBits(10);
	printf("\n");

	// Run a test where the amount of storage is the maximum that that the BitArray can deal with.
	testBitArrayAllBits(512 * 1024);
	printf("\n");

	// Run a test where the amount of storage exceeds the maximum that the BitArray can deal with.
	testBitArrayAllBits(512 * 1024 + 1);
	printf("\n");

	testBitArraySetSpeed(8, 5);
	printf("\n");

	testBitArraySetSpeed(10, 5);
	printf("\n");

	testBitArraySetSpeed(128, 5);
	printf("\n");

	testBitArraySetSpeed(512 * 1024, 20);
	printf("\n");

	testBitArrayCountBits(512 * 1024);
	printf("----------\tBitArray Test End\t----------\n");
}
