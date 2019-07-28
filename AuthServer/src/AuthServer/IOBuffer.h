// IOBuffer.h: interface for the CIOBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOBUFFER_H__F70E60BE_69F4_40D3_A3A0_0C6BF7266F87__INCLUDED_)
#define AFX_IOBUFFER_H__F70E60BE_69F4_40D3_A3A0_0C6BF7266F87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Lock.h"

#define BUFFER_SIZE 8192

class CIOBuffer
{
public:
	char m_buffer[BUFFER_SIZE];
	DWORD m_size;
	CIOBuffer *m_pNext;
	LONG m_ref;
	CIOBuffer();

	virtual ~CIOBuffer();
	static CIOBuffer *Alloc();

	static void FreeAll();
	void AddRef() { InterlockedIncrement(&m_ref); }
	void Release() { if (InterlockedDecrement(&m_ref) == 0) Free(); }
    void Free();
};

class CIOBufferPool
{
public:
	class Slot
	{
	public:
		CIOBuffer *m_pBuffer;
		CLock m_lock;

		Slot() : m_pBuffer(NULL), m_lock(eSystemSpinLock, 4000) {}
	};
	static Slot m_slot[16];
	static long	m_alloc;
	static long	m_free;
	~CIOBufferPool() { CIOBuffer::FreeAll(); }
};

extern CIOBufferPool g_bufferPool;

#endif // !defined(AFX_IOBUFFER_H__F70E60BE_69F4_40D3_A3A0_0C6BF7266F87__INCLUDED_)
