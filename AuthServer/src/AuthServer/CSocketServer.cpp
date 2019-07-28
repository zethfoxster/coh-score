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

BOOL SendSocketEx(SOCKET mys, const char *format, ...);
extern BOOL SendSocket(in_addr , const char *format, ...);
// 여기서 IP서버에 과금 시작이라고 알려주어야 한다.
static bool ServerPlayOk( CSocketServer *mysocket, const unsigned char *packet)
{
	int pwd = 0;
	int uid = 0;

	uid = GetIntFromPacket( packet );
	pwd = GetIntFromPacket( packet );
	ServerId previousServer;
	char account[MAX_ACCOUNT_LEN+1];

	memset(account,0, MAX_ACCOUNT_LEN);
	SOCKET s = accountdb.FindSocket( uid, mysocket->serverid, true, &previousServer, account );
	account[MAX_ACCOUNT_LEN] = 0;
	if ( s && uid ) {	
		if ( previousServer.IsValid() )
		{
			accountdb.KickAccount( uid, S_ALREADY_PLAY_GAME );

			if ( ( previousServer.IsValid() ))
			{
				in_addr address = g_ServerList.GetInternalAddress(previousServer);
				SendSocket( address, "cdcs", SQ_KICK_ACCOUNT, uid ,S_ALREADY_PLAY_GAME, account );
			}
			mysocket->Send( "cdcs", SQ_KICK_ACCOUNT, uid, S_ALREADY_PLAY_GAME, account );
			return false;
		}
	}

	AS_LOG_VERBOSE( "RCV: AS_PLAY_OK pwd:%d, uid:%d, serverid:%d", pwd, uid, mysocket->serverid );
	
	if ( s && uid ){
		AS_LOG_VERBOSE( "SND: AC_PLAY_OK pwd:%d, uid:%d, serverid:%d", pwd, uid, mysocket->serverid );
		if (config.useQueue)
		{
			SendSocketEx( s, "cddc", AC_HANDOFF_TO_QUEUE, pwd, uid, mysocket->serverid );
		}
		else
		{
			SendSocketEx( s, "cddc", AC_PLAY_OK, pwd, uid, mysocket->serverid );
		}
	}

	return false;
}

static bool ServerPlayFail( CSocketServer *mysocket, const unsigned char *packet )
{

	char failcode = GetCharFromPacket( packet );
	int uid      = GetIntFromPacket( packet );

	SOCKET s = accountdb.FindSocket( uid );
	AS_LOG_VERBOSE( "RCV: AS_PLAY_FAIL, uid:%d, failcode:%d", uid, failcode );
	AS_LOG_VERBOSE( "SND: AC_PLAY_FAIL, uid:%d, failcode:%d", uid, failcode );

	// 이 부분에서 처리가 필요하다.
	// IP 과금은 좀 많이 귀찮군..
	
	char account[MAX_ACCOUNT_LEN+1];
	int stat=0;
	in_addr loginip;
	time_t logintime;

	bool result = accountdb.GetAccountInfoForIPStop( uid, account, &stat, &loginip, &logintime );
	if ( result ){
		if ( (stat < 1000) && ( stat > 0))
			ipsessionDB.StopIPCharge( uid, loginip.S_un.S_addr, stat, 0, logintime, mysocket->serverid, account );
	}

	if ( s && uid )
		SendSocketEx( s, "cc", AC_PLAY_FAIL, char(failcode) );


	return false;
}

static bool ServerPlayGame( CSocketServer *mysocket, const unsigned char *packet )
{
	int uid = GetIntFromPacket( packet );

	AS_LOG_VERBOSE( "RCV: AS_PLAY_GAME,uid:%d",uid);
	if ( accountdb.recordGamePlayTime( uid, mysocket->serverid) ){
		return false;
	}else{
		accountdb.KickAccount( uid, S_NO_LOGININFO );
	}
	return false;
}

// AS_QUIT_GAME 

