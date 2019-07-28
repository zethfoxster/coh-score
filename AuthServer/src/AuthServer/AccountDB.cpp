// AccountDB.cpp: implementation of the AccountDB class.
//
//////////////////////////////////////////////////////////////////////

#include "PreComp.h"


// 2003-11-25 darkangel
// Wanted Service쪽으로 사용자 정보를 보낼때 사용한다. 
bool SendWantedServerLogout( char *name, int uid, ServerId gameserver )
{
_BEFORE
	if ( (!config.UseWantedSystem) || WantedServerReconnect || ( !gameserver.IsValid()) )
		return true;

	char sndmsg[24];
	memset(sndmsg,0, 24);
	
	switch( config.gameId ) {
	case LINEAGE2_GAME_CODE :
		sndmsg[0]=2;
		break;
	default:
		return true;
		break;
	}

	memcpy( sndmsg+1, &uid, 4 );
	strcpy( sndmsg+5, name );
	sndmsg[19] = gameserver.GetValueChar();
	time_t stime = time(NULL);
	memcpy( sndmsg+20, &stime, 4 );

	if (pWantedSocket && config.UseWantedSystem && (!WantedServerReconnect)) {
		log.AddLog( LOG_WARN, "Wanted User LogOut, %s", name );
		gWantedLock.ReadLock();
		pWantedSocket->AddRef();
		pWantedSocket->Send("cb", AW_QUIT, 24, sndmsg );
		pWantedSocket->ReleaseRef();
		gWantedLock.ReadUnlock();
	}
_AFTER_FIN
	return true;
}

BOOL SendSocketEx(SOCKET s, const char *format, ...);

AccountDB::AccountDB()
{
}

AccountDB::~AccountDB()
{
}


bool AccountDB::FindAccount( int uid, char *account, ServerId & lastServer, int regions[MAX_REGIONS])
{
	HANDLE hTimer=NULL;
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()) {
		strncpy( account, it->second.account, MAX_ACCOUNT_LEN+1 );
		// 2003-11-25 darkangel
		// 이미 CriticalSection안에 들어와 있는데 InterlockedExchange같은 바보짓은 하지 말아라. 
		hTimer = it->second.timerHandle;
		it->second.timerHandle = NULL;
		lastServer = it->second.lastworld;
        memcpy(regions,it->second.regions,sizeof(it->second.regions));
		m_lock.Leave();
	}else{
		m_lock.Leave();
		lastServer.SetInvalid();
		return false;
	}

	if ( hTimer != NULL  )
		DeleteTimerQueueTimer( NULL, hTimer, NULL );

	return true;
}


bool  AccountDB::FindAccount( int uid, char *account, int *loginflag, int *warnflag, int *pay_stat, int *md5key, int *queueLevel, int *loyalty, int *loyatyLegacy)
{
	bool result=false;
	//int smd5key = 0;
	HANDLE hTimer=NULL;
	
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()) {
		strncpy( account, it->second.account, MAX_ACCOUNT_LEN+1 );
		*loginflag = it->second.loginflag;
		*warnflag = it->second.warnflag;
		*pay_stat = it->second.stat;
		hTimer = it->second.timerHandle;
		*md5key = it->second.md5key;
		*queueLevel = it->second.queueLevel;
		*loyalty = it->second.loyalty;
		*loyatyLegacy = it->second.loyaltyLegacy;
		it->second.timerHandle = NULL;
		result = true;
	}
	m_lock.Leave();
	
	if ( hTimer != NULL  )
		DeleteTimerQueueTimer( NULL, hTimer, NULL );

	return result;
}

// 2003-07-06 darkangel
// UpdateSocket 은 현재 연결된 소켓을 MAP에 반영하기 위한 것이다. 
// 각 원 Time md5key가 고유로 같이 따라 다니도록 되어 있고 현재 게임 중이라면
// Update하지 않는다. 왜냐하면 현재 월드서버와 접속 중이라면 소켓이 넘어올리 없기 때문이다.
int AccountDB::UpdateSocket( int uid, SOCKET s, int md5key, ServerId serverid  )
{
_BEFORE
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end())
	{
		if ( md5key == it->second.md5key )
		{
			if (  it->second.um_mode != UM_IN_GAME ) {
				it->second.s = s;
				it->second.selectedServerid = serverid;
				it->second.serverid.SetInvalid();
				m_lock.Leave();

				return S_ALL_OK;
			} else {
				m_lock.Leave();
				return S_ALREADY_PLAY_GAME;
			}
		} else{
			m_lock.Leave();
			return S_INCORRECT_MD5Key;
		}
	} else{
		m_lock.Leave();
		return S_NO_LOGININFO;
	}
_AFTER_FIN
	return S_NO_LOGININFO;
}

// 2003-07-06 darkangel
// Map에 Account를 등록할때 사용된다. 
// 만일 이미 등록되어 있을 경우에는 이중 로그인으로 간주하고 두 계정을 모두 Kick시키게 된다. 
bool AccountDB::RegAccount( LoginUser *loginuser, int uid, CSocketServerEx *sEx, int remainTime, int quotaTime )
{
	bool result=false;

	m_lock.Enter();
	std::pair<UserMap::iterator, bool> r = usermap.insert(UserMap::value_type(uid, *loginuser));
	result = r.second;
	m_lock.Leave();
	
	if ( result == false ){
		// kick account를 할경우 메세지를 보낼지는 사실 결정 안해도 된다. 
		// 다시 로그인 하라고 하니까.
		KickAccount( uid,S_ALREADY_LOGIN, true );
		sEx->Send( "cc", AC_LOGIN_FAIL, S_ALREADY_LOGIN );
	}  else {
		sEx->um_mode = UM_LOGIN;
        //sEx->Send( "cddddddddd", AC_LOGIN_OK, uid, sEx->GetMd5Key(), 
		//						 g_updateKey, 
		//						 g_updateKey2, 
		//						 loginuser->stat, 
		//						 remainTime, 
		//						 quotaTime, loginuser->warnflag, loginuser->loginflag );

        if (config.useQueue || config.sendQueueLevel)
        {
			if (config.ProtocolVer >= GR_REACTIVATION_PROTOCOL_VERSION) {
				sEx->Send( "cdddddddddddd", AC_LOGIN_OK, uid, sEx->GetMd5Key(), 
								g_updateKey, 
								g_updateKey2, 
								loginuser->stat, 
								loginuser->loyalty,
								remainTime, 
								quotaTime, 
								loginuser->warnflag, 
								loginuser->loginflag,
								config.IsReactivationActive(),
								loginuser->queueLevel );
			} else {
				sEx->Send( "cddddddddddddd", AC_LOGIN_OK, uid, sEx->GetMd5Key(), 
					g_updateKey, 
					g_updateKey2, 
					loginuser->stat, 
					loginuser->loyalty,
					remainTime, 
					quotaTime, 
					loginuser->warnflag, 
					loginuser->loginflag,
					loginuser->queueLevel );
			}
        }
        else
        {
			if (config.ProtocolVer >= GR_REACTIVATION_PROTOCOL_VERSION) {
				sEx->Send( "cddddddddddd", AC_LOGIN_OK, uid, sEx->GetMd5Key(), 
								g_updateKey, 
								g_updateKey2, 
								loginuser->stat, 
								loginuser->loyalty,
								remainTime, 
								quotaTime, 
								loginuser->warnflag, 
								loginuser->loginflag,
								config.IsReactivationActive() );
			} else {
				sEx->Send( "cdddddddddddd", AC_LOGIN_OK, uid, sEx->GetMd5Key(), 
					g_updateKey, 
					g_updateKey2, 
					loginuser->stat, 
					loginuser->loyalty,
					remainTime, 
					quotaTime, 
					loginuser->warnflag, 
					loginuser->loginflag,
					config.IsReactivationActive() );
			}
        }


	}

	return result;
}

