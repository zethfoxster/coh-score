// zlib.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "zlib_cryptopp.h"
#include "zdeflate.h"
#include "zinflate.h"

NAMESPACE_BEGIN(CryptoPP)

static const byte DEFLATE_METHOD = 8;
static const byte FDICT_FLAG = 1 << 5;

// *************************************************************

ZlibCompressor::ZlibCompressor(BufferedTransformation *outQ, unsigned int deflateLevel, unsigned int log2WindowSize)
	: Deflator(outQ, deflateLevel, log2WindowSize)
{
}

void ZlibCompressor::WritePrestreamHeader()
{
	byte cmf = DEFLATE_METHOD | ((GetLog2WindowSize()-8) << 4);
	byte flags = GetCompressionLevel() << 6;
	AttachedTransformation()->PutWord16(RoundUpToMultipleOf(cmf*256+flags, 31));
}

void ZlibCompressor::ProcessUncompressedData(const byte *inString, unsigned int length)
{
	m_adler32.Update(inString, length);
}

void ZlibCompressor::WritePoststreamTail()
{
	SecByteBlock adler32(4);
	m_adler32.Final(adler32);
	AttachedTransformation()->Put(adler32, 4);
}

unsigned int ZlibCompressor::GetCompressionLevel() const
{
	static const unsigned int deflateToCompressionLevel[] = {0, 1, 1, 1, 2, 2, 2, 2, 2, 3};
	return deflateToCompressionLevel[GetDeflateLevel()];
}

// *************************************************************

ZlibDecompressor::ZlibDecompressor(BufferedTransformation *outQueue, bool repeat, int propagation)
	: Inflator(outQueue, repeat, propagation)
{
}

void ZlibDecompressor::ProcessPrestreamHeader()
{
	byte cmf;
	byte flags;

	if (!m_inQueue.Get(cmf) || !m_inQueue.Get(flags))
		throw HeaderErr();

	if ((cmf*256+flags) % 31 != 0)
		throw HeaderErr();

	if ((cmf & 0xf) != DEFLATE_METHOD)
		throw UnsupportedAlgorithm();

	if (flags & FDICT_FLAG)
		throw UnsupportedPresetDictionary();

	m_log2WindowSize = 8 + (cmf >> 4);
}

void ZlibDecompressor::ProcessDecompressedData(const byte *inString, unsigned int length)
{
	AttachedTransformation()->Put(inString, length);
	m_adler32.Update(inString, length);
}

void ZlibDecompressor::ProcessPoststreamTail()
{
	SecByteBlock adler32(4);
	if (m_inQueue.Get(adler32, 4) != 4)
		throw Adler32Err();
	if (!m_adler32.Verify(adler32))
		throw Adler32Err();
}

NAMESPACE_END
