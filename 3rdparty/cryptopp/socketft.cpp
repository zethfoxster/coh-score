// socketft.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "socketft.h"

#ifdef SOCKETS_AVAILABLE

#ifdef USE_BERKELEY_STYLE_SOCKETS
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#endif

NAMESPACE_BEGIN(CryptoPP)

#ifdef USE_WINDOWS_STYLE_SOCKETS
const int SOCKET_EINVAL = WSAEINVAL;
const int SOCKET_EWOULDBLOCK = WSAEWOULDBLOCK;
typedef int socklen_t;
#else
const int SOCKET_ERROR = -1;
const int SOCKET_EINVAL = EINVAL;
const int SOCKET_EWOULDBLOCK = EWOULDBLOCK;
#endif

Socket::Err::Err(socket_t s, const std::string& operation, int error)
	: Exception("Socket: error " + IntToString(error) + " during operation " + operation)
	, m_s(s), m_operation(operation), m_error(error)
{
}

Socket::~Socket()
{
	if (m_own)
	{
		try
		{
			CloseSocket();
		}
		catch (...)
		{
		}
	}
}

void Socket::AttachSocket(socket_t s, bool own)
{
	if (m_own)
		CloseSocket();

	m_s = s;
	m_own = own;
	SocketChanged();
}

socket_t Socket::DetachSocket()
{
	socket_t s = m_s;
	m_s = INVALID_SOCKET;
	SocketChanged();
	return s;
}

void Socket::Create(int nType)
{
	assert(m_s == INVALID_SOCKET);
	m_s = socket(AF_INET, nType, 0);
	CheckAndHandleError("socket", m_s);
	m_own = true;
	SocketChanged();
}

void Socket::CloseSocket()
{
	if (m_s != INVALID_SOCKET)
	{
#ifdef USE_WINDOWS_STYLE_SOCKETS
		CheckAndHandleError("closesocket", closesocket(m_s));
#else
		CheckAndHandleError("close", close(m_s));
#endif
		m_s = INVALID_SOCKET;
		SocketChanged();
	}
}

void Socket::Bind(unsigned int port, const char *addr)
{
	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;

	if (addr == NULL)
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
	else
	{
		unsigned long result = inet_addr(addr);
		if (result == -1)	// Solaris doesn't have INADDR_NONE
		{
			SetLastError(SOCKET_EINVAL);
			CheckAndHandleError("inet_addr", SOCKET_ERROR);
		}
		sa.sin_addr.s_addr = result;
	}

	sa.sin_port = htons((u_short)port);

	Bind((sockaddr *)&sa, sizeof(sa));
}

void Socket::Bind(const sockaddr *psa, socklen_t saLen)
{
	assert(m_s != INVALID_SOCKET);
	// cygwin workaround: needs const_cast
	CheckAndHandleError("bind", bind(m_s, const_cast<sockaddr *>(psa), saLen));
}

void Socket::Listen(int backlog)
{
	assert(m_s != INVALID_SOCKET);
	CheckAndHandleError("listen", listen(m_s, backlog));
}

bool Socket::Connect(const char *addr, unsigned int port)
{
	assert(addr != NULL);

	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(addr);

	if (sa.sin_addr.s_addr == -1)	// Solaris doesn't have INADDR_NONE
	{
		hostent *lphost = gethostbyname(addr);
		if (lphost == NULL)
		{
			SetLastError(SOCKET_EINVAL);
			CheckAndHandleError("gethostbyname", SOCKET_ERROR);
		}

		sa.sin_addr.s_addr = ((in_addr *)lphost->h_addr)->s_addr;
	}

	sa.sin_port = htons((u_short)port);

	return Connect((const sockaddr *)&sa, sizeof(sa));
}

bool Socket::Connect(const sockaddr* psa, socklen_t saLen)
{
	assert(m_s != INVALID_SOCKET);
	int result = connect(m_s, const_cast<sockaddr*>(psa), saLen);
	if (result == SOCKET_ERROR && GetLastError() == SOCKET_EWOULDBLOCK)
		return false;
	CheckAndHandleError("connect", result);
	return true;
}

bool Socket::Accept(Socket& target, sockaddr *psa, socklen_t *psaLen)
{
	assert(m_s != INVALID_SOCKET);
	socket_t s = accept(m_s, psa, psaLen);
	if (s == INVALID_SOCKET && GetLastError() == SOCKET_EWOULDBLOCK)
		return false;
	CheckAndHandleError("accept", s);
	target.AttachSocket(s, true);
	return true;
}

void Socket::GetSockName(sockaddr *psa, socklen_t *psaLen)
{
	assert(m_s != INVALID_SOCKET);
	CheckAndHandleError("getsockname", getsockname(m_s, psa, psaLen));
}

