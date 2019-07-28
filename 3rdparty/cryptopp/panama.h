#ifndef CRYPTOPP_PANAMA_H
#define CRYPTOPP_PANAMA_H

#include "cryptlib.h"
#include "misc.h"
#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class Panama
{
public:
	Panama();
	void Reset();
	void Iterate(unsigned int count, const word32 *p=NULL, word32 *z=NULL, const word32 *y=NULL);

protected:
	typedef word32 Stage[8];
	SecBlock<word32> m_state;
	unsigned int m_bstart;
};

/// <a href="http://www.weidai.com/scan-mirror/md.html#Panama">Panama Hash</a>
template <bool H = false>	// default to little endian
class PanamaHash : protected Panama, public IteratedHash<word32, H, 32>
{
public:
	enum {DIGESTSIZE = 32};
	PanamaHash() : IteratedHash<word32, H, 32>(0) {}
	unsigned int DigestSize() const {return DIGESTSIZE;}
	void TruncatedFinal(byte *hash, unsigned int size);

protected:
	void Init() {Panama::Reset();}
	void vTransform(const word32 *data) {Iterate(1, data);}	// push
	unsigned int HashMultipleBlocks(const word32 *input, unsigned int length);
};

/// Panama MAC
template <bool H = false>	// default to little endian
class PanamaMAC : public PanamaHash<H>, public MessageAuthenticationCode, public VariableKeyLength<32, 0, UINT_MAX>
{
public:
	PanamaMAC(const byte *key, unsigned int keylength=DEFAULT_KEYLENGTH) {Update(key, keylength);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Panama">Panama Stream Cipher</a>
template <bool H = false>	// default to little endian
class PanamaCipher : protected Panama, public StreamCipher, public FixedKeyLength<32>
{
public:
	enum {HIGHFIRST = H, IVLENGTH = 32};
	PanamaCipher(const byte *key, const byte *iv=NULL);

	byte ProcessByte(byte input)
		{PanamaCipher::ProcessString(&input, &input, 1); return input;}
	void ProcessString(byte *inoutString, unsigned int length)
		{PanamaCipher::ProcessString(inoutString, inoutString, length);}
	void ProcessString(byte *outString, const byte *inString, unsigned int length);

protected:
	inline static void CorrectEndianess(word32 *out, const word32 *in, unsigned int byteCount)
	{
		if (!CheckEndianess(HIGHFIRST))
			byteReverse(out, in, byteCount);
		else if (in!=out)
			memcpy(out, in, byteCount);
	}

	SecBlock<word32> m_buf;
	unsigned int m_leftOver;
};

NAMESPACE_END

#endif
