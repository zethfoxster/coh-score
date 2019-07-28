/*****************************************************************************
	created:	2002/08/19
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Ryan M. Prescott
	
	purpose:	
*****************************************************************************/

#include "arda2/core/corFirst.h"
#include "arda2/memory/memMemory.h"


#if CORE_SYSTEM_PS3
#include <stdlib.h>
#else
#include <malloc.h>
#endif

void* MallocAligned(size_t SizeInBytes, size_t AlignmentInBytes )
{
#if CORE_SYSTEM_WINAPI
	return _aligned_malloc(SizeInBytes, AlignmentInBytes);
#else
        // alignment?  we don't need no stinking alignment!
        (void) AlignmentInBytes; // UNREF
	return malloc(SizeInBytes);
#endif
}

void FreeAligned(void* pMemblock)
{
#if CORE_SYSTEM_WINAPI
	_aligned_free(pMemblock);
#else
	free(pMemblock);
#endif
}
