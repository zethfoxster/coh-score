// IPSessionDB.cpp: implementation of the CIPSessionDB class.
//
//////////////////////////////////////////////////////////////////////

#include "IPSessionDB.h"
#include "util.h"
#include "config.h"
#include "buildn.h"
#include "accountdb.h"
#include "IOServer.h"

bool IPServerReconnect = false;
bool g_IPServeropFlag = false;
HANDLE g_hIPServerTimer = NULL;
CRWLock gIPLock;

LONG CIPPacketServer::g_nPendingPacket;
CIPSessionDB ipsessionDB;
extern bool g_bTerminating;
extern BOOL SendSocketEx(SOCKET s, const char *format, ...);


VOID CALLBACK IPSocketTimerRoutine(PVOID lpParam, BYTE TimerOrWaitFired)
{
	AS_LOG_VERBOSE( "IPSocketTimerRoutine" );
	if ( g_hIPServerTimer )
		DeleteTimerQueueTimer( NULL, g_hIPServerTimer, NULL );
	
	g_hIPServerTimer = NULL;

	if ( IPServerReconnect == true ) {
		SOCKET LOGSock = socket(AF_INET, SOCK_STREAM, 0);
		// 2. 소켓 Connection에 사용할 Destination Setting을 한다.
		sockaddr_in Destination;
		Destination.sin_family = AF_INET;
		Destination.sin_addr   = config.IPServer;
		Destination.sin_port   = htons( (u_short)config.IPPort );
		// 3. Connection을 맺는다. 
		
		// 4. 맺어진 Connection을 이용하여 LOGSocket을 생성한다. 
		//    Connection Error가 생겼더라도 관계 없다. 
		//    그렇게 되면 자동적으로 Timer가 작동하여 10초에 한번씩 Reconnection을 시도하게 된다. 

		int ErrorCode = connect( LOGSock, ( sockaddr *)&Destination, sizeof( sockaddr ));
		
		CIPSocket *tempIPSocket = CIPSocket::Allocate(LOGSock);
		tempIPSocket->SetAddress( config.IPServer );
		if ( ErrorCode == SOCKET_ERROR ){
			tempIPSocket->CloseSocket();
			tempIPSocket->ReleaseRef();
		} else {
			gIPLock.WriteLock();
			CIPSocket *tmpIPSocket = pIPSocket;
			pIPSocket = tempIPSocket;
			IPServerReconnect = false;
			config.UseIPServer = true;
			pIPSocket->Initialize( g_hIOCompletionPort );
			gIPLock.WriteUnlock();
			tmpIPSocket->ReleaseRef();
		}	
	} 
}


class CIPPacketServerPool
{
public:
	class CSlot
	{
	public:
		CIPPacketServer*   m_pPacket;
		CLock m_lock;
		CSlot() : m_pPacket(NULL),m_lock(eCustomSpinLock) {}
	};
	static CSlot g_slot[16];
	static long	g_nAlloc;
	static long	g_nFree;
	~CIPPacketServerPool() { CIPPacketServer::FreeAll(); }
};

CIPPacketServerPool::CSlot	CIPPacketServerPool::g_slot[16];
long	CIPPacketServerPool::g_nAlloc = -1;
long	CIPPacketServerPool::g_nFree = 0;
CIPPacketServerPool theIPPacketPool;

CIPPacketServer * CIPPacketServer::Alloc()
{
	CIPPacketServer *newPacket;

	CIPPacketServerPool::CSlot *pSlot =
		&CIPPacketServerPool::g_slot[InterlockedIncrement(&CIPPacketServerPool::g_nAlloc) & 15];
	pSlot->m_lock.Enter();
	if ((newPacket = pSlot->m_pPacket) != NULL) {
		pSlot->m_pPacket = reinterpret_cast<CIPPacketServer *> (newPacket->m_pSocket);
		pSlot->m_lock.Leave();
	}
	else {
		pSlot->m_lock.Leave();
		newPacket = new CIPPacketServer;
	}

	return newPacket;
}

