/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Define standardized integer and floating point types.
*****************************************************************************/

#ifndef   INCLUDED_corTypes
#define   INCLUDED_corTypes
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// use this when it becomes available
// #include <stdint.h>

// TODO[pmf]: Add _MIN and _MAX constant defines for each type

// semaphore to ensure types get defined somewhere below
#undef CORE_TYPES_DEFINED

// Solaris, GNU compiler
#if CORE_SYSTEM_SOLARIS && CORE_COMPILER_GNU
#define CORE_TYPES_DEFINED
// define size_t, and others
#include <stddef.h>
// basic integral types ([u]int{8,16,32,64}_t)
#include <inttypes.h>
#include <stdint.h>

typedef int8_t				int_least8_t;
typedef uint8_t 			uint_least8_t;
typedef int16_t 			int_least16_t;
typedef uint16_t			uint_least16_t;
typedef int32_t 			int_least32_t;
typedef uint32_t			uint_least32_t;
typedef int64_t 			int_least64_t;
typedef uint64_t			uint_least64_t;

// FIXME[pmf]: just guessing at these
typedef int8_t				int_fast8_t;
typedef uint8_t 			uint_fast8_t;
typedef int32_t 			int_fast16_t;
typedef uint32_t			uint_fast16_t;
typedef int32_t 			int_fast32_t;
typedef uint32_t			uint_fast32_t;
typedef int64_t 			int_fast64_t;
typedef uint64_t			uint_fast64_t;

// basic floating point types
typedef float				float32_t;
typedef double				float64_t;
// macros to make 64-bit numbers (only work for constants)
#define MAKEINT64(a) a ## LL
#define MAKEUINT64(a) a ## uLL
// printf format for 64-bit ints
#define PRINTINT64 "%lld"
#define PRINTUINT64 "%llu"

#endif

// Linux, GNU compiler
#if CORE_SYSTEM_LINUX && CORE_COMPILER_GNU
#define CORE_TYPES_DEFINED
// define size_t, and others
#include <stddef.h>
#include <stdint.h>
// basic integral types
typedef signed char 		int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;

typedef int8_t				int_least8_t;
typedef uint8_t 			uint_least8_t;
typedef int16_t 			int_least16_t;
typedef uint16_t			uint_least16_t;
typedef int32_t 			int_least32_t;
typedef uint32_t			uint_least32_t;
typedef int64_t 			int_least64_t;
typedef uint64_t			uint_least64_t;

// FIXME[pmf]: just guessing at these
typedef int8_t				int_fast8_t;
typedef uint8_t 			uint_fast8_t;
typedef int32_t 			int_fast16_t;
typedef uint32_t			uint_fast16_t;
typedef int32_t 			int_fast32_t;
typedef uint32_t			uint_fast32_t;
typedef int64_t 			int_fast64_t;
typedef uint64_t			uint_fast64_t;

// basic floating point types
typedef float		float32_t;
typedef double		float64_t;

// macros to make 64-bit numbers (only work for constants)
#define MAKEINT64(a) a ## LL
#define MAKEUINT64(a) a ## uLL
// printf format for 64-bit ints
#define PRINTINT64 "%lld"
#define PRINTUINT64 "%llu"
#endif

// Win32, Microsoft compiler
#if CORE_SYSTEM_WIN32 && CORE_COMPILER_MSVC
#define CORE_TYPES_DEFINED
// define size_t, and others
#pragma warning( disable:4514 ) // unreferenced inline function
#include <stddef.h>
// basic integral types
typedef signed char 		int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed __int64		int64_t;
typedef unsigned __int64	uint64_t;

typedef int8_t				int_least8_t;
typedef uint8_t 			uint_least8_t;
typedef int16_t 			int_least16_t;
typedef uint16_t			uint_least16_t;
typedef int32_t 			int_least32_t;
typedef uint32_t			uint_least32_t;
typedef int64_t 			int_least64_t;
typedef uint64_t			uint_least64_t;

typedef int8_t				int_fast8_t;
typedef uint8_t 			uint_fast8_t;
typedef int32_t 			int_fast16_t;
typedef uint32_t			uint_fast16_t;
typedef int32_t 			int_fast32_t;
typedef uint32_t			uint_fast32_t;
typedef int64_t 			int_fast64_t;
typedef uint64_t			uint_fast64_t;

