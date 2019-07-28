/*****************************************************************************
	created:	2001/04/23
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_corCompilerAssert
#define   INCLUDED_corCompilerAssert
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Provide compile-time assertion checking, in the form of a template
// class, and a convenience macro.
//
// The class corCompilerAssert<expr> can be subclassed from, in order to
// provide checking in derived classes for template parameters, or
// whatever else can be stuck into 'expr'.
//
// The macro COMPILE_ASSERT(expr) is probably only usable in .cpp
// files, since it defines a class with a name based on __LINE__.
// Using it in headers could lead to two of these having the same
// name, and that would not compile.
//
// COMPILE_ASSERT_H(klass, expr) is usable in headers, and requires
// you to provide a (unique) class name.


// Forward-declare compile-time assertion class, and define the 'good'
// assertion class.  By not defining corCompilerAssert<false>, we'll get
// a relatively understandable error when an assertion fails.

template <bool b> struct corCompilerAssert; // fwd
template <> struct corCompilerAssert<true> { }; // only valid specialization


// (Ignore these, they are necessary to make a uniquely named class.)

#define CA_MAKEVAR2(x,y) x ## y
#define CA_MAKEVAR1(x,y) CA_MAKEVAR2(x,y)
#define CA_MAKEVAR CA_MAKEVAR1(CompilerAssert, __LINE__)

// Finally, the macro.  It simply defines an empty class that inherits
// from corCompilerAssert.  No code should end up in the executable, in
// theory.  Perhaps COMPILER_ASSERT should do nothing in release
// builds, just in case?

#define COMPILER_ASSERT(expr) struct CA_MAKEVAR : public corCompilerAssert<(expr)> { }

// A version that works in a header file.  You must supply the class name.

#define COMPILER_ASSERT_H(klass, expr) struct klass : public corCompilerAssert<(expr)> { }

////////////
// Test code - just in case you don't believe the above actually
// works.  :)

#if 0
// test inheritance method
class CompilerAssertTest1 : public corCompilerAssert<true> { };
//class CompilerAssertTest2 : public corCompilerAssert<false> { };

// test usage as global declaration
COMPILER_ASSERT(true);
//COMPILER_ASSERT(false);

// test usage in a function
void foo()
{
  COMPILER_ASSERT(true);
  //COMPILER_ASSERT(false);
}
#endif

//another implementation of compile time assertions///////////////////////
//this one has nice errors that show up on the top of the .net compiler
// 
// SAMPLE USE:
//
//  template <class To, class From>
//  To safe_reinterpret_cast(From from)
//  {
//      STATIC_CHECK(sizeof(From) <= sizeof(To), THIS_REINTERPRET_CAST_WILL_TRUNCATE_BITS);
//      return reinterpret_cast<To>(from);
//  }
// 
//  will yield an error when the To size is less than the From, mentioning a 
//  failure to instantiate a class called 
//  ERROR_THIS_REINTERPRET_CAST_WILL_TRUNCATE_BITS
//
#define STATIC_CHECK(expr, msg) \
typedef char ERROR_##msg[1][(expr)]

//////////////////////////////////////////////////////////////////////////

namespace CompilerAsserts
{
    template <typename Base, typename Candidate>
    struct InheritsFrom
    {
        typedef char (&yes)[1];
        typedef char (&no)[2];
        static yes Test(Base*);
        static no Test(...);
        enum { IsTrue = (sizeof(Test((Candidate*)0)) == sizeof(yes)) };
    };
}

#define INHERITSFROM_ASSERT(base, candidate) \
COMPILER_ASSERT((CompilerAsserts::InheritsFrom<base, candidate>::IsTrue));

#endif // INCLUDED_corCompilerAssert

