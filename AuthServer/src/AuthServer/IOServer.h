// IOServer.h: interface for the CIOServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOSERVER_H__4F72879C_E81F_4563_9239_7CE0E9BF84D5__INCLUDED_)
#define AFX_IOSERVER_H__4F72879C_E81F_4563_9239_7CE0E9BF84D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Lock.h"
#include "IOObject.h"
#include "IOSocket.h"

#define ACCEPT_BUFFER_SIZE 256

typedef struct _OverlappedAccept
{
    OVERLAPPED ol;
    SOCKET sock;
    char buffer[ACCEPT_BUFFER_SIZE];
    struct _OverlappedAccept *next;
} OverlappedAccept;

typedef std::map<unsigned long, CSocketServer*> SocketMap;
typedef std::map<SOCKET, CSocketServerEx*> SocketExMap;
typedef std::set< OverlappedAccept* > AcceptExSocketMap;

class CIOServer : public CIOObject  
{
public:
	HANDLE m_hAcceptEvent;
	SOCKET m_hSocket;
	
	CIOServer();
	virtual ~CIOServer();

	BOOL Create( int nPort );
	void Close();
	void Stop();

	virtual CIOSocket * CreateSocket( SOCKET s, LPSOCKADDR_IN pAddress ) = 0;
	virtual void OnIOCallback( BOOL bSuccess, DWORD dwTransferred, LPOVERLAPPED lpOverlapped );
	virtual void OnEventCallback( void );
};

typedef CSocketServer* (*SocketAllocator)(SOCKET s);
typedef CSocketServerEx* (*SocketExAllocator)(SOCKET s);
typedef CSocketInt* (*SocketIntAllocator)(SOCKET s);

class CServer : public CIOServer
{
private:
	SocketMap socketmap;	
public:
	HANDLE listenThread;
	bool shutdown;

	CServer();
	~CServer();

	void Run(int port, SocketAllocator allocator, bool restrict);
	void Stop();
	bool InShutdown();
	SOCKET listener;
	HANDLE completionPort;
	CRITICAL_SECTION sockSect;
	SocketAllocator allocator;
	bool restrict;
	CSocketServer *FindSocket( in_addr s );
	virtual CIOSocket *CreateSocket(SOCKET newSocket, LPSOCKADDR_IN pAddress);
	void RemoveSocket( in_addr s );
	bool GetServerStatus( in_addr s );
};

class CServerInt : public CIOServer
{
public:
	HANDLE listenThread;
	bool shutdown;

	CServerInt();
	~CServerInt();

	void Run(int port, SocketIntAllocator allocator);
	void Stop();
	bool InShutdown();
	SOCKET listener;
	HANDLE completionPort;
	CRITICAL_SECTION sockSect;
	SocketIntAllocator allocator;
	bool restrict;

	virtual CIOSocket *CreateSocket(SOCKET newSocket, LPSOCKADDR_IN pAddress);
	virtual void OnIOCallback( BOOL bSuccess, DWORD dwTransferred, LPOVERLAPPED lpOverlapped );
};

class OverlappedPool
{
public:
	class Slot
	{
	public:
		OverlappedAccept *m_pOverlapped;
		CLock m_lock;

		Slot() : m_pOverlapped(NULL), m_lock(eCustomSpinLock) {}
	};
	static Slot m_slot[16];
	static long	m_alloc;
	static long	m_free;

	static OverlappedAccept *Alloc();
    static void Free(OverlappedAccept *pOverlapped);
    static void FreeAll();
    ~OverlappedPool() { FreeAll(); }
};

class CIOServerEx : public CIOObject  
{
private:
	SocketExMap socketmap;
	CRITICAL_SECTION sockSect;

public:
	SOCKET m_hSocket;
	SOCKET m_hAcceptSocket;

	HANDLE m_acceptEvent;

	CIOServerEx();
	virtual ~CIOServerEx();

	BOOL Create( int nPort );
	void Close();
	void Stop();

	virtual CSocketServerEx * CreateSocket( SOCKET s, LPSOCKADDR_IN pAddress );
	virtual void OnIOCallback( BOOL bSuccess, DWORD dwTransferred, LPOVERLAPPED lpOverlapped );
	virtual void OnEventCallback();

	static DWORD WINAPI OnAcceptExCallback( LPVOID param );
	bool WrapAcceptEx( void );
	void Run(int listenPort, SocketExAllocator alloc);
	SocketExAllocator allocator;
	CSocketServerEx *FindSocket( SOCKET s);
	bool RemoveSocket( SOCKET sock );

};

extern CServer *server;
extern CIOServerEx *serverEx;
extern CServerInt *serverInt;

#endif // !defined(AFX_IOSERVER_H__4F72879C_E81F_4563_9239_7CE0E9BF84D5__INCLUDED_)
