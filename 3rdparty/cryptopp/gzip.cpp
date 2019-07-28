// gzip.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "gzip.h"

NAMESPACE_BEGIN(CryptoPP)

Gzip::Gzip(BufferedTransformation *bt, unsigned int deflateLevel, unsigned int log2WindowSize)
	: Deflator(bt, deflateLevel, log2WindowSize)
	, m_totalLen(0)
{
}

void Gzip::WritePrestreamHeader()
{
	AttachedTransformation()->Put(MAGIC1);
	AttachedTransformation()->Put(MAGIC2);
	AttachedTransformation()->Put(DEFLATED);
	AttachedTransformation()->Put(0);		// general flag
	AttachedTransformation()->PutWord32(0);	// time stamp
	byte extra = (GetDeflateLevel() == 1) ? FAST : ((GetDeflateLevel() == 9) ? SLOW : 0);
	AttachedTransformation()->Put(extra);
	AttachedTransformation()->Put(GZIP_OS_CODE);
}

void Gzip::ProcessUncompressedData(const byte *inString, unsigned int length)
{
	m_crc.Update(inString, length);
	m_totalLen += length;
}

void Gzip::WritePoststreamTail()
{
	SecByteBlock crc(4);
	m_crc.Final(crc);
	AttachedTransformation()->Put(crc, 4);
	AttachedTransformation()->PutWord32(m_totalLen, false);
	m_totalLen = 0;
}

// *************************************************************

Gunzip::Gunzip(BufferedTransformation *outQueue, bool repeat, int propagation)
	: Inflator(outQueue, repeat, propagation), m_length(0)
{
}

void Gunzip::ProcessPrestreamHeader()
{
	byte buf[6];
	byte b, flags;

	if (m_inQueue.Get(buf, 2)!=2) throw HeaderErr();
	if (buf[0] != MAGIC1 || buf[1] != MAGIC2) throw HeaderErr();
	if (!m_inQueue.Skip(1)) throw HeaderErr();	 // skip extra flags
	if (!m_inQueue.Get(flags)) throw HeaderErr();
	if (flags & (ENCRYPTED | CONTINUED)) throw HeaderErr();
	if (m_inQueue.Skip(6)!=6) throw HeaderErr();    // Skip file time, extra flags and OS type

	if (flags & EXTRA_FIELDS)	// skip extra fields
	{
		word16 length;
		if (m_inQueue.GetWord16(length, false) != 2) throw HeaderErr();
		if (m_inQueue.Skip(length)!=length) throw HeaderErr();
	}

	if (flags & FILENAME)	// skip filename
		do
			if(!m_inQueue.Get(b)) throw HeaderErr();
		while (b);

	if (flags & COMMENTS)	// skip comments
		do
			if(!m_inQueue.Get(b)) throw HeaderErr();
		while (b);
}

void Gunzip::ProcessDecompressedData(const byte *inString, unsigned int length)
{
	AttachedTransformation()->Put(inString, length);
	m_crc.Update(inString, length);
	m_length += length;
}

void Gunzip::ProcessPoststreamTail()
{
	SecByteBlock crc(4);
	if (m_inQueue.Get(crc, 4) != 4)
		throw TailErr();
	if (!m_crc.Verify(crc))
		throw CrcErr();

	word32 lengthCheck;
	if (m_inQueue.GetWord32(lengthCheck, false) != 4)
		throw TailErr();
	if (lengthCheck != m_length)
		throw LengthErr();
	m_length = 0;
}

NAMESPACE_END