static bool ServerPlayQuit( CSocketServer *mysocket, const unsigned char *packet )
{
_BEFORE
	int uid = GetIntFromPacket( packet );
	short int reason = GetShortFromPacket( packet );
	int usetime = GetIntFromPacket( packet );
	AS_LOG_VERBOSE( "RCV: AS_QUIT_GAME,uid:%d,usetime:%d",uid,usetime);
	if ( accountdb.quitGamePlay( uid, 0, mysocket->serverid) ){
		AS_LOG_VERBOSE( "update status account db and write log, uid : %d, usetime %d, reason %d",uid,usetime, reason);
	} else {
		errlog.AddLog( LOG_ERROR, "quit game error, uid %d, reason %d, usetime %d\r\n", uid, reason, usetime );
	}
	
	if ( config.OneTimeLogOut ){
		accountdb.logoutAccount( uid );
	}
_AFTER_FIN	
	return false;
}

static bool ServerKickAccount( CSocketServer *mysocket, const unsigned char *packet )
{
_BEFORE
	int uid = GetIntFromPacket(packet);
	short reason = GetShortFromPacket(packet);

	AS_LOG_VERBOSE( "RCV: AS_KICK_ACCOUNT,uid:%d,Reason:%d",uid,reason);
	accountdb.quitGamePlay( uid, 0, mysocket->serverid );
	char name[15];
	memset(name, 0, 15);
	accountdb.removeAccount ( uid, name );
	name[14]=0;
_AFTER_FIN
	return false;
}


static bool ServerGetUserNum( CSocketServer *mysocket, const unsigned char *packet )
{
	short userCount = GetShortFromPacket(packet);
	short userLimit = GetShortFromPacket(packet);

	g_ServerList.SetServerUserCount( mysocket->serverid, userCount, userLimit);

	return false;
}

static bool ServerBanUser( CSocketServer *mysocket, const unsigned char *packet )
{
	// block_flag1 의 4번째 Bit를 켠다. 
	int uid = GetIntFromPacket( packet );
	short reason = GetShortFromPacket( packet );
	AS_LOG_VERBOSE( "RCV: AS_BAN_USER,uid:%d,reason:%d",uid, reason);

	return false;
}

static bool ServerVersion( CSocketServer *mysocket, const unsigned char *packet )
{
	// version 
	int version = GetIntFromPacket( packet );
	AS_LOG_VERBOSE( "get server version %d", version );
	
	return false;
}

static bool SetServerId( CSocketServer *mysocket, const unsigned char *packet )
{
	if (config.gameServerSpecifiesId)
	{
		unsigned char tempServerId = GetCharFromPacket(packet);
		short int port = GetShortFromPacket(packet);
		mysocket->serverid.SetValue(tempServerId);

		if (g_ServerList.SetServerSocketById( mysocket->serverid, mysocket, mysocket->getaddr(), port))
		{
			log.AddLog(LOG_ERROR, "Successfuly registered world server (id specified by server): ServerId: %d   IP: %s", 
				static_cast<int>(mysocket->serverid.GetValueChar()),
				mysocket->IP());
		}
		else
		{
			log.AddLog(LOG_ERROR, "Failed to register world server because server id was not found on server list: ServerId: %d   IP: %s", 
				static_cast<int>(mysocket->serverid.GetValueChar()),
				mysocket->IP());
			return true; // disconnects server
		}
	}
	else
	{
		log.AddLog(LOG_ERROR, "Received SetServerId, but ignored it because \"gameServerSpecifiesId=true\" is not specified in the config file:  IP: %s",
			mysocket->IP());
	}

	return false;
}

static bool ServerPing( CSocketServer *mysocket, const unsigned char *packet )
{
// Currently, pings are only used to give the TCP protocol a bump.  By sending some data, we can test for connection
// problems.  Therefore, no reply is required.
	AS_LOG_VERBOSE( "RCV: SQ_PING");
	return false;
}

static bool ServerWriteUserData( CSocketServer *mysocket, const unsigned char *packet )
{
	int Uid = GetIntFromPacket( packet );
	char UserData[MAX_USERDATA+1];

	memcpy( UserData, packet, MAX_USERDATA );
	packet += MAX_USERDATA;
	UserData[MAX_USERDATA] = 0;
	
	AS_LOG_VERBOSE( "RCV: AS_WRITE_USERDATA, uid:%d", Uid);

	CDBConn dbconn(g_linDB);
	
	SQLINTEGER udIndOrg = MAX_USERDATA_ORIG;
	SQLBindParameter( dbconn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, MAX_USERDATA_ORIG, 0, (SQLPOINTER)UserData, MAX_USERDATA_ORIG, &udIndOrg );

	SQLINTEGER udIndNew = MAX_USERDATA_NEW;
	SQLBindParameter( dbconn.m_stmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, MAX_USERDATA_NEW, 0, (SQLPOINTER)(&UserData[MAX_USERDATA_ORIG]), MAX_USERDATA_NEW, &udIndNew );

	SQLINTEGER cbUid=0;
	SQLBindParameter( dbconn.m_stmt, 3, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&Uid), 0, &cbUid );

	dbconn.Execute( "UPDATE user_data SET user_data=?,user_data_new=? WHERE uid = ?" );

	return false;
}

