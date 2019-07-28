/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	Determines operating system, compiler, CPU architecture, and host
				byte order.
*****************************************************************************/

#ifndef   INCLUDED_corPlatform
#define   INCLUDED_corPlatform
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//	This file creates a set of #defines which describe the current operating system,
//	compiler, CPU type, and host byte order. After pre-processing, one of each of the
//	following sets of #defines will have a value of non-zero, and each of the others
//	will be defined as zero. By defining these as non-zero or zero instead of defined
//	or undefined, this prevents accidental misuse from types on definitions.
//
//	Operation system -- one of:
//
//		CORE_SYSTEM_WIN32
//		CORE_SYSTEM_XENON
//		CORE_SYSTEM_WINAPI = CORE_SYSTEM_WIN32 || CORE_SYSTEM_XENON
//		CORE_SYSTEM_LINUX
//      CORE_SYSTEM_PS3
//		CORE_SYSTEM_SOLARIS
//		CORE_SYSTEM_MAC
//
//	Compiler -- one of:
//		CORE_COMPILER_GNU
//		CORE_COMPILER_MSVC
//
//	Architecture -- one of:
//		CORE_ARCH_X86
//      CORE_ARCH_X64
//      CORE_ARCH_POWERPC
//      CORE_ARCH_CELL_PPU
//      CORE_ARCH_CELL_SPU
//		CORE_ARCH_SPARC
//
//	Byte order -- one of:
//		CORE_BYTEORDER_LITTLE
//		CORE_BYTEORDER_BIG


//
// Operating System
//

#define CORE_SYSTEM_WIN32 0
#define CORE_SYSTEM_WIN64 0
#define CORE_SYSTEM_XENON  0
#define CORE_SYSTEM_LINUX   0
#define CORE_SYSTEM_PS3    0
#define CORE_SYSTEM_SOLARIS 0
#define CORE_SYSTEM_MAC     0
#if (_XBOX_VER>=200)
# undef  CORE_SYSTEM_XENON 
# define CORE_SYSTEM_XENON 1

#elif defined(__CELLOS_LV2__) 
# undef  CORE_SYSTEM_PS3 
# define CORE_SYSTEM_PS3 1

#elif defined(WIN64) || defined (_WIN64)
# ifndef WIN64
#  define WIN64
# endif
# undef  CORE_SYSTEM_WIN64
# define CORE_SYSTEM_WIN64 1

#elif defined(WIN32) || defined(_WIN32)
# ifndef WIN32
#  define WIN32
# endif
# undef  CORE_SYSTEM_WIN32
# define CORE_SYSTEM_WIN32 1

#elif defined(linux)
# undef  CORE_SYSTEM_LINUX
# define CORE_SYSTEM_LINUX 1
# define UNIX
# define UNIX_LINUX

#elif defined(sun)
# undef  CORE_SYSTEM_SOLARIS 1
# define CORE_SYSTEM_SOLARIS 1
# define UNIX
# define UNIX_SOLARIS
#else
# error "Unable to determine system type."
#endif // system detection code

#if (CORE_SYSTEM_WIN32 || CORE_SYSTEM_XENON || CORE_SYSTEM_WIN64)
    #define CORE_SYSTEM_WINAPI 1
#endif

//
// Compiler
//

#define CORE_COMPILER_GNU 0
#define CORE_COMPILER_MSVC 0
#if defined(__GNUC__)
# undef  CORE_COMPILER_GNU
# define CORE_COMPILER_GNU 1
#elif defined(_MSC_VER)
# undef  CORE_COMPILER_MSVC
# define CORE_COMPILER_MSVC 1
    #if (_MSC_VER>=1400)
    # undef  CORE_COMPILER_MSVC8
    # define CORE_COMPILER_MSVC8 1
    #elif (_MSC_VER>=1300)
    # undef  CORE_COMPILER_MSVC7
    # define CORE_COMPILER_MSVC7 1
    #endif
#else
# error "Unable to determine compiler."
#endif // compiler detection code

//
// Architecture
//

// detect
#define CORE_ARCH_X86 0
#define CORE_ARCH_X64 0
#define CORE_ARCH_SPARC 0
#define CORE_ARCH_POWERPC 0
#define CORE_ARCH_CELL_PPU 0
#define CORE_ARCH_CELL_SPU 0

#if defined(i386) || defined(_M_IX86)
# undef  CORE_ARCH_X86
# define CORE_ARCH_X86 1
#elif defined(sparc)
# undef  CORE_ARCH_SPARC
# define CORE_ARCH_SPARC 1
#elif (_XBOX_VER>=200)
# undef  CORE_ARCH_POWERPC
# define CORE_ARCH_POWERPC 1
#elif (__PPU__)
# undef  CORE_ARCH_CELL_PPU
# define CORE_ARCH_CELL_PPU 1
#elif (__SPU__)
# undef  CORE_ARCH_CELL_SPU
# define CORE_ARCH_CELL_SPU 1
#elif defined(WIN64)
# undef  CORE_ARCH_X64
# define CORE_ARCH_X64 1
#else
# error "Unable to determine architecture."
#endif // architecture detection code

//
// Byte Order (host)
//

#define CORE_BYTEORDER_LITTLE   0
#define CORE_BYTEORDER_BIG      0
#define CORE_BYTEORDER_UNKNOWN  0

#if CORE_ARCH_X86
# undef  CORE_BYTEORDER_LITTLE
# define CORE_BYTEORDER_LITTLE 1
#elif CORE_ARCH_SPARC
# undef  CORE_BYTEORDER_BIG
# define CORE_BYTEORDER_BIG 1
#elif CORE_ARCH_POWERPC
# undef  CORE_BYTEORDER_BIG
# define CORE_BYTEORDER_BIG 1
#elif CORE_ARCH_CELL_PPU
# undef  CORE_BYTEORDER_BIG
# define CORE_BYTEORDER_BIG 1
#elif CORE_ARCH_CELL_SPU
# undef  CORE_BYTEORDER_BIG
# define CORE_BYTEORDER_BIG 1
#elif CORE_ARCH_X64
# undef  CORE_BYTEORDER_LITTLE
# define CORE_BYTEORDER_LITTLE 1
#else
# error "Unable to determine host byteorder."
#endif

#if (CORE_COMPILER_MSVC && !defined(_DEBUG)) || (CORE_COMPILER_GNU && defined(NDEBUG))
#define CORE_DEBUG 0
#else
#define CORE_DEBUG 1
#endif

#define ISDEBUG() (CORE_DEBUG != 0)

// get other compatibility macros
#include "arda2/core/corPlatformMacros.h"

#endif // INCLUDED_corPlatform

