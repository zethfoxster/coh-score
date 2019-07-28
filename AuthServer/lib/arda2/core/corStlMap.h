/*****************************************************************************
	created:	2001/04/24
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Includes the STL <map> class
*****************************************************************************/

#ifndef   INCLUDED_corStlMap
#define   INCLUDED_corStlMap
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStdPre_P.h"
#include <map>
#include "arda2/core/corStdPost_P.h"

namespace std
{
    // Helper functions to cast native pointer-distance function return values to 32 bits
    template <typename K, typename T> 
    uint32 size32(const std::map<K,T>& theMap)     { return static_cast<uint32>(theMap.size()); }                                 

    template <typename K, typename T, typename A> 
    uint32 size32(const std::map<K,T,A>& theMap)   { return static_cast<uint32>(theMap.size()); }                                 

    template <typename K, typename T> 
    uint32 size32(const std::multimap<K,T>& theMap)     { return static_cast<uint32>(theMap.size()); }                                 

    template <typename K, typename T, typename A> 
    uint32 size32(const std::multimap<K,T,A>& theMap)   { return static_cast<uint32>(theMap.size()); }                                 
}

#endif // INCLUDED_corStlMap

