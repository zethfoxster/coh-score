// secshare.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "secshare.h"
#include "queue.h"
#include "algebra.h"
#include "gf2_32.h"
#include "polynomi.h"

#include "algebra.cpp"
#include "polynomi.cpp"

ANONYMOUS_NAMESPACE_BEGIN
class Field : public CryptoPP::GF2_32
{
public:
	Field() : CryptoPP::GF2_32(0xB1D89917) {}

	Element Divide(Element a, Element b) const
	{
		static Element invTable[256];
		Element bInv;

		if (b<256 && invTable[b])
			bInv = invTable[b];
		else
			bInv = invTable[b] = MultiplicativeInverse(b);

		return Multiply(a, bInv);
	}
};

typedef CryptoPP::PolynomialOverFixedRing<Field, 0> Polynomial;
typedef CryptoPP::RingOfPolynomialsOver<Field> PolynomialRing;

static const Field field;
static const PolynomialRing polynomialRing(field);
NAMESPACE_END

template<> const Field Polynomial::fixedRing(field);

NAMESPACE_BEGIN(CryptoPP)

Fork::Fork(unsigned int n, BufferedTransformation *const *givenOutPorts)
	: numberOfPorts(n), outPorts(n)
{
	currentPort = 0;

	for (unsigned int i=0; i<numberOfPorts; i++)
		outPorts[i].reset(givenOutPorts ? givenOutPorts[i] : new ByteQueue);
}

Fork::Fork(BufferedTransformation *outport0, BufferedTransformation *outport1)
	: numberOfPorts(2), outPorts(2)
{
	currentPort = 0;
	outPorts[0].reset(outport0 ? outport0 : new ByteQueue);
	outPorts[1].reset(outport1 ? outport1 : new ByteQueue);
}

void Fork::SelectOutPort(unsigned int portNumber)
{
	currentPort = portNumber;
}

void Fork::Detach(BufferedTransformation *newOut)
{
	outPorts[currentPort].reset(newOut ? newOut : new ByteQueue);
}

void Fork::MessageEnd(int propagation)
{
	if (propagation)
		for (unsigned int i=0; i<numberOfPorts; i++)
			outPorts[i]->MessageEnd(propagation-1);
}

void Fork::MessageSeriesEnd(int propagation)
{
	if (propagation)
		for (unsigned int i=0; i<numberOfPorts; i++)
			outPorts[i]->MessageSeriesEnd(propagation);
}

void Fork::Put(byte inByte)
{
	for (unsigned int i=0; i<numberOfPorts; i++)
		outPorts[i]->Put(inByte);
}

void Fork::Put(const byte *inString, unsigned int length)
{
	for (unsigned int i=0; i<numberOfPorts; i++)
		outPorts[i]->Put(inString, length);
}

// ********************************************************

Join::Join(unsigned int n, BufferedTransformation *outQ)
	: Filter(outQ),
	  numberOfPorts(n),
	  inPorts(n),
	  interfacesOpen(n),
	  interfaces(n)
{
	for (unsigned int i=0; i<numberOfPorts; i++)
	{
		inPorts[i].reset(new MessageQueue);
		interfaces[i].reset(new JoinInterface(*this, *inPorts[i], i));
	}
}

JoinInterface * Join::ReleaseInterface(unsigned int i)
{
	return interfaces[i].release();
}

bool Join::AllCurrentMessagesAreComplete() const
{
	for (unsigned int i=0; i<NumberOfPorts(); i++)
		if (AccessPort(i).NumberOfMessages() == 0)
			return false;
	return true;
}

// ********************************************************

void JoinInterface::Put(byte inByte)
{
	bq.Put(inByte);
	parent.NotifyInput(id, 1);
}

void JoinInterface::Put(const byte *inString, unsigned int length)
{
	bq.Put(inString, length);
	parent.NotifyInput(id, length);
}

unsigned long JoinInterface::MaxRetrievable() const
{
	return parent.MaxRetrievable();
}

void JoinInterface::MessageEnd(int)
{
	parent.NotifyMessageEnd(id);
}

void JoinInterface::MessageSeriesEnd(int) 
{
	parent.NotifyMessageSeriesEnd(id);
}

void JoinInterface::Detach(BufferedTransformation *bt) 
{
	parent.Detach(bt);
}

void JoinInterface::Attach(BufferedTransformation *bt) 
{
	parent.Attach(bt);
}

unsigned int JoinInterface::Get(byte &outByte) 
{
	return parent.Get(outByte);
}

unsigned int JoinInterface::Get(byte *outString, unsigned int getMax)
{
	return parent.Get(outString, getMax);
}

unsigned int JoinInterface::Peek(byte &outByte) const
{
	return parent.Peek(outByte);
}

unsigned int JoinInterface::Peek(byte *outString, unsigned int peekMax) const
{
	return parent.Peek(outString, peekMax);
}

unsigned long JoinInterface::CopyTo(BufferedTransformation &target, unsigned long copyMax) const
{
	return parent.CopyTo(target, copyMax);
}

// ****************************************************************

ShareFork::ShareFork(RandomNumberGenerator &rng, word32 m, word32 n, BufferedTransformation *const *outports)
	: Fork(n, outports), m_rng(rng), m_threshold(m)
{
	Reset();
}

void ShareFork::Reset()
{
	m_count = 0;
	m_headerWritten = false;
}

void ShareFork::WriteHeader()
{
	assert(!m_headerWritten);
	for (unsigned int i=0; i<NumberOfPorts(); i++)
	{
		AccessPort(i).PutWord32(m_threshold);
		AccessPort(i).PutWord32(i+1);
	}
	m_headerWritten = true;
}