static bool ServerReadUserData( CSocketServer *mysocket, const unsigned char *packet )
{
    // server is using the user_data database table?
	if ( config.UserData )
    {
        int uid = GetIntFromPacket( packet );

        AS_LOG_VERBOSE( "RCV: AS_READ_USERDATA, uid:%d", uid);

	    ServerId previousServer;
	    char account[MAX_ACCOUNT_LEN+1];
	    memset(account,0, MAX_ACCOUNT_LEN);

	    SOCKET s = accountdb.FindSocket( uid, mysocket->serverid, true, &previousServer, account );
        account[MAX_ACCOUNT_LEN] = 0;

	    char userdata[MAX_USERDATA];
	    memset( userdata, 0, MAX_USERDATA );

		// class CDBConn appears to have some global status that places severe limits on it's use.  In particular, it's best to blow the
		// first of these away before trying to instantiate the second.  Hence the enclosing of these two code blocks in braces, which
		// forces CDBConn::~CDBConn() to be called.
		{
		CDBConn dbconn(g_linDB);
		SQLINTEGER UserInd=0;
		SQLBindCol( dbconn.m_stmt, 1, SQL_C_BINARY, (char *)(userdata), MAX_USERDATA_ORIG, &UserInd );

		SQLINTEGER UserIndNew=0;
		SQLBindCol( dbconn.m_stmt, 2, SQL_C_BINARY, (char *)(&userdata[MAX_USERDATA_ORIG]), MAX_USERDATA_NEW, &UserIndNew );

		SQLINTEGER cbUid=0;
		SQLBindParameter( dbconn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&uid), 0, &cbUid );

		dbconn.Execute( "SELECT user_data, user_data_new FROM user_data WHERE uid = ?" );
		dbconn.Fetch();
		}

		if ( !g_ServerList.IsServerUp(mysocket->serverid) )
		{
#ifdef _DEBUG
			log.AddLog( LOG_ERROR, "Invalid Serverid :%d, %s", mysocket->serverid, account );
#endif 
		} 
        else 
        {
            // send user data back to game server
			int len = MAX_USERDATA;
			mysocket->Send( "cdb", SQ_USER_DATA, uid, len, userdata );
		}
	} 
    else 
    {
#ifdef _DEBUG
        log.AddLog( LOG_ERROR, "Not set up to use user data!");
#endif
	}

	return false;
}

static bool ServerWriteGameData( CSocketServer *mysocket, const unsigned char *packet )
{
	int Uid = GetIntFromPacket( packet );
	char gamedata[MAX_USERDATA+1];

	memcpy( gamedata, packet, MAX_USERDATA );
	packet += MAX_USERDATA;
	gamedata[MAX_USERDATA] = 0;
	
	AS_LOG_VERBOSE( "RCV: AS_WRITE_GAMEDATA, uid:%d", Uid);

	CDBConn dbconn(g_linDB);
	
	SQLINTEGER udIndOrg = MAX_USERDATA_ORIG;
	SQLBindParameter( dbconn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, MAX_USERDATA_ORIG, 0, (SQLPOINTER)gamedata, MAX_USERDATA_ORIG, &udIndOrg );

	SQLINTEGER udIndNew = MAX_USERDATA_NEW;
	SQLBindParameter( dbconn.m_stmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, MAX_USERDATA_NEW, 0, (SQLPOINTER)(&gamedata[MAX_USERDATA_ORIG]), MAX_USERDATA_NEW, &udIndNew );

	SQLINTEGER cbUid=0;
	SQLBindParameter( dbconn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&Uid), 0, &cbUid );

	dbconn.Execute( "UPDATE user_data SET user_game_data=?,user_game_data_new=? WHERE uid = ?" );

	return false;
}