/**
 * Called when a user is done waiting in the queue and has actually logged into the game.
 * Update the login time to the current time (queue entered time stays as-is)
 */
void AccountDB::FinishedQueue( int uid )
{
    m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end())
	{
		it->second.logintime=time(NULL);
	}
    m_lock.Leave();
}

// IA_IP_KICK에 의한 Kick일 경우에는 IPServer쪽으로 불필요한 ReleaseSession을 요청하게 된다. 
// reasoncode로 구분할수 있으나 시간이 된다면 KickAccount 를 하나 더 만드는것이 맞을것 같다. 
bool AccountDB::KickAccount( int uid, char reasoncode, bool sendmsg )
{
	bool result = false;
	ServerId serverid;
	char account[MAX_ACCOUNT_LEN+1];

	SOCKET oldSocket=INVALID_SOCKET;
	UserMode usermode=UM_PRE_LOGIN;
	char gender=0;
	in_addr ip;
	account[0]=0;
	int ssn=0, ssn2=0, stat = 0;
	UINT warn_flag=0;
	char age=0;
	short int cdkind=0;
	time_t login_time;
	time_t queue_time;

	ip.S_un.S_addr = 0;
	login_time = 0;

	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end())
	{
		oldSocket = it->second.s;
		usermode  = it->second.um_mode;
		serverid  = it->second.serverid;
		gender    = it->second.gender;
		ssn       = it->second.ssn;
		ssn2      = it->second.ssn2;
		stat	  = it->second.stat;
		ip        = it->second.loginIp;
		login_time= it->second.logintime;
		queue_time = it->second.queuetime;
		age		  = it->second.age;
		cdkind    = it->second.cdkind;
		warn_flag = it->second.warnflag;
		strncpy( account, it->second.account, MAX_ACCOUNT_LEN+1 );
		usermap.erase(it);
		m_lock.Leave();
		result = true;
		account[MAX_ACCOUNT_LEN]=0;
	} else
		m_lock.Leave();
	

	// Map에 없다면 Kick시킬 필요가 없다. 
	if ( result == true ) {
		if ( config.UseWantedSystem ){
			if ( (warn_flag & 4) == 4 )
				SendWantedServerLogout( account, uid, serverid );
		}
		StdAccount( account );
		if ((strlen(account)>= 2) && (gender < 7))
			WriteLogD( LOG_ACCOUNT_LOGOUT2, account, ip, stat, age, gender, 0, 0, uid );

		if ( (usermode == UM_IN_GAME) || ( usermode == UM_PLAY_OK )) {
			if ( usermode == UM_IN_GAME ) {
				if ( serverid.IsValid() )
					RecordLogout( reasoncode, uid, login_time, queue_time, serverid, ip, config.gameId, account, stat, ssn, ssn2, gender, age, cdkind );
			} 
			
			AS_LOG_VERBOSE( "SND: SQ_KICK_ACCOUNT,%d,uid:%d, account:%s", reasoncode, uid, account );

			if ( !g_ServerList.IsServerUp(serverid) )
			{
	#ifdef _DEBUG
				log.AddLog( LOG_ERROR, "Invalid Serverid :%d, %s", serverid, account );
	#endif
				if ( (stat < 1000) && ( stat > 0)){
					int sessionid = ipsessionDB.DelSessionID( uid );
					if ( (sessionid > 0) && config.UseIPServer )
						ipsessionDB.ReleaseSessionRequest( sessionid, ip, stat );
				}	

				return result;
			}
			else
				SendSocket( g_ServerList.GetInternalAddress(serverid), "cdcs", SQ_KICK_ACCOUNT, uid ,reasoncode, account ); // It is required
		
			if ( (stat < 1000) && ( stat > 0)){
				int sessionid = ipsessionDB.DelSessionID( uid );
				if ( (sessionid > 0) && config.UseIPServer )
					ipsessionDB.ReleaseSessionRequest( sessionid, ip, stat );
			}	

			return result;
		}

		if ( (stat < 1000) && ( stat > 0)){
			int sessionid = ipsessionDB.DelSessionID( uid );
			if ( (sessionid > 0) && config.UseIPServer )
				ipsessionDB.ReleaseSessionRequest( sessionid, ip, stat );
		}	

		if ( oldSocket != INVALID_SOCKET && sendmsg ){
			SendSocketEx( oldSocket, "cc", AC_ACCOUNT_KICKED, reasoncode);
			AS_LOG_VERBOSE( "SND: AC_ACCOUNT_KICKED,%d,uid:%d,%x", reasoncode, uid, oldSocket );
		}
	}

	return result;
}

VOID CALLBACK TimerRoutine(PVOID lpParam, unsigned char TimerOrWaitFired)
{
_BEFORE
	int uid = PtrToInt( lpParam );
	accountdb.TimerCallback( uid );
_AFTER_FIN
	return;
}

