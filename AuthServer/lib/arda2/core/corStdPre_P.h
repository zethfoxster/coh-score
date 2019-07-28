/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Things to do before including a C++ library/STL header
*****************************************************************************/

// This file is included before any STL file, so it does not and should not
// contain include guards

//
// Microsoft VC++
//
#if CORE_COMPILER_MSVC

#pragma warning( push )	// post file has corresponding pop

// unreachable code
#pragma warning( disable : 4702 )

#endif // CORE_COMPILER_MSVC

//
// GNU C++
//
#if CORE_COMPILER_GNU

// turn off SGI STL's 'concept checking' code - while it's a good
// idea, gcc barfs on some of them

#if !defined(_STL_NO_CONCEPT_CHECKS)
# define _STL_NO_CONCEPT_CHECKS
#endif

#endif // CORE_COMPILER_GNU
