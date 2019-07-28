// WantedSocket.cpp: implementation of the CWantedSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "PreComp.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool WantedServerReconnect = false;
CRWLock gWantedLock;
LONG CWantedPacketServer::g_nPendingPacket;
CWantedSocket *pWantedSocket;
HANDLE g_hWantedServerTimer=NULL;

VOID CALLBACK WantedSocketTimerRoutine(PVOID lpParam, BYTE TimerOrWaitFired)
{
	AS_LOG_DEBUG( "WantedSocketTimerRoutine" );
	if ( g_hWantedServerTimer )
		DeleteTimerQueueTimer( NULL, g_hWantedServerTimer, NULL );
	g_hWantedServerTimer = NULL;

	if ( WantedServerReconnect == true ) {
		SOCKET WantedSock = socket(AF_INET, SOCK_STREAM, 0);
		// 2. 소켓 Connection에 사용할 Destination Setting을 한다.
		sockaddr_in Destination;
		Destination.sin_family = AF_INET;
		Destination.sin_addr   = config.WantedIP;
		Destination.sin_port   = htons( (u_short)config.WantedPort );
		// 3. Connection을 맺는다. 
		
		// 4. 맺어진 Connection을 이용하여 LOGSocket을 생성한다. 
		//    Connection Error가 생겼더라도 관계 없다. 
		//    그렇게 되면 자동적으로 Timer가 작동하여 10초에 한번씩 Reconnection을 시도하게 된다. 

		int ErrorCode = connect( WantedSock, ( sockaddr *)&Destination, sizeof( sockaddr ));
		
		CWantedSocket *tempWantedSocket = CWantedSocket::Allocate(WantedSock);
		tempWantedSocket->SetAddress( config.WantedIP );
		if ( ErrorCode == SOCKET_ERROR ){
			tempWantedSocket->CloseSocket();
			tempWantedSocket->ReleaseRef();
		} else {
			gWantedLock.WriteLock();
			CWantedSocket *tmpWantedSocket = pWantedSocket;
			pWantedSocket = tempWantedSocket;
			tmpWantedSocket->ReleaseRef();
			WantedServerReconnect = false;
			config.UseWantedSystem = true;
			pWantedSocket->Initialize( g_hIOCompletionPortInt );
			gWantedLock.WriteUnlock();
		}	
	} 
}

class CWantedPacketServerPool
{
public:
	class CSlot
	{
	public:
		CWantedPacketServer*   m_pPacket;
		CLock m_lock;
		CSlot() : m_pPacket(NULL),m_lock(eCustomSpinLock) {}
	};
	static CSlot g_slot[16];
	static long	g_nAlloc;
	static long	g_nFree;
	~CWantedPacketServerPool() { CWantedPacketServer::FreeAll(); }
};

CWantedPacketServerPool::CSlot	CWantedPacketServerPool::g_slot[16];
long	CWantedPacketServerPool::g_nAlloc = -1;
long	CWantedPacketServerPool::g_nFree = 0;
CWantedPacketServerPool theWantedPacketPool;


CWantedPacketServer * CWantedPacketServer::Alloc()
{
	CWantedPacketServer *newPacket;

	CWantedPacketServerPool::CSlot *pSlot =
		&CWantedPacketServerPool::g_slot[InterlockedIncrement(&CWantedPacketServerPool::g_nAlloc) & 15];
	pSlot->m_lock.Enter();
	if ((newPacket = pSlot->m_pPacket) != NULL) {
		pSlot->m_pPacket = reinterpret_cast<CWantedPacketServer *> (newPacket->m_pSocket);
		pSlot->m_lock.Leave();
	}
	else {
		pSlot->m_lock.Leave();
		newPacket = new CWantedPacketServer;
	}

	return newPacket;
}

void CWantedPacketServer::FreeAll()
{
	for (int i = 0 ; i < 16; i++) {
		CWantedPacketServerPool::CSlot *pSlot = &CWantedPacketServerPool::g_slot[i];
		pSlot->m_lock.Enter();
		CWantedPacketServer *pPacket;
		while ((pPacket = pSlot->m_pPacket) != NULL) {
			pSlot->m_pPacket = reinterpret_cast<CWantedPacketServer *> (pPacket->m_pSocket);
			delete pPacket;
		}
		pSlot->m_lock.Leave();
	}
}

void CWantedPacketServer::Free()
{
	CWantedPacketServerPool::CSlot *pSlot =
		&CWantedPacketServerPool::g_slot[InterlockedDecrement(&CWantedPacketServerPool::g_nFree) & 15];
	pSlot->m_lock.Enter();
	m_pSocket = reinterpret_cast<CIOSocket *>(pSlot->m_pPacket);
	pSlot->m_pPacket = this;
	pSlot->m_lock.Leave();
}

void CWantedPacketServer::OnIOCallback(BOOL bSuccess, DWORD dwTransferred, LPOVERLAPPED lpOverlapped)
{
_BEFORE
	unsigned char *packet = (unsigned char *) m_pBuf->m_buffer + dwTransferred;

	if ((*m_pFunc)(m_pSocket, packet + 1)) {
		m_pSocket->CIOSocket::CloseSocket();
	}

	m_pSocket->ReleaseRef();
	m_pBuf->Release();
	InterlockedDecrement(&g_nPendingPacket);
	Free();
_AFTER_FIN
	return;
}

