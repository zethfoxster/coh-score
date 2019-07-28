
#include "beaconPrivate.h"
#include "beaconGenerate.h"
#include "strings_opt.h"
#include "utils.h"
#include "groupnetsend.h"
#include "file.h"

typedef struct BeaconDiskSwapBlockGrid {
	BeaconDiskSwapBlock*	blocks[201][201];
} BeaconDiskSwapBlockGrid;

BeaconProcessBlocks bp_blocks;

#define BUFSIZE 100
static const char* beaconGetDiskSwapHash(int x, int z){
	static char* hashName;
	
	if(!hashName){
		hashName = malloc(BUFSIZE);
	}
	
	STR_COMBINE_BEGIN_S(hashName, BUFSIZE);
	STR_COMBINE_CAT_D(x);
	STR_COMBINE_CAT("_");
	STR_COMBINE_CAT_D(z);
	STR_COMBINE_END();	
	
	return hashName;
}
#undef BUFSIZE

BeaconDiskSwapBlock* beaconGetDiskSwapBlockByGrid(int x, int z){
	int gridx = x + ARRAY_SIZE(bp_blocks.grid->blocks[0]) / 2;
	int gridz = z + ARRAY_SIZE(bp_blocks.grid->blocks) / 2;
	
	if(	gridx >= 0 && gridx < ARRAY_SIZE(bp_blocks.grid->blocks[0]) &&
		gridz >= 0 && gridz < ARRAY_SIZE(bp_blocks.grid->blocks))
	{
		if(bp_blocks.grid){
			return bp_blocks.grid->blocks[gridz][gridx];
		}
	}else{
		if(bp_blocks.table){
			const char* hashName = beaconGetDiskSwapHash(x, z);
			StashElement element;
			stashFindElement(bp_blocks.table, hashName, &element);
	
			if(element){
				BeaconDiskSwapBlock* block = stashElementGetPointer(element);
				assert(block);
				return block;
			}
		}
	}
	
	return NULL;
}

BeaconDiskSwapBlock* beaconGetDiskSwapBlock(int x, int z, int create){
	BeaconDiskSwapBlock* block = NULL;
	StashElement element;
	const char* hashName;
	int gridx;
	int gridz;
	
	x = floor((F32)x / BEACON_GENERATE_CHUNK_SIZE);
	z = floor((F32)z / BEACON_GENERATE_CHUNK_SIZE);
	
	gridx = x + ARRAY_SIZE(bp_blocks.grid->blocks[0]) / 2;
	gridz = z + ARRAY_SIZE(bp_blocks.grid->blocks) / 2;

	if(	gridx >= 0 && gridx < ARRAY_SIZE(bp_blocks.grid->blocks[0]) &&
		gridz >= 0 && gridz < ARRAY_SIZE(bp_blocks.grid->blocks))
	{
		if(!bp_blocks.grid){
			bp_blocks.grid = beaconAllocateMemory(sizeof(*bp_blocks.grid));
		}
		
		block = bp_blocks.grid->blocks[gridz][gridx];
	}else{
		if(!bp_blocks.table){
			bp_blocks.table = stashTableCreateWithStringKeys(512, StashDeepCopyKeys);
		}
	
		hashName = beaconGetDiskSwapHash(x, z);
	
		stashFindElement(bp_blocks.table, hashName, &element);
	
		if(element){
			block = stashElementGetPointer(element);
			assert(block);
		}
	}
	
	if(!block && create){
		block = calloc(sizeof(*block), 1);
		
		block->nextSwapBlock = bp_blocks.list;
		bp_blocks.list = block;

		block->createdIndex = bp_blocks.count++;

		if(	gridx >= 0 && gridx < ARRAY_SIZE(bp_blocks.grid->blocks[0]) &&
			gridz >= 0 && gridz < ARRAY_SIZE(bp_blocks.grid->blocks))
		{
			bp_blocks.grid->blocks[gridz][gridx] = block;
		}else{
			stashAddPointer(bp_blocks.table, hashName, block, false);
		}
		
		block->x = x * BEACON_GENERATE_CHUNK_SIZE;
		block->z = z * BEACON_GENERATE_CHUNK_SIZE;

		if(x < bp_blocks.grid_min_xyz[0])
			bp_blocks.grid_min_xyz[0] = x;
		if(x > bp_blocks.grid_max_xyz[0])
			bp_blocks.grid_max_xyz[0] = x;

		if(z < bp_blocks.grid_min_xyz[2])
			bp_blocks.grid_min_xyz[2] = z;
		if(z > bp_blocks.grid_max_xyz[2])
			bp_blocks.grid_max_xyz[2] = z;
		
		beaconAddToInMemoryList(block);
	}
	
	return block;
}

