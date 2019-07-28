/*****************************************************************************
	created:	2001-07-25
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Tom Gambill
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_matTypes
#define   INCLUDED_matTypes
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if CORE_SYSTEM_WINAPI
// intentionally blank
#define D3D_DEBUG_INFO
#elif CORE_SYSTEM_LINUX|CORE_SYSTEM_PS3    
// intentionally blank
#else
# error "Unknown system, or you forgot to include palantir/core/corFirst.h"
#endif

// Base math (and D3DX Emulation) functions
#include "arda2/math/matD3DXMathEmu.h"


// this type is used to distinguish constructors which initialize an object to identity or
// zero, in order to distuingish from constructors which leave data uninitialized. Such 
// constructors should not actually use the value passed, but should merely by overloaded
// by type.
enum matZeroType
{
	matZero = 0
};

enum matIdentityType
{
	matIdentity = 1
};

enum matMinimumType
{
	matMinimum = 2
};

enum matMaximumType
{
	matMaximum = 3
};

#if CORE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4035)		// turn off warning about no return value
#endif

//* Return the shift value for the passed power-of-two dimension.
inline int GetShift( int n )
{
#if CORE_COMPILER_MSVC && CORE_SYSTEM_WIN32
	__asm
	{
		mov		edx, [n]
		bsr		eax, edx
	}
#else
    int edx = n;
	int eax = 31;
	int mask = 0x80000000;
	while ((edx & mask) == 0 && mask) {
		mask >>= 1;
		--eax;
	}
    return eax;
#endif
}


inline int GetZeroShift( int n )
{
#if CORE_COMPILER_MSVC && CORE_SYSTEM_WIN32
	__asm
	{
		mov		edx, [n]
		bsf		eax, edx
	}
#else
    int edx = n;
	int eax = 0;
	int mask = 0x00000001;
	while ((edx & mask) == 0 && mask)
    {
		mask <<= 1;
		++eax;
	}
    return eax;
#endif
}

#if CORE_COMPILER_MSVC
#pragma warning(pop)
#endif


//* Useful templates
template <class T>
inline bool IsPower2( T n )
{
    return n && ((n & (n-1)) == 0);
}


//* Return the next power of two that contains the passed in number.
template <class T>
inline T MakePower2Up( T n )
{
	return T((n == 0) ? 1 : (2 << GetShift(n)));
}


//* Return the previous power of two that contains the passed in number.
//* This is useful for finding the most significant bit.
template <class T>
inline T MakePower2Down( T n )
{
	return T((n == 0) ? 0 : (1 << GetShift(n)));
}


#endif // INCLUDED_matTypes
