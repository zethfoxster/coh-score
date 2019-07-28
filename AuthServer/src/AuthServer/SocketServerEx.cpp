#include "GlobalAuth.h"
#include "Packet.h"
#include "IOSocket.h"
#include "buildn.h"
#include "protocol.h"
#include "DBConn.h"
#include "account.h"
#include "util.h"
#include "des.h"
#include "ServerList.h"
#include "Thread.h"
#include "AccountDB.h"
#include "IOServer.h"
#include "config.h"
#include "ipsessiondb.h"
#include "iplist.h"
AccountDB accountdb;

BOOL SendSocket(in_addr , const char *format, ...);

#include "../../../3rdparty/cryptopp/rng.h"
#include "../../../3rdparty/cryptopp/rsa.h"

using namespace CryptoPP;
using CryptoPP::InvertibleRSAFunction;
using CryptoPP::RSAES_OAEP_SHA_Encryptor;
using CryptoPP::RSAES_OAEP_SHA_Decryptor;

struct RSA
{
	typedef RSAFunction PublicKey;
	typedef InvertibleRSAFunction PrivateKey;
};
static InvertibleRSAFunction *RSA_Params = NULL;

void InitRSAParams()
{
	LC_RNG rng(time(NULL));
	RSA_Params = new InvertibleRSAFunction(rng, 1152);
}

void DecryptRSAText(char*ciphertextstr, int bufferLen)
{
	RSA::PrivateKey privateKey(*RSA_Params);

	__declspec(thread) static RSAES_OAEP_SHA_Decryptor *decryptor = NULL;
	if (!decryptor)
		decryptor = new RSAES_OAEP_SHA_Decryptor( privateKey );

	////////////////////////////////////////////////
	// Decryption
	CryptoPP::SecByteBlock ciphertext((byte*)ciphertextstr, bufferLen);
	// Now that there is a concrete object, we can check sizes
	assert( 0 != decryptor->CipherTextLength() );
	assert( ciphertext.Size() <= decryptor->CipherTextLength() );

	// Create recovered text space
	unsigned int dpl = decryptor->MaxPlainTextLength();
	assert( 0 != dpl );
	CryptoPP::SecByteBlock recovered( dpl );

	decryptor->Decrypt( ciphertext, recovered);
	memcpy(ciphertextstr, recovered.Begin(), recovered.Size());
}


