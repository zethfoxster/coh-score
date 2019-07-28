/*****************************************************************************
	created:	2002/02/08
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Mix-in class for handles to reference counted impls. This
				class handles all the implementation details for ref-counting,
				including copy construction, assignment, and destruction.
				The template parameter should be a class derived from
				utlRefCount, or a class that provides a similar interface.
*****************************************************************************/

#ifndef   INCLUDED_utlRefHandle
#define   INCLUDED_utlRefHandle
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "arda2/core/corAssert.h"
#include "arda2/core/corStlHashTraits.h"

// Tracing macros - we WILL need these.
#ifdef PAL_REFCOUNT_DEBUG
#include "arda2/util/utlSafeTypeinfo.h"
#define REFHANDLE_INCREF(pRefObj) \
{ \
	ERR_REPORTV(ES_Debug, ("%s:<%x> (IncRef) %s:<%x>", SAFE_TYPEINFO(this).c_str(), this, SAFE_TYPEINFO(pRefObj).c_str(), pRefObj)); \
	pRefObj->IncRef(); \
}
#define REFHANDLE_DECREF(pRefObj) \
{ \
	ERR_REPORTV(ES_Debug, ("%s:<%x> (DecRef) %s:<%x>", SAFE_TYPEINFO(this).c_str(), this, SAFE_TYPEINFO(pRefObj).c_str(), pRefObj)); \
	pRefObj->DecRef(); \
}
#else
#define REFHANDLE_INCREF(pRefObj) { pRefObj->IncRef(); }
#define REFHANDLE_DECREF(pRefObj) { pRefObj->DecRef(); }
#endif


template <class T>
class utlRefHandle
{
public:
	utlRefHandle() : m_pImpl(NULL)
	{
	};

	~utlRefHandle()
	{
		if ( m_pImpl )
		{
			REFHANDLE_DECREF(m_pImpl);
		}
	}

	utlRefHandle( const utlRefHandle& rObj )
	{
		if ( rObj.m_pImpl ) 
			REFHANDLE_INCREF(rObj.m_pImpl);
		m_pImpl = rObj.m_pImpl;
	}

	utlRefHandle& operator=( const utlRefHandle& rObj )
	{
		SetImpl(rObj.m_pImpl);
		return *this;
	}

	void ReleaseImpl()
	{
		if ( m_pImpl )
		{
			REFHANDLE_DECREF(m_pImpl);
			m_pImpl = NULL;
		}
	}
	void ReleaseImplementation() { ReleaseImpl(); }

	void SetImpl( T *pImpl )
	{
		if ( pImpl ) 
			REFHANDLE_INCREF(pImpl);
		if ( m_pImpl )
			REFHANDLE_DECREF(m_pImpl);
		m_pImpl = pImpl;
	}
	void SetImplementation( T *pImpl ) { SetImpl(pImpl); }

	T* GetImpl() const { return m_pImpl; }
	T* GetImplementation() const { return m_pImpl; }

    void Swap( utlRefHandle<T>& _Right)
    {
        std::swap(m_pImpl, _Right.m_pImpl);
    }

protected:
	T*	m_pImpl;
};


template<class T> inline
void swap(utlRefHandle<T>& _Left, utlRefHandle<T>& _Right)
{	// swap _Left and _Right vectors
    _Left.Swap(_Right);
}


/********************************************************************
 *  Specialization of utlRefHandle that gives us the ability to
 *  put refcounted things in STL collections. No virtual methods, 
 *  because nothing should derive from this.
 *
 ********************************************************************/
template <class T>
class utlContainerRefHandle : public utlRefHandle<T>
{
public:
	utlContainerRefHandle() : utlRefHandle<T>() {}
	explicit utlContainerRefHandle(T* pImpl) : utlRefHandle<T>() { SetImpl(pImpl); }
	~utlContainerRefHandle() {}

	// copy and assignment
	utlContainerRefHandle(const utlContainerRefHandle& rhs) : utlRefHandle<T>(rhs) {}
	const utlContainerRefHandle& operator=(const utlContainerRefHandle& rhs)
	{	utlRefHandle<T>::operator =(rhs); return *this; }

	// find support
	bool operator==(const utlContainerRefHandle<T>& rhs) const
		{ return utlContainerRefHandle<T>::GetImpl() == rhs.GetImpl(); }
	bool operator<(const utlContainerRefHandle<T>& rhs) const
		{ return utlContainerRefHandle<T>::GetImpl() < rhs.GetImpl(); }
};



/********************************************************************
 *  Traits struct for comparing utloContainerRefHandle in a hash map.
 ********************************************************************/
template <class T>
struct utlContainerRefHandleHashTraits : 
  public corHashTraitsBase< utlContainerRefHandle<T> >
{
  static inline size_t Hash(const utlContainerRefHandle<T>& h)
    { return corPointerHashFunc::Hash(h.GetImpl()); }
  static inline bool Less(const utlContainerRefHandle<T>& lhs, const utlContainerRefHandle<T>& rhs)
    { return lhs < rhs; }
  static inline bool Equal(const utlContainerRefHandle<T>& lhs, const utlContainerRefHandle<T>& rhs)
    { return lhs == rhs; }
};


#endif // INCLUDED_utlRefHandle