void AccountDB::TimerCallback( int uid )
{
	HANDLE timer=NULL;
	UserMode um;
	bool result = false;
	ServerId serverid;
	ServerId preserverid;
	int stat=0;
	BYTE reasoncode = 0;
	char account[MAX_ACCOUNT_LEN+1];
	in_addr ip;
	UINT warn_flag=0;
_BEFORE
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end())
	{
		um = it->second.um_mode;
		serverid = it->second.serverid;
		preserverid = it->second.selectedServerid;
		strncpy( account, it->second.account, MAX_ACCOUNT_LEN+1 );
		warn_flag = it->second.warnflag;
		if ( um != UM_IN_GAME ){
			stat = it->second.stat;
			ip.S_un.S_addr = it->second.loginIp.S_un.S_addr;
			usermap.erase(it);
			result = true;
		}
		else{
			it->second.timerHandle = NULL;
		}
	}
	m_lock.Leave();

	account[MAX_ACCOUNT_LEN] = 0;
	// 로그인해 있고 erase되었으면 IP관련된 처리를 한다. 
	if (result){ 
		if ( config.UseWantedSystem ){
			if ( ( warn_flag & 4 ) == 4 )	
				SendWantedServerLogout( account, uid, serverid );
		}
		AS_LOG_DEBUG( "timer expire account erase %d", uid );
		if ( (stat < 1000) && ( stat > 0)) {
			int sessionid = ipsessionDB.DelSessionID( uid );
			if ( sessionid ){
				ipsessionDB.ReleaseSessionRequest( sessionid,  ip, stat );
			}
		}
	}

	if ( result && (um == UM_PLAY_OK) ) {
		if ( g_ServerList.IsServerUp(preserverid))
		{
			SendSocket( g_ServerList.GetInternalAddress(preserverid), "cdcs", SQ_KICK_ACCOUNT, uid ,reasoncode, account );
		}
		if ( !g_ServerList.IsServerUp(preserverid))
		{
#ifdef _DEBUG
			log.AddLog( LOG_ERROR, "Invalid Serverid :%d, %s", serverid, account );
#endif 
		} else {
			SendSocket( g_ServerList.GetInternalAddress(serverid), "cdcs", SQ_KICK_ACCOUNT, uid ,reasoncode, account );
		}
	}
AFTER_FIN
	
	return;
}

void AccountDB::RemoveAll(ServerId s )
{
	int sessionid=0;
_BEFORE
	m_lock.Enter();
	for (UserMap::iterator it = usermap.begin(); it != usermap.end(); ) {
		if ( (it->second.serverid == s) || ( it->second.selectedServerid == s)) {
			if ( it->second.stat < 1000 ){
				sessionid = ipsessionDB.DelSessionID( it->first );
				if ( sessionid ) {
					ipsessionDB.ReleaseSessionRequest( sessionid, (it->second.loginIp), it->second.stat );
				}
			}
			it = usermap.erase(it);
			InterlockedDecrement( &reporter.m_InGameUser );				
		}else
			it++;
	}
	m_lock.Leave();
_AFTER_FIN
}

SOCKET AccountDB::FindSocket( int uid, bool SetTimer )
{
	SOCKET s=NULL;
	HANDLE hTimer=NULL, tTimer=NULL;

	if (SetTimer)
		CreateTimerQueueTimer( &hTimer, NULL, TimerRoutine, (void *)(INT_PTR)uid, 300000, 0, 0 );

	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()) {
		s = it->second.s;
		tTimer = it->second.timerHandle;
		it->second.timerHandle = hTimer;
	}
	m_lock.Leave();

	if ( tTimer != NULL ){
		DeleteTimerQueueTimer( NULL, tTimer, NULL );
	}
	
	return s;
}

// 2003-07-06 darkangel
// 서버를 선택했을때 소켓을 찾는 것이다. 
// 만일 setTimer가 True이면 5분내로 월드서버에서 로그인 메세지를 보내주어야 한다. 
// 보내지 않으면 자동으로 로그아웃 시켜 버린다.
SOCKET AccountDB::FindSocket( int uid, ServerId serverid, bool SetTimer, ServerId *preserverid, char *account )
{
	SOCKET s=NULL;
_BEFORE
	HANDLE hTimer=NULL, tTimer=NULL;

	if (SetTimer){
		CreateTimerQueueTimer( &hTimer, NULL, TimerRoutine, (void *)(INT_PTR)uid, 300000, 0, 0 );
	}
	
	preserverid->SetInvalid();
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()) {
		s = it->second.s;
		tTimer = it->second.timerHandle;
		it->second.timerHandle = hTimer;

		if ( serverid != it->second.selectedServerid ) {
			*preserverid = it->second.selectedServerid;
			strncpy( account, it->second.account, MAX_ACCOUNT_LEN);
		} else {
			it->second.serverid = serverid;
			it->second.um_mode = UM_PLAY_OK;
			strncpy( account, it->second.account, MAX_ACCOUNT_LEN);
		}
		it->second.selectedServerid.SetInvalid();
	}
	m_lock.Leave();

	if ( tTimer != NULL ){
		DeleteTimerQueueTimer( NULL, tTimer, NULL );
	}
_AFTER_FIN
	return s;
}

// 2003-07-06 darkangel
// 사용자 정보를 MAP에서 삭제한다. 
// logout과 틀린점은 사용시간을 기록하지 않고 순수하게 데이타만 삭제한다. 
// 하지만 PC방이면 글쎄.. 기록해야 하지 않을까?
bool AccountDB::removeAccount( int uid, char *account )
{
	bool result=false;
	ServerId serverid;
	int stat=0;
	in_addr ip;
	UINT warn_flag = 0;
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()) {
		strncpy( account, it->second.account, 15 );
		warn_flag = it->second.warnflag;
		stat = it->second.stat;
		ip   = it->second.loginIp;
		usermap.erase(it);
		result = true;
	}
	m_lock.Leave();
	account[MAX_ACCOUNT_LEN]=0;
	if ( result ){
		if ( config.UseWantedSystem ) {
			if ( (warn_flag & 4) == 4 )
				SendWantedServerLogout( account, uid, serverid );
		}
		if (stat<1000) {
			int sessionid = ipsessionDB.DelSessionID( uid );
			//sessionid가 존재해야 의미가 있다.
			if ( sessionid )
				ipsessionDB.ReleaseSessionRequest( sessionid,  ip, stat );
		}
	}
	return result;
}

