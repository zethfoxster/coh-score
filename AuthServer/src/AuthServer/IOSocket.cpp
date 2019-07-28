// IOSocket.cpp: implementation of the CIOSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "IOSocket.h"
#include "Protocol.h"
#include "GlobalAuth.h"
#include "util.h"
#include "buildn.h"
#include "Packet.h"
#include "IOServer.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define TIMEOUT	60

LONG CPacketServer::g_nPendingPacket;


CIOSocket::CIOSocket( SOCKET s )
{
	InitializeCriticalSection( &m_cs );
	m_hSocket = s;

	memset(&m_overlappedRead, 0, sizeof(OVERLAPPED));
	memset(&m_overlappedWrite, 0, sizeof(OVERLAPPED));
	m_pReadBuf = CIOBuffer::Alloc();
	m_nPendingWrite = 0;
	m_pFirstBuf = m_pLastBuf = NULL;
}

CIOSocket::~CIOSocket()
{
	DeleteCriticalSection( &m_cs );
	m_pReadBuf->Release();
	while (m_pFirstBuf) {
		CIOBuffer *pBuf = m_pFirstBuf;
		m_pFirstBuf = m_pFirstBuf->m_pNext;
		pBuf->Free();
	}

}

void CIOSocket::OnClose( SOCKET )
{
}

void CIOSocket::CloseSocket( void )
{
	SOCKET hSocket = InterlockedExchange((LPLONG)&m_hSocket, INVALID_SOCKET);
	if (hSocket != INVALID_SOCKET) {
		OnClose(hSocket);
		LINGER linger;
		linger.l_onoff = 1;
		linger.l_linger = 0;
		setsockopt(hSocket, SOL_SOCKET, SO_LINGER, (char *)&linger, sizeof(linger));
		closesocket(hSocket);
		ReleaseRef();
	}
}

void CIOSocket::Initialize( HANDLE hIOCompletionPort )
{
	int zero = 0;
	setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char *)&zero, sizeof(zero));
	zero = 0;
	setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero));
	AddRef();
	if (CreateIoCompletionPort((HANDLE) m_hSocket, hIOCompletionPort, (DWORD)PtrToUint(this), 0) == NULL) {
		log.AddLog( LOG_ERROR, "Initilize CompletionPort Error CloseSocket" );
		CloseSocket();
		return;
	}
	OnCreate();
	ReleaseRef();
}

void CIOSocket::OnCreate( void )
{
	// IntSocket과 ServerSocket의 Read부분이 여기서 틀려지게 된다. 
	OnRead();
}

void CIOSocket::OnIOCallback( BOOL bSuccess, DWORD dwTransferred, LPOVERLAPPED lpOverlapped )
{
	if (!bSuccess) {
		if(lpOverlapped == &m_overlappedRead || lpOverlapped == &m_overlappedWrite)
			CloseSocket();
	}
	else if (lpOverlapped == &m_overlappedRead)
		OnReadCallback(dwTransferred);
	else if (lpOverlapped == &m_overlappedWrite)
		OnWriteCallback(dwTransferred);
	else
		_ASSERTE(0);

	ReleaseRef();
	return;
}

void CIOSocket::OnReadCallback( DWORD dwTransferred )
{
	if (dwTransferred == 0) {
		CloseSocket();
		return;
	}
	if (this->GetRef() <= 0){
		log.AddLog(LOG_ERROR, "Invalid Socket!");
		return;
	}

	m_pReadBuf->m_size += dwTransferred;
	OnRead();
}