static bool LoginPacketSecure(CSocketServerEx *mysocket, const unsigned char *packet, bool useMD5)
{
	CAccount account;
	unsigned char RSAblock[512];
	int  subscription;
	char name[15];
	char passwordLineage2[MAX_PWD_LEN+1];
	char passwordSHA512[ENC_PWD_LEN+1];
	char  err_msg = 0;
	unsigned short cdkind=0;
	
	// 2003-11-25 by darkangel
	// 로그인 금지된 IP  List안에 포함되어 있을 경우.
	// 경고 메세지도 없이 서버와의 연결이 무조건 끊긴다.
	in_addr cip = mysocket->getaddr();
	
	if ( config.useForbiddenIPList ) {
		if ( forbiddenIPList.IpExists( cip ) ){
			mysocket->Send( "cc", AC_LOGIN_FAIL, S_BLOCKED_IP );
			return false;
		}
	}

	mysocket->m_lastIO = GetTickCount();

	memset( name, 0, 15 );
	memset( passwordLineage2, 0, MAX_PWD_LEN+1 );
	memset( passwordSHA512, 0, ENC_PWD_LEN+1 );

	if ( config.DesApply ){
		int size = GetIntFromPacket( packet );
		if (size<0 || size>sizeof(RSAblock))
		{
			log.AddLog( LOG_ERROR, "SND: AC_LOGIN_FAIL,invalid_key_size" );
			mysocket->Send( "cc", AC_LOGIN_FAIL, S_INCORRECT_MD5Key );
			return false;
		}
		memcpy( RSAblock, packet, size );
		DecryptRSAText((char*)RSAblock, size);
		strncpy( name, (char *)RSAblock, 14);
		memcpy( passwordLineage2, (char *)RSAblock+14, 16);
		memcpy( passwordSHA512, (char *)RSAblock+30, 64);

		packet += size;
	} 
	else
	{
		assert(0);	//	not supported anymore
	}
	subscription = GetIntFromPacket( packet );
	cdkind		 = GetShortFromPacket( packet );
	
    log.AddLog( LOG_VERBOSE, "RCV: AQ_LOGIN,account:%s,ip:%d.%d.%d.%d,subscription:%d,cdkind:%d", name, cip.S_un.S_un_b.s_b1, cip.S_un.S_un_b.s_b2, cip.S_un.S_un_b.s_b3, cip.S_un.S_un_b.s_b4, subscription, cdkind);
	log.AddLog( LOG_DEBUG, "RCV: AQ_LOGIN,useMD5:%i, MD5Key %i", useMD5, mysocket->GetMd5Key());

	err_msg = account.CheckPassword( name, passwordLineage2, passwordSHA512, mysocket->GetMd5Key(), useMD5 );

	if ( err_msg != S_ALL_OK )
	{
#ifdef _DEBUG
		if ( config.Country == CC_KOREA ) {
			log.AddLog( LOG_ERROR, "계정 %s 암호 체크에 실패 했습니다.  MSG: %d", name, err_msg );
		} else
			log.AddLog( LOG_ERROR, "SND: AC_LOGIN_FAIL,Error Msg: %d", err_msg );
#endif
		mysocket->Send( "cc", AC_LOGIN_FAIL, err_msg );
		return false;
	}

	if ( account.block_flag2 != 0 ){
		AS_LOG_VERBOSE( "SND: AC_BLOCKED_ACCOUNT,reason1:%d", account.block_flag2 );
		mysocket->Send( "cdd", AC_BLOCKED_ACCOUNT,account.block_flag2, account.block_end_date );		
		return false;
	}

	if ( account.block_flag != 0 ){
		char bmsg[4096];
		memset(bmsg, 0, 4096);
		int len=account.MakeBlockInfo( bmsg+3 );
		int len2 = len+config.PacketSizeType;
		bmsg[0] = len2;
		bmsg[1] = len2>>8;
		bmsg[2] = AC_BLOCKED_ACCOUNT_WITH_MSG;
		AS_LOG_VERBOSE( "SND: AC_BLOCKED_ACCOUNT_WITH_MSG;,block_flag:%d",account.block_flag);
		mysocket->Send( bmsg, 3+len);
		return false;
	}
	
	if ( account.login_flag & 16 ) {
		if ( config.RestrictGMIP ) {
			if( (mysocket->getaddr()).S_un.S_addr != (config.GMIP).S_un.S_addr ){
				in_addr loginaddr = mysocket->getaddr();
				CDBConn conn(g_linDB);
				conn.Execute( "INSERT gm_illegal_login ( account, ip ) VALUES ( '%s', '%d.%d.%d.%d' )", 
					account.account, loginaddr.S_un.S_un_b.s_b1, loginaddr.S_un.S_un_b.s_b2, loginaddr.S_un.S_un_b.s_b3, loginaddr.S_un.S_un_b.s_b4 );
				mysocket->Send( "cc", AC_LOGIN_FAIL, S_SYSYTEM_ERROR );
				return false;
			}
		}
	}

	if ( config.FreeServer ) {
		if ( account.pay_stat != 0 )
			account.pay_stat = 1002;
	}

	int isRetail = 0;
	if (config.UserData)
	{
		unsigned char userdata[MAX_USERDATA];
		
		memset( userdata, 0, MAX_USERDATA );

		{
		CDBConn dbconn(g_linDB);
		SQLINTEGER UserInd=0;
		SQLBindCol( dbconn.m_stmt, 1, SQL_C_BINARY, (char *)(userdata), MAX_USERDATA_ORIG, &UserInd );

		SQLINTEGER UserIndNew=0;
		SQLBindCol( dbconn.m_stmt, 2, SQL_C_BINARY, (char *)(&userdata[MAX_USERDATA_ORIG]), MAX_USERDATA_NEW, &UserIndNew );

		SQLINTEGER cbUid=0;
		SQLBindParameter( dbconn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&account.uid), 0, &cbUid );

		dbconn.Execute( "SELECT user_data, user_data_new FROM user_data WHERE uid = ?" );
		dbconn.Fetch();
		}

		isRetail = userdata[12] & 0x10;
	}

	LoginUser *loginuser = new LoginUser;
	loginuser->loginIp   = mysocket->getaddr();
	loginuser->md5key    = mysocket->GetMd5Key();
	loginuser->stat      = isRetail && config.IsReactivationActive() ? config.ReactivationValue : account.pay_stat;
	loginuser->s         = mysocket->GetSocket();
	loginuser->gender    = account.gender;
	loginuser->timerHandle = NULL;
	loginuser->um_mode   = UM_LOGIN;
	loginuser->ssn       = account.nSSN;
	loginuser->ssn2      = account.ssn2;
	loginuser->loginflag = account.login_flag;
	loginuser->warnflag  = account.warn_flag;
	loginuser->age       = account.age;
	loginuser->cdkind	 = cdkind;
	strncpy( loginuser->account, account.account, MAX_ACCOUNT_LEN+1 );
	loginuser->account[MAX_ACCOUNT_LEN]=0;
	loginuser->lastworld = account.lastworld;
	loginuser->logintime = time(NULL);
	loginuser->queuetime = time(NULL);
	loginuser->serverid.SetInvalid();
	loginuser->selectedServerid.SetInvalid();
	loginuser->queueLevel = account.queueLevel;
	loginuser->loyalty = account.loyalty;
	loginuser->loyaltyLegacy = account.loyaltyLegacy;
    memcpy(loginuser->regions, account.regions, sizeof(account.regions));
	
	mysocket->uid = account.uid;
	// 무료 기간으로 세팅할 경우이다.
	int OperationCode = (int)(( account.pay_stat% 1000 ) / 100 );

	if ( config.UseIPServer){

		int result=S_ALL_OK;
	
		if ( config.PCCafeFirst )
		{
			result = ipsessionDB.AcquireSessionRequest( loginuser, account.uid );
			switch ( result){
			case IP_ALREADY_WAIT:
				mysocket->Send( "cc", AC_LOGIN_FAIL, S_ALREADY_LOGIN );
				delete loginuser;
				break;
			case IP_ALL_OK:
				break;
			default:
				accountdb.CheckPersonalPayStat( mysocket, loginuser, account.uid );
				delete loginuser;
				break;
			}

			return false;
		}
		
		if ( account.pay_stat == 0 || OperationCode == PERSONAL_SPECIFIC){
			result= ipsessionDB.AcquireSessionRequest( loginuser, account.uid ) ;
			switch ( result){
			case IP_ALREADY_WAIT:
				mysocket->Send( "cc", AC_LOGIN_FAIL, S_ALREADY_USED_IP );
				delete loginuser;
				break;
			case IP_ALL_OK:
				break;
			default:
				accountdb.CheckPersonalPayStat( mysocket, loginuser, account.uid );
				delete loginuser;
				break;
			}

			return false;
		}

	};
	
	accountdb.CheckPersonalPayStat( mysocket, loginuser, account.uid );

	delete loginuser;
	return false;
}

