/*****************************************************************************
    created:    2001/04/24
    copyright:  2001, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese
    
    purpose:    Includes the STL <vector> class
*****************************************************************************/

#ifndef   INCLUDED_corStlVector
#define   INCLUDED_corStlVector
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStdPre_P.h"
#include <vector>
#include "arda2/core/corStdPost_P.h"

namespace std
{
    // Helper functions to cast native pointer-distance function return values to 32 bits
    template <typename T> 
    uint32 size32(const std::vector<T>& theVec)   { return static_cast<uint32>(theVec.size()); }                                 

    template <typename T, typename A> 
    uint32 size32(const std::vector<T,A>& theVec) { return static_cast<uint32>(theVec.size()); }                                 
}

#ifdef CORE_SYSTEM_WINAPI
// NOT HERE for unix, thats for sure.

// this function does a swap - resize remove
//  take care to not leak memory, delete all pointers in 
//  plain old data structures before calling this function
//  
// non trivial copy constructors might be slow...but should be faster
// than using remove() always
// 
// Note: this does *not* preserve ordering!
template <typename T> void SwapRemove(  std::vector<T>& theVec, 
                                        typename std::vector<T>::iterator& theElement)
{
    (*theElement) = theVec.back();
    theVec.resize( theVec.size() - 1 );
}

template <typename T> void SwapRemove(  std::vector<T>& theVec, 
                                        uint i)
{
    errAssert(theVec.size() > i);

    std::vector<T>::iterator it = theVec.begin();
    advance(it, i);
    SwapRemove(theVec, it);
}

#endif

#endif // INCLUDED_corStlVector