bool AccountDB::removeAccountPreLogIn( int uid, SOCKET s )
{
_BEFORE
	bool result=false;
	int stat=0;
	in_addr ip;
	int age=0;
	int gender=0;
	ServerId serverid;
	UINT warn_flag = 0;
	char account[15];
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end())
	{
		if ( it->second.um_mode != UM_PLAY_OK  && it->second.s == s){
			stat = it->second.stat;
			ip   = it->second.loginIp;
			strcpy( account, it->second.account);
			age = it->second.age;
			gender = it->second.gender;
			serverid = it->second.serverid;
			warn_flag = it->second.warnflag;
			usermap.erase(it);
			result = true;
		}
	}
	m_lock.Leave();
	account[14]=0;
	// 로그인 정보가 존재하면.
	if ((stat<1000) && result ){
		int sessionid = ipsessionDB.DelSessionID( uid );
		//sessionid가 존재해야 의미가 있다.
		if ( sessionid ){
			ipsessionDB.ReleaseSessionRequest( sessionid,  ip, stat );
		}
	}

	if ( result == true ){
		if ( config.UseWantedSystem ){
			if (( warn_flag & 4 ) == 4)
				SendWantedServerLogout( account, uid, serverid );
		}
		if ((strlen(account)>= 2) && (gender < 7))
			WriteLogD( LOG_ACCOUNT_LOGOUT2, account, ip, stat, age, gender, 0, 0, uid ); 
	}

_AFTER_FIN
	return true;
}

// 2003-12-18 serverid 파라미터 추가
bool AccountDB::logoutAccount( int uid, int md5key )
{
	bool result=false;
_BEFORE
	char account[MAX_ACCOUNT_LEN+1];
	in_addr ip;
	char gender;
	int	 stat=0;
	int  ssn = 0;
	int  age = 0;
	ServerId serverid;
	UINT warn_flag = 0;
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()){
		if ( it->second.md5key == md5key ){
			if ( it->second.um_mode == UM_IN_GAME ){
				m_lock.Leave();
				result = accountdb.quitGamePlay( uid, 0, ServerId::s_invalid );
				if ( result ) {
					result = accountdb.logoutAccount(uid);
				}
				return result;
			}

			strncpy( account, it->second.account, MAX_ACCOUNT_LEN+1 );
			ip = it->second.loginIp;
			stat = it->second.stat;
			gender = it->second.gender;
			ssn = it->second.ssn;
			age = it->second.age;
			serverid = it->second.serverid;
			warn_flag = it->second.warnflag;
			usermap.erase(it);
			result = true;
		}
	}
	m_lock.Leave();
	// 로그인 정보가 존재하면
	account[MAX_ACCOUNT_LEN] = 0;
	if ( result ) {
		if ( config.UseWantedSystem ){
			if ( (warn_flag & 4 ) == 4 )
				SendWantedServerLogout( account, uid, serverid );
		}
		if ( (stat < 1000) && ( stat > 0)){
			int sessionid = ipsessionDB.DelSessionID( uid );
			if( sessionid ){
				ipsessionDB.ReleaseSessionRequest( sessionid, ip, stat );
			}
		}
		if ((strlen(account)>= 2) && (gender < 7))
			WriteLogD( LOG_ACCOUNT_LOGOUT2, account, ip, stat, age, gender, 0, 0, uid ); 
	}
_AFTER_FIN	
	return result;
}

bool AccountDB::logoutAccount( int uid )
{
	bool result=false;

	char account[MAX_ACCOUNT_LEN+1];
	in_addr ip;
	char gender=0;
	int stat=0;
	int ssn=0;
	int age=0;
	ServerId serverid;
	UINT warn_flag = 0;
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()){
		strncpy( account, it->second.account, MAX_ACCOUNT_LEN+1 );
		ip = it->second.loginIp;
		gender = it->second.gender;
		stat = it->second.stat;
		ssn  = it->second.ssn;
		age  = it->second.age;
		serverid = it->second.serverid;
		usermap.erase(it);
		m_lock.Leave();
		result = true;
	}else
		m_lock.Leave();
	account[MAX_ACCOUNT_LEN] = 0;
	if ( result ){
		if ( config.UseWantedSystem ){
			if (( warn_flag & 4 ) == 4 )
				SendWantedServerLogout( account, uid, serverid );
		}
		if ( (stat < 1000) && ( stat > 0)){
			int sessionid = ipsessionDB.DelSessionID( uid );
			if ( sessionid )
				ipsessionDB.ReleaseSessionRequest(sessionid, ip, stat);
		}
		if ((strlen(account)>= 2) && (gender < 7))
			WriteLogD( LOG_ACCOUNT_LOGOUT2, account, ip, stat, age, gender, 0, 0, uid );
	}

	return result;
}


// 2003-07-20 darkangel
// Game시작을 알려준다. 실제 월드서버에 접속한 때를 알려주는 것이고
// 이 루틴을 사용자가 ByPass하게 되면 사용시간이 남지 않는다. ( 설마 그런일이 발생할까? )

bool AccountDB::recordGamePlayTime( int uid , ServerId serverid)
{
	bool result = false;
_BEFORE
	HANDLE mTimer=NULL;
	char account[MAX_ACCOUNT_LEN+1];
	memset( account, 0, MAX_ACCOUNT_LEN+1 );
	char gender=0;
	in_addr loginip;
	int ssn=0;
	int stat=0;
	int age=0;
	UINT warn_flag=0;
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()){
		it->second.logintime = time(0);
		it->second.serverid = serverid;
		it->second.um_mode = UM_IN_GAME;		
		mTimer = it->second.timerHandle;
		it->second.timerHandle = NULL;
		it->second.selectedServerid.SetInvalid();
		strncpy(account, it->second.account,MAX_ACCOUNT_LEN+1);
		loginip = it->second.loginIp;
		gender = it->second.gender;
		stat = it->second.stat;
		ssn =  it->second.ssn;
		age =  it->second.age;
		warn_flag = it->second.warnflag;
		result = true;
	}
	m_lock.Leave();

	// IPServer쪽에 시작을 알려주도록 한다.
	if ( (stat < 1000) && ( stat > 0) && result ){
		ipsessionDB.ConfirmIPCharge( uid, loginip.S_un.S_addr, stat, serverid );
	}

	if ( mTimer != NULL)
		DeleteTimerQueueTimer( NULL, mTimer, NULL );

	if ( result ){
		InterlockedIncrement( &reporter.m_InGameUser );
		WriteLogD( LOG_ACCOUNT_LOGIN, account, loginip, stat, age, gender, 0, reporter.m_InGameUser, uid );
	}

	if ( pWantedSocket != NULL && config.UseWantedSystem && ( (warn_flag & 4) == 4 ) && result) {

		char sndmsg[28];
		memset(sndmsg,0, 28);
	// wanted socket이기 때문에 확인해야 한다. 
		if ( config.gameId == 8 )
			sndmsg[0]=2;
		memcpy( sndmsg+1, &uid, 4 );
		strncpy( sndmsg+5, account, 14 );
		sndmsg[19]=serverid.GetValueChar();
		time_t stime = time(NULL);
		memcpy( sndmsg+20, &stime, 4 );
		memcpy( sndmsg+24, &loginip, 4 );
		if ( config.UseWantedSystem && (!WantedServerReconnect) ){
			gWantedLock.ReadLock();
			pWantedSocket->AddRef();
			pWantedSocket->Send( "cb", AW_START, 28, sndmsg );
			pWantedSocket->ReleaseRef();
			gWantedLock.ReadUnlock();
		}
	}

