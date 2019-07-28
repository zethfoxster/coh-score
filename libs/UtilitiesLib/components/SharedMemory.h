#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H
/************************************************************************
 * SharedMemory module
 * 
 * This is designed so that multiple processes that all need access to the
 * same static data can share it.  The procedure is for one process to
 * acquire a lock on the memory and then fill it out, and any subsequent
 * requests from other processes for the shared memory block will return
 * a pointer into the data filled in by the first process.  Writing to
 * this data after it is unlocked and shared between processes should be
 * possible, but has not been thoroughly tested.
 *
 * As a possible way of handling errors, any time an error occurs trying
 * to get at the mutex object/shared memory , we could just have it
 * operate in non-shared memory mode.  But, if it failed to get at it,
 * the other program crashed while creating it, so so will this one!
/************************************************************************/

C_DECLARATIONS_BEGIN

#define SHARED_HANDLE_NAME_LEN 128

typedef struct SharedMemoryHandle SharedMemoryHandle;

typedef enum SharedMemoryMode {
	SMM_DISABLED, // Has been disabled, either because it can't get at the shared memory, or because the user said so
	SMM_UNINITED, // Hasn't been initialized yet
	SMM_ENABLED,  // Enabled and functioning
} SharedMemoryMode;

#ifndef PAGE_NOACCESS
#define PAGE_NOACCESS          0x01
#endif
#ifndef PAGE_READONLY
#define PAGE_READONLY          0x02
#endif
#ifndef PAGE_READWRITE
#define PAGE_READWRITE         0x04
#endif
#ifndef PAGE_WRITECOPY
#define PAGE_WRITECOPY         0x08
#endif


void sharedMemorySetMode(SharedMemoryMode mode);
SharedMemoryMode sharedMemoryGetMode();
void sharedMemoryMutexAbandoned(const char * name);

// Used to set the base address (only really matters on the
//   first program who creates the shared memory, the rest
//   use the addresses specified there)
void sharedMemorySetBaseAddress(void *base_address);

SharedMemoryHandle *sharedMemoryCreate(void);

typedef enum SM_AcquireResult {
	SMAR_DataAcquired,
	SMAR_FirstCaller,
	SMAR_Error,
} SM_AcquireResult;

// sharedMemoryAcquire
// If handle is pointing to NULL, it will allocate a handle for you
// Returns SMAR_DataAcquired if shared memory is already there and filled in (by another process), in which case
//   it fills in the handle,  pointing to the shared memory, if it is currently locked by another
//   process (it is still being filled in), it waits until they are done and then returns.  Note
//   that while one process is writing to it, all other processes will hang waiting to acquire it.
// Returns SMAR_FirstCaller if shared memory has not been inited (this is the first caller), in which case it becomes
//   locked and you must fill in the structure by calling the following in order:
//      sharedMemorySetSize
//		sharedMemoryUnlock
// Returns SMAR_Error on fatal error
SM_AcquireResult sharedMemoryAcquire(SharedMemoryHandle **phandle, const char *name);

// Allocates the required amount of shared memory and returns a pointer to it.
//   This can only be called after sharedMemoryAcquire and only if
//   sharedMemoryAquire returned 1
void *sharedMemorySetSizeEx(SharedMemoryHandle *handle, uintptr_t size, int allowFakeSharing);
void *sharedMemorySetSize(SharedMemoryHandle *handle, uintptr_t size);

// Simply returns the known allocated size of a given handle
//   This can only be called after sharedMemoryAcquire and only if
//   sharedMemoryAquire returned 1
size_t sharedMemoryGetSize(SharedMemoryHandle *handle);

// Commits a block of shared memory, so that another block can be allocated
//   This can only be called after sharedMemoryAcquire and only if
//   sharedMemoryAquire returned 1, and after sharedMemorySetSize has
//   been called
void sharedMemoryCommit(SharedMemoryHandle *handle);

// Unlocks a block of shared memory, so that other processes can now access it
//   This can only be called after sharedMemoryAcquire and only if
//   sharedMemoryAquire returned 1, and after sharedMemorySetSize has
//   been called
void sharedMemoryUnlock(SharedMemoryHandle *handle);

// "Locks" a set of shared memory, note that if this memory has been previously
//   aquired by another process, it can be easily read/written to without
//   being locked, unless the code explicitly locks/unlocks every time it wants
//   to read.  In other words, this function will probably only be used internally
//   by the shared memory manager.
void sharedMemoryLock(SharedMemoryHandle *handle);

// Dangerously unprotects shared memory so that it can be written to while it is unlocked
void sharedMemoryUnprotect(SharedMemoryHandle *handle, unsigned long page_access);

// "Allocates" a chunk of memory off of this handle, returns NULL if the amount allocated
//   would exceed the size of the memory on this handle.  This does no real bookkeeping, just
//   remembers the "top" of the allocated memory for the next call
void *sharedMemoryAlloc(SharedMemoryHandle *handle, size_t size);

// Frees all handles to the shared memory, but leaves it around for the OS to clean up
// or other processes to use
void sharedMemoryDestroy(SharedMemoryHandle *handle);

// Tests to see if a pointer is in a range that is from shared memory
bool isSharedMemory(const void *ptr);

// Takes a pointer, and it it's part of shared memory removes the shared memory block from
//  the shared list so other instances will not be able to access it
void sharedMemoryUnshare(void *ptr);

void printSharedMemoryUsage(void);
void testSharedMemory(void);

void *sharedMemoryGetDataPtr(SharedMemoryHandle *handle);

#define SM_SAFE_FREE(x) { if ((x) && !isSharedMemory(x)) { free(x); } x = NULL; }

C_DECLARATIONS_END

#endif