static bool LoginPacket(CSocketServerEx *mysocket, const unsigned char *packet)
{
	return LoginPacketSecure(mysocket, packet, false);
}

static bool SelectServerPacket( CSocketServerEx *pSocket, const unsigned char *packet)
{
	int uid, md5key, loginflag=0, warnflag=0;
	char account[MAX_ACCOUNT_LEN+1];
_BEFORE
	memset(account, 0, MAX_ACCOUNT_LEN+1);

	uid    = GetIntFromPacket( packet );
	md5key = GetIntFromPacket( packet );
	unsigned char temp = GetCharFromPacket( packet );
	ServerId serverid(temp);

	AS_LOG_VERBOSE( "RCV: AQ_ABOUT_TO_PLAY,uid:%d,serverid:%d", uid, serverid.GetValueChar() );

	int pay_stat=0;
	int pKey=0;
	int queueLevel=0;
	int loyalty = 0;
	int loyaltyLegacy = 0;
	pSocket->m_lastIO = GetTickCount();
	
	if ( !accountdb.FindAccount( uid , account, &loginflag, &warnflag, &pay_stat, &pKey, &queueLevel, &loyalty, &loyaltyLegacy ) ) {

		AS_LOG_VERBOSE( "SND: AC_PLAY_FAIL,NO Login Info");
		pSocket->Send( "cc", AC_PLAY_FAIL, S_NO_LOGININFO );		
		return false;		
	}
	if ( pKey != md5key ){
		pSocket->Send( "cc", AC_PLAY_FAIL, S_NO_LOGININFO );		
		log.AddLog( LOG_ERROR, "md5key does not matching" );
		return false;
	}

	StdAccount( account );

	if ( !g_ServerList.IsServerUp(serverid))
	{
		AS_LOG_VERBOSE( "SND: AC_PLAY_FAIL,server down : invalid serverid", S_SERVER_DOWN );
		pSocket->Send( "cc", AC_PLAY_FAIL, S_SERVER_DOWN );		
		return false;
	}
	
	/******************************************************************************************/
	// 여기 까지는 일단 정량 정액, PC방 모두 같은 로직이기 때문에 일단은 통과한다. 
	// 이제는 정량및 PC방 시간 체크 로직이 들어가야 한다. 
	// 이 밑의 로직은 일반적인 정액 사용자를 위한 로직이다. 
	// 일단 PayStat을 확인해야 한다. 

	int OperationCode = (int)(( pay_stat% 1000 ) / 100 );	
	int RemainTime=0;
	char RetCode = S_ALL_OK;

	if ( OperationCode== 0 ){
	// 정액일 경우부터 별도의 체크를 할것인지 생각한다. ( 추후 구현 )
	} else if ( OperationCode == PERSONAL_SPECIFIC ) {
		RetCode= accountdb.CheckUserTime( uid ,&RemainTime );
	}

	if ( RetCode != S_ALL_OK ){
		pSocket->Send( "cc", AC_PLAY_FAIL, RetCode );
		return false;
	}

	if ( g_ServerList.IsServerVIPonly(serverid))
	{
		int payment_plan_id = pay_stat % 1000;
		if (payment_plan_id == 0 || payment_plan_id == 11 || payment_plan_id == 15 || payment_plan_id == 16)
		{
			if (loyalty <= 1)
			{
				pSocket->Send( "cc", AC_PLAY_FAIL, S_VIP_ONLY );
				return false;
			}
		}
	}

	// 이 아래부분은 PC방도 똑같은 형태로 진행되어야 하기 때문에 따로 뽑아 내도록 하자. 
	// 모든 체크를 통과했으면 정상적인 프로세스로 진행하도록 한다. 
	// 인수는 uid, serverid, account, time_left, loginflag, warnflag, md5key, socket->m_hSocket,RemainTime
	pSocket->AddRef();
	accountdb.AboutToPlay( uid, account, RemainTime, loginflag, warnflag, md5key, pSocket, serverid, pay_stat, queueLevel, loyalty, loyaltyLegacy);
	pSocket->ReleaseRef();
_AFTER_FIN
	return false;
}
static bool LogoutPacket( CSocketServerEx *mysocket, const unsigned char *packet)
{
	int uid = GetIntFromPacket( packet );
	int md5key = GetIntFromPacket( packet );

	if ( !accountdb.logoutAccount( uid, md5key ) )
	{
		AS_LOG_DEBUG( "md5key does not match is it hack??");
		return false;
	}
	AS_LOG_VERBOSE( "AQ_LOGOUT, uid %d, md5key %d", uid, md5key);
	return true;

}

