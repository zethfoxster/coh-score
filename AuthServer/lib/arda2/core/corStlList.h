/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Includes the STL <list> class
*****************************************************************************/

#ifndef   INCLUDED_corStlList
#define   INCLUDED_corStlList
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStdPre_P.h"
#include <list>
#include "arda2/core/corStdPost_P.h"

namespace std
{
    // Helper functions to cast native pointer-distance function return values to 32 bits
    template <typename T> 
    uint32 size32(const std::list<T>& theList)     { return static_cast<uint32>(theList.size()); }                                 

    template <typename T, typename A> 
    uint32 size32(const std::list<T,A>& theList)   { return static_cast<uint32>(theList.size()); }                                 
}

#endif // INCLUDED_corStlList