void CIPPacketServer::FreeAll()
{
	for (int i = 0 ; i < 16; i++) {
		CIPPacketServerPool::CSlot *pSlot = &CIPPacketServerPool::g_slot[i];
		pSlot->m_lock.Enter();
		CIPPacketServer *pPacket;
		while ((pPacket = pSlot->m_pPacket) != NULL) {
			pSlot->m_pPacket = reinterpret_cast<CIPPacketServer *> (pPacket->m_pSocket);
			delete pPacket;
		}
		pSlot->m_lock.Leave();
	}
}

void CIPPacketServer::Free()
{
	CIPPacketServerPool::CSlot *pSlot =
		&CIPPacketServerPool::g_slot[InterlockedDecrement(&CIPPacketServerPool::g_nFree) & 15];
	pSlot->m_lock.Enter();
	m_pSocket = reinterpret_cast<CIOSocket *>(pSlot->m_pPacket);
	pSlot->m_pPacket = this;
	pSlot->m_lock.Leave();
}

void CIPPacketServer::OnIOCallback(BOOL bSuccess, DWORD dwTransferred, LPOVERLAPPED lpOverlapped)
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


CIPSessionDB::CIPSessionDB()
{

}

CIPSessionDB::~CIPSessionDB()
{

}

bool CIPSessionDB::DelUserWait( int uid, LoginUser **lu )
{
	bool result = false;
_BEFORE
	UserPointerMap::iterator it;

	WaitUserLock.Enter();
	it = WaitingUser.find( uid );
	if ( it != WaitingUser.end() ){
		*lu = it->second;
		WaitingUser.erase(it);
		WaitUserLock.Leave();
		result = true;
	} else {
		WaitUserLock.Leave();
	}
_AFTER_FIN
	return result;
}

bool CIPSessionDB::AddUserWait( int uid, LoginUser *lu )
{
	bool result = false;
_BEFORE
	WaitUserLock.Enter();
	std::pair<UserPointerMap::iterator, bool> r	= WaitingUser.insert(UserPointerMap::value_type( uid, lu ));
	result = r.second;
	WaitUserLock.Leave();
	
	// 만일 기존에 등록이 된 흔적이 아직도 남아 있다면 IP서버에서 아직 응답이 오지 않은 것으로 간주한다. 
	// 그대로 두면 영원히 로그인을 못할 수 있기 때문에 그때에는  그 정보를 지워버리고 다시 처음부터 프로세스를 
	// 시작 하도록 처리한다. 
	if ( result == false ) {
		LoginUser *prvLu=NULL; // Dummy 용이다. 
		DelUserWait( uid, &prvLu );
	}
_AFTER_FIN
	return result;
}

int CIPSessionDB::FindSessionID( int Uid )
{
	int SessionID = 0;
_BEFORE
	IPSessionLock.Enter();
	SESSIONMAP::iterator it = IPSessionMap.find( Uid );
	if ( it != IPSessionMap.end() )
		SessionID = it->second;
	IPSessionLock.Leave();
_AFTER_FIN
	return SessionID;
}

int CIPSessionDB::DelSessionID( int Uid )
{
	int SessionID = 0;
_BEFORE
	IPSessionLock.Enter();
	SESSIONMAP::iterator it = IPSessionMap.find( Uid );
	if ( it != IPSessionMap.end() ){
		SessionID = it->second;
		IPSessionMap.erase(it);
	}
	IPSessionLock.Leave();
_AFTER_FIN
	return SessionID;
}

bool CIPSessionDB::DellAllWaitingSessionID( void )
{
_BEFORE
	UserPointerMap::iterator it;
	WaitUserLock.Enter();
	for( it = WaitingUser.begin(); it != WaitingUser.end(); ){
		delete it->second;
		it = WaitingUser.erase(it);
	}
	WaitUserLock.Leave();
_AFTER_FIN
	return true;
}

