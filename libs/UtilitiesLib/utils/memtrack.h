// memory tracking logs
#ifndef MEMTRACK_H
#define MEMTRACK_H

// Define this to 1 to build memtrack capability into the target
#define MEMTRACK_BUILD 0

// Define this to 1 to have memtrack begin capturing data as soon
// as it is initialized. Otherwise capture must begin with an
// explicit memtrack_capture_begin()
#define MEMTRACK_AUTO_BEGIN 0


#if MEMTRACK_BUILD
void memtrack_init(void);
bool memtrack_enabled(void);

void memtrack_capture_enable(bool bEnableCapture );

void memtrack_mark(int type, const char* msg);
void memtrack_alloc( void* pAllocatedBlock, size_t size );
void memtrack_realloc( void* pAllocatedBlock, size_t size, void* pPrevBlock );
void memtrack_free( void* pAllocatedBlock );

void memtrack_flush(void);

#if !_CRTDBG_MAP_ALLOC
void *memtrack_do_malloc(size_t size);
void *memtrack_do_calloc(size_t num, size_t size);
void *memtrack_do_realloc(void *ptr, size_t size);
char *memtrack_do_strdup(const char *s);
void  memtrack_do_free(void *ptr);
#undef malloc
#undef calloc
#undef realloc
#undef free
#define malloc(size)		memtrack_do_malloc(size)
#define calloc(num,size)	memtrack_do_calloc(num,size)
#define realloc(ptr,size)	memtrack_do_realloc(ptr,size)
#define _strdup(s)			memtrack_do_strdup(s)
#define free(ptr)			memtrack_do_free(ptr)
#endif // !_CRTDBG_MAP_ALLOC

#else // !MEMTRACK_BUILD
#	define memtrack_init() 
#	define memtrack_enabled() (false)
#	define memtrack_capture_enable(enable) 
#	define memtrack_mark(type, msg)
#	define memtrack_alloc( pAllocatedBlock, size)
#	define memtrack_realloc(pAllocatedBlock, size, pPrevBlock )
#	define memtrack_free(pAllocatedBlock )
#	define memtrack_flush() 
#endif // MEMTRACK_BUILD

#endif

