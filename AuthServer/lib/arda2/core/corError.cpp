/*****************************************************************************
created:	2001/04/22
copyright:	2001, NCSoft. All Rights Reserved
author(s):	Peter M. Freese

purpose:	
*****************************************************************************/

#include "arda2/core/corFirst.h"
#include "arda2/core/corError.h"
#include "arda2/core/corErrorHandler.h"

#include <signal.h> // for raise()
#if CORE_SYSTEM_XENON
#include <Xbdm.h>
#endif


const char errUnimplementedMessage[] = "Unimplemented code reached";
const char errOutOfMemoryMessage[] = "Out of memory";

errHandlerResult errReport( const char* fileName, int iLineNumber, 
						   errSeverity severity, const char* description )
{
	errHandlerResult r = corErrorHandler::HandleError(fileName, iLineNumber, severity, description);

	switch (r)
    {
        case EH_Abort:
    	{
#if CORE_SYSTEM_XENON

            // First stop at this breakpoint if the debugger is attached.
            errDbgBreak();

            // Then will disconnect debugger and reboot console if continued
            DmReboot(DMBOOT_COLD);

            // Alternate solution...but doesn't work very well...
//             assert(0);
//             abort();
#endif

	    	raise(SIGABRT);
		    /* We usually won't get here, but it's possible that
		    SIGABRT was ignored.  So hose the program anyway. */
#if CORE_SYSTEM_WIN32||CORE_SYSTEM_WIN64
            ExitProcess(3);
#else
            exit(3);
#endif
            break;
	    }

        case EH_Break:
        {
            errDbgBreak();
            break;
        }
		default:
			break;
    }
	return r;
}

