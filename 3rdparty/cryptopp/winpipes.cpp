// winpipes.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#if defined(_WIN32) && !defined(NO_OS_DEPENDENCE)

#include "winpipes.h"

NAMESPACE_BEGIN(CryptoPP)

WindowsHandle::WindowsHandle(HANDLE h, bool own)
	: m_h(h), m_own(own)
{
}

WindowsHandle::~WindowsHandle()
{
	if (m_own)
	{
		try
		{
			CloseHandle();
		}
		catch (...)
		{
		}
	}
}

bool WindowsHandle::HandleValid() const
{
	return m_h && m_h != INVALID_HANDLE_VALUE;
}

void WindowsHandle::AttachHandle(HANDLE h, bool own)
{
	if (m_own)
		CloseHandle();

	m_h = h;
	m_own = own;
	HandleChanged();
}

HANDLE WindowsHandle::DetachHandle()
{
	HANDLE h = m_h;
	m_h = INVALID_HANDLE_VALUE;
	HandleChanged();
	return h;
}

void WindowsHandle::CloseHandle()
{
	if (m_h != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_h);
		m_h = INVALID_HANDLE_VALUE;
		HandleChanged();
	}
}

void WindowsPipe::CheckAndHandleError(const char *operation, BOOL result) const
{
	if (!result)
	{
		DWORD err = GetLastError();
		throw Err(m_h, operation, err);
	}
}

WindowsPipe::Err::Err(HANDLE s, const std::string& operation, int error)
	: Exception("WindowsPipeSink: error " + IntToString(error) + " during operation " + operation)
	, m_h(s), m_operation(operation), m_error(error)
{
}

// *************************************************************

WindowsReadPipe::WindowsReadPipe(HANDLE h, bool own)
	: WindowsPipe(h, own), m_inProgress(false), m_lastResult(0), m_eofReceived(false)
{
	m_event.AttachHandle(CreateEvent(NULL, true, false, NULL), true);
	CheckAndHandleError("CreateEvent", m_event.HandleValid());
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_overlapped.hEvent = m_event;
}

bool WindowsReadPipe::Receive(byte* buf, unsigned int bufLen)
{
	assert(!m_inProgress && !m_eofReceived);

	DWORD read = 0;
	BOOL result = ReadFile(m_h, buf, bufLen, &read, &m_overlapped);
	if (result)
		m_lastResult = read;
	else switch (GetLastError())
	{
	default:
		CheckAndHandleError("ReadFile", false);
	case ERROR_BROKEN_PIPE:
	case ERROR_HANDLE_EOF:
		m_lastResult = 0;
		m_eofReceived = true;
		break;
	case ERROR_IO_PENDING:
		m_inProgress = true;
	}
	return !m_inProgress;
}

bool WindowsReadPipe::ReceiveResultReady(unsigned long timeout)
{
	if (!m_inProgress)
		return true;

	switch (WaitForSingleObject(m_event, timeout))
	{
	default:
		CheckAndHandleError("WaitForSingleObject", false);
	case WAIT_TIMEOUT:
		return false;
	case WAIT_OBJECT_0:
		;
	}

	BOOL result = GetOverlappedResult(m_h, &m_overlapped, &m_lastResult, false);
	if (!result)
	{
		switch (GetLastError())
		{
		default:
			CheckAndHandleError("GetOverlappedResult", false);
		case ERROR_BROKEN_PIPE:
		case ERROR_HANDLE_EOF:
			m_lastResult = 0;
			m_eofReceived = true;
		}
	}

	m_inProgress = false;
	return true;
}

// *************************************************************

WindowsWritePipe::WindowsWritePipe(HANDLE h, bool own)
	: WindowsPipe(h, own), m_inProgress(false), m_lastResult(0)
{
	m_event.AttachHandle(CreateEvent(NULL, true, false, NULL), true);
	CheckAndHandleError("CreateEvent", m_event.HandleValid());
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_overlapped.hEvent = m_event;
}

bool WindowsWritePipe::Send(const byte* buf, unsigned int bufLen)
{
	DWORD written = 0;
	BOOL result = WriteFile(m_h, buf, bufLen, &written, &m_overlapped);
	if (result)
	{
		m_inProgress = false;
		m_lastResult = written;
		return true;
	}
	else
	{
		if (GetLastError() != ERROR_IO_PENDING)
			CheckAndHandleError("WriteFile", false);

		m_inProgress = true;
		return false;
	}
}

bool WindowsWritePipe::SendResultReady(unsigned long timeout)
{
	if (!m_inProgress)
		return true;

	switch (WaitForSingleObject(m_event, timeout))
	{
	default:
		CheckAndHandleError("WaitForSingleObject", false);
	case WAIT_TIMEOUT:
		return false;
	case WAIT_OBJECT_0:
		break;
	}

	BOOL result = GetOverlappedResult(m_h, &m_overlapped, &m_lastResult, false);
	CheckAndHandleError("GetOverlappedResult", result);
	m_inProgress = false;
	return true;
}

NAMESPACE_END

#endif
