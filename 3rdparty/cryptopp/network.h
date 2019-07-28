#ifndef CRYPTOPP_NETWORK_H
#define CRYPTOPP_NETWORK_H

#include "filters.h"
#include "hrtimer.h"

NAMESPACE_BEGIN(CryptoPP)

const unsigned long INFINITE_TIME = ULONG_MAX;

//! a Source class that can pump from a device for a specified amount of time.
class NonblockingSource : public Source, public BufferedTransformationWithAutoSignal
{
public:
	NonblockingSource(BufferedTransformation *outQ)
		: Source(outQ), m_messagePumped(false) {}

	//! pump up to maxSize bytes using at most maxTime milliseconds
	/*! If checkDelimiter is true, pump up to delimiter, which itself is not extracted or pumped. */
	virtual unsigned long GeneralPump(unsigned long maxSize=ULONG_MAX, unsigned long maxTime=INFINITE_TIME, bool checkDelimiter=false, byte delimiter='\n') =0;

	unsigned long Pump(unsigned long pumpMax=ULONG_MAX) {return GeneralPump(pumpMax);}
	unsigned long TimedPump(unsigned long maxTime) {return GeneralPump(ULONG_MAX, maxTime);}
	unsigned long PumpLine(byte delimiter='\n', unsigned long maxSize=1024) {return GeneralPump(maxSize, INFINITE_TIME, true, delimiter);}

	unsigned int PumpMessages(unsigned int count=UINT_MAX);

private:
	bool m_messagePumped;
};

//! Network Receiver
class NetworkReceiver
{
public:
	virtual bool ReceiveReady(unsigned long timeout=0) =0;
	virtual bool Receive(byte* buf, unsigned int bufLen) =0;
	virtual bool ReceiveResultReady(unsigned long timeout=0) =0;
	virtual unsigned int GetReceiveResult() =0;
	virtual bool EofReceived() const =0;
};

//! a Sink class that queues input and can flush to a device for a specified amount of time.
class NonblockingSink : public Sink
{
public:
	void Flush(bool completeFlush, int propagation=-1)
		{TimedFlush(completeFlush ? INFINITE_TIME : 0);}

	virtual unsigned int TimedFlush(unsigned long maxTime) =0;

	virtual void SetMaxBufferSize(unsigned int maxBufferSize) =0;
	virtual void SetAutoFlush(bool autoFlush = true) =0;

	virtual unsigned int GetMaxBufferSize() const =0;
	virtual unsigned int GetCurrentBufferSize() const =0;
};

//! Network Sender
class NetworkSender
{
public:
	virtual bool SendReady(unsigned long timeout=0) =0;
	virtual bool Send(const byte* buf, unsigned int bufLen) =0;
	virtual bool SendResultReady(unsigned long timeout=0) =0;
	virtual unsigned int GetSendResult() =0;
	virtual void SendEof() =0;
};

#ifdef HIGHRES_TIMER_AVAILABLE

//! Network Source
class NetworkSource : virtual public NetworkReceiver, public NonblockingSource
{
public:
	NetworkSource(BufferedTransformation *outQ);
	unsigned long GeneralPump(unsigned long maxSize=ULONG_MAX, unsigned long maxTime=INFINITE_TIME, bool checkDelimiter=false, byte delimiter='\n');

private:
	SecByteBlock m_buf;
	unsigned int m_bufSize;
	bool m_needReceiveResult;
};

//! Network Sink
class NetworkSink : virtual public NetworkSender, public NonblockingSink
{
public:
	NetworkSink(unsigned int maxBufferSize, bool autoFlush)
		: m_maxBufferSize(maxBufferSize), m_autoFlush(autoFlush), m_needSendResult(false) {}

	void Put(byte b) {NetworkSink::Put(&b, 1);}
	void Put(const byte *str, unsigned int bc);

	void MessageEnd(int propagation=-1) {TimedFlush(INFINITE_TIME); SendEof();}

	unsigned int TimedFlush(unsigned long maxTime);

	void SetMaxBufferSize(unsigned int maxBufferSize) {m_maxBufferSize = maxBufferSize;}
	void SetAutoFlush(bool autoFlush = true) {m_autoFlush = autoFlush;}

	unsigned int GetMaxBufferSize() const {return m_maxBufferSize;}
	unsigned int GetCurrentBufferSize() const {return m_buffer.CurrentSize();}

private:
	unsigned int m_maxBufferSize;
	bool m_autoFlush, m_needSendResult;
	ByteQueue m_buffer;
};

#endif	// #ifdef HIGHRES_TIMER_AVAILABLE

NAMESPACE_END

#endif
