/*****************************************************************************
	created:    2002-09-23
	copyright:	2002, NCsoft. All Rights Reserved
	author(s):	Jason Beardsley
	
	purpose:	Hash functions for hashed containers.
*****************************************************************************/

#ifndef INCLUDED_corStlHashFunc
#define INCLUDED_corStlHashFunc

#include "arda2/core/corStdString.h"

#include <cstring>

/**
 * The default: cast it to size_t, see what happens.  This works for
 * most numeric types.
 **/

template <typename T>
struct corHashFunc
{
  inline size_t operator()(const T& x) const
  {
    return static_cast<size_t>(x);
  }
};


/**
 * Specialize for 64-bit int types.
 **/

template <>
struct corHashFunc<uint64>
{
  inline size_t operator()(const uint64 x) const
  {
    // low32 XOR hi32
    uint32 l = static_cast<uint32>(x & 0xFFFFFFFF);
    uint32 h = static_cast<uint32>((x >> 32) & 0xFFFFFFFF);
    return l ^ h;
  }
};

template <>
struct corHashFunc<int64>
{
  inline size_t operator()(const int64 x) const
  {
    // low32 XOR hi32
    uint32 l = static_cast<uint32>(x & 0xFFFFFFFF);
    uint32 h = static_cast<uint32>((x >> 32) & 0xFFFFFFFF);
    return l ^ h;
  }
};


/**
 * Strings
 **/

namespace corStringHashFunc
{
  // C strings
  size_t Hash(const char* str);

  // C++ strings
  size_t Hash(const std::string& str);
}


/**
 * Wide character strings
 *
 * Note: these aren't portable, because sizeof(wchar_t) is not
 * well-defined.  But the code itself is portable.
 **/

namespace corWStringHashFunc
{
  // C strings
  size_t Hash(const wchar_t* str);

  // C++ strings
  size_t Hash(const std::wstring& str);
}

// By default, case matters.  Because that's usually true.

template <>
struct corHashFunc<const char*>
{
  inline size_t operator()(const char* str) const
  {
    return corStringHashFunc::Hash(str);
  }
};

template <>
struct corHashFunc<std::string>
{
    inline size_t operator()(const std::string& str) const
  {
    return corStringHashFunc::Hash(str);
  }
};


/**
 * Generic pointers.  Note that we don't define a corHashFunc<>
 * specialization, if you're doing to use pointers as hash keys, use
 * the corHashTraitsPointer<> class.
 **/

namespace corPointerHashFunc
{
  inline size_t Hash(const void* ptr)
  {
    return (size_t) ptr;
  }
}

#endif // INCLUDED_corStlHashFunc
