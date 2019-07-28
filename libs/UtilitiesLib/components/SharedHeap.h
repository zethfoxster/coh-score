#ifndef SHAREDHEAP_H
#define SHAREDHEAP_H

#include "stdtypes.h"


typedef struct SharedMemoryHandle SharedMemoryHandle;

#define MAX_HEAP_REF_STRING_LENGTH 256
typedef struct SharedHeapHandle
{
	void*		data;
	char		pcName[MAX_HEAP_REF_STRING_LENGTH];
	size_t		uiRefCount;
	size_t		uiSize;
} SharedHeapHandle;


typedef enum SharedHeapAcquireResult
{
	SHAR_DataAcquired,
	SHAR_FirstCaller,
	SHAR_Error,
} SharedHeapAcquireResult;

// Disables the shared heap, for future interaction
void sharedHeapTurnOff( const char* pcReason );

bool sharedHeapEnabled();
bool sharedHeapLockHeld();

void* sharedMalloc( size_t bytes );
void* sharedCalloc( size_t num_elements, size_t bytes );
void sharedFree( void* mem );

void* customSharedMalloc(void * data, size_t size);

// sharedHeapAcquire - modeled after sharedMemoryAcquire
// If handle is pointing to NULL, it will allocate a handle for you
// Returns SHAR_DataAcquired if shared memory is already there and filled in (by another process), in which case
//   it fills in the handle,  pointing to the shared memory, if it is currently locked by another
//   process (it is still being filled in), it waits until they are done and then returns.  Note
//   that while one process is writing to it, all other processes will hang waiting to acquire it.
// Note that this increases the reference count on the shared object
// So it is your responsibility to call sharedHeapFree on that handle
// if it's shared memory we might eventually want to be freed...
// Returns SHAR_FirstCaller if shared memory has not been inited (this is the first caller), in which case it becomes
//   locked and you must fill in the structure by calling the following:
//      sharedHeapAlloc
// Returns SHAR_Error on fatal error
SharedHeapAcquireResult sharedHeapAcquireEx(SharedHeapHandle** ppHandle, const char* pcName, const char* pcFile, U32 uiLineNum);
#define sharedHeapAcquire(a, b) sharedHeapAcquireEx(a, b, __FILE__, __LINE__)



// Given a valid handle (which must have returned SHAR_FirstCaller when you acquired it
// this allocates a chunk of memory on the shared heap, storing the address in pHandle->data
bool sharedHeapAlloc(SharedHeapHandle* pHandle, size_t size);


// Decrements the reference count on the handle, and frees it entirely if it hits zero
void sharedHeapRelease(SharedHeapHandle* pHandle );
void sharedHeapReleaseNeverAllocated(SharedHeapHandle* pHandle );

// generates the shared page name from a number, useful for making sure you have
// mapped all of the heap pages
const char* makeSharedPageName( U32 uiNumpage );

void initSharedHeapMemoryManager(void);

// Call this before doing anything to the shared heap
void sharedHeapMemoryManagerLockEx(const char* pcHandleName, const char* pcFile, U32 uiLine);
#define sharedHeapMemoryManagerLock() sharedHeapMemoryManagerLockEx(NULL, __FILE__, __LINE__)

// Call this after doing anything to the shared heap
void sharedHeapMemoryManagerUnlock(void);

bool sharedHeapLazySyncHandlesOnly();

bool sharedHeapLazySync();

// Returns how many pages we have locally mapped
U32 getSharedHeapMemoryManagerPageAllocs();

// Increments the total number of shared pages mapped
void incSharedHeapMemoryManagerPageAllocs( SharedMemoryHandle* pHandle);

// Returns the size of the allocation of a given handle
size_t getHandleAllocatedSize(SharedHeapHandle* pHandle);

// Attempts to reserve a large block of space in the shared heap, and then free it
// without freeing the pages associated. This insures that there will be enough
// space available, and that it is contiguous.
// If it fails, it trys again recursively with half the reserve size, down to a minimum
// reservation (32MB right now)
size_t reserveSharedHeapSpace( size_t uiReserveSize );

size_t getSharedHeapReservationSize( );
void setSharedHeapReservationSize( size_t uiSize );

// Prints how much shared heap memory is reserved and in use
// along with a complete breakdown of every alloc on the heap.
void printSharedHeapInfo();

// Simply stores a string in the shared heap, and then retreives and prints it
void testSharedHeap( );

void sharedHeapEmergencyMutexRelease();

#endif
