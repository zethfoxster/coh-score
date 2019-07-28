/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Primary include file
*****************************************************************************/

#ifndef   INCLUDED_corFirst
#define   INCLUDED_corFirst

// ======================================================================


#include "arda2/core/corPlatform.h"
#include "arda2/core/corTypes.h"
#include "arda2/core/corWindows.h"
#include "arda2/core/corError.h"
#include "arda2/core/corAssert.h"
#include "arda2/core/corCast.h"
#include "arda2/perf/prfSizer.h"

// [PMF] Putting this here (temporarily) until I can figure out why the STL
// ones won't 

//----------------------------------------------------------------------------
// Basic Min and Max functions as inline templates
//
template <class T>
inline const T& Min(const T& x, const T& y)
{
	return x < y ? x : y;
}

template <class T>
inline const T& Min(const T& x, const T& y, const T& z)
{
	return Min(x, Min(y, z));
}

template <class T>
inline const T& Max(const T& x, const T& y)
{
	return x > y ? x : y;
}

template <class T>
inline const T& Max(const T& x, const T& y, const T& z)
{
	return Max(x, Max(y, z));
}

#if CORE_COMPILER_GNU
#include <string.h>
//#include <cstring>
inline void ZeroMemory(void* p, size_t s)
{
  memset(p, 0, s);
}
#endif

template <class T> inline void ZeroObject( T &p )
{
	::ZeroMemory(&p, sizeof(p));
}



template <class T> inline void SafeDelete( T * &p )
{
	delete p;
	p = NULL;
}


template <class T> inline void SafeDeleteArray( T * &p )
{
	delete [] p;
	p = NULL;
}


template <class T> inline void SafeRelease( T * &p )
{
	if( p )	
		p->Release();
	p = NULL;
}

template <class T> inline long ComRefCount( T *pUnknown )
{
	if ( pUnknown )
	{
		pUnknown->AddRef();
		return pUnknown->Release();
	}
	return 0;
}


#if !defined(IN)
#define IN
#endif

#if !defined(OUT)
#define OUT
#endif

#if !defined(INOUT)
#define INOUT
#endif


// Macro for deprecating functions
#if CORE_COMPILER_MSVC
#define DEPRECATED __declspec(deprecated)
#else
#define DEPRECATED
#endif

#if !defined(ARRAY_LENGTH)
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))
#endif

// Declare copy constructor and assignment operator for a class; use
// inside a private section to make the class uncopyable.
#ifndef DECLARE_NOCOPY
# define DECLARE_NOCOPY(klass) klass(const klass&); klass& operator=(const klass&);
#endif

// no-op
#ifndef NOP
    #ifdef CORE_COMPILER_MSVC
        #if (1300 <= _MSC_VER) //VC 7.0 defines __noop, and gives warning if you don't use it
            # define NOP __noop
        #endif
    #endif

    //If we still haven't defined NOP by the time we get here, we're not compiling under VC7.0 or above
    //So we use the default NOP    
    #ifndef NOP
        # define NOP ((void*)0)
    #endif
#endif

#endif // INCLUDED_corFirst
