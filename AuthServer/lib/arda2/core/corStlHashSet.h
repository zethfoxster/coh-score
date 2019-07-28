/*****************************************************************************
	created:	2002/05/02
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Includes the (non-standard) <hashset> template class
*****************************************************************************/

#ifndef   INCLUDED_corStlHashSet
#define   INCLUDED_corStlHashSet
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStdPre_P.h"
#if CORE_COMPILER_GNU&!CORE_SYSTEM_PS3
#include <ext/hash_set>
#elif CORE_SYSTEM_XENON 
#include <hash_set>
#else
#include <hash_set>
#endif
#include "arda2/core/corStdPost_P.h"

#include "arda2/core/corStlHashTraits.h"

// The idea here is to define HashSet/HashMultiSet template
// classes that mask incompatibilities between VC.NET and GCC.

#if CORE_COMPILER_MSVC7 || CORE_COMPILER_MSVC8

#define stdhashset stdext

template < typename KeyType,
           class Traits = corHashTraits<KeyType> >
class HashSet : public stdhashset::hash_set<KeyType, corHashTraitsAux<Traits> > { };

template < typename KeyType,
           class Traits = corHashTraits<KeyType> >
class HashMultiSet : public stdhashset::hash_multiset<KeyType, corHashTraitsAux<Traits> > { };

#if defined(_STDEXT)
#endif

#elif CORE_COMPILER_MSVC || CORE_SYSTEM_PS3

#define stdhashset std

template < typename KeyType,
            class Traits = corHashTraits<KeyType> >
class HashSet : public stdhashset::hash_set<KeyType, corHashTraitsAux<Traits> > { };

template < typename KeyType,
            class Traits = corHashTraits<KeyType> >
class HashMultiSet : public stdhashset::hash_multiset<KeyType, corHashTraitsAux<Traits> > { };

#if defined(_STDEXT)
#endif

#elif CORE_COMPILER_GNU

#define stdhashset 

template < typename KeyType,
           class Traits = corHashTraits<KeyType> >
class HashSet : public hash_set<KeyType, corHashTraitsAux<Traits>, corHashTraitsAux<Traits> > { };

template < typename KeyType,
           class Traits = corHashTraits<KeyType> >
class HashMultiSet : public hash_multiset<KeyType, corHashTraitsAux<Traits>, corHashTraitsAux<Traits> > { };

#else
# error "Unknown compiler."
#endif

#if CORE_COMPILER_GNU&!CORE_SYSTEM_PS3

// We also have to specialize insert_iterator, because GCC's hash_map
// doesn't have the supposedly standard 'insert(iter, value)' method.
// What a pain in the ass.

// Note: this is copied verbatim from stl_hash_map.h and modified
// using the 'search and replace' advanced programming technique.
// Don't worry if it looks ugly.  It makes shit work.

namespace std
{

template <class KeyType, class Traits>
class insert_iterator< HashSet<KeyType, Traits> >
{
protected:
  typedef HashSet<KeyType, Traits> _Container;
  _Container* container;
public:
  typedef _Container          container_type;
  typedef output_iterator_tag iterator_category;
  typedef void                value_type;
  typedef void                difference_type;
  typedef void                pointer;
  typedef void                reference;

  insert_iterator(_Container& __x) : container(&__x) {}
  insert_iterator(_Container& __x, typename _Container::iterator)
    : container(&__x) {}
  insert_iterator<_Container>&
  operator=(const typename _Container::value_type& __value) {
    container->insert(__value);
    return *this;
  }
  insert_iterator<_Container>& operator*() { return *this; }
  insert_iterator<_Container>& operator++() { return *this; }
  insert_iterator<_Container>& operator++(int) { return *this; }
};

template <class KeyType, class Traits>
class insert_iterator< HashMultiSet<KeyType, Traits> >
{
protected:
  typedef HashMultiSet<KeyType, Traits> _Container;
  _Container* container;
public:
  typedef _Container          container_type;
  typedef output_iterator_tag iterator_category;
  typedef void                value_type;
  typedef void                difference_type;
  typedef void                pointer;
  typedef void                reference;

  insert_iterator(_Container& __x) : container(&__x) {}
  insert_iterator(_Container& __x, typename _Container::iterator)
    : container(&__x) {}
  insert_iterator<_Container>&
  operator=(const typename _Container::value_type& __value) {
    container->insert(__value);
    return *this;
  }
  insert_iterator<_Container>& operator*() { return *this; }
  insert_iterator<_Container>& operator++() { return *this; }
  insert_iterator<_Container>& operator++(int) { return *this; }
};

} // namespace std

#endif // GCC

#endif // INCLUDED_corStlHashSet