static bool DeprecatedPacket(CSocketServerEx * /*mysocket*/, const unsigned char * /*packet*/)
{
	log.AddLog( LOG_ERROR, "Received a deprecated packet from client connection.");
	return true;
}

static bool ServerListExtPacket(CSocketServerEx *mysocket, const unsigned char *packet)
{
	int  uid    = GetIntFromPacket( packet );
	UINT md5key = GetIntFromPacket( packet );
	char ListKind = GetCharFromPacket( packet );
	
	if ( ListKind !=1)
	{
		mysocket->Send( "cc", AC_SEND_SERVER_FAIL, S_NO_SERVERLIST );
		return false;
	}

	char account[MAX_ACCOUNT_LEN+1];

	AS_LOG_VERBOSE( "RCV: AQ_SERVER_LIST_EXT,uid:%d, md5key:%d,ListKind:%d",uid, md5key, ListKind );
	
	mysocket->uid = uid;
	ServerId serverid;
    int regions[MAX_REGIONS];
	bool result = accountdb.FindAccount( uid , account, serverid, regions );	
	if ( !result)
	{
		return true;
	}
	mysocket->m_lastIO = GetTickCount();

	std::vector<char> message;
	message.reserve(2048);
	g_ServerList.MakeServerListPacket(message, serverid, regions);

	mysocket->Send( &(message[0]), (int)message.size() );

	if (config.useQueue)
	{
		message.clear();
		g_ServerList.MakeQueueSizePacket(message);
		mysocket->Send( &(message[0]), (int)message.size() );
	}

	return false;
}


