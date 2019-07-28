// IOBuffer.cpp: implementation of the CIOBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "IOBuffer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOBuffer::CIOBuffer()
{
}

CIOBuffer::~CIOBuffer()
{
}

CIOBufferPool::Slot CIOBufferPool::m_slot[16];
long CIOBufferPool::m_alloc = -1;
long CIOBufferPool::m_free = 0;

CIOBufferPool g_bufferPool;

CIOBuffer *CIOBuffer::Alloc()
{
	CIOBufferPool::Slot *pSlot = 
		&CIOBufferPool::m_slot[InterlockedIncrement(&CIOBufferPool::m_alloc) & 15];
	CIOBuffer *pNewBuffer;
	pSlot->m_lock.Enter();
	if((pNewBuffer = pSlot->m_pBuffer) != NULL) {
		pSlot->m_pBuffer = pNewBuffer->m_pNext;
		pSlot->m_lock.Leave();
    } else {
		pSlot->m_lock.Leave();
		pNewBuffer = new CIOBuffer;
	}
//	memset(&(pNewBuffer->m_buffer), 0, sizeof(BUFFER_SIZE));
    pNewBuffer->m_size = 0;
    pNewBuffer->m_ref = 1;
    pNewBuffer->m_pNext = NULL;

    return pNewBuffer;
}

void CIOBuffer::Free()
{
	CIOBufferPool::Slot *pSlot = 
		&CIOBufferPool::m_slot[InterlockedDecrement(&CIOBufferPool::m_free) & 15];
	pSlot->m_lock.Enter();
	m_pNext = pSlot->m_pBuffer;
	pSlot->m_pBuffer = this;
	pSlot->m_lock.Leave();
}

void CIOBuffer::FreeAll()
{
	for(int i = 0 ; i < 16; i++) {
		CIOBufferPool::Slot *pSlot = &CIOBufferPool::m_slot[i];
		pSlot->m_lock.Enter();
		CIOBuffer *pBuffer;
		while ((pBuffer = pSlot->m_pBuffer) != NULL) {
			pSlot->m_pBuffer = pBuffer->m_pNext;
			delete pBuffer;
		}
		pSlot->m_lock.Leave();
	}
}