_AFTER_FIN
	return result;
}

bool AccountDB::quitGamePlay( int uid, int usetime, ServerId serverID)
{
	bool result = false;
_BEFORE
	LoginUser lu;

	UserMap::iterator it;

	HANDLE hTimer=NULL;

	if ( config.OneTimeLogOut != true )
		CreateTimerQueueTimer( &hTimer, NULL, TimerRoutine, (void *)(INT_PTR)uid, config.SocketTimeOut, 0, 0 );

	m_lock.Enter();
	it = usermap.find(uid);
	if ( it != usermap.end()){
		if ( it->second.serverid == serverID || !serverID.IsValid()) {
			lu = it->second;
			it->second.um_mode = UM_LOGIN;	
			it->second.lastworld = it->second.serverid;
			it->second.serverid.SetInvalid();
			it->second.timerHandle = hTimer;
			result = true;
		} 
		m_lock.Leave();
	}else
		m_lock.Leave();
	
	if ( result )
	{
		lu.account[MAX_ACCOUNT_LEN] = '\0';
		if ( ( lu.warnflag & 4 ) == 4 )
			SendWantedServerLogout( lu.account, uid, lu.serverid );
		if ( lu.serverid.IsValid() )
		{
			AS_LOG_DEBUG( "quitgame, account:%s, ip:%d.%d.%d.%d, uid:%d", lu.account, lu.loginIp.S_un.S_un_b.s_b1,lu.loginIp.S_un.S_un_b.s_b2,lu.loginIp.S_un.S_un_b.s_b3,lu.loginIp.S_un.S_un_b.s_b4, uid );
			RecordLogout( 'L', uid, lu.logintime, lu.queuetime, lu.serverid, lu.loginIp, config.gameId, lu.account, lu.stat, lu.ssn, lu.ssn2, lu.gender, lu.age, lu.cdkind );
		}
	}
_AFTER_FIN
	return result;	
}

void AccountDB::transferPlayer(int uid, unsigned char shard)
{
	ServerId serverid(shard);
    m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if (it != usermap.end())
	{
		it->second.selectedServerid = serverid;
	}
	else
	{
		// error.  WTF do we do?
	}
    m_lock.Leave();
}

int AccountDB::checkInGame( int uid, int md5key )
{
_BEFORE
	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()){
		if( it->second.um_mode == UM_IN_GAME ){
				m_lock.Leave();
				return S_ALREADY_LOGIN;
		}
		if ( it->second.md5key != md5key ){
			m_lock.Leave();
			return S_INCORRECT_MD5Key;
		}
	} else {
		m_lock.Leave();
		return S_NO_LOGININFO;
	}
	m_lock.Leave();
_AFTER_FIN
	return S_ALL_OK;
}