BOOL SendSocket( in_addr ina, const char *format, ...)
{
_BEFORE
	CSocketServer *pSocket = server->FindSocket(ina);
	if (!pSocket)
		return FALSE;
	CIOBuffer *pBuffer = CIOBuffer::Alloc();
	char *buffer = pBuffer->m_buffer;
	va_list ap;
	va_start(ap, format);
	int len = Assemble(buffer + 2, BUFFER_SIZE - 2, format, ap);
	va_end(ap);
	if (len == 0) {
		log.AddLog(LOG_ERROR, "%d: assemble too large packet. format %s", pSocket->GetSocket(), format);
	} else {
		len -= 1;
		len = len + config.PacketSizeType;
		buffer[0] = len;
		buffer[1] = len >> 8;
	}
	pBuffer->m_size = len+3-config.PacketSizeType;
	pSocket->Write(pBuffer);
	pSocket->ReleaseRef();
_AFTER_FIN
	return TRUE;
}

// MD5Key를 적용해서 처리합니다. 아직 미구현.
static bool LoginPacketMd5Key( CSocketServerEx *pSocket, const unsigned char *packet)
{
	return LoginPacketSecure(pSocket, packet, true);
}

static ServerPacketFuncEx ServerPacketEx[] = {
	LoginPacket,
	DeprecatedPacket,
	SelectServerPacket,
	LogoutPacket,
	LoginPacketMd5Key,
	ServerListExtPacket, 
};


VOID CALLBACK SocketExTimerCallback(PVOID lpParam, unsigned char TimerOrWaitFired)
{
	SOCKET s = (SOCKET)lpParam;
	CSocketServerEx *mysocket = serverEx->FindSocket( s );
	if ( mysocket )
		mysocket->OnTimerCallback( );
}

CSocketServerEx::CSocketServerEx( SOCKET aSoc )
: CIOSocket( aSoc )
{
	addr.s_addr = 0;
	host = 0;
	mode = SM_READ_LEN;
	packetTable =ServerPacketEx;
	um_mode = UM_PRE_LOGIN;
	uid = 0;
	m_TimerHandle = NULL;
	int zero = 0;
	

	// 처음
	m_lastIO = GetTickCount();
	InterlockedIncrement( &(reporter.m_SocketCount ));
//	InterlockedIncrement( &(reporter.m_SocketEXObjectCount ));
//  흠.. 뭐 대충 8000명은 확실히 버티니까 3분..-.-
	int ret = CreateTimerQueueTimer( &m_TimerHandle, g_hSocketTimer,SocketExTimerCallback, (LPVOID)GetSocket(), config.SocketTimeOut, config.SocketTimeOut , 0 );
//	int ret = RegisterTimer( (config.SocketTimeOut), TRUE );
	if ( ret == 0 ) {
		log.AddLog(LOG_ERROR, "create socket timer error" );
	}
	if ( config.ProtocolVer == USA_AUTH_PROTOCOL_VERSION )
	{
		EncPacket = Encrypt;
		DecPacket = Decrypt;
	} else if ( config.ProtocolVer >= GLOBAL_AUTH_PROTOCOL_VERSION ) {
		EncPacket = BlowFishEncryptPacket;
		DecPacket = BlowFishDecryptPacket;
	}
}