// SessionID는 어떠한 경우에도 0이 오면 안된다.
int CIPSessionDB::AddSessionID ( int uid, int sessionid )
{
_BEFORE
	bool result=false;

	if ( sessionid == 0 )
		return sessionid;

	IPSessionLock.Enter();
	std::pair<SESSIONMAP::iterator, bool> r	= IPSessionMap.insert(SESSIONMAP::value_type(uid, sessionid));		
	result = r.second;
	IPSessionLock.Leave();
	if ( result == true )
		return sessionid;
	else
		return 0;
_AFTER_FIN
	return 0;
}
char CIPSessionDB::AcquireSessionRequest(LoginUser *lu, int uid)
{
// account, inIP, gameid, uid
	char ErrorCode = IP_ALL_OK;
_BEFORE
	bool result = AddUserWait(uid, lu);
	if (result) {
		if ( pIPSocket == NULL || IPServerReconnect || ( !config.UseIPServer)) {
			ErrorCode = IP_SERVER_SOCKET_FAIL;
			LoginUser **tmp = 0;
			DelUserWait( uid, tmp );
		}else if ( !(pIPSocket->Send( "csddd", AI_IP_ACQUIRE, lu->account, lu->loginIp, config.gameId, uid ))){
			ErrorCode = IP_SERVER_SOCKET_FAIL;
			LoginUser **tmp=0;			
			DelUserWait( uid, tmp );
		}
	}else
		ErrorCode = IP_ALREADY_WAIT;
//	AddUserWait가 실패하는 경우는 이미 무언가 되어 있는 것이므로 다시 시도하도록 한다. 
//  즉 DelWaitUser를 하고 다시 로그인 하게 된다. 
_AFTER_FIN
	return ErrorCode;
}

char CIPSessionDB::ReleaseSessionRequest(int IPSession, in_addr IP, int kind)
{
	char ErrorCode = 0;
_BEFORE
	if ( IPSession == 0 )
		return ErrorCode;

	if ( config.UseIPServer && (!IPServerReconnect) &&( pIPSocket != NULL) ){
		gIPLock.ReadLock();
		pIPSocket->Send( "cddddd", AI_IP_RELEASE, IPSession, pIPSocket->ConnectSessionKey, config.gameId, IP.S_un.S_addr, kind);
		gIPLock.ReadUnlock();
	}
_AFTER_FIN
	return ErrorCode;
}


