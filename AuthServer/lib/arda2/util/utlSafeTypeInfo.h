/*****************************************************************************
	created:	  2002/11/11
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Matt Walker
	
	purpose:	  Utility for gleaning RTTI type information for debugging.
*****************************************************************************/

#ifndef   INCLUDED_utlRefDbg
#define   INCLUDED_utlRefDbg
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#if CORE_DEBUG
#include <typeinfo.h>
#include "arda2/core/corStdString.h"
template<class T>
static inline std::string SAFE_TYPEINFO(const T* pObj)
{
	string type;
	try
	{
		type = typeid(*pObj).name();
	}
	catch(__non_rtti_object)
	{
		pObj;	// compiler warns about unreferenced parm if not
		type = "<unknown>";
	}
	return type;
}
#else
// for debugging release builds.
#include "arda2/core/corFirst.h"
#include "arda2/core/corStdString.h"
#include "arda2/core/corError.h"
template<class T>
static inline std::string SAFE_TYPEINFO(const T* pObj)
{
	pObj; // unref
	return std::string("<unknown>");
}

#endif // CORE_DEBUG

#endif 
