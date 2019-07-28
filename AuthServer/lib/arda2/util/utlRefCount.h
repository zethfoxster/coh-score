/*****************************************************************************
	created:	2001/08/13
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Reference counting mixin class
*****************************************************************************/

#ifndef   INCLUDED_utlRefCount
#define   INCLUDED_utlRefCount
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corAssert.h"


// Tracing macros - we WILL need these.
#ifdef PAL_REFCOUNT_DEBUG
#include "arda2/util/utlSafeTypeinfo.h"
#define REFCOUNT_INFO(pRefObj) {ERR_REPORTV(ES_Debug, ("%s: <%x> refcount = %d", SAFE_TYPEINFO(pRefObj).c_str(), pRefObj, pRefObj->RefCount());}
#define REFCOUNT_DEBUG(refmethod) {ERR_REPORTV(ES_Debug, ("%s: <%x> (%s) refcount = %d", SAFE_TYPEINFO(this).c_str(), this, #refmethod, RefCount()));}
#else
#define REFCOUNT_INFO(pRefObj) ((void) 0)
#define REFCOUNT_DEBUG(refmethod) ((void) 0)
#endif

// Helper template macros
template <class T> inline void SafeDecRef( T * &p )
{
    if( p )	
        p->DecRef();
    p = NULL;
}
template <class T> inline void SafeIncRef( T * &p )
{
    if( p )	
        p->IncRef();
}


class utlRefCount
{
public:
	utlRefCount() : m_refCount(0)
	{
	};

	virtual ~utlRefCount()
	{
		errAssertV(m_refCount == 0, ("Deleting object <%x> with refcount %d", this, m_refCount));
	}

	void IncRef() const;
	void DecRef() const;
	int RefCount() const	{ return m_refCount; }

	// clear the reference count to zero. Used by factory classes to allow cleanup without asserting.
	void ClearRefCount()	{ m_refCount = 0; }

	// The Grip and Ungrip functions are declared pure to force any deriving class to provide the
	// proper functionality. They also guarantee that any resource management using a "this" pointer
	// will get the proper pointer in case utlRefCount is part of a multiple inheritance base. For example,
	// we should not provide an Ungrip() function here which does a "delete this", because it would
	// fail horribly if this class was not the primary MI base.

protected:

	// called when refcount transitions 0 -> 1
	virtual void Grip() const = 0;

	// called when refcount transitions 1 -> 0  
	virtual void Ungrip() const = 0;

private:
	// disable default copy constructor and assignment operator
	utlRefCount( const utlRefCount& ) : m_refCount(0) {};
	void operator=( const utlRefCount& ) {};

	mutable int m_refCount;
};


/********************************************************************
 *  utlRefCount Inline methods
 *
 ********************************************************************/

inline void utlRefCount::IncRef() const
{
	++m_refCount;
	REFCOUNT_DEBUG(IncRef);
	if (m_refCount == 1) 
	{
		Grip(); 
	}
}


inline void utlRefCount::DecRef() const		
{ 
	errAssertV(m_refCount > 0, ("Illegal DecRef() on <%x> with refcount = %d", this, m_refCount));
	--m_refCount;
	REFCOUNT_DEBUG(DecRef);
	if (m_refCount == 0) 
	{
		Ungrip(); 
	}
}

#endif // INCLUDED_utlRefCount
