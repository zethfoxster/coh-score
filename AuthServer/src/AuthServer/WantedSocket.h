// WantedSocket.h: interface for the CWantedSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WANTEDSOCKET_H__B350EB8B_18D2_4B9F_9CE3_1E21DC9593EA__INCLUDED_)
#define AFX_WANTEDSOCKET_H__B350EB8B_18D2_4B9F_9CE3_1E21DC9593EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AuthType.h"
#include "IOSocket.h"

class CWantedSocket;

typedef CWantedSocket* (*WantedSocketAllocator)(SOCKET s);
typedef bool (*WantedPacketFunc)(CWantedSocket* , const unsigned char* packet);

class CWantedSocket : public CIOSocket  
{
public:
	CWantedSocket( SOCKET s);
	virtual ~CWantedSocket();
	static CWantedSocket* Allocate(SOCKET s);
	virtual void OnCreate();
	virtual void OnRead();
	virtual void OnClose(SOCKET closedSocket);
	virtual void OnTimerCallback( );
	friend class CWantedPacketServer;

	in_addr getaddr( void ) { return addr; };
	void SetAddress( in_addr in_Addr ){
		addr = in_Addr;
	};
	bool Send(const char* format, ...);
	const char * IP();

protected:
	int packetLen;

	SocketMode mode;
	WantedPacketFunc *packetTable;
	in_addr addr;
	sockaddr_in WantedService;
	char* host;
};

class CWantedPacketServer : public CIOObject
{
public:
	typedef bool (*WantedPacketFunc)(CIOSocket* , const unsigned char* packet);

	CIOSocket *m_pSocket;
	CIOBuffer   *m_pBuf;
	WantedPacketFunc	m_pFunc;
	static LONG g_nPendingPacket;
	static CWantedPacketServer* Alloc();
	static void FreeAll();
	void Free();
	virtual void OnIOCallback(BOOL bSuccess, DWORD dwTransferred, LPOVERLAPPED lpOverlapped);
};

extern CWantedSocket *pWantedSocket;
extern bool WantedServerReconnect;
extern CRWLock gWantedLock;


#endif // !defined(AFX_WANTEDSOCKET_H__B350EB8B_18D2_4B9F_9CE3_1E21DC9593EA__INCLUDED_)
