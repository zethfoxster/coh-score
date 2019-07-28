/*****************************************************************************
	created:    2002-09-23
	copyright:	2002, NCsoft. All Rights Reserved
	author(s):	Jason Beardsley
	
	purpose:	Hash traits for hashed containers.
*****************************************************************************/

#ifndef INCLUDED_corStlHashTraits
#define INCLUDED_corStlHashTraits

#include <functional>

#include "arda2/core/corStlHashFunc.h"

//
// corHashTraitsBase class.  Every HashTraits class needs to derive
// from this, in order to be usable.
//

template < typename T >
struct corHashTraitsBase
{
  // this typedef is used by corHashTraitsAux to get at 'T'
  typedef T hash_type;
};

//
// corHashTraits class.  Defines the hash function, equality, and
// ordering functions for a given type T.
//
// The default implementation uses corHashFunc<T>, operator==, and
// operator< respectively.
//

template < typename T,
           class HashT = corHashFunc<T>,  // hash functor
           class EqualT = std::equal_to<T>,  // equality functor
           class LessT = std::less<T> >  // ordering functor
struct corHashTraits : public corHashTraitsBase<T>
{
  static inline size_t Hash(const T& x)
  {
    return HashT()(x);
  }

  static inline bool Less(const T& lhs, const T& rhs)
  {
    return LessT()(lhs, rhs);
  }

  static inline bool Equal(const T& lhs, const T& rhs)
  {
    return EqualT()(lhs, rhs);
  }
};

//
// corHashTraitsAux class.  These are compiler-specific adaptors,
// because VC.NET and GCC have different ideas on what operator()
// needs to return.  And there was much rejoicing.  Not.  Fortunately
// you don't need to know these exist.
//

#if CORE_COMPILER_MSVC || CORE_SYSTEM_PS3

template <class Traits>
struct corHashTraitsAux
{
  // hash_map parameters
  enum
  {
    bucket_size = 4,  // 0 < bucket_size
    min_buckets = 8,  // min_buckets = 2 ^^ N, 0 < N
  };

  // hash function
  inline size_t operator()(const typename Traits::hash_type& x) const
  {
    return Traits::Hash(x);
  }

  // ordering function (equivalence)
  inline bool operator()(const typename Traits::hash_type& lhs,
                         const typename Traits::hash_type& rhs) const
  {
    return Traits::Less(lhs, rhs);
  }
};

#elif CORE_COMPILER_GNU

template <class Traits>
struct corHashTraitsAux
{
  // hash function
  inline size_t operator()(const typename Traits::hash_type& x) const
  {
    return Traits::Hash(x);
  }

  // ordering function (equality)
  inline bool operator()(const typename Traits::hash_type& lhs,
                         const typename Traits::hash_type& rhs) const
  {
    return Traits::Equal(lhs, rhs);
  }
};

#else
# error "Unknown compiler."
#endif

//
// Some specialized hash traits classes.
//


// Case-insensitive (w)strings.  To use them, add
// "corHashTraits(W)StringNoCase" to your HashSet/HashMap template, e.g.
//
//   HashSet<std::string, corHashTraitsStringNoCase>
//   HashMap<std::wstring, Foo, corHashTraitsWStringNoCase>

struct corHashTraitsString : public corHashTraitsBase<std::string>
{
  static inline size_t Hash(const std::string& x)
  {
    return corStringHashFunc::Hash(x);
  }

  static inline bool Less(const std::string& lhs, const std::string& rhs)
  {
    return strcmp(lhs.c_str(), rhs.c_str()) < 0;  
  }

  static inline bool Equal(const std::string& lhs, const std::string& rhs)
  {
    return strcmp(lhs.c_str(), rhs.c_str()) == 0;  
  }
};

struct corHashTraitsWString : public corHashTraitsBase<std::wstring>
{
  static inline size_t Hash(const std::wstring& x)
  {
    return corWStringHashFunc::Hash(x);
  }

  static inline bool Less(const std::wstring& lhs, const std::wstring& rhs)
  {
    return wcscmp(lhs.c_str(), rhs.c_str()) < 0;  
  }

  static inline bool Equal(const std::wstring& lhs, const std::wstring& rhs)
  {
    return wcscmp(lhs.c_str(), rhs.c_str()) == 0;  
  }
};


// Generic (but type-safe) const pointers.  To use:
//
//  HashSet<Foo*, corHashTraitsPointer<Foo> >
//  HashMap<Foo*, Bar, corHashTraitsPointer<Foo> >

template <typename T>
struct corHashTraitsPointer : public corHashTraitsBase<T*>
{
  static inline size_t Hash(const T* x)
  {
    return corPointerHashFunc::Hash(x);
  }

  static inline bool Less(const T* lhs, const T* rhs)
  {
    return lhs < rhs;
  }

  static inline bool Equal(const T* lhs, const T* rhs)
  {
    return lhs == rhs;
  }
};

#endif
