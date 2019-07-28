/*****************************************************************************
	created:	2002/02/20
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Simple smart pointer implementation based on utlRefCount and
				utlRefHandle. The template parameter should be a class derived
				from utlRefCount, or a class that provides a similar interface.
*****************************************************************************/

#ifndef   INCLUDED_utlRefSmartPtr
#define   INCLUDED_utlRefSmartPtr
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/util/utlRefHandle.h"

template <class T>
class utlRefSmartPtr : public utlRefHandle<T>
{
public:
	utlRefSmartPtr( T *pObject = NULL)
	{
		SetImplementation(pObject);
	};

	// Equality
	bool operator == (const utlRefSmartPtr &rObject)
	{
		return (utlRefHandle<T>::GetImplementation() == rObject.GetImplementation());
	}

	// Inequality
	bool operator != (const utlRefSmartPtr &rObject)
	{
		return !(this == rObject);
	}

	// validation
	operator bool() const
	{
		return utlRefHandle<T>::GetImplementation() != NULL;
	}

	// Dereference
	T& operator*() const
	{
		return *utlRefHandle<T>::GetImplementation();
	}

	T* operator->() const
	{
		return utlRefHandle<T>::GetImplementation();
	}

	T* Get() const
	{
		return utlRefHandle<T>::GetImplementation();
	}
};

#endif // INCLUDED_utlRefSmartPtr

