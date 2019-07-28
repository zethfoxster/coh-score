/* File MemoryPool.h
 *	A memory pool manages a large array of structures of a fixed size.  
 *	The main advantage of are
 *		- Low memory management overhead
 *			It manages large pieces of memory without the need for additional memory for
 *			management purposes, unlike malloc/free.
 *		- Fast at serving mpAlloc/mpFree requests.
 *			Very few operations are needed before it can complete both operations.
 *		- Suitable for managing both statically and dynamically allocated arrays.
 *
 *
 */

#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
#include "assert.h"

C_DECLARATIONS_BEGIN

// MS: Some hideous macro code to make memory pools easier.
// Usage: To create and use a memory pool for a type called FunType, use this code (sorry for lame #if 0 thing):

#if 0
	// Create the global memory pool like this.  This create a variable called memPoolFunType.
	// If you ever want to destroy the memory pool, use MP_DESTROY(FunType).

	MP_DEFINE(FunType);

	FunType* createFunType(){
		FunType* funType;
		
		// Initialize the memory pool (only does anything the first time it's called).
		// 1000 is how many elements are in a MemoryPool chunk.
		
		MP_CREATE(FunType, 1000); 
		
		// Allocate a new instance from the memory pool.
		
		funType = MP_ALLOC(FunType);
		
		// Initialize funType if you want, then return it.
		
		return funType;
	}

	void destroyFunType(FunType* funType){
		// Give the memory back to the memory pool.
	
		MP_FREE(FunType, funType);
	}
#endif

// The hideous macros.

#define MP_NAME(type) memPool##type

#define MP_CREATE(type, amount)												\
	if(!MP_NAME(type)){														\
		MP_NAME(type) = createMemoryPoolNamed(#type, __FILE__, __LINE__);	\
		initMemoryPool(MP_NAME(type), sizeof(type), amount);				\
		mpSetMode(MP_NAME(type), ZeroMemoryBit);							\
	}

#define MP_CREATE_EX(type, amount, size)									\
	if(!MP_NAME(type)){														\
		MP_NAME(type) = createMemoryPoolNamed(#type, __FILE__, __LINE__);	\
		initMemoryPool(MP_NAME(type), size, amount);						\
		mpSetMode(MP_NAME(type), ZeroMemoryBit);							\
	}


#define MP_DESTROY(type)					\
	if(MP_NAME(type)){						\
		destroyMemoryPool(MP_NAME(type));	\
		MP_NAME(type) = 0;					\
	}

#define MP_ALLOC(type) ((type*)(MP_NAME(type) ? mpAlloc(MP_NAME(type)) : 0))

#define MP_FREE(type, mem) if(mem){ mpFree(MP_NAME(type), mem); (mem)=0;}
#define MP_FREE_NOCLEAR(type, mem) if(mem){ mpFree(MP_NAME(type), mem);}

#define MP_DEFINE(type) static MemoryPool MP_NAME(type) = 0

// The functions and stuff for MemoryPool.

typedef void (*Destructor)(void*);

// MemoryPool internals are hidden.  Accidental changes are *bad* for memory management.
typedef struct MemoryPoolImp *MemoryPool;


#define ZERO_MEMORY_BIT				1

typedef enum{
	TurnOffAllFeatures =		0,
	Default =					ZERO_MEMORY_BIT,
	ZeroMemoryBit =				ZERO_MEMORY_BIT,
} MemoryPoolMode;


// MemoryPool mode query/alteration
MemoryPoolMode mpGetMode(MemoryPool pool);
void mpSetMode(MemoryPool pool, MemoryPoolMode mode);

void mpGetCompactParams(MemoryPool pool, int *frequency, float *limit);
void mpSetCompactParams(MemoryPool pool, int frequency, float limit); // set to 0 to disable automatic compaction


typedef void (*MemoryPoolForEachFunc)(MemoryPool pool, void *userData);

/************************************************************************
 * Normal Memory Pool
 */

// constructor/destructors
#define createMemoryPool() createMemoryPoolNamed(__FUNCTION__, __FILE__, __LINE__)
MemoryPool createMemoryPoolNamed(const char *name, const char *fileName, int fileLine);

void initMemoryPoolOffset(MemoryPool pool, int structSize, int structCount, int offset);
#define initMemoryPool(pool, structSize, structCount) initMemoryPoolOffset(pool, structSize, structCount, 0)

#define initMemoryPoolLazy(pool,structSize,structCount) initMemoryPoolLazy_dbg(pool,structSize,structCount, __FILE__, __LINE__);
MemoryPool initMemoryPoolLazy_dbg(MemoryPool pool, int structSize, int structCount, const char* callerName, int line);

void destroyMemoryPool(MemoryPool pool);
void destroyMemoryPoolGfxNode(MemoryPool pool);

// Allocate a piece of memory.
#define mpAlloc(pool) mpAlloc_dbg(pool, __FILE__, __LINE__)
void* mpAlloc_dbg(MemoryPool pool, const char* callerName, int line);

// Retains all allocated memory.
#define mpFree(pool, mem) mpFree_dbg(pool, mem, __FILE__, __LINE__)
void mpFree_dbg(MemoryPool pool, void *memory, const char* callerName, int line);

// Frees all allocated memory.
void mpFreeAll(MemoryPool pool);
void mpFreeAllocatedMemory(MemoryPool pool);

int mpVerifyFreelist(MemoryPool pool);
int mpFindMemory(MemoryPool pool, void* memory);

typedef void (*MemoryPoolForEachAllocationFunc)(MemoryPool pool, void *data, void *userData);
void mpForEachAllocation(MemoryPool pool, MemoryPoolForEachAllocationFunc func, void *userData);

// Get the structure size of this memory pool.
size_t mpStructSize(MemoryPool pool);

// Get the allocated struct count.
size_t mpGetAllocatedCount(MemoryPool pool);

// Get the amount of memory allocated for chunks.
size_t mpGetAllocatedChunkMemory(MemoryPool pool);

// Check if a piece of memory has been returned into a memory pool.
int mpReclaimed(void* memory);

const char* mpGetName(MemoryPool pool);
const char* mpGetFileName(MemoryPool pool);
int	mpGetFileLine(MemoryPool pool);

void mpForEachMemoryPool(MemoryPoolForEachFunc callbackFunc, void *userData);

// Compact all pools in need of compaction since last call to this function
// If never called, pools get compacted on free, so call this once per frame
// to enable delayed compaction (faster)
void mpCompactPools(void);

void testMemoryPool(void);

void mpEnableSuperFreeDebugging(void);

int mpVerifyAllFreelists(void);

C_DECLARATIONS_END

#endif
