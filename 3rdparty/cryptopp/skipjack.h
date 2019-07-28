#ifndef CRYPTOPP_SKIPJACK_H
#define CRYPTOPP_SKIPJACK_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class SKIPJACK : public FixedBlockSize<8>, public FixedKeyLength<10>
{
protected:
	SKIPJACK(const byte *userKey);

	static const byte fTable[256];

	SecBlock<byte[256]> tab;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SKIPJACK">SKIPJACK</a>
class SKIPJACKEncryption : public SKIPJACK
{
public:
	SKIPJACKEncryption(const byte *userKey, unsigned int = 0)
		: SKIPJACK(userKey) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{SKIPJACKEncryption::ProcessBlock(inoutBlock, inoutBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SKIPJACK">SKIPJACK</a>
class SKIPJACKDecryption : public SKIPJACK
{
public:
	SKIPJACKDecryption(const byte *userKey, unsigned int = 0)
		: SKIPJACK(userKey) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{SKIPJACKDecryption::ProcessBlock(inoutBlock, inoutBlock);}
};

NAMESPACE_END

#endif
