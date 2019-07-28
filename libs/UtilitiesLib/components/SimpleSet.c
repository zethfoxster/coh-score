#include "SimpleSet.h"
#include "MemoryPool.h"
#include "stdtypes.h"

#include <stdlib.h>

#define hashmask(n) (n-1)

// Same as log2 in mathutil.c
static unsigned int determineRealTableSize(unsigned int requestedSize){
	unsigned int i;
	
	for(i = 0; i < 32; i++)
	{
		if((unsigned int)(1 << i) >= requestedSize)
			return 1 << i;
	}
	return -1;
}

MP_DEFINE(SimpleSet);
#define	SS_SIZE_FOR_MEMPOOL (64*1024)
MP_DEFINE(SimpleSetElement);

SimpleSet* createSimpleSet(){
	MP_CREATE(SimpleSet, 4);
	return MP_ALLOC(SimpleSet);
}

void destroySimpleSet(SimpleSet* set){
	if(set){
		if (set->maxSize == SS_SIZE_FOR_MEMPOOL) {
			MP_FREE(SimpleSetElement, set->storage);
		} else {
			free(set->storage);
		}
		MP_FREE(SimpleSet, set);
	}
}

void initSimpleSet(SimpleSet* set, unsigned int setSize, SimpleSetHash hash){
	set->maxSize = determineRealTableSize(setSize);
	if (set->maxSize == SS_SIZE_FOR_MEMPOOL) {
		// HACK: Make SimpleSets used by the networking code use MemoryPools!
		MP_CREATE_EX(SimpleSetElement, 4, SS_SIZE_FOR_MEMPOOL*sizeof(SimpleSetElement));
		set->storage = MP_ALLOC(SimpleSetElement);
	} else {
		set->storage = calloc(set->maxSize, sizeof(SimpleSetElement));
	}
	set->hash = hash;
	
}

// Returns whether the new entry was added successfully.
// The default behavior when a slot conflict is found is to discard the new entry.
int sSetAddElement(SimpleSet* set, void* value){
	int index = set->hash(value) & hashmask(set->maxSize);

	set->insertionAttemptCount++;
	
	// Add the key and value to the hash table if there is nothing residing in the determined
	// spot.
	if(set->storage[index].value == NULL){
		set->size++;
		set->storage[index].value = value;
		return 1;
	}else
		set->insertionSlotConflict++;
	
	return 0;
}

SimpleSetElement* sSetFindElement(SimpleSet* set, void* value){
	int index = set->hash(value) & hashmask(set->maxSize);
	if(set->storage[index].value){
		return &set->storage[index];
	}
	return NULL;
}