// basic floating point types
typedef float				float32_t;
typedef double				float64_t;
// macros to make 64-bit numbers (only work for constants)
#define MAKEINT64(a) a ## I64
#define MAKEUINT64(a) a ## uI64
// printf format for 64-bit ints
#define PRINTINT64 "%I64d"
#define PRINTUINT64 "%I64u"
#endif



// Win32, Microsoft compiler
#if CORE_SYSTEM_WIN64 && CORE_COMPILER_MSVC
#define CORE_TYPES_DEFINED
// define size_t, and others
#pragma warning( disable:4514 ) // unreferenced inline function
//#include <stddef.h>
// basic integral types
typedef signed char 		int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed __int64		int64_t;
typedef unsigned __int64	uint64_t;

typedef int8_t				int_least8_t;
typedef uint8_t 			uint_least8_t;
typedef int16_t 			int_least16_t;
typedef uint16_t			uint_least16_t;
typedef int32_t 			int_least32_t;
typedef uint32_t			uint_least32_t;
typedef int64_t 			int_least64_t;
typedef uint64_t			uint_least64_t;

typedef int8_t				int_fast8_t;
typedef uint8_t 			uint_fast8_t;
typedef int32_t 			int_fast16_t;
typedef uint32_t			uint_fast16_t;
typedef int32_t 			int_fast32_t;
typedef uint32_t			uint_fast32_t;
typedef int64_t 			int_fast64_t;
typedef uint64_t			uint_fast64_t;

// basic floating point types
typedef float				float32_t;
typedef double				float64_t;
// macros to make 64-bit numbers (only work for constants)
#define MAKEINT64(a) a ## I64
#define MAKEUINT64(a) a ## uI64
// printf format for 64-bit ints
#define PRINTINT64 "%I64d"
#define PRINTUINT64 "%I64u"
#endif


// PPC64 - Xenon, Microsoft compiler
#if CORE_SYSTEM_XENON && CORE_COMPILER_MSVC
#define CORE_TYPES_DEFINED
// define size_t, and others
#pragma warning( disable:4514 ) // unreferenced inline function
//#include <stddef.h>
// basic integral types
typedef signed char 		int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed __int64		int64_t;
typedef unsigned __int64	uint64_t;

typedef int8_t				int_least8_t;
typedef uint8_t 			uint_least8_t;
typedef int16_t 			int_least16_t;
typedef uint16_t			uint_least16_t;
typedef int32_t 			int_least32_t;
typedef uint32_t			uint_least32_t;
typedef int64_t 			int_least64_t;
typedef uint64_t			uint_least64_t;

typedef int8_t				int_fast8_t;
typedef uint8_t 			uint_fast8_t;
typedef int32_t 			int_fast16_t;
typedef uint32_t			uint_fast16_t;
typedef int32_t 			int_fast32_t;
typedef uint32_t			uint_fast32_t;
typedef int64_t 			int_fast64_t;
typedef uint64_t			uint_fast64_t;

// basic floating point types
typedef float				float32_t;
typedef double				float64_t;
// macros to make 64-bit numbers (only work for constants)
#define MAKEINT64(a) a ## I64
#define MAKEUINT64(a) a ## uI64
// printf format for 64-bit ints
#define PRINTINT64 "%I64d"
#define PRINTUINT64 "%I64u"
#endif

// Win32, GNU compiler
#if CORE_SYSTEM_WIN32 && CORE_COMPILER_GNU
#define CORE_TYPES_DEFINED
// define size_t, and others
#include <stddef.h>
// basic integral types
typedef char         		int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;

typedef int8_t				int_least8_t;
typedef uint8_t 			uint_least8_t;
typedef int16_t 			int_least16_t;
typedef uint16_t			uint_least16_t;
typedef int32_t 			int_least32_t;
typedef uint32_t			uint_least32_t;
typedef int64_t 			int_least64_t;
typedef uint64_t			uint_least64_t;

