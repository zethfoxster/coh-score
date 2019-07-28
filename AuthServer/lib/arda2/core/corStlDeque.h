/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Includes the STL <deque> class
*****************************************************************************/

#ifndef   INCLUDED_corStlDeque
#define   INCLUDED_corStlDeque
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStdPre_P.h"
#include <deque>
#include "arda2/core/corStdPost_P.h"

namespace std
{
    // Helper functions to cast native pointer-distance function return values to 32 bits
    template <typename T> 
    uint32 size32(const deque<T>& theDeque)   { return static_cast<uint32>(theDeque.size()); }                                 

    template <typename T, typename A> 
    uint32 size32(const deque<T,A>& theDeque) { return static_cast<uint32>(theDeque.size()); }                                 
}

#endif // INCLUDED_corStlDeque

