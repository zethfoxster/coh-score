#ifndef CRYPTOPP_RC6_H
#define CRYPTOPP_RC6_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class RC6Base : public FixedBlockSize<16>, public VariableKeyLength<16, 0, 255>
{
public:
	typedef word32 RC6_WORD;

	enum {DEFAULT_ROUNDS=20};

protected:
	RC6Base(const byte *key, unsigned int keyLen, unsigned int rounds);

	const unsigned int r;       // number of rounds
	SecBlock<RC6_WORD> sTable;  // expanded key table
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#RC6">RC6</a>
class RC6Encryption : public RC6Base
{
public:
	RC6Encryption(const byte *key, unsigned int keyLen=DEFAULT_KEYLENGTH, unsigned int rounds=DEFAULT_ROUNDS)
		: RC6Base(key, keyLen, rounds) {}

	void ProcessBlock(byte * inoutBlock) const
		{RC6Encryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#RC6">RC6</a>
class RC6Decryption : public RC6Base
{
public:
	RC6Decryption(const byte *key, unsigned int keyLen=DEFAULT_KEYLENGTH, unsigned int rounds=DEFAULT_ROUNDS)
		: RC6Base(key, keyLen, rounds) {}

	void ProcessBlock(byte * inoutBlock) const
		{RC6Decryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
};

NAMESPACE_END

#endif
