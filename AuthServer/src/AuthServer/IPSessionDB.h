// IPSessionDB.h: interface for the CIPSessionDB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IPSESSIONDB_H__A0282416_467D_443B_A67A_355A35748F4F__INCLUDED_)
#define AFX_IPSESSIONDB_H__A0282416_467D_443B_A67A_355A35748F4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AuthType.h"
#include "IOSocket.h"

class CIPSessionDB  
{
private:
	// IPSession Value와 account Mapping DB ( 로그인 되었을때 )
	SESSIONMAP IPSessionMap; // uid로 세션을 담아 둔다. 
	// 로그인전에 LoingUser Info를 담아둘 DB가 필요하다.
	UserPointerMap    WaitingUser;
	CLock WaitUserLock;
	CLock IPSessionLock;

	bool AddUserWait( int uid, LoginUser *lu );
	bool DelUserWait( int uid, LoginUser **lu );
public:
	char StopIPCharge( UINT uid, UINT ip, int kind, int UseTime, time_t loginTime, ServerId lastworld, const char *account );
	char StartIPCharge( UINT uid, UINT ip, int kind, ServerId WorldID);
	char ReleaseSessionRequest(int IPSession,in_addr IP, int kind);
	
	char ReadyToIPCharge( UINT uid, UINT ip, int kind, ServerId WorldID );
	char ConfirmIPCharge( UINT uid, UINT ip, int kind, ServerId WorldID );

	char AcquireSessionRequest(LoginUser *lu, int uid);
	char AcquireSessionSuccess( int Uid, int IPSession, char ErrorCode, int SpecificTime=0, int Kind=0 );
	char AcquireSessionFail( int Uid, int IPSession, char ErrorCode );
	
	int FindSessionID( int Uid );
	int DelSessionID( int Uid );
	int AddSessionID ( int Uid, int IPSession );
	bool DellAllWaitingSessionID( void );

	CIPSessionDB();
	virtual ~CIPSessionDB();

};

class CIPSocket;

typedef CIPSocket* (*IPSocketAllocator)(SOCKET s);
typedef bool (*IPPacketFunc)(CIPSocket* , const unsigned char* packet);

class CIPSocket
: public CIOSocket
{
public:
//	bool reconnect;

	CIPSocket(SOCKET sock);

	virtual ~CIPSocket();
	static CIPSocket* Allocate(SOCKET s);
	virtual void OnCreate();
	virtual void OnRead();
	virtual void OnClose(SOCKET closedSocket);
	virtual void OnTimerCallback( );
	friend class CIPPacketServer;
	
	in_addr getaddr( void ) { return addr; };
	void SetAddress( in_addr in_Addr ){
		addr = in_Addr;
	};
	void SetConnectSessionKey( UINT sessionKey ){
		ConnectSessionKey = sessionKey;
	}
	bool Send(const char* format, ...);
	const char * IP();

//	char serverid;
	int opFlag;
	UINT ConnectSessionKey;
protected:
	int packetLen;

	SocketMode mode;
	IPPacketFunc *packetTable;
	in_addr addr;
	sockaddr_in Destination;
	char* host;
};


class CIPPacketServer : public CIOObject
{
public:
	typedef bool (*IPPacketFunc)(CIOSocket* , const unsigned char* packet);

	CIOSocket *m_pSocket;
	CIOBuffer   *m_pBuf;
	IPPacketFunc	m_pFunc;
	static LONG g_nPendingPacket;
	static CIPPacketServer* Alloc();
	static void FreeAll();
	void Free();
	virtual void OnIOCallback(BOOL bSuccess, DWORD dwTransferred, LPOVERLAPPED lpOverlapped);
};

extern CIPSocket *pIPSocket;
extern CIPSessionDB ipsessionDB;
extern bool IPServerReconnect;
extern CRWLock gIPLock;
#endif // !defined(AFX_IPSESSIONDB_H__A0282416_467D_443B_A67A_355A35748F4F__INCLUDED_)