bool AccountDB::RecordLogout( char reasoncode, int uid, time_t loginTime, time_t enteredQueueTime, ServerId lastWorldId, in_addr LastIP, int LastGame, const char *account, int stat, int ssn1, int ssn2, char gender, int age, int cdkind )
{
	time_t logout_time;
	
	struct tm logoutTM;
	struct tm loginTM;
	struct tm queueloginTM;
	char szIP[16];
	unsigned long lastWorld = lastWorldId.GetValueChar();

	sprintf( szIP, "%d.%d.%d.%d", LastIP.S_un.S_un_b.s_b1,
								  LastIP.S_un.S_un_b.s_b2,
								  LastIP.S_un.S_un_b.s_b3,
								  LastIP.S_un.S_un_b.s_b4);
	
	TIMESTAMP_STRUCT dblogout, 
					 dblogin,
					 dbqueuelogin;


	logout_time = time(0);
	
	int usetime = (int)(logout_time - loginTime);
	
	WriteLogD( LOG_ACCOUNT_LOGOUT, (char *)account, LastIP, stat, age, gender, 0, usetime, uid );  
	

	logoutTM = *localtime(&logout_time);
	loginTM  = *localtime(&loginTime);
	queueloginTM = *localtime(&enteredQueueTime);
	
	dblogout.year     = logoutTM.tm_year + 1900;
	dblogout.month    = logoutTM.tm_mon + 1;
	dblogout.day      = logoutTM.tm_mday;
	dblogout.hour     = logoutTM.tm_hour;
	dblogout.minute   = logoutTM.tm_min;
	dblogout.second   = logoutTM.tm_sec;
	dblogout.fraction = 0;

	dblogin.year     = loginTM.tm_year + 1900;
	dblogin.month    = loginTM.tm_mon + 1;
	dblogin.day      = loginTM.tm_mday;
	dblogin.hour     = loginTM.tm_hour;
	dblogin.minute   = loginTM.tm_min;
	dblogin.second   = loginTM.tm_sec;
	dblogin.fraction = 0;

	dbqueuelogin.year     = queueloginTM.tm_year + 1900;
	dbqueuelogin.month    = queueloginTM.tm_mon + 1;
	dbqueuelogin.day      = queueloginTM.tm_mday;
	dbqueuelogin.hour     = queueloginTM.tm_hour;
	dbqueuelogin.minute   = queueloginTM.tm_min;
	dbqueuelogin.second   = queueloginTM.tm_sec;
	dbqueuelogin.fraction = 0;

	{
		CDBConn conn(g_linDB);

		SQLINTEGER cbUid=0;
		SQLBindParameter( conn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&uid), 0, &cbUid );
		
		SQLINTEGER cblastlogin=0;
		SQLBindParameter( conn.m_stmt, 2, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,SQL_TIMESTAMP, 19, 0, (SQLPOINTER)&dblogin, 0, &cblastlogin );
		
		SQLINTEGER cblastlogout=0;
		SQLBindParameter( conn.m_stmt, 3, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,SQL_TIMESTAMP, 19, 0, (SQLPOINTER)&dblogout, 0, &cblastlogout );
		
		SQLINTEGER cblastgame=0;
		SQLBindParameter( conn.m_stmt, 4, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&LastGame), 0, &cblastgame );

		SQLINTEGER cblastworld=0;
		SQLBindParameter( conn.m_stmt, 5, SQL_PARAM_INPUT, SQL_C_UTINYINT, SQL_TINYINT, 0, 0, (SQLPOINTER)(&lastWorldId), 0, &cblastworld );

		SQLINTEGER cblastIP=SQL_NTS;
		SQLBindParameter( conn.m_stmt, 6, SQL_PARAM_INPUT, SQL_C_TCHAR, SQL_VARCHAR, 15, 0, (SQLPOINTER)szIP, (SQLINTEGER)strlen(szIP), &cblastIP );

		char buffer[256];
		sprintf( buffer, "{CALL dbo.ap_SLog (?,?,?,?,?,?) }" );
		RETCODE RetCode= SQLExecDirect( conn.m_stmt, (SQLCHAR*)buffer, SQL_NTS );
		
		if ( RetCode == SQL_SUCCESS ) {
			conn.ResetHtmt();
		} else {
			conn.Error(SQL_HANDLE_STMT, conn.m_stmt, buffer);
			conn.ResetHtmt();
		}

	}

	{ //NEW for NCAustin version of auth server, this allows us to have a login/logout history
	  //instead of just the last login/logout record which is also the system does normally

		CDBConn conn(g_linDB);

		//Add logout entry to the activity log
		SQLINTEGER accountName_strLen=SQL_NTS;
		SQLBindParameter( conn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 14, 0, (SQLPOINTER)account, 14, &accountName_strLen);

		SQLINTEGER userIdLen = 4;
		SQLBindParameter( conn.m_stmt, 2, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&uid), 0, &userIdLen);

 
		SQLINTEGER serverIdLen = 4;
		SQLBindParameter( conn.m_stmt, 3, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&lastWorld, 0, &serverIdLen);


		SQLINTEGER clientIPLen=SQL_NTS;
		SQLBindParameter( conn.m_stmt, 4, SQL_PARAM_INPUT, SQL_C_TCHAR, SQL_VARCHAR, 15, 0, (SQLPOINTER)szIP, (SQLINTEGER)strlen(szIP), &clientIPLen );


		SQLINTEGER lastLoginLen=0;
		SQLBindParameter( conn.m_stmt, 5, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,SQL_TIMESTAMP, 19, 0, (SQLPOINTER)&dblogin, 0, &lastLoginLen );

		SQLINTEGER queueLoginLen=0;
		SQLBindParameter( conn.m_stmt, 6, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,SQL_TIMESTAMP, 19, 0, (SQLPOINTER)&dbqueuelogin, 0, &queueLoginLen );
	
		//Current system doesnt keep track of different auth logins and game logins, it just overwrites
		//the same login value on the user account
		SQLINTEGER gameLoginLen=0;
		SQLBindParameter( conn.m_stmt, 7, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,SQL_TIMESTAMP, 19, 0, (SQLPOINTER)&dblogin, 0, &gameLoginLen );
	

		SQLINTEGER lastLogoutLen=0;
		SQLBindParameter( conn.m_stmt, 8, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,SQL_TIMESTAMP, 19, 0, (SQLPOINTER)&dblogout, 0, &lastLogoutLen );
	
	    SQLINTEGER logoutTypeLen=0;
		SQLBindParameter( conn.m_stmt, 9, SQL_PARAM_INPUT, SQL_C_CHAR,SQL_CHAR, 1, 0, (SQLPOINTER)&reasoncode, 1, &logoutTypeLen );

		SQLINTEGER cdKindLen=0;
		SQLBindParameter( conn.m_stmt, 10, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)&cdkind, 1, &cdKindLen );

		char buffer[256];
		sprintf( buffer, "{CALL dbo.sp_LogAuthActivity (?,?,?,?,?,?,?,?,?,?) }" );
		RETCODE RetCode= SQLExecDirect( conn.m_stmt, (SQLCHAR*)buffer, SQL_NTS );
		
		if ( RetCode == SQL_SUCCESS ) {
			conn.ResetHtmt();
		} else {
			conn.Error(SQL_HANDLE_STMT, conn.m_stmt, buffer);
			conn.ResetHtmt();
		}
		
	}

	filelog.AddLog( LOG_NORMAL, "%d-%d-%d %d:%d:%d,%d-%d-%d %d:%d:%d,%s,%d,%s,%d,%d,%d,%06d%07d,%d,%d,%d,%d\r\n", 
				logoutTM.tm_year + 1900, logoutTM.tm_mon + 1, logoutTM.tm_mday,
				logoutTM.tm_hour, logoutTM.tm_min, logoutTM.tm_sec,
				loginTM.tm_year + 1900, loginTM.tm_mon + 1, loginTM.tm_mday,
				loginTM.tm_hour, loginTM.tm_min, loginTM.tm_sec,
				account,
				lastWorld,
				szIP, 
				stat,
				usetime,
				usetime,
				ssn1, ssn2,
				gender, logoutTM.tm_wday, age, cdkind);
	
	int OperationCode = (int)(( stat % 1000 ) / 100 );

	if ( (stat < 1000) && ( stat > 0)){
		ipsessionDB.StopIPCharge( uid, LastIP.S_un.S_addr, stat, usetime, loginTime, lastWorldId, account );
	
	} else if ( OperationCode == PERSONAL_SPECIFIC ) {

		CDBConn conn(g_linDB);
		SQLINTEGER cUseTime=0;
		SQLBindParameter( conn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&usetime), 0, &cUseTime );
		SQLINTEGER cbUid=0;
		SQLBindParameter( conn.m_stmt, 2, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&uid), 0, &cbUid );

		char buffer[256];
		sprintf( buffer, "{CALL dbo.ap_SUserTime (?,?) }" );
		RETCODE RetCode= SQLExecDirect( conn.m_stmt, (SQLCHAR*)buffer, SQL_NTS );
		if ( RetCode == SQL_SUCCESS ) {
			conn.ResetHtmt();
		} else {
			conn.Error(SQL_HANDLE_STMT, conn.m_stmt, buffer);
			conn.ResetHtmt();
		}

	} else if ( OperationCode == PERSONAL_POINT ) {
		CDBConn conn(g_linDB);
		SQLINTEGER cbAccount=0;
		SQLINTEGER cbTime=0;
		char buffer[256];
		SQLBindParameter( conn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_TCHAR, SQL_VARCHAR, MAX_ACCOUNT_LEN, 0, (SQLPOINTER)account, (SQLINTEGER)strlen(account), &cbAccount );
		SQLINTEGER cblastlogin=0;
		SQLBindParameter( conn.m_stmt, 2, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,SQL_TIMESTAMP, 19, 0, (SQLPOINTER)&dblogin, 0, &cblastlogin );
		SQLINTEGER cblastlogout=0;
		SQLBindParameter( conn.m_stmt, 3, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,SQL_TIMESTAMP, 19, 0, (SQLPOINTER)&dblogout, 0, &cblastlogout );

		sprintf( buffer, "{CALL dbo.ap_LogoutWithPoint( ?,?,? )}" );
		RETCODE RetCode = SQLExecDirect( conn.m_stmt, (SQLCHAR*)buffer, SQL_NTS);
		if ( RetCode == SQL_SUCCESS ){
			conn.ResetHtmt();
		} else {
			conn.Error( SQL_HANDLE_STMT, conn.m_stmt, buffer );
			conn.ResetHtmt();
		}
	}

	InterlockedDecrement( &reporter.m_InGameUser );	

	return true;
}