char CIPSessionDB::AcquireSessionSuccess( int Uid, int IPSession, char ErrorCode, int SpecificTime, int Kind )
{
_BEFORE
	char rtCode;

	LoginUser *lu=NULL;
	DelUserWait( Uid, &lu );

	if ( lu == NULL ) {
		rtCode = IP_DB_ERROR;
		in_addr IP;
		IP.S_un.S_addr = 0;
		ReleaseSessionRequest( IPSession,  IP, Kind );
		return rtCode;
	}
	// 성공이면 사용자에게 로그인 성공 메세지를 보내도록 한다. 
	CSocketServerEx *pSocket = serverEx->FindSocket(lu->s);		
	if (!pSocket) {
		ReleaseSessionRequest( IPSession,  lu->loginIp, Kind );
		delete lu;
		return FALSE;
	}
	// 정상적으로 Acquire되었다면 등록한다. 
	// Acquire된 IP Kind를 Stat에 등록한 다음에 accountDB에 등록한다. 
	lu->stat = Kind;
	if ( accountdb.RegAccount( lu, Uid, pSocket, SpecificTime, 0 ) ){
		int result = AddSessionID( Uid, IPSession );
		// SessionID 저장을 실패했다면 과금이 불가능해 지기 때문에 모두 로그 아웃 처리한다. 
		if ( result == 0 ) {
			accountdb.logoutAccount( Uid ); 
			// AddSessionID가 실패 했기 때문에 accountdb내의 Release루틴은 이 작동하지 않기 때문에
			// 명시적으로 ReleaseSessionCall을 해주어야 한다. 
			ReleaseSessionRequest( IPSession,  lu->loginIp, Kind );
		}
	} else {
		ReleaseSessionRequest( IPSession,  lu->loginIp, Kind );
	}
	
	pSocket->ReleaseRef();
	delete lu;
_AFTER_FIN
	return ErrorCode;
}
char CIPSessionDB::AcquireSessionFail( int Uid, int IPSession, char ErrorCode )
{
	// IP Login이 성공했다면 사용자에게 메세지를 보내야 하겠지만
	// IP Login이 실패 했다고 해서 사용자에게 메세지를 보낼 필요는 없다. 
	// 현재의 결제 프로세스는 총 2가지로 나눈다. 
	// 개인정액 -> PC방정액 -> PC방 정량 -> 개인정량   또는
	// PC방정액->PC방정량->개인정액->개인정량의 순서이기 때문에
	// PC방은 실패해도 관계해도 굳이 사용자 클라이언트에 메세지를 보낼 필요 없다. 
	LoginUser *lu=NULL;
	DelUserWait( Uid, &lu );
	
	if ( lu == NULL ) {
		ErrorCode = IP_DB_ERROR;

		return ErrorCode;
	}
	
	// Stat을 검사해서 정량인지 판별한다. 
	// Session 획득에 실패 했다면 개인결제 체크에 들어가도록 한다. 
	
	CSocketServerEx *pSocket = serverEx->FindSocket(lu->s);		
	if (!pSocket) {
		delete lu;
		return ErrorCode;
	}

	if ( ErrorCode == IP_ALREADY_USE ) {
		char reasoncode = S_ALREADY_USED_IP;
		pSocket->Send( "cc",AC_ACCOUNT_KICKED, reasoncode);
		delete lu;
		pSocket->ReleaseRef();	
		return ErrorCode;
	}

	// 개인정액을 먼저 체크할 경우에는 이중이 되나 별로 상관 없다고 생각한다. 
	char result = accountdb.CheckPersonalPayStat( pSocket, lu, Uid );
	pSocket->ReleaseRef();	
	
	delete lu;

	return result;
}

static bool DummyPacket( CIPSocket *s, const unsigned char *packet )
{
	log.AddLog( LOG_WARN, "Call DummyPacket What What What" );
	return false;
}
static bool StartIPChargeFail( CIPSocket *s, const unsigned char *packet )
{
	//"cdcddds", IA_IP_START_FAIL, Uid, (char)ErrorCode, SessionID, g_SKey, IP, account );	
_BEFORE
	UINT uid=0;
	char ErrorCode=0;
	UINT sessionid = 0;
	UINT g_Skey = 0;
	char account[15];
	memset(account, 0, 15 );
	in_addr ip;

	uid		  = GetIntFromPacket( packet );
	ErrorCode = GetCharFromPacket( packet );
	sessionid = GetIntFromPacket( packet );
	g_Skey    = GetIntFromPacket( packet );
	ip		  = GetAddrFromPacket( packet );
	GetStrFromPacket( packet, 15, account );
	account[14]=0;
#ifdef _DEBUG
	log.AddLog( LOG_WARN, "Call StartIPChargeFail,%s,%d,%d.%d.%d.%d", account, ErrorCode, ip.S_un.S_un_b.s_b1, ip.S_un.S_un_b.s_b2, ip.S_un.S_un_b.s_b3,ip.S_un.S_un_b.s_b4);	
#endif
	if ( uid > 0 )
		accountdb.logoutAccount( uid );
_AFTER_FIN	
	return false;
}

