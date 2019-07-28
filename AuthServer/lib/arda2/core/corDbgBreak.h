/*****************************************************************************
	created:	2001/04/22
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_corDbgBreak_h
#define   INCLUDED_corDbgBreak_h

// This file defines a macro, corDbgBreak(), that triggers a debugger
// breakpoint, for supported platforms.  Typically, when a breakpoint
// is hit and the program is not being debugged, the program will trap
// and exit.
//
// Supported platforms:
//
// MSVC: x86
// GNU: x86, sparc
//
// For unsupported platforms, corDbgBreak() is equivalent to abort().
//
// Breakpoints are only enabled in non-release mode.  In release mode
// (NDEBUG defined), the macro turns into a no-op, on all platforms.

// [Note: MSVC used to also have Alpha support, but who uses that?]


// MSVC
#if CORE_SYSTEM_WINAPI
#  define errDbgBreak() DebugBreak()
#endif

// GCC
#if CORE_COMPILER_GNU
# if CORE_ARCH_X86
#  define errDbgBreak() asm("int $3")
# elif CORE_ARCH_SPARC
// this seems to generate a trap, but you can't step past it.
// when I learn sparc assembly I'll try to fix it.
// #  define corDbgBreak() asm("ta 1")
# endif
#endif

// none defined?
#if !defined(errDbgBreak)
# include <stdlib.h>
# define errDbgBreak() abort()
#endif

// release mode - no breaks
#if !ISDEBUG()
# undef errDbgBreak
# define errDbgBreak() ((void)0)
#endif

#endif // INCLUDED_corDbgBreak_h