// 정량 결제를 체크한다. 
// 남은 시간이 있는지를 검사해서 있으면 등록하고 없으면 남은 시간 없음을 표시한다. 

char AccountDB::UserTimeLogin( int uid, LoginUser *lu, int *RemainTime )
{
	char ErrorCode=S_ALL_OK;

	ErrorCode = accountdb.CheckUserTime( uid, RemainTime );

	if ( ErrorCode == S_ALL_OK ) {
	} else {
		ErrorCode = S_NO_SPECIFICTIME;
	}
	return ErrorCode;
}

// 개인 정액및 정량 결제를 체크 한다. 
char AccountDB::CheckPersonalPayStat(CSocketServerEx *pSocket, LoginUser *lu, int Uid)
{
	char result=S_ALL_OK;

	int OperationCode = (int)(( lu->stat% 1000 ) / 100 );
	int RemainTime=0;

//  결제 stat이  0이면 더이상 체크할 필요가 없다. 
	if ( lu->stat == 0 ) {
		result = S_NOT_PAID;
	} else if ( OperationCode == PERSONAL_SPECIFIC ) {
		// 정량 결제를 체크해서 성공이면 밑으로 진행. 실패이면 사용자에게 결과 통보하고 끝
		result=UserTimeLogin( Uid, lu, &RemainTime );
	} else if ( OperationCode == PERSONAL_POINT ) {
		CDBConn dbconn(g_linDB);
		
	}

	// 정액과 정량결제 모두 실패인 경우
	if ( result != S_ALL_OK ) {
		pSocket->Send( "cc", AC_LOGIN_FAIL, result );
		return result; // 로그인 Fail 이므로 더이상 진행할 필요 없다. 
	}
	
	// 정액과 정량 둘중에 하나라도 통과하면  사용 가능한 계정으로 등록한다. 등록에 실패하면  이중 로그인이 된다. 
	if ( accountdb.RegAccount( lu, Uid, pSocket, RemainTime, 0 ) ){

	    log.AddLog( LOG_VERBOSE, "SND: AC_LOGIN_OK,uid:%d,account:%s", Uid, lu->account);

		pSocket->m_lastIO = GetTickCount();
//		WriteAction( "login", lu->account, lu->loginIp, lu->ssn, lu->gender, 0, lu->stat );
		WriteLogD( LOG_ACCOUNT_AUTHED, lu->account, lu->loginIp, lu->stat, lu->age, lu->gender, 0, reporter.m_UserCount, Uid );
	} else{
		
        log.AddLog( LOG_WARN, "SND: AC_LOGIN_FAIL,uid:%d,account:%s,ip:%d.%d.%d.%d,%x", Uid, lu->account, lu->loginIp.S_un.S_un_b.s_b1, lu->loginIp.S_un.S_un_b.s_b2, lu->loginIp.S_un.S_un_b.s_b3, lu->loginIp.S_un.S_un_b.s_b4, pSocket->GetSocket() );
	}

	return result;
}

char AccountDB::CheckUserTime(int Uid, int *RemainTime)
{
	// UserTimeLogin을 Check할 경우에는 user_time 테이블이 존재한다.
	// Logic : user_time Table에서 total_time을 Select해온다. 
	char ErrorCode=S_ALL_OK;

	CDBConn conn(g_linDB);
	
	SQLINTEGER cbUid=0;
	SQLBindParameter( conn.m_stmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(&Uid), 0, &cbUid );
	
	SQLINTEGER cbUserTime=0;
	SQLBindParameter( conn.m_stmt, 2, SQL_PARAM_OUTPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, (SQLPOINTER)(RemainTime), 0, &cbUserTime );
	
	char buffer[256];
	sprintf( buffer, "{CALL dbo.ap_GUserTime (?,?) }" );
	RETCODE RetCode= SQLExecDirect( conn.m_stmt, (SQLCHAR*)buffer, SQL_NTS );	
	
	bool nodata;
	if ( RetCode == SQL_SUCCESS ) {
		conn.Fetch(&nodata);
		if ( *RemainTime <= 0 ){
			ErrorCode = S_NO_SPECIFICTIME;
		}
	}else{
		ErrorCode = S_DATABASE_FAIL;
	}
	
	conn.ResetHtmt();

	return ErrorCode;
}

// 2003-07-06 darkangel
// 서버 선택후에 선택된 서버로 사용자 정보를 보내주도록 한다. 

