/*****************************************************************************
	created:	2001/08/13
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_thrKernelMutex
#define   INCLUDED_thrKernelMutex
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class thrKernelMutex : public thrKernelObject
{
public:
	thrKernelMutex( bool bInitialOwner = false, char *szName = NULL)
	{
		SetHandle(::CreateMutex(NULL, bInitialOwner, szName));
	}
	virtual ~thrKernelMutex()
	{
	}

      // releases the mutex after previously acquiring it via Wait()
    bool Release(void)
	{
		return  ::ReleaseMutex(GetHandle()) != 0;
	}

protected:
	
private:
    // disable copy constructor and assignment operator
    thrKernelMutex( const thrKernelMutex& );
    thrKernelMutex& operator=( const thrKernelMutex& );
};


#endif // INCLUDED_thrKernelMutex