static bool StartIPCharge( CIPSocket *s, const unsigned char *packet )
{
_BEFORE
#ifdef _DEBUG
	log.AddLog( LOG_WARN, "RCV: IA_IP_START_OK," );
#endif
	int Uid=0, SpecTime=0, kind=0, ip=0;
	Uid = GetIntFromPacket( packet );
	unsigned char tempWorldId = GetCharFromPacket( packet );	
	ServerId WorldID(tempWorldId);
	SpecTime = GetIntFromPacket( packet );
	kind     = GetIntFromPacket( packet );
	ip       = GetIntFromPacket( packet );
	char account[15];

	// accountdb에서 가져 와야 하는것..
	// account, loginflag, warnflag, md5key, socket
	int loginflag = 0, warnflag=0, md5key=0;
	SOCKET ps;
	bool result = accountdb.GetAccountInfo( Uid, account, &loginflag, &warnflag, &md5key, &ps );
	
	if ( !result )
		return false;

	account[14]=0;	
	CSocketServerEx *pSocket = serverEx->FindSocket(ps);		
	if ( pSocket ) {
		char ErrorCode = accountdb.AboutToPlay( Uid, account, SpecTime, loginflag, warnflag, md5key, pSocket, WorldID, kind, 0, 0, 0);
		pSocket->ReleaseRef();
		if ( ErrorCode != S_ALL_OK ){
			ipsessionDB.StopIPCharge( Uid, ip, kind, 0, time(0), WorldID, account );
		}
	}
_AFTER_FIN
	return false;
}

static bool GetIPAcquireSuccess( CIPSocket *s, const unsigned char *packet )
{
_BEFORE
	int  uid  = GetIntFromPacket( packet );
	char kind = GetCharFromPacket( packet );
	int  SpecTime = GetIntFromPacket( packet );
	int  SessionID = GetIntFromPacket( packet );

#ifdef _DEBUG
	log.AddLog( LOG_WARN, "IA_IP_USE_Success,uid:%d,kind:%d,SpecTime:%d,SessionID:%d", uid, kind, SpecTime, SessionID );
#endif
	ipsessionDB.AcquireSessionSuccess( uid, SessionID, 0, SpecTime, (int)kind );
_AFTER_FIN
	return false;
}

static bool GetIPAcquireFail( CIPSocket *s, const unsigned char *packet )
{
_BEFORE
	int  uid;
	char ErrorCode;

	uid = GetIntFromPacket( packet );
	ErrorCode = GetCharFromPacket( packet );
	
	if ( uid > 0 )
		ipsessionDB.AcquireSessionFail( uid, 0, ErrorCode );

#ifdef _DEBUG
	log.AddLog( LOG_WARN, "IA_IP_USE_FAIL,FAILCODE:%d,UID:%d",ErrorCode, uid );
#endif
_AFTER_FIN
	return false;
}

static bool GetConnectSessionKey( CIPSocket *s, const unsigned char *packet )
{
_BEFORE
	UINT SessionKey = (UINT)GetIntFromPacket( packet );
	s->SetConnectSessionKey( SessionKey );
	AS_LOG_VERBOSE( "IA_SERVER_VERSION,SessionKey %d", SessionKey );
_AFTER_FIN	
	return false;
}


static bool GetIPKick( CIPSocket *s, const unsigned char *packet )
{
_BEFORE
	in_addr ip;
	ip = GetAddrFromPacket( packet );
	int	  kind = GetIntFromPacket( packet );
	time_t   loginTime = GetIntFromPacket( packet );
	char account[16];
	memset( account, 0, 16 );
	GetStrFromPacket( packet, 16, account );
	unsigned int uid = GetIntFromPacket( packet );
	account[14]=0;
	accountdb.KickAccount( uid, S_ALREADY_USED_IP, true );
	AS_LOG_VERBOSE( "IA_IP_KICK, %s, %d.%d.%d.%d", account, ip.S_un.S_un_b.s_b1,ip.S_un.S_un_b.s_b2,ip.S_un.S_un_b.s_b3,ip.S_un.S_un_b.s_b4 );
_AFTER_FIN

	return false;
}

