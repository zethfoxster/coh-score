#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

typedef struct BloomFilter BloomFilter;

typedef enum eFalsePositiveRate
{
	eFalsePositivePercentage_1,
} eFalsePositiveRate;

// The number of expected unique elements coupled with the allowable false positive rate will determine
// the size of the bloomfilter.
BloomFilter* bloomFilterCreate(U32 uiExpectedUniqueElements, F32 fFalsePositiveRate);
void bloomFilterDestroy(BloomFilter* bf);
void bloomAddIntKey(BloomFilter* bf, U32 uiKey);
bool bloomFindIntKey(BloomFilter* bf, U32 uiKey);
bool bloomFindAndAddIntKey(BloomFilter* bf, U32 uiKey);
void bloomClearTable(BloomFilter* bf, U32 uiKey);

void testBloomFilter();


#endif