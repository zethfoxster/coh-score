/*****************************************************************************
	created:	2002/04/11
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_utlEnumOperators
#define   INCLUDED_utlEnumOperators
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//
// This macro allows you to use a typesafe bitflag parameter as a strict enum rather than
// simple an int or dword. The operators need to be defined so you can combine and test
// the component flags
#define MAKE_ENUM_OPERATORS(x) \
  inline x operator | (x a, x b) \
{ return static_cast<x> (static_cast<int>(a) | static_cast<int>(b)); } \
  inline x operator & (x a, x b) \
{ return static_cast<x> (static_cast<int>(a) & static_cast<int>(b)); } \
  inline x operator |= (x& a, x b) \
{  return (a = a | b); } \
  inline x operator &= (x& a, x b) \
{  return (a = a & b); } \
  inline x operator ~(x a) \
{ return static_cast<x> (~(static_cast<int>(a))); } 

#endif // INCLUDED_utlEnumOperators