void CIOSocket::OnWriteCallback( DWORD dwTransferred )
{
	EnterCriticalSection( &m_cs );
	if (dwTransferred != m_pFirstBuf->m_size) {
		log.AddLog(LOG_ERROR,  "different write count %x(%x) %d != %d", m_hSocket, this, dwTransferred, m_pFirstBuf->m_size);
		LeaveCriticalSection(&m_cs);
		ReleaseRef();
		return;	
	}
	m_nPendingWrite -= m_pFirstBuf->m_size;
	CIOBuffer *pFirstBuf = m_pFirstBuf;
	if ((m_pFirstBuf = m_pFirstBuf->m_pNext) != NULL) {
		LeaveCriticalSection(&m_cs);
		AddRef();
		WSABUF wsabuf;
		wsabuf.len = m_pFirstBuf->m_size;
		wsabuf.buf = m_pFirstBuf->m_buffer;
		DWORD dwSent;
		if (WSASend(m_hSocket, &wsabuf, 1, &dwSent, 0, &m_overlappedWrite, NULL) && GetLastError( ) != ERROR_IO_PENDING) {
			int nErr = GetLastError();
			if (nErr != WSAENOTSOCK && nErr != WSAECONNRESET && nErr != WSAECONNABORTED)
				log.AddLog(LOG_ERROR, "CIOSocket::WriteCallback %x(%x) err=%d", m_hSocket, this, nErr);
			ReleaseRef();
		}
	}
	else {
		m_pLastBuf = NULL;
		LeaveCriticalSection(&m_cs);
	}
	pFirstBuf->Free();
}

void CIOSocket::Read(DWORD dwLeft)
{
	m_pReadBuf->m_size -= dwLeft;
	if (m_pReadBuf->m_ref != 1) {
		CIOBuffer *pNextBuf = CIOBuffer::Alloc();
		memcpy(pNextBuf->m_buffer, m_pReadBuf->m_buffer + m_pReadBuf->m_size, dwLeft);
		m_pReadBuf->Release();
		m_pReadBuf = pNextBuf;
	}
	else {
		memmove(m_pReadBuf->m_buffer, m_pReadBuf->m_buffer + m_pReadBuf->m_size, dwLeft);
	}
	m_pReadBuf->m_size = dwLeft;

	AddRef();
	WSABUF wsabuf;
	wsabuf.len = BUFFER_SIZE - m_pReadBuf->m_size;
	wsabuf.buf = m_pReadBuf->m_buffer + m_pReadBuf->m_size;
	DWORD dwRecv;
	DWORD dwFlag = 0;
	if (WSARecv(m_hSocket, &wsabuf, 1, &dwRecv, &dwFlag, &m_overlappedRead, NULL) && GetLastError() != ERROR_IO_PENDING) {
		int nErr = GetLastError();
		if (nErr != WSAENOTSOCK && nErr != WSAECONNRESET && nErr != WSAECONNABORTED)
			log.AddLog(LOG_ERROR, "CIOSocket::Read %x(%x) err = %d", m_hSocket, this, nErr);
		CloseSocket();
		ReleaseRef();
	}
}

void CIOSocket::Write(CIOBuffer *pBuffer)
{
	// 보내는 Size가 0이면 필요 없음...
	if (pBuffer->m_size == 0) {
		pBuffer->Free();
		return;
	}
	EnterCriticalSection(&m_cs);
	m_nPendingWrite += pBuffer->m_size;
	if (m_pLastBuf == NULL) {
		m_pFirstBuf = m_pLastBuf = pBuffer;
		LeaveCriticalSection(&m_cs);
		AddRef();
		WSABUF wsabuf;
		wsabuf.len = pBuffer->m_size;
		wsabuf.buf = pBuffer->m_buffer;
		DWORD dwSent=0;

		if (WSASend(m_hSocket, &wsabuf, 1, &dwSent, 0, &m_overlappedWrite, NULL)
			&& GetLastError() != ERROR_IO_PENDING) {
			int nErr = GetLastError();
			if (nErr != WSAENOTSOCK && nErr != WSAECONNRESET && nErr != WSAECONNABORTED)
				log.AddLog(LOG_ERROR, "CIOSocket::Write %x(%x) err=%d", m_hSocket, this, nErr);
			ReleaseRef(); 
		}
	}
	else if (m_pFirstBuf != m_pLastBuf && m_pLastBuf->m_size + pBuffer->m_size <= BUFFER_SIZE) {
		memcpy(m_pLastBuf->m_buffer+m_pLastBuf->m_size, pBuffer->m_buffer, pBuffer->m_size);
		m_pLastBuf->m_size += pBuffer->m_size;
		LeaveCriticalSection(&m_cs);
		pBuffer->Free();
	}
	else {
		m_pLastBuf->m_pNext = pBuffer;
		m_pLastBuf = pBuffer;
		LeaveCriticalSection(&m_cs);
	}
}