static bool DummyPacket( CWantedSocket *s, const unsigned char *packet )
{
	log.AddLog( LOG_WARN, "Call DummyPacket What What What" );
	return false;
}
static bool GetVersion( CWantedSocket *s, const unsigned char *packet )
{
	int version = GetIntFromPacket( packet );
	AS_LOG_VERBOSE( "Get Version %d", version );
	return false;
}
static bool GetSendOK( CWantedSocket *s, const unsigned char *packet )
{
	int clientnum = GetIntFromPacket( packet );
	return false;
}
static bool GetSendFail( CWantedSocket *s, const unsigned char *packet )
{
_BEFORE
	UCHAR FailReason = GetCharFromPacket( packet );
//	int uid = GetIntFromPacket( packet );
_AFTER_FIN
	return false;
}

static WantedPacketFunc WantedPacketFuncTable[] = {
	GetVersion,
	GetSendOK,
	GetSendFail,
	DummyPacket,
};

CWantedSocket *CWantedSocket::Allocate( SOCKET s )
{
	return new CWantedSocket( s );
}

CWantedSocket::CWantedSocket( SOCKET aSoc )
: CIOSocket( aSoc )
{
	addr = config.WantedIP;
	host = 0;
	mode = SM_READ_LEN;
	packetTable =WantedPacketFuncTable;
	WantedServerReconnect = false;
}
CWantedSocket::~CWantedSocket()
{
	log.AddLog( LOG_ERROR, "WantedSocket Deleted" );
}

void CWantedSocket::OnClose(SOCKET closedSocket)
{
	// Must not use the GetSocket() function from within
 	// this function!  Instead, use the socket argument
 	// passed into the function.

	mode = SM_CLOSE;
	WantedServerReconnect = true;
	config.UseWantedSystem = false;

	log.AddLog(LOG_ERROR, "*close connection WantedSocket from %s, %x(%x)", IP(), closedSocket, this);
	AddRef();
	CreateTimerQueueTimer( &g_hWantedServerTimer, NULL, WantedSocketTimerRoutine, this, config.WantedReconnectInterval, 0, 0 );
}


void CWantedSocket::OnTimerCallback( void )
{
}

const char *CWantedSocket::IP()
{
	return inet_ntoa(addr);
}

void CWantedSocket::OnCreate()
{
	AddRef();
	OnRead();
}

void CWantedSocket::OnRead()
{
	int pi = 0;
	int ri = m_pReadBuf->m_size;
	unsigned char *inBuf = (unsigned char *)m_pReadBuf->m_buffer;
	if (mode == SM_CLOSE) {
		CloseSocket();
		return;
	}

	for  ( ; ; ) {
		if (pi >= ri) {
			pi = 0;
			Read(0);
			return;
		}
		if (mode == SM_READ_LEN) {
			if (pi + 3 <= ri) {
				packetLen = inBuf[pi] + (inBuf[pi + 1] << 8) + 1;
				if (packetLen <= 0 || packetLen > BUFFER_SIZE) {
					log.AddLog(LOG_ERROR, "%d: bad packet size %d", m_hSocket, packetLen);
					break;
				} else {
					pi += 2;
					mode = SM_READ_BODY;
				}
			} else {
				Read(ri - pi);
				return;
			}
		} else if (mode == SM_READ_BODY) {
			if (pi + packetLen <= ri) {

				if (inBuf[pi] >= WA_MAX) {
					log.AddLog(LOG_ERROR, "unknown protocol %d", inBuf[pi]);
					break;
				} else {
					CWantedPacketServer *pPacket = CWantedPacketServer::Alloc();
					pPacket->m_pSocket = this;
					pPacket->m_pBuf = m_pReadBuf;
					pPacket->m_pFunc = (CWantedPacketServer::WantedPacketFunc) packetTable[inBuf[pi]];
					CIOSocket::AddRef();
					m_pReadBuf->AddRef();
					InterlockedIncrement(&CWantedPacketServer::g_nPendingPacket);
					pPacket->PostObject(pi, g_hIOCompletionPort);
					pi += packetLen;
					mode = SM_READ_LEN;
				}
			} else {
				Read(ri - pi);
				return;
			}
		}
		else
			break;
	}
	CIOSocket::CloseSocket();
}


bool CWantedSocket::Send(const char* format, ...)
{
	AddRef();
	if (mode == SM_CLOSE || WantedServerReconnect || !config.UseWantedSystem || m_hSocket == INVALID_SOCKET) {
		ReleaseRef();
		return false;
	}

	CIOBuffer *pBuffer = CIOBuffer::Alloc();
	char *buffer = pBuffer->m_buffer;
	va_list ap;
	va_start(ap, format);
	int len = Assemble(buffer + 2, BUFFER_SIZE - 2, format, ap);
	va_end(ap);
	if (len == 0) {
		log.AddLog(LOG_ERROR, "%d: assemble too large packet. format %s", m_hSocket, format);
	} else {
		len -= 1;
		len = len;
		buffer[0] = len;
		buffer[1] = len >> 8;
	}
	pBuffer->m_size = len+3;
	Write(pBuffer);
	ReleaseRef();
	return true;
}