static bool ReadyIPOK( CIPSocket *s, const unsigned char *packet )
{
_BEFORE
#ifdef _DEBUG
	log.AddLog( LOG_WARN, "RCV: IA_IP_READY_OK," );
#endif
	int Uid=0, SpecTime=0, kind=0, ip=0;
	Uid = GetIntFromPacket( packet );
	unsigned char tempWorldID = GetCharFromPacket( packet );
	ServerId WorldID(tempWorldID);
	SpecTime = GetIntFromPacket( packet );
	kind     = GetIntFromPacket( packet );
	ip       = GetIntFromPacket( packet );
	char account[15];
	memset( account, 0, 15 );
	// accountdb에서 가져 와야 하는것..
	// account, loginflag, warnflag, md5key, socket
	int loginflag = 0, warnflag=0, md5key=0;
	SOCKET ps;
	bool result = accountdb.GetAccountInfo( Uid, account, &loginflag, &warnflag, &md5key, &ps );
	if ( !result )
		return false;

	account[14]=0;	
	CSocketServerEx *pSocket = serverEx->FindSocket(ps);		
	if ( pSocket ) {
		char ErrorCode = accountdb.AboutToPlay( Uid, account, SpecTime, loginflag, warnflag, md5key, pSocket, WorldID, kind, 0, 0, 0);
		pSocket->ReleaseRef();
		if ( ErrorCode != S_ALL_OK ){
			ipsessionDB.StopIPCharge( Uid, ip, kind, 0, time(0), WorldID, account );
		}
	}
_AFTER_FIN
	return false;
}
// ReadyIPFail은 Acquire가 된 다음에 사용되는 것이다.  그렇기 때문에 AcquireSessionFail과는 틀리다.
// logout을 불러줄수 있다면 그게 좋다.
static bool ReadyIPFail( CIPSocket *s, const unsigned char *packet )
{
_BEFORE
	int  uid=0;
	char ErrorCode;

	uid = GetIntFromPacket( packet );
	ErrorCode = GetCharFromPacket( packet );
	AS_LOG_VERBOSE( "Get IA_IP_READY_FAIL, uid %d", uid );
	if ( uid > 0 )
		accountdb.logoutAccount( uid );

_AFTER_FIN
	return false;
}

static bool SetStartTimeFail( CIPSocket *s, const unsigned char *packet )
{
_BEFORE
	int uid = 0;
	
	uid = GetIntFromPacket( packet);
	if ( uid > 0 )
		accountdb.logoutAccount( uid );
_AFTER_FIN
	return false;
}

static IPPacketFunc IPPacketFuncTable[] = {
	GetConnectSessionKey, // 0
	DummyPacket, // 1
	GetIPAcquireSuccess, //2
	StartIPCharge, //3 
	StartIPChargeFail,//4
	GetIPAcquireFail, //5
	DummyPacket, //6
	DummyPacket, //7
	DummyPacket, //8
	GetIPKick, //9
	ReadyIPFail, //10
	ReadyIPOK, // 11
	DummyPacket,//IA_IP_SET_STARTTIME_OK, //12
	SetStartTimeFail,//IA_IP_SET_STARTTIME_FAIL, //13
};

CIPSocket *CIPSocket::Allocate( SOCKET s )
{
	return new CIPSocket( s );
}

CIPSocket::CIPSocket( SOCKET aSoc )
: CIOSocket( aSoc )
{
	addr = config.IPServer;
	host = 0;
	mode = SM_READ_LEN;
	packetTable =IPPacketFuncTable;
	opFlag = 0;
	Destination.sin_family = AF_INET;
	Destination.sin_addr   = config.IPServer;
	Destination.sin_port   = htons( (u_short)config.IPPort );
	ConnectSessionKey = 0;
	IPServerReconnect = false;
}

