/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "ncstd.h"
#include "ncutils.h"
#include <time.h>
#include <process.h>

unsigned int	ncutils_seed_random = 0;

#ifdef OVERRIDE_MALLOC
// force linking of tcmalloc symbols. 
// this will actually cause a static ctor to be called: tcmalloc.cc:TCMallocGuard module_enter_exit_hook;
// so calling it is redundant, but if you don't do this the functions will get optimized out.
// ab: gotta be a better way to force this to happen...
extern "C" void tcmalloc_PatchWindowsFunctions();
typedef void (TcmallocInitFptr)();
TcmallocInitFptr *tcmalloc_init_fptr = tcmalloc_PatchWindowsFunctions;
#endif // OVERRIDE_MALLOC
extern "C" void fileio_init(void);

void ncutils_init(void)
{
#ifdef OVERRIDE_MALLOC
	if(tcmalloc_init_fptr)
    {
        char *malloc_test = (char*)malloc(1024);
        free(malloc_test);
		printf("tcmalloc initted %p\n", tcmalloc_init_fptr);
    }
#endif
    
    
    // init random.
    while (ncutils_seed_random == 0) {
        ncutils_seed_random = (unsigned int)(time(0) ^ _getpid());
    }
    srand(ncutils_seed_random);
}
 