void CIOSocket::Write(char *buf, DWORD size)
{
	while (size) {
		CIOBuffer *pBuffer = CIOBuffer::Alloc();
		DWORD n = min(BUFFER_SIZE, size);
		memcpy(pBuffer->m_buffer, buf, n);
		pBuffer->m_size = n;
		Write(pBuffer);
		buf += n;
		size -= n;
	}
}



class CPacketServerPool
{
public:
	class CSlot
	{
	public:
		CPacketServer *   m_pPacket;
		CLock m_lock;
		CSlot() : m_pPacket(NULL),m_lock(eCustomSpinLock) {}
	};
	static CSlot g_slot[16];
	static long	g_nAlloc;
	static long	g_nFree;
	~CPacketServerPool() { CPacketServer::FreeAll(); }
};

CPacketServerPool::CSlot	CPacketServerPool::g_slot[16];
long	CPacketServerPool::g_nAlloc = -1;
long	CPacketServerPool::g_nFree = 0;

CPacketServerPool thePacketPool;

CPacketServer * CPacketServer::Alloc()
{
	CPacketServer *newPacket;
	CPacketServerPool::CSlot *pSlot =
		&CPacketServerPool::g_slot[InterlockedIncrement(&CPacketServerPool::g_nAlloc) & 15];
	pSlot->m_lock.Enter();
	if ((newPacket = pSlot->m_pPacket) != NULL) {
		pSlot->m_pPacket = reinterpret_cast<CPacketServer *> (newPacket->m_pSocket);
		pSlot->m_lock.Leave();
	}
	else {
		pSlot->m_lock.Leave();
		newPacket = new CPacketServer;
	}
	return newPacket;
}

void CPacketServer::Free()
{
	CPacketServerPool::CSlot *pSlot =
		&CPacketServerPool::g_slot[InterlockedDecrement(&CPacketServerPool::g_nFree) & 15];
	pSlot->m_lock.Enter();
	m_pSocket = reinterpret_cast<CIOSocket *>(pSlot->m_pPacket);
	pSlot->m_pPacket = this;
	pSlot->m_lock.Leave();
}

void CPacketServer::FreeAll()
{
	for (int i = 0 ; i < 16; i++) {
		CPacketServerPool::CSlot *pSlot = &CPacketServerPool::g_slot[i];
		pSlot->m_lock.Enter();
		CPacketServer *pPacket;
		while ((pPacket = pSlot->m_pPacket) != NULL) {
			pSlot->m_pPacket = reinterpret_cast<CPacketServer *> (pPacket->m_pSocket);
			delete pPacket;
		}
		pSlot->m_lock.Leave();
	}
}

void CPacketServer::OnIOCallback(BOOL bSuccess, DWORD dwTransferred, LPOVERLAPPED lpOverlapped)
{
	unsigned char *packet = (unsigned char *) m_pBuf->m_buffer + dwTransferred;

	if ((*m_pFunc)(m_pSocket, packet + 1)) {
		m_pSocket->CIOSocket::CloseSocket();
		log.AddLog( LOG_ERROR,"ServerClose:PacketServerClose" );
	}

	m_pSocket->ReleaseRef();
	m_pBuf->Release();
	InterlockedDecrement(&g_nPendingPacket);
	Free();

	return;
}