static bool ServerReadGameData( CSocketServer *mysocket, const unsigned char *packet )
{
    // server is using the user_data database table?
	if ( config.UserData )
    {
        int uid = GetIntFromPacket( packet );

        AS_LOG_VERBOSE( "RCV: AS_READ_GAMEDATA, uid:%d", uid);

	    ServerId previousServer;
	    char account[MAX_ACCOUNT_LEN+1];
	    memset(account,0, MAX_ACCOUNT_LEN);

	    SOCKET s = accountdb.FindSocket( uid, mysocket->serverid, true, &previousServer, account );
        account[MAX_ACCOUNT_LEN] = 0;

	    char gamedata[MAX_USERDATA];
	    memset( gamedata, 0, MAX_USERDATA );

		// class CDBConn appears to have some global status that places severe limits on it's use.  In particular, it's best to blow the
		// first of these away before trying to instantiate the second.  Hence the enclosing of these two code blocks in braces, which
		// forces CDBConn::~CDBConn() to be called.
		{
		CDBConn dbconn(g_linDB);
		SQLINTEGER UserInd=0;
		SQLBindCol( dbconn.m_stmt, 1, SQL_C_BINARY, (char *)(gamedata), MAX_USERDATA_ORIG, &UserInd );

		SQLINTEGER UserIndNew=0;
		SQLBindCol( dbconn.m_stmt, 2, SQL_C_BINARY, (char *)(&gamedata[MAX_USERDATA_ORIG]), MAX_USERDATA_NEW, &UserIndNew );

		SQLINTEGER cbUid=0;
		SQLBindParameter( dbconn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&uid), 0, &cbUid );

		dbconn.Execute( "SELECT user_game_data, user_game_data_new FROM user_data WHERE uid = ?" );
		dbconn.Fetch();
		}

		if ( !g_ServerList.IsServerUp(mysocket->serverid) )
		{
#ifdef _DEBUG
			log.AddLog( LOG_ERROR, "Invalid Serverid :%d, %s", mysocket->serverid, account );
#endif 
		} 
        else 
        {
            // send user data back to game server
			int len = MAX_USERDATA;
			mysocket->Send( "cdb", SQ_GAME_DATA, uid, len, gamedata );
		}
	} 
    else 
    {
#ifdef _DEBUG
        log.AddLog( LOG_ERROR, "Not set up to use user's game data!");
#endif
	}

	return false;
}

static bool ServerShardTransfer( CSocketServer *mysocket, const unsigned char *packet )
{
	int uid;
	int shard;

	uid = GetIntFromPacket(packet);
	shard = GetIntFromPacket(packet);

	accountdb.transferPlayer(uid, (unsigned char) shard);

	return false;
}

// 서버쪽에서 이제 로그인을 받아도 좋다고 하는 패킷 처리이다. 
static bool ServerSetActive( CSocketServer *mysocket, const unsigned char *packet )
{
	AS_LOG_VERBOSE( "RCV: AS_SET_CONNECT");
	CDBConn conn(g_linDB);
	int isVip;
	conn.Execute( "update worldstatus set status=1 where idx=%d", mysocket->serverid );
	
	isVip = GetIntFromPacket(packet);
	g_ServerList.SetServerStatus( mysocket->serverid, 1 );
	g_ServerList.SetServerVIPStatus( mysocket->serverid, isVip );
	mysocket->bSetActiveServer = true;

	return false;
}