CSocketServerEx::~CSocketServerEx()
{
//	InterlockedDecrement( &(reporter.m_SocketEXObjectCount ));
}

CSocketServerEx *CSocketServerEx::Allocate( SOCKET s )
{
	return new CSocketServerEx( s );
}

void CSocketServerEx::OnClose(SOCKET closedSocket)
{
	// Must not use the GetSocket() function from within
 	// this function!  Instead, use the socket argument
 	// passed into the function.
	HANDLE timerhandle = NULL;
	#pragma warning( push )
	#pragma warning( disable: 4312 )
	timerhandle = InterlockedExchangePointer( &m_TimerHandle, NULL );
	#pragma warning( pop )
	
	if ( timerhandle )
		DeleteTimerQueueTimer( g_hSocketTimer, timerhandle, NULL );
	m_TimerHandle = NULL;
	mode = SM_CLOSE;
	InterlockedDecrement( &(reporter.m_SocketCount ));
	serverEx->RemoveSocket(closedSocket);
	log.AddLog(LOG_DEBUG, "*close connection from %s, %x(%x)", IP(), closedSocket, this);
	if ( um_mode != UM_PLAY_OK && uid != 0 )
		accountdb.removeAccountPreLogIn( uid, closedSocket );
	else
		accountdb.UpdateSocket( uid, INVALID_SOCKET, oneTimeKey, ServerId::s_invalid );

	IPaccessLimit.DelAccessIP( getaddr() );
}

