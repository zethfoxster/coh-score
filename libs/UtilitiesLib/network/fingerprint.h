/* SimpleHashTable.h
 *	This module contains a really simple hash table that maps any 32bit value to another
 *	32bit value.  The behaviors of this hash table is as follows:
 *		- Key compares are "shallow" compares only.
 *		- Hash slot conflicts are resolved by discarding new (incoming) entry.
 *		- Table size is a prime number.
 *
 *	This module will most likely not be useful at all except for in the patcher.
 */

#ifndef FINGERPRINT_H
#define FINGERPRINT_H

#include "stdtypes.h"

typedef struct{
	void* key;
	void* value;
	U32 hitCount;
	U32 hadCollision;
} SimpleHashElement;

typedef	int (*SimpleHashFunction)(const void* key, int matchSize);
typedef struct SimpleHashTable SimpleHashTable;

struct SimpleHashTable{
	U32 size;
	U32 maxSize;
	SimpleHashElement* storage;
	U32 minMatchSize;
	U32 insertionSlotConflict;
	U32 insertionAttemptCount;
	U32 insertionKeyCollisions;
	U32 insertionDuplicateValues;

	U32 recursiveDepth;
	SimpleHashTable* collisionTable;
	SimpleHashTable* matchTable;

	U16 checksum_low,checksum_high;
};


SimpleHashTable* createSimpleHashTable();
void destroySimpleHashTable(SimpleHashTable* table);
void initSimpleHashTable(SimpleHashTable* table, U32 tableSize, U32 minMatchSize);

int sHashAddElement(SimpleHashTable* table, void* key, U32 maxKeySize, void* value);
SimpleHashElement* sHashFindElement(SimpleHashTable* table, void* key, U32 maxKeySize);
//SimpleHashElement* sHashFindElementByHash(SimpleHashTable* table, void* key, U32 hash);

int checksumInit(SimpleHashTable* table,const unsigned char* buf, int size);
int checksumRoll(SimpleHashTable* table,const unsigned char* buf,int size);
void fingerPrintStats(SimpleHashTable *FingerPrintTable);

#endif