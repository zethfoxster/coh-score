
extern "C" {
#include "file.h"
#include "stdtypes.h"
#include "timing.h"
}

void operator delete(void* memory)
{
	free(memory);
}

void operator delete[](void *memory)
{
	free(memory);
}

#if _CRTDBG_MAP_ALLOC
void* ::operator new( size_t cb, int nBlockUse, const char* szFileName, int nLine)
{
#ifdef ENABLE_LEAK_DETECTION
	return GC_debug_malloc_uncollectable(cb, szFileName, nLine);
#else
    return malloc_timed(cb, nBlockUse, szFileName, nLine);
#endif
}

void* ::operator new[](size_t cb, int nBlockUse, const char* szFileName, int nLine)
{
#ifdef ENABLE_LEAK_DETECTION
	return GC_debug_malloc_uncollectable(cb, szFileName, nLine);
#else
    return malloc_timed(cb, nBlockUse, szFileName, nLine);
#endif
}
#endif

void * operator new( size_t cb )
{
#ifdef ENABLE_LEAK_DETECTION
	return GC_MALLOC_UNCOLLECTABLE(cb);
#else
	return malloc(cb);
#endif
}

void * operator new[]( size_t cb )
{
#ifdef ENABLE_LEAK_DETECTION
	return GC_MALLOC_UNCOLLECTABLE(cb);
#else
	return malloc(cb);
#endif
}
