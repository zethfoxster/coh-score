/*****************************************************************************
	created:	2001/04/23
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_corAssert
#define   INCLUDED_corAssert
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corError.h"

// Assertion macros definitions.
//
// Much like <assert.h>, it is at compile time that the assertion
// macros are either expanded into code or turned into no-ops.  This
// behavior is governed by the existence of the standard NDEBUG
// symbol.  If NDEBUG is not defined, then it is "debug mode".
// Otherwise, i.e. NDEBUG is defined (regardless of its value), then
// it is "release mode".
//
// The system uses the CoError mechanism for handling and reporting
// errors, using the "ER_Assertion" error type.
//
// Unlike <assert.h>, corAssert.h is not meant to be included multiple
// times for varying definitions of NDEBUG.  The reason is pretty
// simple: pre-compiled headers.


////////////
// errAssert(expr) - if the given expression evaluates to false, in
// debug mode, aborts the program with a message indicating where the
// failed check occurred.  In release mode, no-op.

#if ISDEBUG()
# define errAssert(expr) do { if (!(expr)) ERR_REPORT(ES_Assertion, "Expression: "#expr); } while (0)
#else
# define errAssert(expr) ((void) 0)
#endif

////////////
// errAssertV(expr) - if the given expression evaluates to false, in
// debug mode, aborts the program with a message indicating where the
// failed check occurred.  In release mode, no-op.

#if ISDEBUG()
# define errAssertV(expr, arglist) do { if (!(expr)) ERR_REPORTV(ES_Assertion, arglist); } while (0)
#else
# define errAssertV(expr, arglist) ((void) 0)
#endif

////////////
// ASSERT(expr) - equivalent to errAssert, unless already defined.

#if !defined(ASSERT)
#define ASSERT(x) errAssert(x)
#endif

#if !defined(ASSERTV)
#define ASSERTV(x,y) errAssertV(x,y)
#endif

#if !defined(assert)
#define assert(x) errAssert(x)
#endif

// unreferenced parameter/value
#ifndef UNREF
# define UNREF(x) ((void)(x))
#endif

#endif // INCLUDED_corAssert