// 전체 몇명인지 알려주어야 한다. 
// 한번에 100명을 넘지 않도록 하자. L2는 아래면 충분하다.
// account, uid, stat, ip, loginflag, warnflag 
// 하지만 다른게임이 필요할시에는 md5key를 주어야 한다.
// 이 Function은 현재 지원중인 게임 CODE에 따라서 따로 동작할 필요가 있다.
// select ssn from user_info with (nolock) where account='%s'
static bool ServerPlayUserList( CSocketServer *mysocket, const unsigned char *packet )
{
	// 순서를 생각해서 처리해보자.
	// 순서는 먼저 IP Stat이 아니라면 그냥 각종 사항을 세팅해 준다.. 
	// db에서 읽어와야 할것 .. ssn, loginflag, warnflag, 
	// sessionkey는 발급해주어야 한다.
    // md5key를 가져올수 없기 때문에 reconnect는 사실상 없다고 봐야 한다.
	
	int playerNum = GetIntFromPacket( packet );

	AS_LOG_VERBOSE( "RCV: AS_PLAY_USER_LIST, playerNum:%d", playerNum);
	
	int usercount = 0;
	int loginflag = 0, warnflag = 0 , paystat = 0, uid = 0;
	int dbret = 0;
	in_addr ip;
	bool nodata = false, bSuccess = false;
	char account[MAX_ACCOUNT_LEN+1];
	char ssn[14]; // 한국에서만 사용 된다.. 다른 곳에서는 필요 없어. 
	int gender=0, age = 0, nSSN, ssn2;
	time_t currentTime = time(NULL);
	char   curYear[2];
	
	struct tm *today = localtime(&currentTime);
	curYear[0] = ( today->tm_year/10 ) + '0'; // ':' = 2000
	curYear[1] = ( today->tm_year%10 ) + '0';


	for( usercount = 0; usercount < playerNum; usercount++ ) {

		memset( account, 0, MAX_ACCOUNT_LEN+1 );
		GetStrFromPacket( packet, MAX_ACCOUNT_LEN, account);
		uid  = GetIntFromPacket( packet );
		paystat = GetIntFromPacket( packet );
		ip   = GetAddrFromPacket( packet );
		loginflag = GetIntFromPacket( packet );
		warnflag  = GetIntFromPacket ( packet );
		
		// user_time 테이블을 가져 와야 한다.

		if ( config.Country == CC_KOREA ) {
			CDBConn conn(g_linDB);		
			conn.ResetHtmt();
			conn.Bind( ssn, MAX_SSN_LEN+1);
			bSuccess = true;
			if ( conn.Execute( "Select ssn From user_info with (nolock) Where account = '%s'", account ))
			{
				if (conn.Fetch(&nodata)) {
					if (nodata){
						bSuccess = false;
					}
				}
				else{
					bSuccess = false;
				}
			} else{
				//
				bSuccess = false;
			}
			if ( bSuccess == false ) {
				// KICK List 추가 Routine가 들어간다.
				continue;
			}
			gender = ssn[6] - '0';
		
			// 2003-11-25 darkangel
			// 5와 6은 국내 거주 외국인들의 외국인 등록번호이다. 
			// 이경우에는 2000년 이전 태생으로 간주한다.

			if ( ssn[6] == '1' || ssn[6] == '2' || ssn[6] == '5' || ssn[6] == '6')   // before 2000
				age = (curYear[0] - ssn[0]) * 10 + (curYear[1] - ssn[1]);
			else									// after 2000
				age = (ssn[0]-'0') * 10 + ( ssn[1] -'0');
			

			int cur_mmdd = (today->tm_mon+1)*100 + today->tm_mday;
			int nSsnmmdd = (ssn[2]-'0') * 1000 + (ssn[3]-'0')*100 + (ssn[4]-'0') * 10 + ssn[5] -'0';

			if ( cur_mmdd < nSsnmmdd )
				--age;
			if (age < 0) { // Born in the future..?
				age = 0;
			}
			
			nSSN = nSsnmmdd + ( (ssn[0]-'0') * 10 + ( ssn[1] -'0') ) * 10000;
			ssn2 = atoi( ssn+6);					
			conn.ResetHtmt();
		} else  {

			nSSN = 0;
			ssn2 = 0;
			gender = 0;
			age = 0;

		}

		LoginUser *lu = new LoginUser; //TODO:  add queueLevel to the list packet?
		strncpy( lu->account, account, MAX_ACCOUNT_LEN );
		lu->account[MAX_ACCOUNT_LEN] = 0;
		lu->cdkind = 0;
		lu->lastworld = mysocket->serverid;
		lu->loginflag = loginflag;
		lu->warnflag = warnflag;
		lu->um_mode  = UM_IN_GAME;
		lu->loginIp.S_un.S_addr  = ip.S_un.S_addr;
		lu->logintime= currentTime;
		lu->queuetime=currentTime;
		lu->serverid = mysocket->serverid;
		lu->selectedServerid.SetInvalid();
		lu->s = INVALID_SOCKET;
		lu->md5key = 0;
		lu->timerHandle = NULL;
		lu->stat = paystat;
		lu->gender = gender;
		lu->ssn = nSSN;
		lu->ssn2 = ssn2;
		lu->age = age;
        // Not setting lu->regions, because it is not needed at this point in the code

		// 여기까지 왔으면 로그인 처리를 하도록 한다. 
		if (! accountdb.RegAccountByServer( lu, uid, mysocket, 0, 0 ) )
		{
			delete lu;
			// Kick List에 넣도록 한다.
			continue;

		}
		delete lu;

		if ( paystat < 1000 ) {
			// 이 경우에는 PC방이다. 
			// 일단 나중에 처리하자.
		}
	}
	// 처리 패킷을 보내주어야 한다.
	
	AS_LOG_VERBOSE( "SND: SQ_COMPLETE_USERLIST");

	mysocket->Send( "cd", SQ_COMPLETE_USERLIST, usercount );

	return false;
}