char AccountDB::AboutToPlay(int uid, char *account, int time_left, int loginflag, int warnflag, int md5key, CSocketServerEx *pSocket, ServerId serverid, int stat, int queueLevel, int loyalty, int loyaltyLegacy)
{
	char error = S_ALL_OK;

	int result=0;

	if (config.payStatOverride != -1)
	{
		stat = config.payStatOverride;
		log.AddLog(LOG_WARN, "PayStatOverride is set to %d!", config.payStatOverride);
	}

	// user data가 필요한 경우에 user_data를 읽어 오도록 한다. 
	if ( config.UserData ){
		unsigned char userdata[MAX_USERDATA];
		
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

		int len = MAX_USERDATA;

		if ( !g_ServerList.IsServerUp(serverid) )
		{
#ifdef _DEBUG
			log.AddLog( LOG_ERROR, "Invalid Serverid :%d, %s", serverid, account );
#endif 
		} else {
			AS_LOG_VERBOSE( "User data from SQL for uid %d: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x",
				uid,
				userdata[ 0], userdata[ 1], userdata[ 2], userdata[ 3],
				userdata[ 4], userdata[ 5], userdata[ 6], userdata[ 7],
				userdata[ 8], userdata[ 9], userdata[10], userdata[11],
				userdata[12], userdata[13], userdata[14], userdata[15]);
			char filler[32];
			_snprintf_s(filler, sizeof(filler), sizeof(filler) - 1, "%d", uid);
			int i;
			for (i = 0; filler[i]; i++)
			{
				filler[i] = ' ';
			}
			for (i = 16; i < MAX_USERDATA; i += 16)
			{
				AS_LOG_VERBOSE( "                           %s  %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x",
					filler,
				userdata[i +  0], userdata[i +  1], userdata[i +  2], userdata[i +  3],
				userdata[i +  4], userdata[i +  5], userdata[i +  6], userdata[i +  7],
				userdata[i +  8], userdata[i +  9], userdata[i + 10], userdata[i + 11],
				userdata[i + 12], userdata[i + 13], userdata[i + 14], userdata[i + 15]);
			}
			if (config.useQueue || config.sendQueueLevel)
			{
				result = SendSocket( g_ServerList.GetInternalAddress(serverid),
						"cdsdddbdc",
						SQ_ABOUT_TO_PLAY, uid, account, time_left, loginflag, warnflag, len, userdata, stat, queueLevel );
			}
			else
			{
				result = SendSocket( g_ServerList.GetInternalAddress(serverid),
					"cdsdddbddd",
					SQ_ABOUT_TO_PLAY, uid, account, time_left, loginflag, warnflag, len, userdata, stat, loyalty, loyaltyLegacy );
			}
		}

	} else {
		if ( !g_ServerList.IsServerUp(serverid) )
		{
#ifdef _DEBUG
			log.AddLog( LOG_ERROR, "Invalid Serverid :%d, %s", serverid, account );
#endif
		}else{
			// server에 제대로 데이타를 보내면.
			result = SendSocket( g_ServerList.GetInternalAddress(serverid), "cdsdddd", SQ_ABOUT_TO_PLAY, uid, account, time_left, loginflag, warnflag, stat, queueLevel );
		}
	}

	// pSocket이 NULL이 아닐때에만 동작해야 한다....
	if ( pSocket ) {
		if ( result == FALSE ){
			AS_LOG_VERBOSE( "SND: AC_PLAY_FAIL,server down", S_SERVER_DOWN );
			error = S_SERVER_DOWN;
			pSocket->Send( "cc", AC_PLAY_FAIL, error );		
		} else {

			AS_LOG_VERBOSE( "SND: SQ_ABOUT_TO_PLAY,account:%s", account );
		
			// 소켓을 업데이트 한다. 
			if( (error=UpdateSocket( uid, pSocket->GetSocket(), md5key, serverid )) != S_ALL_OK){
				AS_LOG_VERBOSE( "SND: AC_PLAY_FAIL,error:%d", error);
				pSocket->Send( "cc", AC_PLAY_FAIL, error );		
			}
		}
	}

	return error;
}


// 2003-07-06 darkangel
// 사용자 정보를 저장된 userMap에서 가져온다. 
// StartIPCharge에서 부르며 그때는 이미 로그인도 되고 게임도 된다는 것이기 때문에 Timer를 지워야 한다.

bool AccountDB::GetAccountInfo( int uid, char *account, int *loginflag, int *warnflag, int *md5key, SOCKET *s )
{
	bool result=false;

	HANDLE hTimer=NULL;

	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()) {
		strncpy( account, it->second.account, MAX_ACCOUNT_LEN+1 );
		*loginflag = it->second.loginflag;
		*warnflag = it->second.warnflag;
		*s = it->second.s;
		*md5key = it->second.md5key;
		hTimer = it->second.timerHandle;
		it->second.timerHandle = NULL;
		result = true;
	}
	m_lock.Leave();
	
	if ( hTimer != NULL  )
		DeleteTimerQueueTimer( NULL, hTimer, NULL );

	return result;
}

// IP 과금을 정지 시키기 위한 정보를 복사해 올때 사용한다. 
bool AccountDB::GetAccountInfoForIPStop( int uid, char *account, int *stat, in_addr *loginip, time_t *loginTime )
{
	bool result = false;

	m_lock.Enter();
	UserMap::iterator it = usermap.find(uid);
	if ( it != usermap.end()){
		strncpy( account, it->second.account, MAX_ACCOUNT_LEN+1);
		*stat = it->second.stat;
		*loginip = it->second.loginIp;
		*loginTime = it->second.logintime;
		m_lock.Leave();
		result = true;
	} else
		m_lock.Leave();
	
	return result;
}

bool AccountDB::RegAccountByServer( LoginUser *loginuser, int uid, CSocketServer *s, int remainTime, int quotaTime )
{
	bool result = false;

	m_lock.Enter();
	std::pair<UserMap::iterator, bool> r = usermap.insert(UserMap::value_type(uid, *loginuser));
	result = r.second;
	m_lock.Leave();
	
	if ( result == false ){
		// kick account를 할경우 메세지를 보낼지는 사실 결정 안해도 된다. 
		// 다시 로그인 하라고 하니까.
		KickAccount( uid,S_ALREADY_LOGIN, true );
	}  else {
		// 그냥 잘 로그인 된거다. 뭐 
	}
	return result;
}	
