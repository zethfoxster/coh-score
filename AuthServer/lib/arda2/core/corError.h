/**
    copyright:  (c) 2003, NCsoft. All Rights Reserved
    author(s):  Peter Freese
    purpose:    Error reporting macros
    modified:   $DateTime: 2006/06/05 17:48:40 $
                $Author: cthurow $
                $Change: 254334 $
                @file
**/

#ifndef   INCLUDED_corError_h
#define   INCLUDED_corError_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "arda2/core/corErrorTypes.h"
#include "arda2/core/corDbgBreak.h"
#include "arda2/core/corArgFormatter.h"

#if CORE_COMPILER_MSVC
// conditional expression is constant
// (do {...} while(0) triggers this; unfortunate, because that is a common macro idiom)
#pragma warning( disable : 4127 )
// unreachable code
#pragma warning( disable : 4702 )
#endif

//// report an error at a given location, and return it back
errHandlerResult errReport( const char* szFileName, int iLineNumber, 
                           errSeverity severity, const char* szDescription );

// commonly used messages (here to prevent littering the code with duplicate strings)
extern const char errUnimplementedMessage[];
extern const char errUntestedMessage[];
extern const char errOutOfMemoryMessage[];

////////////
// Evaluate error; return true if it is not a fatal error.  (That is,
// if it is ERR_OK or a warning.)
//
// if (!ISOK(myError))
// {
//   DieFlamingDeath();
// }
// DoMoreStuff();
//
inline bool ISOK(const errResult er)
{
  return (er >= ER_Success);
}

// The following macros can be divided into two general categories:
// "procedural", meaning they perform some action but return nothing;
// and "exit" macros, which do something and may also return from the
// calling C++ function (i.e. serve as a replacement for the 'return'
// keyword).  Be careful how you use them.
//
// Note: All of these macros evaluate their arguments exactly once,
// like any well-written macro should.

////
//// Procedural Macros - ones that perform an action
////

////////////
// "void ERR_REPORT(errSeverity err, const char* description)"
// Report error, and continue.
//
// if (marvin.Gun().GetState() == MisFired)
// {
//   ERR_REPORT(ES_Warn, "Where's the kaboom?  There was supposed to be an earth-shattering kaboom.");
//   marvin.Gun().ReLoad(); // this line is reached
// }
//
#define ERR_REPORT( err, desc ) \
    do{ \
        if (EH_Break == errReport(__FILE__, __LINE__, (err), (desc))) \
            errDbgBreak(); \
    } while(0)

////
//// Exit Macros - may return from calling function.
////

////////////
// "ERR_RETURN(errSeverity err, const char* description)"
// Report error and 'return' the error code.
//
// if (ncc1701e.Nacelle(Port).IsOffLine())
// {
//   ERR_RETURN(ERR_Error, "Warp engines not functioning.");
//   // this line is not reached
// }
//
#define ERR_RETURN( err, desc ) \
  do { ERR_REPORT((err), (desc)); return ER_Failure; } while (0)

////////////
// "ERR_TEST(errSeverity err, const char* description)"
// Evaluate error; report and 'return' it if not OK.
//
// errResult err = hal.OpenPodBayDoors();
// ERR_TEST(err, "I'm sorry Dave, but I'm afraid I can't do that.");
// shuttle.EnterPod();
//
#define ERR_TEST( err, desc ) \
  do { if (!ISOK(err)) ERR_RETURN(ES_Error, (desc)); } while (0)

////////////
// "ERR_CHECK(errSeverity err, const char* description)"
// Evaluate error; report if not OK.
//
// errResult err = hal.OpenPodBayDoors();
// ERR_CHECK(err, "I'm sorry Dave, but I'm afraid I can't do that.");
// shuttle.EnterPod();
//
#define ERR_CHECK( err, desc ) \
do { if (!ISOK(err)) ERR_REPORT(ES_Error, (desc)); } while (0)

////////////
// "ERR_UNIMPLEMENTED"
// Report and 'return' a standard "not implemented" error.
//
// const errResult StormTrooperBlaster::FireStraight()
// {
//   ERR_RETURN_UNIMPLEMENTED();
// }
//
#define ERR_UNIMPLEMENTED() \
  ERR_REPORT(ES_Error, errUnimplementedMessage)
#define ERR_RETURN_UNIMPLEMENTED() \
  do { ERR_UNIMPLEMENTED(); return ER_Failure; } while (0)


#define ERR_OUTOFMEMORY() \
  ERR_REPORT(ES_Fatal, errOutOfMemoryMessage)
#define ERR_RETURN_OUTOFMEMORY() \
  do { ERR_OUTOFMEMORY(); return ER_Failure; } while (0)

//
//  Variable argument macro versions
// 

#define ERR_REPORTV(err, arglist) \
  do { corArgFormatter temp; ERR_REPORT(err, temp.Format arglist ); } while (0)
#define ERR_RETURNV(err, arglist) \
  do { corArgFormatter temp; ERR_RETURN(err, temp.Format arglist ); } while (0)
#define ERR_TESTV(err, arglist) \
  do { corArgFormatter temp; ERR_TEST(err, temp.Format arglist ); } while (0)

#ifdef _DEBUG
    #define ERR_DEBUG(err, desc)     ERR_REPORT(err, desc)
    #define ERR_DEBUGV(err, arglist) ERR_REPORTV(err, arglist)
#else
    #define ERR_DEBUG(err, desc)
    #define ERR_DEBUGV(err, arglist)
#endif

#endif // INCLUDED_corError_h