unsigned int Socket::Send(const byte* buf, unsigned int bufLen, int flags)
{
	assert(m_s != INVALID_SOCKET);
	int result = send(m_s, (const char *)buf, bufLen, flags);
	CheckAndHandleError("send", result);
	return result;
}

unsigned int Socket::Receive(byte* buf, unsigned int bufLen, int flags)
{
	assert(m_s != INVALID_SOCKET);
	int result = recv(m_s, (char *)buf, bufLen, flags);
	CheckAndHandleError("recv", result);
	return result;
}

void Socket::ShutDown(int how)
{
	assert(m_s != INVALID_SOCKET);
	int result = shutdown(m_s, how);
	CheckAndHandleError("shutdown", result);
}

void Socket::IOCtl(long cmd, unsigned long *argp)
{
	assert(m_s != INVALID_SOCKET);
#ifdef USE_WINDOWS_STYLE_SOCKETS
	CheckAndHandleError("ioctlsocket", ioctlsocket(m_s, cmd, argp));
#else
	CheckAndHandleError("ioctl", ioctl(m_s, cmd, argp));
#endif
}

bool Socket::SendReady(const timeval *timeout)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(m_s, &fds);
	// cygwin workaround: needs const_cast
	int ready = select(m_s+1, NULL, &fds, NULL, const_cast<timeval *>(timeout));
	CheckAndHandleError("select", ready);
	return ready > 0;
}

bool Socket::ReceiveReady(const timeval *timeout)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(m_s, &fds);
	// cygwin workaround: needs const_cast
	int ready = select(m_s+1, &fds, NULL, NULL, const_cast<timeval *>(timeout));
	CheckAndHandleError("select", ready);
	return ready > 0;
}

unsigned int Socket::PortNameToNumber(const char *name, const char *protocol)
{
	int port = atoi(name);
	if (IntToString(port) == name)
		return port;

	servent *se = getservbyname(name, protocol);
	if (!se)
		throw Err(INVALID_SOCKET, "getservbyname", SOCKET_EINVAL);
	return ntohs(se->s_port);
}

void Socket::StartSockets()
{
#ifdef USE_WINDOWS_STYLE_SOCKETS
	WSADATA wsd;
	int result = WSAStartup(0x0002, &wsd);
	if (result != 0)
		throw Err(INVALID_SOCKET, "WSAStartup", result);
#endif
}

void Socket::ShutdownSockets()
{
#ifdef USE_WINDOWS_STYLE_SOCKETS
	int result = WSACleanup();
	if (result != 0)
		throw Err(INVALID_SOCKET, "WSACleanup", result);
#endif
}

int Socket::GetLastError()
{
#ifdef USE_WINDOWS_STYLE_SOCKETS
	return WSAGetLastError();
#else
	return errno;
#endif
}

void Socket::SetLastError(int errorCode)
{
#ifdef USE_WINDOWS_STYLE_SOCKETS
	WSASetLastError(errorCode);
#else
	errno = errorCode;
#endif
}

void Socket::CheckAndHandleError(const char *operation, int result) const
{
	if (result == SOCKET_ERROR)
		throw Err(m_s, operation, GetLastError());
}

#ifdef USE_WINDOWS_STYLE_SOCKETS
void Socket::CheckAndHandleError(const char *operation, socket_t result) const
{
	if (result == INVALID_SOCKET)
		throw Err(m_s, operation, GetLastError());
}
#endif

SocketSource::SocketSource(socket_t s, bool pumpAll, BufferedTransformation *outQueue)
	: Socket(s), NetworkSource(outQueue), m_lastResult(0), m_eofReceived(false)
{
	if (pumpAll)
		PumpAll();
}

bool SocketSource::Receive(byte* buf, unsigned int bufLen)
{
	m_lastResult = Socket::Receive(buf, bufLen);
	if (bufLen > 0 && m_lastResult == 0)
		m_eofReceived = true;
	return true;
}

bool SocketSource::ReceiveReady(unsigned long timeout)
{
	if (timeout == INFINITE_TIME)
		return Socket::ReceiveReady(NULL);
	else
	{
		timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		return Socket::ReceiveReady(&tv);
	}
}

SocketSink::SocketSink(socket_t s, unsigned int maxBufferSize, bool autoFlush)
	: Socket(s), NetworkSink(maxBufferSize, autoFlush), m_lastResult(0)
{
}

bool SocketSink::SendReady(unsigned long timeout)
{
	if (timeout == INFINITE_TIME)
		return Socket::SendReady(NULL);
	else
	{
		timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		return Socket::SendReady(&tv);
	}
}

bool SocketSink::Send(const byte* buf, unsigned int bufLen)
{
	m_lastResult = Socket::Send(buf, bufLen);
	return true;
}

NAMESPACE_END

#endif	// #ifdef SOCKETS_AVAILABLE
