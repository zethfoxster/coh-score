#ifndef CRYPTOPP_ZINFLATE_H
#define CRYPTOPP_ZINFLATE_H

#include "filters.h"

NAMESPACE_BEGIN(CryptoPP)

//! .
class LowFirstBitReader
{
public:
	LowFirstBitReader(BufferedTransformation &store)
		: m_store(store), m_buffer(0), m_bitsBuffered(0) {}
	unsigned long BitsLeft() const {return m_store.MaxRetrievable() * 8 + m_bitsBuffered;}
	unsigned int BitsBuffered() const {return m_bitsBuffered;}
	unsigned long PeekBuffer() const {return m_buffer;}
	bool FillBuffer(unsigned int length);
	unsigned long PeekBits(unsigned int length);
	void SkipBits(unsigned int length);
	unsigned long GetBits(unsigned int length);

private:
	BufferedTransformation &m_store;
	unsigned long m_buffer;
	unsigned int m_bitsBuffered;
};

//! Huffman Decoder
class HuffmanDecoder
{
public:
	typedef unsigned int code_t;
	typedef unsigned int value_t;
	enum {MAX_CODE_BITS = sizeof(code_t)*8};

	class Err : public Exception {public: Err(const std::string &what) : Exception("HuffmanDecoder: " + what) {}};

	HuffmanDecoder() {}
	HuffmanDecoder(const unsigned int *codeBits, unsigned int nCodes)	{Initialize(codeBits, nCodes);}

	void Initialize(const unsigned int *codeBits, unsigned int nCodes);
	unsigned int Decode(code_t code, /* out */ value_t &value) const;
	bool Decode(LowFirstBitReader &reader, value_t &value) const;

private:
	struct CodeInfo
	{
		CodeInfo(code_t code=0, unsigned int len=0, value_t value=0) : code(code), len(len), value(value) {}
		bool operator<(const CodeInfo &rhs) const {return code < rhs.code;}
		//friend bool CodeLessThan(code_t lhs, const CodeInfo &rhs) {return lhs < rhs.code;}
		friend bool CodeLessThan(const CodeInfo &lhs, const CodeInfo &rhs) {return lhs.code < rhs.code;}
		code_t code;
		unsigned int len;
		value_t value;
	};

	struct LookupEntry
	{
		unsigned int type;
		union
		{
			value_t value;
			const CodeInfo *begin;
		};
		union
		{
			unsigned int len;
			const CodeInfo *end;
		};
	};

	static code_t NormalizeCode(code_t code, unsigned int codeBits);

	unsigned int m_maxCodeBits, m_cacheBits, m_cacheMask;
	SecBlock<CodeInfo> m_codeToValue;
	SecBlock<LookupEntry> m_cache;
};

//! DEFLATE (RFC 1951) decompressor

class Inflator : public Filter, public BufferedTransformationWithAutoSignal
{
public:
	class Err : public BufferedTransformation::Err
	{
	public:
		Err(ErrorType e, const std::string &s)
			: BufferedTransformation::Err(e, s) {}
	};
	class UnexpectedEndErr : public Err {public: UnexpectedEndErr() : Err(INVALID_DATA_FORMAT, "Inflator: unexpected end of compressed block") {}};
	class BadBlockErr : public Err {public: BadBlockErr() : Err(INVALID_DATA_FORMAT, "Inflator: error in compressed block") {}};

	/*! \param repeat decompress multiple compressed streams in series
		\param autoSignalPropagation 0 to turn off MessageEnd signal
	*/
	Inflator(BufferedTransformation *outQueue = NULL, bool repeat = false, int autoSignalPropagation = -1);
	void Put(byte b) {Inflator::Put(&b, 1);}
	void Put(const byte *inString, unsigned int length);

	void Flush(bool completeFlush, int propagation=-1);
	void MessageEnd(int propagation=-1);

	virtual unsigned int GetLog2WindowSize() const {return 15;}

protected:
	ByteQueue m_inQueue;

private:
	virtual unsigned int MaxPrestreamHeaderSize() const {return 0;}
	virtual void ProcessPrestreamHeader() {}
	virtual void ProcessDecompressedData(const byte *string, unsigned int length)
		{AttachedTransformation()->Put(string, length);}
	virtual unsigned int MaxPoststreamTailSize() const {return 0;}
	virtual void ProcessPoststreamTail() {}

	void ProcessInput(bool flush);
	void DecodeHeader();
	bool DecodeBody();
	void FlushOutput();
	void OutputByte(byte b);
	void OutputString(const byte *string, unsigned int length);
	void OutputPast(unsigned int length, unsigned int distance);

	enum State {PRE_STREAM, WAIT_HEADER, DECODING_BODY, POST_STREAM, AFTER_END};
	State m_state;
	bool m_repeat, m_eof, m_decodersInitializedWithFixedCodes;
	byte m_blockType;
	word16 m_storedLen;
	enum NextDecode {LITERAL, LENGTH_BITS, DISTANCE, DISTANCE_BITS};
	NextDecode m_nextDecode;
	unsigned int m_literal, m_distance;	// for LENGTH_BITS or DISTANCE_BITS
	HuffmanDecoder m_literalDecoder, m_distanceDecoder;
	LowFirstBitReader m_reader;
	SecByteBlock m_window;
	unsigned int m_maxDistance, m_current, m_lastFlush;
};

NAMESPACE_END

#endif
