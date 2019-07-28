#ifndef CRYPTOPP_SECSHARE_H
#define CRYPTOPP_SECSHARE_H

// This file is being maintained for backwards compatibility only.
// New applications should use the classes in "ida.h".

#include "filters.h"
#include "mqueue.h"

NAMESPACE_BEGIN(CryptoPP)

class Fork : public BufferedTransformation
{
public:
	Fork(unsigned int number_of_outports, BufferedTransformation *const *outports = NULL);
	Fork(BufferedTransformation *outport0, BufferedTransformation *outport1 = NULL);

	void SelectOutPort(unsigned int portNumber);

	bool Attachable() {return true;}
	void Detach(BufferedTransformation *newOut = NULL);
	void MessageEnd(int propagation);
	void MessageSeriesEnd(int propagation);

	BufferedTransformation *AttachedTransformation()
		{return outPorts[currentPort]->AttachedTransformation();}
	unsigned long MaxRetrievable() const
		{return outPorts[currentPort]->MaxRetrievable();}

	unsigned int Get(byte &outByte)
		{return outPorts[currentPort]->Get(outByte);}
	unsigned int Get(byte *outString, unsigned int getMax)
		{return outPorts[currentPort]->Get(outString, getMax);}
	unsigned int Peek(byte &outByte) const
		{return outPorts[currentPort]->Peek(outByte);}
	unsigned int Peek(byte *outString, unsigned int peekMax) const
		{return outPorts[currentPort]->Peek(outString, peekMax);}
	unsigned long CopyTo(BufferedTransformation &target, unsigned long copyMax=ULONG_MAX) const
		{return outPorts[currentPort]->CopyTo(target, copyMax);}

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);

protected:
	unsigned int NumberOfPorts() const {return numberOfPorts;}
	BufferedTransformation& AccessPort(unsigned int i) {return *outPorts[i];}

private:
	Fork(const Fork &); // no copying allowed

	unsigned int numberOfPorts, currentPort;
	vector_member_ptrs<BufferedTransformation> outPorts;
};

class Join;

class JoinInterface : public BufferedTransformation
{
public:
	JoinInterface(Join &p, MessageQueue &b, int i)
		: parent(p), bq(b), id(i) {}

	unsigned long MaxRetrievable() const;
	void MessageEnd(int propagation);
	void MessageSeriesEnd(int propagation);
	bool Attachable() {return true;}
	void Detach(BufferedTransformation *bt);
	void Attach(BufferedTransformation *bt);

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);
	unsigned int Get(byte &outByte);
	unsigned int Get(byte *outString, unsigned int getMax);
	unsigned int Peek(byte &outByte) const;
	unsigned int Peek(byte *outString, unsigned int peekMax) const;
	unsigned long CopyTo(BufferedTransformation &target, unsigned long copyMax=ULONG_MAX) const;

private:
	Join &parent;
	MessageQueue &bq;
	const int id;
};

class Join : public Filter
{
public:
	Join(unsigned int number_of_inports, BufferedTransformation *outQ = NULL);

	// Note that ReleaseInterface is similar but not completely compatible 
	// with SelectInterface of version 2.0.  ReleaseInterface can be called
	// only once for each interface, and if an interface is released,
	// the caller will be responsible for deleting it.
	JoinInterface *ReleaseInterface(unsigned int i);

	void Put(byte inByte) {AttachedTransformation()->Put(inByte);}
	void Put(const byte *inString, unsigned int length)
		{AttachedTransformation()->Put(inString, length);}

protected:
	friend class JoinInterface;

	virtual void NotifyInput(unsigned int interfaceId, unsigned int length) =0;
	virtual void NotifyMessageEnd(unsigned int interfaceId) =0;
	virtual void NotifyMessageSeriesEnd(unsigned int interfaceId) {}

	unsigned int NumberOfPorts() const {return numberOfPorts;}
	MessageQueue& AccessPort(unsigned int i) {return *inPorts[i];}
	const MessageQueue& AccessPort(unsigned int i) const {return *inPorts[i];}
	unsigned int InterfacesOpen() const {return interfacesOpen;}
	bool AllCurrentMessagesAreComplete() const;

private:
	Join(const Join &); // no copying allowed

	unsigned int numberOfPorts;
	vector_member_ptrs<MessageQueue> inPorts;
	unsigned int interfacesOpen;
	vector_member_ptrs<JoinInterface> interfaces;
};

class ShareFork : public Fork
{
public:
	// fork into n shares, with m necessary to reconstruct
	ShareFork(RandomNumberGenerator &rng, word32 m, word32 n,
			  BufferedTransformation *const *outports = NULL);

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);
	void MessageEnd(int propagation=-1);

protected:
	void Reset();
	void WriteHeader();
	void Process(unsigned int message);
	virtual void Share(word32 message);

	RandomNumberGenerator &m_rng;
	word32 m_threshold;
	word32 m_buffer;
	unsigned int m_count;
	bool m_headerWritten;
};

class ShareJoin : public Join
{
public:
	ShareJoin(unsigned int n, BufferedTransformation *outQ = NULL);

protected:
	void ReadIndex();
	virtual void Assemble(unsigned long);
	void Output(word32);

	void NotifyInput(unsigned int interfaceId, unsigned int length);
	void NotifyMessageEnd(unsigned int);

	word32 m_threshold;
	SecBlock<word32> m_x;
	word32 m_buffer;
	bool m_indexRead, m_firstOutput;
};

class DisperseFork : public ShareFork
{
public:
	DisperseFork(unsigned int m, unsigned int n, BufferedTransformation *const *outports = NULL);

	void MessageEnd(int propagation=-1);

protected:
	void Reset();
	void Share(word32 message);

	SecBlock<word32> m_poly;
	unsigned int m_polyCount;
};

class DisperseJoin : public ShareJoin
{
public:
	DisperseJoin(unsigned int n, BufferedTransformation *outQ = NULL);

	void NotifyMessageEnd(unsigned int id);

protected:
	virtual void Assemble(unsigned long);

	SecBlock<word32> m_polyBuffer;
	bool m_firstPolyOutput;
};

NAMESPACE_END

#endif
