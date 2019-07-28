/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_corPlatformMacros
#define   INCLUDED_corPlatformMacros

// This file exists because of minor differences between Unix and
// Windows, regarding quasi-standard functions.  For example,
// "snprintf()" is a function commonly found on Unix systems, that
// acts exactly like sprintf(), except it won't overrun its output
// buffer.  Microsoft calls their version "_snprintf()", taking the
// exact arguments.
//
// In order to facilitate portable code, this file defines stuff so
// that in your code, you can use either "snprintf" or "_snprintf", no
// matter what system you are compiling under .  So, it defines
// "_snprintf" under Unix, and "snprintf" under Windows.
//
// Note that this file only defines the macros, it does not include
// the files where the real functions exist.  So, you still need to
// include <stdio.h> to get snprintf(), for example.
//
// The following is a list of the functions/macros defined here:
//
// Unix         Microsoft       Function
// ------------ --------------- --------------------------------------
// snprintf     _snprintf       safe sprintf()
// vsnprintf    _vsnprintf      varargs version of snprintf()
// strcasecmp   _stricmp        case-insensitive strcmp()
// wcscasecmp   _wcsicmp        case-insensitive wchar_t strcmp()
// strncasecmp  _strnicmp       case-insensitive string compare, w/len
// putenv       _putenv         put entry into app environment
// atoi64		_atoi64			convert ascii string to 64-bit integer

//
// MSVC
//
#if CORE_COMPILER_MSVC

// Disable windows security warnings
#pragma warning(disable:4141)
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NONSTDC_NO_DEPRECATE

// functions
#if !defined(snprintf)
#define snprintf     _snprintf
#endif
#if !defined(vsnprintf)
//#define vsnprintf    _vsnprintf
#endif
#if !defined(strcasecmp)
#define strcasecmp   _stricmp
#endif
#if !defined(wcscasecmp)
#define wcscasecmp   _wcsicmp
#endif
#if !defined strncasecmp
#define strncasecmp  _strnicmp
#endif
#if !defined putenv
#define putenv       _putenv
#endif
#if !defined atoi64
#define atoi64       _atoi64
#endif
// constants
#if !defined(PATH_MAX)
#define PATH_MAX     _MAX_PATH
#endif
#endif // MSVC

//
// Gnu C
//
#if CORE_COMPILER_GNU
// functions
#define _snprintf   snprintf
#define _vsnprintf  vsnprintf
#define _vsnwprintf vswprintf
#define _stricmp    strcasecmp
#define _stricmp    strcasecmp
#define _strnicmp   strncasecmp
#define _wcsicmp    wcscasecmp
#define _putenv     putenv
#define atoi64      atoll
// constants
#define _MAX_PATH   260 /* max. length of full pathname */
#define _MAX_DRIVE  3   /* max. length of drive component */
#define _MAX_DIR    256 /* max. length of path component */
#define _MAX_FNAME  256 /* max. length of file name component */
#define _MAX_EXT    256 /* max. length of extension component */
#define INFINITE    0xFFFFFFFF
#endif // GNU


#endif // INCLUDED_corPlatformMacros