CIPSocket::~CIPSocket()
{
//	if( reconnect == true )
//		log.AddLog( LOG_ERROR, "Reconnected set" );
	log.AddLog( LOG_ERROR, "IPSocket Deleted" );
}

void CIPSocket::OnClose(SOCKET closedSocket)
{
	// Must not use the GetSocket() function from within
 	// this function!  Instead, use the socket argument
 	// passed into the function.

	mode = SM_CLOSE;
	IPServerReconnect = true;
	config.UseIPServer = false;

	log.AddLog(LOG_ERROR, "*close connection IPServer from %s, %x(%x)", IP(), closedSocket, this);
/*
	if ( !g_bTerminating ) {
		EnterCriticalSection( &m_cs );
		if ( !reconnect ){
			RegisterTimer( 30000, true );
			reconnect = true;
			config.UseIPServer = false;
		}
		LeaveCriticalSection( &m_cs );
	}
*/
	ipsessionDB.DellAllWaitingSessionID();
	AddRef();
	CreateTimerQueueTimer( &g_hIPServerTimer, NULL, IPSocketTimerRoutine, this, config.IPConnectInterval, 0, 0 );
}

void CIPSocket::OnTimerCallback( void )
{
/*
	log.AddLog( LOG_WARN, "Timer Callback Reconnect IPServer Timer Called" );
	if ( g_bTerminating )
	{
		ReleaseRef();
		return ;
	}

	EnterCriticalSection( &m_cs );
	if ( !reconnect ){
		LeaveCriticalSection( &m_cs );
		return ;
	}
	if ( m_hSocket != INVALID_SOCKET ){
		closesocket( m_hSocket );
		m_hSocket = INVALID_SOCKET;
	}
	m_hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_hSocket == INVALID_SOCKET) {
		log.AddLog(LOG_ERROR, "socket error %d", WSAGetLastError());
		RegisterTimer( 30000, true );
		LeaveCriticalSection( &m_cs );
		ReleaseRef();
		return ;
	}

	int ErrorCode = connect( m_hSocket, ( sockaddr *)(&Destination), sizeof( sockaddr ));
	
	if ( ErrorCode == SOCKET_ERROR ){
		closesocket( m_hSocket );
		RegisterTimer( 30000, true );
		LeaveCriticalSection( &m_cs );
		ReleaseRef();
		return ;
	} else {
		reconnect = false;
		mode = SM_READ_LEN;
		LeaveCriticalSection( &m_cs );
		Initialize( g_hIOCompletionPort );
		config.UseIPServer = true;
		ReleaseRef();
		return ;
	}
*/
}

const char *CIPSocket::IP()
{
	return inet_ntoa(addr);
}

void CIPSocket::OnCreate()
{
	AddRef();
	OnRead();
	Send( "cdc", AI_SERVER_VERSION, buildNumber, config.gameId );
}