void beaconAddToInMemoryList(BeaconDiskSwapBlock* block){
	if(block->isInMemory){
		return;
	}

	block->isInMemory = 1;
	
	assert(!bp_blocks.inMemory.count == !bp_blocks.inMemory.head);

	if(bp_blocks.inMemory.tail){
		bp_blocks.inMemory.tail->inMemory.next = block;
		block->inMemory.prev = bp_blocks.inMemory.tail;
		bp_blocks.inMemory.tail = block;
		block->inMemory.next = NULL;
	}else{
		bp_blocks.inMemory.head = bp_blocks.inMemory.tail = block;
		block->inMemory.next = block->inMemory.prev = NULL;
	}
	
	bp_blocks.inMemory.count++;
}
	
static void beaconRemoveFromInMemoryList(BeaconDiskSwapBlock* block){
	if(block->isInMemory){
		return;
	}
	
	block->isInMemory = 0;

	assert(bp_blocks.inMemory.count > 0);
	
	if(block->inMemory.prev){
		block->inMemory.prev->inMemory.next = block->inMemory.next;
	}else{
		bp_blocks.inMemory.head = block->inMemory.next;
	}
	
	if(block->inMemory.next){
		block->inMemory.next->inMemory.prev = block->inMemory.prev;
	}else{
		bp_blocks.inMemory.tail = block->inMemory.prev;
	}
	
	block->inMemory.next = block->inMemory.prev = NULL;
	
	bp_blocks.inMemory.count--;
	
	assert(!bp_blocks.inMemory.count == !bp_blocks.inMemory.head);
}
	
void beaconMakeDiskSwapBlocks(){
	BeaconDiskSwapBlock* swapBlock;
	int i;
	
	// Split beacons into XZ squares for disk-swapping.
	
	// Calculate how many beacons are in each swap block.
	
	for(i = 0; i < combatBeaconArray.size; i++){
		Beacon* b = combatBeaconArray.storage[i];

		swapBlock = beaconGetDiskSwapBlock(posX(b), posZ(b), 1);
		
		swapBlock->beacons.size++;

		beacon_process.infoArray[b->userInt].diskSwapBlock = swapBlock;
	}
	
	// Allocate the memory for the DiskSwapBlocks.
	
	for(swapBlock = bp_blocks.list; swapBlock; swapBlock = swapBlock->nextSwapBlock){
		int size = swapBlock->beacons.size;

		swapBlock->beacons.size = 0;
		
		initArray(&swapBlock->beacons, size);
	}

	// Put the beacons into the swap blocks.

	for(i = 0; i < combatBeaconArray.size; i++){
		Beacon* b = combatBeaconArray.storage[i];
		
		swapBlock = beacon_process.infoArray[b->userInt].diskSwapBlock;

		assert(swapBlock->beacons.size < swapBlock->beacons.maxSize);

		swapBlock->beacons.storage[swapBlock->beacons.size++] = b;
	}
	
}

void beaconClearNonAdjacentSwapBlocks(BeaconDiskSwapBlock* centerBlock){
	BeaconDiskSwapBlock* block;
	
	for(block = bp_blocks.inMemory.head; block;){
		BeaconDiskSwapBlock* next = block->inMemory.next;
		
		if(	!centerBlock ||
			abs(centerBlock->x - block->x) > BEACON_GENERATE_CHUNK_SIZE * 2 ||
			abs(centerBlock->z - block->z) > BEACON_GENERATE_CHUNK_SIZE * 2)
		{
			beaconClearSwapBlockChunk(block);
			
			beaconRemoveFromInMemoryList(block);
		}
		
		block = next;
	}
}

void beaconDestroyDiskSwapInfo(int quiet){
	BeaconDiskSwapBlock* block;
	
	if (bp_blocks.table)
		stashTableDestroy(bp_blocks.table);
	bp_blocks.table = NULL;

	for(block = bp_blocks.list; block;){
		BeaconDiskSwapBlock* next = block->nextSwapBlock;
		
		destroyArrayPartial(&block->beacons);
		
		beaconClearSwapBlockChunk(block);
		
		SAFE_FREE(block->geoRefs.refs);
		ZeroStruct(&block->geoRefs);
		
		SAFE_FREE(block->generatedBeacons.beacons);
		ZeroStruct(&block->generatedBeacons);

		SAFE_FREE(block->clients.clients);
		ZeroStruct(&block->clients);
		
		while(block->legalCompressed.areasHead){
			BeaconLegalAreaCompressed* next = block->legalCompressed.areasHead->next;
			destroyBeaconLegalAreaCompressed(block->legalCompressed.areasHead);
			block->legalCompressed.areasHead = next;
		}
		
		SAFE_FREE(block);
		
		block = next;
	}

	bp_blocks.inMemory.count = 0;
	bp_blocks.inMemory.head = NULL;
	bp_blocks.inMemory.tail = NULL;
	
	bp_blocks.list = NULL;

	SAFE_FREE(bp_blocks.grid);
}
