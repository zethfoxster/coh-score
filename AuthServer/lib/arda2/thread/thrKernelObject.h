/*****************************************************************************
	created:	2001/08/13
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Wrapper for win32 kernel objects.
*****************************************************************************/

#ifndef   INCLUDED_thrKernelObject
#define   INCLUDED_thrKernelObject
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if CORE_SYSTEM_WINAPI

//////////////////////////////////////////////////////////////////////////////
// class KernelObject declaration

class thrKernelObject
{
public:
	// clears mHandle
	thrKernelObject() : m_Handle(NULL)
	{
	}

	// calls Close() if mHandle not null
	virtual ~thrKernelObject()
	{
		if ( m_Handle )
		{
			ResetHandle();
		}
	}

	  // gives back the current handle
	HANDLE GetHandle(void) const
	{
		return m_Handle;
	}

	  // is there a handle?
	bool IsValid(void) const
	{
		return m_Handle != NULL;
	}

	// waits on this (asserts if handle invalid)
	bool Wait(DWORD milliseconds = INFINITE) const
	{
		errAssert( IsValid() );
		return ::WaitForSingleObject( m_Handle, milliseconds) == WAIT_OBJECT_0;
	}

protected:
	  // asserts if there already is a handle
	void SetHandle(HANDLE hNewHandle)
	{
		errAssert( m_Handle == NULL && hNewHandle != NULL );
		m_Handle = hNewHandle;
	}

	  // asserts if there is not a handle
	void ResetHandle(void)
	{
		errAssert( m_Handle != NULL );
		::CloseHandle(m_Handle);
		m_Handle = NULL;
	}

private:
	HANDLE	m_Handle;

    // disable copy constructor and assignment operator
    thrKernelObject( const thrKernelObject& );
    thrKernelObject& operator=( const thrKernelObject& );
};

#else

class thrKernelObject { };

#endif // Win32

#endif // INCLUDED_theKernelObject