typedef int8_t				int_fast8_t;
typedef uint8_t 			uint_fast8_t;
typedef int32_t 			int_fast16_t;
typedef uint32_t			uint_fast16_t;
typedef int32_t 			int_fast32_t;
typedef uint32_t			uint_fast32_t;
typedef int64_t 			int_fast64_t;
typedef uint64_t			uint_fast64_t;

// basic floating point types
typedef float				float32_t;
typedef double				float64_t;
// macros to make 64-bit numbers (only work for constants)
#define MAKEINT64(a) a ## ll
#define MAKEUINT64(a) a ## ull
// printf format for 64-bit ints
#define PRINTINT64 "%lld"
#define PRINTUINT64 "%ulld"
#endif

// PPU-SPU PS3, GNU compiler
#if CORE_SYSTEM_PS3 && CORE_COMPILER_GNU
#define CORE_TYPES_DEFINED
// define size_t, and others
#include <stddef.h>
// basic integral types
typedef signed char      	int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed long	int64_t;
typedef unsigned long	uint64_t;

typedef int8_t				int_least8_t;
typedef uint8_t 			uint_least8_t;
typedef int16_t 			int_least16_t;
typedef uint16_t			uint_least16_t;
typedef int32_t 			int_least32_t;
typedef uint32_t			uint_least32_t;
typedef int64_t 			int_least64_t;
typedef uint64_t			uint_least64_t;

typedef int8_t				int_fast8_t;
typedef uint8_t 			uint_fast8_t;
typedef int32_t 			int_fast16_t;
typedef uint32_t			uint_fast16_t;
typedef int32_t 			int_fast32_t;
typedef uint32_t			uint_fast32_t;
typedef int64_t 			int_fast64_t;
typedef uint64_t			uint_fast64_t;

// basic floating point types
typedef float				float32_t;
typedef double				float64_t;
// macros to make 64-bit numbers (only work for constants)
#define MAKEINT64(a) a ## ll
#define MAKEUINT64(a) a ## ull
// printf format for 64-bit ints
#define PRINTINT64 "%lld"
#define PRINTUINT64 "%ulld"

// Windows types
typedef unsigned char BYTE;

#endif

// check that one of the above was true
#if !defined(CORE_TYPES_DEFINED)
# error "unknown system/compiler, fix!"
#endif
#undef CORE_TYPES_DEFINED

// simplified type aliases - NO PLATFORM SPECIFIC definitions below
typedef char	 char_t;
typedef uint8_t  byte_t;
typedef uint16_t unichar_t; // Unicode character
typedef uint32_t timeunit_t;

typedef int8_t			int8;
typedef uint8_t			uint8;
typedef int16_t			int16;
typedef uint16_t		uint16;
typedef int32_t			int32;
typedef uint32_t		uint32;
typedef int64_t			int64;
typedef uint64_t		uint64;

typedef int_least8_t	int_least8;
typedef uint_least8_t	uint_least8;
typedef int_least16_t	int_least16;
typedef uint_least16_t	uint_least16;
typedef int_least32_t	int_least32;
typedef uint_least32_t	uint_least32;
typedef int_least64_t	int_least64;
typedef uint_least64_t	uint_least64;

typedef int_fast8_t		int_fast8;
typedef uint_fast8_t	uint_fast8;
typedef int_fast16_t	int_fast16;
typedef uint_fast16_t	uint_fast16;
typedef int_fast32_t	int_fast32;
typedef uint_fast32_t	uint_fast32;
typedef int_fast64_t	int_fast64;
typedef uint_fast64_t	uint_fast64;

typedef float32_t	float32;
typedef float64_t	float64;

// other good shorter aliases
typedef signed		   char  schar;
typedef unsigned	   char  uchar;
typedef unsigned	   int	 uint;
typedef unsigned short int	 ushort;
typedef unsigned long  int	 ulong;
typedef unsigned	   char  byte;
typedef unsigned short       word;
typedef unsigned long        dword;
#if			CORE_COMPILER_MSVC
typedef unsigned __int64     qword;
#elif		CORE_COMPILER_GNU
typedef unsigned long long   qword;
#endif
typedef wchar_t				 wchar;

#endif // INCLUDED_corTypes