static bool ServerUserNumByQueueLevel( CSocketServer *mysocket, const unsigned char *packet )
{
	// what to do????
	return false;
}

static bool FinishedQueue( CSocketServer *mysocket, const unsigned char *packet )
{
	int uid=GetIntFromPacket(packet);
	accountdb.FinishedQueue(uid);
	return false;
}

static bool SetLoginFrequency( CSocketServer *mysocket, const unsigned char *packet )
{	
	// This message is sent between the game server and the queue server --- the auth server should ignore it
	return false;
}

static bool QueueSizes( CSocketServer *mysocket, const unsigned char *packet )
{
	size_t count = static_cast<size_t>(GetCharFromPacket( packet ));
	for (size_t i=0; i<count; ++i)
	{
		int queueLevel=GetCharFromPacket(packet);
		int queueSize=GetIntFromPacket(packet);
		int queueTime=GetIntFromPacket(packet);

		g_ServerList.SetServerQueueSize(mysocket->serverid, queueLevel, queueSize, queueTime);		
	}
	return false;
}

static ServerPacketFunc ServerPacket[] = {
	ServerPlayOk,
	ServerPlayFail,
	ServerPlayGame,
	ServerPlayQuit,
	ServerKickAccount,
	ServerGetUserNum,
	ServerBanUser,
	ServerVersion,
	ServerPing,
	ServerWriteUserData,
	ServerSetActive,
	ServerPlayUserList,
	SetServerId,
	ServerUserNumByQueueLevel,
	FinishedQueue,
	SetLoginFrequency,
	QueueSizes,
	ServerReadUserData,   // for AS_READ_USERDATA
    ServerWriteGameData,  // for AS_WRITE_GAMEDATA
    ServerReadGameData,   // for AS_READ_GAMEDATA
    ServerShardTransfer,  // for AS_SHARD_TRANSFER
};

BOOL SendSocketEx(SOCKET mys, const char *format, ...)
{
_BEFORE
	CSocketServerEx *pSocket = serverEx->FindSocket(mys);
	if (!pSocket)
		return FALSE;
	CIOBuffer *pBuffer = CIOBuffer::Alloc();
	char *buffer = pBuffer->m_buffer;
	va_list ap;
	va_start(ap, format);
	int len = Assemble(buffer + 2, BUFFER_SIZE - 2, format, ap);
	va_end(ap);
	if (len == 0) {
		log.AddLog(LOG_ERROR, "%d: assemble too large packet. format %s", mys, format);
	} else {
		len -= 1;
	}
	if ( buffer[2] == AC_PLAY_OK )
		pSocket->um_mode = UM_PLAY_OK;

	int tmplen = len+1;
	
	pSocket->EncPacket( (unsigned char *)(buffer+2), pSocket->EncOneTimeKey, tmplen);
	int len2 = tmplen - 1 + config.PacketSizeType;
	buffer[0] = len2;
	buffer[1] = len2 >> 8;

	pBuffer->m_size = tmplen+2;
	pSocket->Write(pBuffer);
	pSocket->ReleaseRef();
_AFTER_FIN
	return TRUE;
}
CSocketServer::CSocketServer( SOCKET aSoc )
: CIOSocket( aSoc )
{
	addr.s_addr = 0;
	host = 0;
	mode = SM_READ_LEN;
	packetTable =ServerPacket;
	opFlag = 0;
}

CSocketServer::~CSocketServer()
{
}

