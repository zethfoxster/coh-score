// network.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "network.h"

NAMESPACE_BEGIN(CryptoPP)

unsigned int NonblockingSource::PumpMessages(unsigned int count)
{
	if (m_messagePumped || count == 0)
		return 0;
	else
	{
		while (Pump());
		Filter::MessageEnd(GetAutoSignalPropagation());
		m_messagePumped = true;
		return 1;
	}
}

// *************************************************************

#ifdef HIGHRES_TIMER_AVAILABLE

NetworkSource::NetworkSource(BufferedTransformation *outQ)
	: NonblockingSource(outQ), m_buf(1024*4), m_bufSize(0), m_needReceiveResult(false)
{
}

unsigned long NetworkSource::GeneralPump(unsigned long maxSize, unsigned long maxTime, bool checkDelimiter, byte delimiter)
{
	unsigned long totalPumpSize = 0;
	bool forever = maxTime == INFINITE_TIME;
	Timer timer(Timer::MILLISECONDS);
	unsigned long elapsed, timeout;
	unsigned int i=0;

	while (true)
	{
		i++;

		if (m_needReceiveResult)
		{
			if (forever)
				timeout = INFINITE_TIME;
			else if ((elapsed = timer.ElapsedTime()) <= maxTime)
				timeout = maxTime - elapsed;
			else
				timeout = 0;

			if (!ReceiveResultReady(timeout))
				break;

			unsigned int recvResult = GetReceiveResult();
			assert(recvResult > 0 || EofReceived());
			m_bufSize += recvResult;
			m_needReceiveResult = false;
		}

		if (m_bufSize == 0)
		{
			if (EofReceived())
				break;
		}
		else
		{
			byte *p, *end = m_buf + STDMIN((unsigned long)m_bufSize, maxSize - totalPumpSize);
			if (checkDelimiter)
				p = std::find(m_buf.ptr, end, delimiter);
			else
				p = end;

			AttachedTransformation()->Put(m_buf, p-m_buf.ptr);
			totalPumpSize += p-m_buf.ptr;
			m_bufSize = m_buf+m_bufSize-p;
			if (m_bufSize > 0)
				memmove(m_buf, p, m_bufSize);

			if (checkDelimiter && p != end)
				break;
		}

		if (totalPumpSize == maxSize)
			break;

		if (forever)
			timeout = INFINITE_TIME;
		else if ((elapsed = timer.ElapsedTime()) <= maxTime)
			timeout = maxTime - elapsed;
		else
			break;

		if (!ReceiveReady(timeout))
			break;

		Receive(m_buf+m_bufSize, m_buf.size-m_bufSize);
		m_needReceiveResult = true;
	}

	return totalPumpSize;
}

// *************************************************************

void NetworkSink::Put(const byte *str, unsigned int bc)
{
	LazyPutter lp(m_buffer, str, bc);

	while (m_buffer.CurrentSize() > m_maxBufferSize)
	{
		if (m_needSendResult)
			SendResultReady(INFINITE_TIME);
		else
			SendReady(INFINITE_TIME);

		Flush(0);
	}

	if (m_autoFlush)
		Flush(0);
}

unsigned int NetworkSink::TimedFlush(unsigned long maxTime)
{
	if (m_buffer.IsEmpty())
		return 0;

	bool forever = maxTime == INFINITE_TIME;
	Timer timer(Timer::MILLISECONDS);
	unsigned long elapsed, timeout;
	unsigned int totalFlushSize = 0;

	while (true)
	{
		if (m_needSendResult)
		{
			if (forever)
				timeout = INFINITE_TIME;
			else if ((elapsed = timer.ElapsedTime()) <= maxTime)
				timeout = maxTime - elapsed;
			else
				timeout = 0;

			if (!SendResultReady(timeout))
				break;

			unsigned int sendResult = GetSendResult();
			m_buffer.Skip(sendResult);
			totalFlushSize += sendResult;
			m_needSendResult = false;

			if (m_buffer.IsEmpty())
				break;
		}

		if (forever)
			timeout = INFINITE_TIME;
		else if ((elapsed = timer.ElapsedTime()) <= maxTime)
			timeout = maxTime - elapsed;
		else
			break;

		if (!SendReady(timeout))
			break;

		unsigned int contiguousSize = 0;
		const byte *block = m_buffer.Spy(contiguousSize);

		Send(block, contiguousSize);
		m_needSendResult = true;
	}

	return totalFlushSize;
}

#endif	// #ifdef HIGHRES_TIMER_AVAILABLE

NAMESPACE_END
