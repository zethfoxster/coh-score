#ifndef CRYPTOPP_WINPIPES_H
#define CRYPTOPP_WINPIPES_H

#if !defined(NO_OS_DEPENDENCE) && defined(_WIN32)

#include "network.h"
#include "queue.h"
#include <windows.h>

#define WINDOWS_PIPES_AVAILABLE

NAMESPACE_BEGIN(CryptoPP)

//! Windows Handle
class WindowsHandle
{
public:
	WindowsHandle(HANDLE h = INVALID_HANDLE_VALUE, bool own=false);
	WindowsHandle(const WindowsHandle &h) : m_h(h.m_h), m_own(false) {}
	virtual ~WindowsHandle();

	bool GetOwnership() const {return m_own;}
	void SetOwnership(bool own) {m_own = own;}

	operator HANDLE() {return m_h;}
	HANDLE GetHandle() {return m_h;}
	bool HandleValid() const;
	void AttachHandle(HANDLE h, bool own=false);
	HANDLE DetachHandle();
	void CloseHandle();

protected:
	virtual void HandleChanged() {}

	HANDLE m_h;
	bool m_own;
};

//! Windows Pipe
class WindowsPipe : public WindowsHandle
{
public:
	class Err : public Exception
	{
	public:
		Err(HANDLE h, const std::string& operation, int error);

		HANDLE GetHandle() const {return m_h;}
		const std::string & GetOperation() const {return m_operation;}
		int GetError() const {return m_error;}

	private:
		HANDLE m_h;
		std::string m_operation;
		int m_error;
	};

	WindowsPipe(HANDLE h=INVALID_HANDLE_VALUE, bool own=false)
		: WindowsHandle(h, own) {}

protected:
	virtual void CheckAndHandleError(const char *operation, BOOL result) const;
};

//! Windows Read Pipe
class WindowsReadPipe : public WindowsPipe, virtual public NetworkReceiver
{
public:
	WindowsReadPipe(HANDLE h=INVALID_HANDLE_VALUE, bool own=false);

	bool ReceiveReady(unsigned long timeout=0) {return true;}
	bool Receive(byte* buf, unsigned int bufLen);
	bool ReceiveResultReady(unsigned long timeout=0);
	unsigned int GetReceiveResult() {return m_lastResult;}
	bool EofReceived() const {return m_eofReceived;}

	HANDLE GetEvent() {return m_event;}

private:
	WindowsReadPipe(const WindowsReadPipe &h);	// no copying

	bool m_inProgress;
	WindowsHandle m_event;
	OVERLAPPED m_overlapped;
	DWORD m_lastResult;
	bool m_eofReceived;
};

//! Windows Pipe Source
class WindowsPipeSource : public WindowsReadPipe, public NetworkSource
{
public:
	WindowsPipeSource(HANDLE h=INVALID_HANDLE_VALUE, bool pumpAll=false, BufferedTransformation *outQueue=NULL)
		: WindowsReadPipe(h), NetworkSource(outQueue)
	{
		if (pumpAll)
			PumpAll();
	}
};

//! Windows Write Pipe
class WindowsWritePipe : public WindowsPipe, virtual public NetworkSender
{
public:
	WindowsWritePipe(HANDLE h=INVALID_HANDLE_VALUE, bool own=false);

	bool SendReady(unsigned long timeout=0) {return true;}
	bool Send(const byte* buf, unsigned int bufLen);
	bool SendResultReady(unsigned long timeout=0);
	unsigned int GetSendResult() {return m_lastResult;}
	void SendEof() {}

	HANDLE GetEvent() {return m_event;}

private:
	bool m_inProgress;
	WindowsHandle m_event;
	OVERLAPPED m_overlapped;
	DWORD m_lastResult;
};

//! Windows Pipe Sink
class WindowsPipeSink : public WindowsWritePipe, public NetworkSink
{
public:
	WindowsPipeSink(HANDLE h=INVALID_HANDLE_VALUE, unsigned int maxBufferSize=0, bool autoFlush=false)
		: WindowsWritePipe(h), NetworkSink(maxBufferSize, autoFlush) {}
};

NAMESPACE_END

#endif	// #if !defined(NO_OS_DEPENDENCE) && defined(_WIN32)

#endif
