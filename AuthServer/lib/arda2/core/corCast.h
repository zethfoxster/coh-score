/*****************************************************************************
	created:	2001/08/06
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	templatized cast operations
*****************************************************************************/

#ifndef   INCLUDED_corCast
#define   INCLUDED_corCast
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corAssert.h"


/*****************************************************************************
* verify_cast<>
*
* This works exactly like static_cast<> except that in debug builds, it 
* does an additional dynamic_cast<> and asserts if it fails. This is useful
* if we want to ensure that code is only doing valid casts, but without the
* performance penalty of dynamic_cast<> in release mode.
*
*****************************************************************************/
template <typename To, typename From>
inline To verify_cast(const From& f)
{
#if ISDEBUG()
	errAssert( dynamic_cast<To>(f) );
#endif

  return static_cast<To>(f);
}


/*****************************************************************************
* implicit_cast<>
*
* This causes the compiler to do an implicit conversion only if it would have
* been legal, otherwise, it's an error. It's safer than a C style cast, but
* makes it clear to the reader what's going on. This does not appear in the
* Standard C++ Library mainly because it was proposed too late, after the
* committee had resolved not to add anything more.
*
* For more information, see CUJ Dec 1998 pg 79
*
*****************************************************************************/
template <typename To, typename From>
inline To implicit_cast(const From& f)
{
  return f;
}


/*****************************************************************************
* force_cast<>
*
* This is a more severe cast than reinterpret case. It allows bitwise
* reinterpretation of the data, which is inherently unsafe. Use of this cast
* is completely non-portable.
*
* Example:
// ex: float f;
//     int i = force_cast <int> (f);
*
*****************************************************************************/
template <typename To, typename From>
inline To force_cast(const From& f)
{
  return *reinterpret_cast<To*>(const_cast<From *>(&f));
}


#endif // INCLUDED_corCast
