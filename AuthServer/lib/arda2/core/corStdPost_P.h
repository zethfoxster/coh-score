/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Things to do after including a C++ library/STL header file
*****************************************************************************/

//
// Microsoft VC++
//
#if CORE_COMPILER_MSVC

// Restore VC++ warning level to its anal-retentive glory
#pragma warning(pop)

// namespaces?  we don't need no stinking namespaces!
namespace std { }

#endif // MS VC++

//
// GNU C++
//
#if CORE_COMPILER_GNU

// namespaces?  we don't need no stinking namespaces!
namespace std { }

#if (__GNUC__ == 3) && (__GNUC_MINOR__ >= 2) || (__GNUC__ > 3)
namespace __gnu_cxx { }
using namespace __gnu_cxx;
#endif

#endif // GNU C++