void ShareFork::Put(byte inByte)
{
	if (!m_headerWritten)
		WriteHeader();
	m_buffer = (m_buffer<<8) | inByte;
	if (++m_count == 4)
	{
		Share(m_buffer);
		m_count = 0;
		m_buffer = 0;
	}
}

void ShareFork::Put(const byte *inString, unsigned int length)
{
	while (length--)
		Put(*inString++);
}

void ShareFork::Share(word32 message)
{
	Polynomial::RandomizationParameter param(m_threshold, 0);
	Polynomial poly(m_rng, param);
	poly.SetCoefficient(0, message);

	for (unsigned int i=0; i<NumberOfPorts(); i++)
		AccessPort(i).PutWord32(poly.EvaluateAt(i+1));
}

void ShareFork::MessageEnd(int propagation)
{
	if (!m_headerWritten)
		WriteHeader();

	byte filler = 4-m_count;
	assert(filler > 0 && filler <= 4);

	for (byte i = 0; i<filler; i++)
		Put(filler);

	assert(m_count == 0);

	Fork::MessageEnd(propagation);

	Reset();
}

// ****************************************************************

ShareJoin::ShareJoin(unsigned int n, BufferedTransformation *outQ)
	: Join(n, outQ), m_x(n), m_indexRead(false), m_firstOutput(true)
{
	assert(n>0);
}

void ShareJoin::NotifyInput(unsigned int /* interfaceId */, unsigned int /* length */)
{
	unsigned long n = AccessPort(0).MaxRetrievable();

	for (unsigned int i=1; n && i<NumberOfPorts(); i++)
		n = STDMIN(n, AccessPort(i).MaxRetrievable());

	if (!m_indexRead && n>=8)
	{
		ReadIndex();
		n -= 8;
	}

	if (m_indexRead)
		Assemble(n);
}

void ShareJoin::ReadIndex()
{
	for (unsigned int i=0; i<NumberOfPorts(); i++)
	{
		AccessPort(i).GetWord32(m_threshold);
		AccessPort(i).GetWord32(m_x[i]);
	}

	m_indexRead = true;
}

void ShareJoin::NotifyMessageEnd(unsigned int)
{
	if (!AllCurrentMessagesAreComplete())
		return;

	byte filler = m_buffer & 0xff;
	for (unsigned int i=3; i && i>=filler; --i)
		AttachedTransformation()->Put(byte(m_buffer>>(8*i)));

	AttachedTransformation()->MessageEnd();

	m_indexRead = false;
	m_firstOutput = true;
}

void ShareJoin::Assemble(unsigned long n)
{
	SecBlock<word32> y(NumberOfPorts());

	while (n>=4)
	{
		for (unsigned int i=0; i<NumberOfPorts(); i++)
			AccessPort(i).GetWord32(y[i]);

		Output(polynomialRing.InterpolateAt(0, m_x, y, NumberOfPorts()));
		n -= 4;
	}
}

void ShareJoin::Output(word32 message)
{
	if (m_firstOutput)
		m_firstOutput = false;
	else
		AttachedTransformation()->PutWord32(m_buffer);

	m_buffer = message;
}

// ************************************************************

DisperseFork::DisperseFork(unsigned int m, unsigned int n, BufferedTransformation *const *outports)
	: ShareFork(*(RandomNumberGenerator *)0, m, n, outports),
	  m_poly(m)
{
	Reset();
}

void DisperseFork::Reset()
{
	ShareFork::Reset();
	m_polyCount = 0;
}

void DisperseFork::Share(word32 message)
{
	m_poly[m_polyCount++] = message;

	if (m_polyCount==m_threshold)
	{
		Polynomial poly(m_poly.Begin(), m_poly.End());

		for (unsigned int i=0; i<NumberOfPorts(); i++)
			AccessPort(i).PutWord32(poly.EvaluateAt(i+1));

		m_polyCount = 0;
	}
}

void DisperseFork::MessageEnd(int propagation)
{
	if (!m_headerWritten)
		WriteHeader();

	{
		byte filler = 4-m_count;
		assert(filler > 0 && filler <= 4);

		for (byte i = 0; i<filler; i++)
			Put(filler);

		assert(m_count == 0);
	}

	{
		word32 filler = m_threshold - m_polyCount;
		for (word32 i=0; i<filler; i++)
			Share(filler);
	}

	Fork::MessageEnd(propagation);

	Reset();
}

DisperseJoin::DisperseJoin(unsigned int n, BufferedTransformation *outQ)
	: ShareJoin(n, outQ), m_firstPolyOutput(true)
{
}

void DisperseJoin::Assemble(unsigned long n)
{
	while (n>=4)
	{
		SecBlock<word32> y(NumberOfPorts());
		unsigned int i;

		for (i=0; i<NumberOfPorts(); i++)
			AccessPort(i).GetWord32(y[i]);

		Polynomial poly(polynomialRing.Interpolate(m_x, y, NumberOfPorts()));

		if (m_firstPolyOutput)
		{
			m_polyBuffer.Grow(m_threshold);
			m_firstPolyOutput = false;
		}
		else
		{
			for (i=0; i<m_threshold; i++)
				Output(m_polyBuffer[i]);
		}

		for (i=0; i<m_threshold; i++)
			m_polyBuffer[i] = poly[i];

		n -= 4;
	}
}

void DisperseJoin::NotifyMessageEnd(unsigned int id)
{
	if (!AllCurrentMessagesAreComplete())
		return;

	word32 filler = m_polyBuffer[m_threshold-1];
	for (word32 i=0; i+filler < m_threshold; ++i)
		Output(m_polyBuffer[i]);

	ShareJoin::NotifyMessageEnd(id);
}

NAMESPACE_END