CSocketServer *CSocketServer::Allocate( SOCKET s )
{
	return new CSocketServer( s );
}
void CSocketServer::OnClose(SOCKET closedSocket)
{
	// Must not use the GetSocket() function from within
	// this function!  Instead, use the socket argument
	// passed into the function.

	mode = SM_CLOSE;

	time_t ActionTime;
	struct tm ActionTm;

	ActionTime = time(0);
	ActionTm = *localtime(&ActionTime);

	log.AddLog(LOG_ERROR, "*close connection from %s, %x(%x)", IP(), closedSocket, this);
	errlog.AddLog( LOG_NORMAL, "%d-%d-%d %d:%d:%d,main server connection close from %s, 0x%x\r\n",		
				ActionTm.tm_year + 1900, ActionTm.tm_mon + 1, ActionTm.tm_mday,
				ActionTm.tm_hour, ActionTm.tm_min, ActionTm.tm_sec, IP(), closedSocket );	

	if (serverid.IsValid())
		accountdb.RemoveAll( serverid );
	g_ServerList.RemoveSocket(this);
	server->RemoveSocket(addr);
	CDBConn conn(g_linDB);
	conn.Execute( "update worldstatus set status=0 where idx=%d", serverid );	
}

void CSocketServer::OnRead()
{
	int pi = 0;
	int ri = m_pReadBuf->m_size;
	int errorNum = 0;
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
					errorNum = 1;
					log.AddLog(LOG_ERROR, "%d: bad packet size %d", GetSocket(), packetLen);
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

				if (inBuf[pi] >= AS_MAX) {
					log.AddLog(LOG_ERROR, "unknown protocol %d", inBuf[pi]);
					errorNum = 2;
					break;
				} else {
					CPacketServer *pPacket = CPacketServer::Alloc();
					pPacket->m_pSocket = this;
					pPacket->m_pBuf = m_pReadBuf;
					pPacket->m_pFunc = (CPacketServer::ServerPacketFunc) packetTable[inBuf[pi]];
					CIOSocket::AddRef();
					m_pReadBuf->AddRef();
					InterlockedIncrement(&CPacketServer::g_nPendingPacket);
					pPacket->PostObject(pi, g_hIOCompletionPort);
					
					pi += packetLen;
					mode = SM_READ_LEN;
				}
			} else {
				Read(ri - pi);
				return;
			}
		}
		else{
			errorNum = 3;
			break;
		}
	}
	
	time_t ActionTime;
	struct tm ActionTm;

	ActionTime = time(0);
	ActionTm = *localtime(&ActionTime);

	errlog.AddLog( LOG_NORMAL, "main server connection close. invalid status and protocol %s, errorNum :%d", IP(), errorNum);
	CIOSocket::CloseSocket();
}

const char *CSocketServer::IP()
{
	return inet_ntoa(addr);
}

void CSocketServer::OnCreate()
{
	OnRead();
	int reactivationActive = config.IsReactivationActive();

	if (config.ProtocolVer >= GR_REACTIVATION_PROTOCOL_VERSION)
	{
		AS_LOG_VERBOSE( "SND: SQ_VERSION buildnumber :%d, protocol version : %d, reactivation active: %d",
			buildNumber, config.ProtocolVer, reactivationActive );
		Send( "cddd", SQ_VERSION, buildNumber, config.ProtocolVer, reactivationActive );
	}
	else
	{
		AS_LOG_VERBOSE( "SND: SQ_VERSION buildnumber :%d, protocol version : %d",
			buildNumber, PROTOCOL_VERSION );
		Send( "cdd", SQ_VERSION, buildNumber, PROTOCOL_VERSION );
	}
}

void CSocketServer::SetAddress(in_addr inADDR )
{
	addr = inADDR;

//  2004-01-29 darkangel
//  reconnect 를 서포트 하도록 되어 있다면 이것은 Connect패킷이 왔을때 처리한다.

	if ( !config.supportReconnect ) {

//  이미 서버 ID가 세팅되어 있다고 간주한다. 
//  현재 서버가 살아 있다고 세팅한다. 
		CDBConn conn(g_linDB);
		conn.Execute( "update worldstatus set status=1 where idx=%d", serverid );
		g_ServerList.SetServerStatus( serverid, 1 );
		bSetActiveServer = true;
	}
}

void CSocketServer::Send(const char* format, ...)
{
_BEFORE
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
		log.AddLog(LOG_ERROR, "%d: assemble too large packet. format %s", GetSocket(), format);
	} else {
		len -= 1;
		len = len + config.PacketSizeType;
		buffer[0] = len;
		buffer[1] = len >> 8;

	}
	pBuffer->m_size = len+3 - config.PacketSizeType;
	Write(pBuffer);
_AFTER_FIN
}


bool CServerKickList::AddKickUser( int uid ) 
{
	bool result = false;

	return result;
}
