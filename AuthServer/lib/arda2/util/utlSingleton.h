/*****************************************************************************
	created:	2001/04/23
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_utlSingleton
#define   INCLUDED_utlSingleton
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corAssert.h"

template <typename T> class utlSingleton
{
  static T* ms_Singleton;

public:
  utlSingleton()
  {
    errAssertV( !ms_Singleton, ("Singleton already exists.") );

    // Handle vtable offsets
    ptrdiff_t offset = (byte *)(T*)1 - (byte *)(utlSingleton <T>*)(T*)1;
    ms_Singleton = (T*)((byte *)this + offset);

    // This code should preform the same function as that above
    // But it doesn't always work, this breaks SkyDemo
//    ms_Singleton = static_cast<T*>(this);
  }
  ~utlSingleton()
  {
    errAssertV( ms_Singleton, ("Attemting to delete a Singleton that was never allocated.") );
    ms_Singleton = NULL;
  }
  static T& GetSingleton()
  {
    errAssertV( ms_Singleton, ("Singleton was never initialized.") );
    return ( *ms_Singleton );
  }
  static T* GetSingletonPtr()
  {
    return ms_Singleton;
  }
};

template <typename T> T* utlSingleton <T>::ms_Singleton = NULL;


#endif // INCLUDED_utlSingleton