void CIPSocket::OnRead()
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

				if (inBuf[pi] >= IA_MAX) {
					log.AddLog(LOG_ERROR, "unknown protocol %d", inBuf[pi]);
					break;
				} else {
					CIPPacketServer *pPacket = CIPPacketServer::Alloc();
					pPacket->m_pSocket = this;
					pPacket->m_pBuf = m_pReadBuf;
					pPacket->m_pFunc = (CIPPacketServer::IPPacketFunc) packetTable[inBuf[pi]];
					CIOSocket::AddRef();
					m_pReadBuf->AddRef();
					InterlockedIncrement(&CIPPacketServer::g_nPendingPacket);
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


bool CIPSocket::Send(const char* format, ...)
{
	AddRef();
	if (mode == SM_CLOSE || IPServerReconnect || !config.UseIPServer ) {
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

char CIPSessionDB::StartIPCharge(UINT uid, UINT ip, int kind, ServerId WorldID)
{
//  과금을 위해 IPServer에 보내야 하는 것은 다음과 같다. 
//	UINT SessionID	//	UINT g_SKey		//	int	Uid		//	char WorldID //	int IP	//	int     Kind		

	char ErrorCode=IP_ALL_OK;
_BEFORE
	UINT IPSessionID = FindSessionID( uid );

	if ( pIPSocket == NULL || IPServerReconnect || !config.UseIPServer || (IPSessionID == 0) )
		return IP_SERVER_SOCKET_FAIL;
	
	gIPLock.ReadLock();
	bool result = pIPSocket->Send( "cdddcdd", AI_IP_START_CHARGE, 
											  IPSessionID, 
											  pIPSocket->ConnectSessionKey,
											  uid,
											  WorldID,
											  ip,
											  kind);
	gIPLock.ReadUnlock();
	if ( !result )
		ErrorCode = IP_SERVER_SOCKET_FAIL;
_AFTER_FIN	
	return ErrorCode;
}


char CIPSessionDB::StopIPCharge(UINT uid, UINT ip, int kind, int UseTime, time_t loginTime, ServerId lastworld, const char *account)
{
	char ErrorCode=IP_ALL_OK;
_BEFORE
	int SessionID = FindSessionID( uid );
	bool result = true;
	if ( SessionID == 0 )
		return IP_SERVER_SOCKET_FAIL;

	if ( (config.UseIPServer) && (SessionID) != 0 && (pIPSocket!=NULL) && ( !IPServerReconnect)){
		gIPLock.ReadLock();
		result = pIPSocket->Send( "cddddcddsd", AI_IP_STOP_CHARGE, 
									   pIPSocket->ConnectSessionKey, 
									   SessionID,
									   ip,
									   kind,
									   lastworld,
									   UseTime,
									   loginTime, 
									   account,
									   config.gameId );
		gIPLock.ReadUnlock();
	}
	if (!result)
		ErrorCode = IP_SERVER_SOCKET_FAIL;
_AFTER_FIN
	
	return ErrorCode;
}

char CIPSessionDB::ReadyToIPCharge(UINT uid, UINT ip, int kind, ServerId WorldID)
{
//  과금을 위해 IPServer에 보내야 하는 것은 다음과 같다. 
//	UINT SessionID	//	UINT g_SKey		//	int	Uid		//	char WorldID //	int IP	//	int     Kind		
	char ErrorCode=IP_ALL_OK;
_BEFORE

	UINT IPSessionID = FindSessionID( uid );

	if ( pIPSocket == NULL || IPServerReconnect || !config.UseIPServer || (IPSessionID == 0) )
		return IP_SERVER_SOCKET_FAIL;
	
	gIPLock.ReadLock();
	bool result = pIPSocket->Send( "cdddcdd", AI_IP_READY_GAME, 
											  IPSessionID, 
											  pIPSocket->ConnectSessionKey,
											  uid,
											  WorldID,
											  ip,
											  kind);
	gIPLock.ReadUnlock();

	if ( !result )
		ErrorCode = IP_SERVER_SOCKET_FAIL;
_AFTER_FIN	
	return ErrorCode;
}

char CIPSessionDB::ConfirmIPCharge( UINT uid, UINT ip, int kind, ServerId WorldID )
{
	char ErrorCode = IP_ALL_OK;
_BEFORE
	UINT IPSessionID = FindSessionID(uid);

	if ( pIPSocket == NULL || IPServerReconnect || !config.UseIPServer || (IPSessionID == 0) )
		return IP_SERVER_SOCKET_FAIL;

	gIPLock.ReadLock();
	bool result = pIPSocket->Send( "cdddcdd", AI_IP_SET_START_TIME, 
											  IPSessionID,
											  pIPSocket->ConnectSessionKey, 
											  uid, 
											  WorldID, 
											  ip,
											  kind );
	gIPLock.ReadUnlock();

	if (!result)
		ErrorCode = IP_SERVER_SOCKET_FAIL;
_AFTER_FIN
	return ErrorCode;
}