/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Includes the STL <set> class
*****************************************************************************/

#ifndef   INCLUDED_corStlSet
#define   INCLUDED_corStlSet
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStdPre_P.h"
#include <set>
#include "arda2/core/corStdPost_P.h"

namespace std
{
    // Helper functions to cast native pointer-distance function return values to 32 bits
    template <typename T> 
    uint32 size32(const set<T>& theSet)     { return static_cast<uint32>(theSet.size()); }                                 

    template <typename T, typename A> 
    uint32 size32(const set<T,A>& theSet)   { return static_cast<uint32>(theSet.size()); }                                 

    template <typename T> 
    uint32 size32(const multiset<T>& theSet)     { return static_cast<uint32>(theSet.size()); }                                 

    template <typename T, typename A> 
    uint32 size32(const multiset<T,A>& theSet)   { return static_cast<uint32>(theSet.size()); }                                 
}
#endif // INCLUDED_corStlSet


