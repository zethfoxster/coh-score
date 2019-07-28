/*****************************************************************************
	created:	2001/08/07
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Tom Gambill
	
	purpose:	C++ common callback template
*****************************************************************************/

#ifndef   INCLUDED_utlCallback
#define   INCLUDED_utlCallback
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corAssert.h"

template <typename T>
class utlCallback
{
public:
  //* Each callback object must define this method. It will be called when the 
  //* device surface is lost and restored(thus empty) to refill it with data.
  virtual errResult operator()( T *pContext ) = 0;
	
protected:
  utlCallback() {};
  utlCallback& operator=( const utlCallback& rSource );
  utlCallback(const utlCallback &rSource);
};


#endif // INCLUDED_utlCallback
