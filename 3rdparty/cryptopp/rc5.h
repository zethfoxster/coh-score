#ifndef CRYPTOPP_RC5_H
#define CRYPTOPP_RC5_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class RC5Base : public FixedBlockSize<8>, public VariableKeyLength<16, 0, 255>
{
public:
	typedef word32 RC5_WORD;

	enum {DEFAULT_ROUNDS=16};

protected:
	RC5Base(const byte *key, unsigned int keyLen, unsigned int rounds);

	const unsigned int r;       // number of rounds
	SecBlock<RC5_WORD> sTable;  // expanded key table
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#RC5">RC5</a>
class RC5Encryption : public RC5Base
{
public:
	RC5Encryption(const byte *key, unsigned int keyLen=DEFAULT_KEYLENGTH, unsigned int rounds=DEFAULT_ROUNDS)
		: RC5Base(key, keyLen, rounds) {}

	void ProcessBlock(byte * inoutBlock) const
		{RC5Encryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#RC5">RC5</a>
class RC5Decryption : public RC5Base
{
public:
	RC5Decryption(const byte *key, unsigned int keyLen=DEFAULT_KEYLENGTH, unsigned int rounds=DEFAULT_ROUNDS)
		: RC5Base(key, keyLen, rounds) {}

	void ProcessBlock(byte * inoutBlock) const
		{RC5Decryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
};

NAMESPACE_END

#endif
