#ifndef CRYPTOPP_SOCKETFT_H
#define CRYPTOPP_SOCKETFT_H

#include "config.h"

#ifndef NO_OS_DEPENDENCE

#ifdef __GNUC__
#include <_G_config.h>
#endif

#if defined(_G_HAVE_SYS_SOCKET) && _G_HAVE_SYS_SOCKET
#define HAS_BERKELEY_STYLE_SOCKETS
#endif

#if defined(_WIN32)
#define HAS_WINDOWS_STYLE_SOCKETS
#endif

#include "hrtimer.h"

#if defined(HIGHRES_TIMER_AVAILABLE) && (defined(HAS_BERKELEY_STYLE_SOCKETS) || defined(HAS_WINDOWS_STYLE_SOCKETS))
#define SOCKETS_AVAILABLE
#endif

#endif	// #ifndef NO_OS_DEPENDENCE

#ifdef SOCKETS_AVAILABLE

#if defined(HAS_WINDOWS_STYLE_SOCKETS) && (!defined(HAS_BERKELEY_STYLE_SOCKETS) || defined(PREFER_WINDOWS_STYLE_SOCKETS))
#define USE_WINDOWS_STYLE_SOCKETS
#else
#define USE_BERKELEY_STYLE_SOCKETS
#endif

#include "network.h"
#include "queue.h"

#ifdef USE_WINDOWS_STYLE_SOCKETS
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

NAMESPACE_BEGIN(CryptoPP)

#ifdef USE_WINDOWS_STYLE_SOCKETS
typedef ::SOCKET socket_t;
#else
typedef int socket_t;
const socket_t INVALID_SOCKET = -1;
// cygwin 1.1.4 doesn't have SHUT_RD
const int SD_RECEIVE = 0;
const int SD_SEND = 1;
const int SD_BOTH = 2;
#endif

#ifndef socklen_t	// "#define socklen_t int" appears in socket.h on cygwin
#ifdef HAS_WINDOWS_STYLE_SOCKETS	// use HAS_ instead of USE_ because cygwin doesn't have socklen_t
typedef int socklen_t;
#else
typedef ::socklen_t socklen_t;
#endif
#endif

//! wrapper for Windows or Berkeley Sockets
class Socket
{
public:
	//! exception thrown by Socket class
	class Err : public Exception
	{
	public:
		Err(socket_t s, const std::string& operation, int error);
		~Err() throw() {}	// needed with GCC 3.0.2, not sure why

		socket_t GetSocket() const {return m_s;}
		const std::string & GetOperation() const {return m_operation;}
		int GetError() const {return m_error;}

	private:
		socket_t m_s;
		std::string m_operation;
		int m_error;
	};

	Socket(socket_t s = INVALID_SOCKET, bool own=false) : m_s(s), m_own(own) {}
	Socket(const Socket &s) : m_s(s.m_s), m_own(false) {}
	virtual ~Socket();

	bool GetOwnership() const {return m_own;}
	void SetOwnership(bool own) {m_own = own;}

	operator socket_t() {return m_s;}
	socket_t GetSocket() {return m_s;}
	void AttachSocket(socket_t s, bool own=false);
	socket_t DetachSocket();
	void CloseSocket();

	void Create(int nType = SOCK_STREAM);
	void Bind(unsigned int port, const char *addr=NULL);
	void Bind(const sockaddr* psa, socklen_t saLen);
	void Listen(int backlog=5);
	// the next three functions return false if the socket is in nonblocking mode
	// and the operation cannot be completed immediately
	bool Connect(const char *addr, unsigned int port);
	bool Connect(const sockaddr* psa, socklen_t saLen);
	bool Accept(Socket& s, sockaddr *psa=NULL, socklen_t *psaLen=NULL);
	void GetSockName(sockaddr *psa, socklen_t *psaLen);
	unsigned int Send(const byte* buf, unsigned int bufLen, int flags=0);
	unsigned int Receive(byte* buf, unsigned int bufLen, int flags=0);
	void ShutDown(int how = SD_SEND);

	void IOCtl(long cmd, unsigned long *argp);
	bool SendReady(const timeval *timeout);
	bool ReceiveReady(const timeval *timeout);

	//! look up the port number given its name, returns 0 if not found
	static unsigned int PortNameToNumber(const char *name, const char *protocol="tcp");
	//! start Windows Sockets 2
	static void StartSockets();
	//! calls WSACleanup for Windows Sockets
	static void ShutdownSockets();
	//! returns errno or WSAGetLastError
	static int GetLastError();
	//! sets errno or calls WSASetLastError
	static void SetLastError(int errorCode);

protected:
	virtual void SocketChanged() {}
	virtual void CheckAndHandleError(const char *operation, int result) const;
#ifdef USE_WINDOWS_STYLE_SOCKETS
	virtual void CheckAndHandleError(const char *operation, socket_t result) const;
#endif

	socket_t m_s;
	bool m_own;
};

//! contributed by Denis Bider
class SocketsInitializer
{
public:
	SocketsInitializer() {Socket::StartSockets();}
	~SocketsInitializer() {try {Socket::ShutdownSockets();} catch (...) {}}
};

//! .
class SocketSource : public Socket, public NetworkSource
{
public:
	SocketSource(socket_t s = INVALID_SOCKET, bool pumpAndClose = false, BufferedTransformation *outQueue = NULL);

	bool ReceiveReady(unsigned long timeout=0);
	bool Receive(byte* buf, unsigned int bufLen);
	bool ReceiveResultReady(unsigned long timeout=0) {return true;}
	unsigned int GetReceiveResult() {return m_lastResult;}
	bool EofReceived() const {return m_eofReceived;}

private:
	unsigned int m_lastResult;
	bool m_eofReceived;
};

//! .
class SocketSink : public Socket, public NetworkSink
{
public:
	SocketSink(socket_t s = INVALID_SOCKET, unsigned int maxBufferSize=0, bool autoFlush=false);

	bool SendReady(unsigned long timeout=0);
	bool Send(const byte* buf, unsigned int bufLen);
	bool SendResultReady(unsigned long timeout=0) {return true;}
	unsigned int GetSendResult() {return m_lastResult;}
	void SendEof() {ShutDown(SD_SEND);}

private:
	unsigned int m_lastResult;
};

NAMESPACE_END

#endif	// #ifdef SOCKETS_AVAILABLE

#endif
