#ifndef CRYPTOPP_ZDEFLATE_H
#define CRYPTOPP_ZDEFLATE_H

#include "filters.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

//! .
class LowFirstBitWriter : public Filter
{
public:
	LowFirstBitWriter(BufferedTransformation *outQ);
	void PutBits(unsigned long value, unsigned int length);
	void FlushBitBuffer();

	void StartCounting();
	unsigned long FinishCounting();

protected:
	bool m_counting;
	unsigned long m_bitCount;
	unsigned long m_buffer;
	unsigned int m_bitsBuffered, m_bytesBuffered;
	SecByteBlock m_outputBuffer;
};

//! Huffman Encoder
class HuffmanEncoder
{
public:
	typedef unsigned int code_t;
	typedef unsigned int value_t;

	HuffmanEncoder() {}
	HuffmanEncoder(const unsigned int *codeBits, unsigned int nCodes);
	void Initialize(const unsigned int *codeBits, unsigned int nCodes);

	static void GenerateCodeLengths(unsigned int *codeBits, unsigned int maxCodeBits, const unsigned int *codeCounts, unsigned int nCodes);

	void Encode(LowFirstBitWriter &writer, value_t value) const;

	struct Code
	{
		unsigned int code;
		unsigned int len;
	};

	SecBlock<Code> m_valueToCode;
};

//! DEFLATE (RFC 1951) compressor

class Deflator : public LowFirstBitWriter
{
public:
	enum {DEFAULT_DEFLATE_LEVEL = 6, DEFAULT_LOG2_WINDOW_SIZE = 15};
	Deflator(BufferedTransformation *outQ=NULL, unsigned int deflateLevel=DEFAULT_DEFLATE_LEVEL, unsigned int log2WindowSize=DEFAULT_LOG2_WINDOW_SIZE);

	void SetDeflateLevel(unsigned int deflateLevel);
	unsigned int GetDeflateLevel() const {return m_deflateLevel;}
	unsigned int GetLog2WindowSize() const {return m_log2WindowSize;}

	void Put(byte inByte)
		{Deflator::Put(&inByte, 1);}
	void Put(const byte *str, unsigned int length);
	void Flush(bool completeFlush, int propagation=-1);
	void MessageEnd(int propagation=-1);

private:
	virtual void WritePrestreamHeader() {}
	virtual void ProcessUncompressedData(const byte *string, unsigned int length) {}
	virtual void WritePoststreamTail() {}

	enum {STORED = 0, STATIC = 1, DYNAMIC = 2};
	enum {MIN_MATCH = 3, MAX_MATCH = 258};

	void Reset();
	unsigned int FillWindow(const byte *str, unsigned int length);
	unsigned int ComputeHash(const byte *str) const;
	unsigned int LongestMatch(unsigned int &bestMatch) const;
	void InsertString(unsigned int start);
	void ProcessBuffer();

	void LiteralByte(byte b);
	void MatchFound(unsigned int distance, unsigned int length);
	void EncodeBlock(bool eof, unsigned int blockType);
	void EndBlock(bool eof);

	struct EncodedMatch
	{
		unsigned literalCode : 9;
		unsigned literalExtra : 5;
		unsigned distanceCode : 5;
		unsigned distanceExtra : 13;
	};

	unsigned int m_deflateLevel, m_log2WindowSize;
	unsigned int DSIZE, DMASK, HSIZE, HMASK, GOOD_MATCH, MAX_LAZYLENGTH, MAX_CHAIN_LENGTH;
	bool m_headerWritten, m_matchAvailable;
	unsigned int m_dictionaryEnd, m_stringStart, m_lookahead, m_minLookahead, m_previousMatch, m_previousLength;
	HuffmanEncoder m_staticLiteralEncoder, m_staticDistanceEncoder, m_dynamicLiteralEncoder, m_dynamicDistanceEncoder;
	SecByteBlock m_byteBuffer;
	SecBlock<word16> m_head, m_prev;
	SecBlock<unsigned int> m_literalCounts, m_distanceCounts;
	SecBlock<EncodedMatch> m_matchBuffer;
	unsigned int m_matchBufferEnd, m_blockStart, m_blockLength;
};

NAMESPACE_END

#endif