void CSocketServerEx::OnRead()
{
	int pi = 0;
	int ri = m_pReadBuf->m_size;
	bool result;
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
				packetLen = inBuf[pi] + (inBuf[pi + 1] << 8) + 1 - config.PacketSizeType;
				if (packetLen <= 0 || packetLen > BUFFER_SIZE) {
					log.AddLog(LOG_ERROR, "%d: bad packet size %d", m_hSocket, packetLen);
					break;
				} else {
/////////////////////////////
//decode routine need
/////////////////////////////
					log.AddLog( LOG_DEBUG, "Received %d bytes DecKey 0x87546CA1, 0x%x", packetLen+2, DecOneTimeKey);
					pi += 2;
					mode = SM_READ_BODY;
				}
			} else {
				Read(ri - pi);
				return;
			}
		} else if (mode == SM_READ_BODY) {
			if (pi + packetLen <= ri) {
				
				result = DecPacket(inBuf+2, DecOneTimeKey , packetLen);
				if ( !result ){
#ifdef _DEBUG
					log.AddLog( LOG_ERROR, "unknown protocol,decrypt error" );
#endif
					break;
				}
#ifdef _DEBUG
				DumpPacket( inBuf, packetLen+2 );
#endif
				if (inBuf[pi] >= AQ_MAX) {
					log.AddLog(LOG_ERROR, "unknown protocol %d", inBuf[pi]);
					break;
				} else {
					CPacketServerEx *pPacket = CPacketServerEx::Alloc();
					pPacket->m_pSocket = this;
					pPacket->m_pBuf = m_pReadBuf;
					pPacket->m_pFunc = (CPacketServerEx::ServerPacketFuncEx) ServerPacketEx[inBuf[pi]];
					CIOSocket::AddRef();
					m_pReadBuf->AddRef();
					InterlockedIncrement(&CPacketServerEx::g_nPendingPacket);
					pPacket->PostObject(pi, g_hIOCompletionPort);
					pi += packetLen;
					mode = SM_READ_LEN;
					m_lastIO = GetTickCount();
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

const char *CSocketServerEx::IP()
{
	return inet_ntoa(addr);
}

void CSocketServerEx::OnCreate()
{
	static byte *modBuffer = NULL;
	static byte *expBuffer = NULL;
	static int modByteCount;
	static int expByteCount;

	if (!modBuffer) {
		modByteCount = RSA_Params->GetModulus().MinEncodedSize();
		expByteCount = RSA_Params->GetExponent().MinEncodedSize();
		modBuffer = new byte[modByteCount];
		expBuffer = new byte[expByteCount];
		RSA_Params->GetModulus().Encode(modBuffer, modByteCount);
		RSA_Params->GetExponent().Encode(expBuffer, expByteCount);
	}

	oneTimeKey = rand() << 16 | rand();

	EncOneTimeKey = oneTimeKey | PRIVATE_KEY;
	DecOneTimeKey = oneTimeKey | PRIVATE_KEY;

	if ( config.encrypt == true)
		Send("cdddbdb", AC_PROTOCOL_VER, oneTimeKey, config.ProtocolVer, 
			expByteCount, expByteCount, expBuffer, modByteCount, modByteCount, modBuffer);
	else{
		NonEncSend("cdddbdb", AC_PROTOCOL_VER, oneTimeKey, 0, 
			expByteCount, expByteCount, expBuffer, modByteCount, modByteCount, modBuffer);
	}
	AS_LOG_VERBOSE( "SND: AC_PROTOCOL_VER,onetimekey:%d,buildNumber:%d", oneTimeKey, buildNumber );
	OnRead();
}

void CSocketServerEx::SetAddress(in_addr inADDR )
{
	addr = inADDR;
}

void CSocketServerEx::Send(const char* format, ...)
{
	if (mode == SM_CLOSE) {
		return;
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
		// 길이에서 type 부분을 제외한다. 
		// sizeType에 따라 Length를 조절한다. 

		len = len -1 + config.PacketSizeType;
		
		log.AddLog( LOG_DEBUG, "Sent %d bytes", len+3-config.PacketSizeType);
		int tmplen = 	len+1-config.PacketSizeType;
		EncPacket( (unsigned char *)(buffer+2), EncOneTimeKey, tmplen );
#ifdef _DEBUG
		DumpPacket( (unsigned char*)buffer, len+3-config.PacketSizeType );
#endif
		len = tmplen - 1 + config.PacketSizeType;
		buffer[0] = len;
		buffer[1] = len >> 8;
		
		pBuffer->m_size = tmplen+2;

		Write(pBuffer);
		m_lastIO = GetTickCount();
	}
}

// Encryption을 하지 않고 패킷을 보낼 때 사용한다. 
void CSocketServerEx::NonEncSend(const char* format, ...)
{
	if (mode == SM_CLOSE) {
		return;
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
		len = len + config.PacketSizeType;
		buffer[0] = len;
		buffer[1] = len >> 8;

	}
	pBuffer->m_size = len+3-config.PacketSizeType;
	log.AddLog( LOG_DEBUG, "Sent %d bytes, Non Encrypt", len+3-config.PacketSizeType);
#ifdef _DEBUG
	DumpPacket( (unsigned char *)buffer, len+3-config.PacketSizeType );
#endif
	Write(pBuffer);
	m_lastIO = GetTickCount();
}

void CSocketServerEx::Send( const char *sendmsg, int msglen )
{
	if (mode == SM_CLOSE) {
		return;
	}

	CIOBuffer *pBuffer = CIOBuffer::Alloc();
	char *buffer = pBuffer->m_buffer;
	memcpy( buffer, sendmsg, msglen);
	log.AddLog( LOG_DEBUG, "Sent %d bytes, EncKey 0x87546CA1, 0x%x", msglen, EncOneTimeKey);
#ifdef _DEBUG
	DumpPacket( (unsigned char *)buffer, msglen );
#endif
	int tmplen = msglen - 2;
	EncPacket( (unsigned char *)(buffer+2), EncOneTimeKey, tmplen );
	int len = tmplen - 1 + config.PacketSizeType;
	buffer[0] = len;
	buffer[1] = len >> 8;
	pBuffer->m_size = tmplen+2;
	Write(pBuffer);
}

void CSocketServerEx::OnTimerCallback( void )
{
	if (mode == SM_CLOSE) {
		ReleaseRef();
		return;
	}
	DWORD curTick = GetTickCount();
	if ( curTick - (DWORD)m_lastIO >= (DWORD)config.SocketTimeOut - (DWORD)500 ){
		CIOSocket::CloseSocket();
	} 
	ReleaseRef();
	
	return;
}
