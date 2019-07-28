#ifndef CRYPTOPP_TEA_H
#define CRYPTOPP_TEA_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class TEA : public FixedBlockSize<8>, public FixedKeyLength<16>
{
public:
	TEA(const byte *userKey);

	enum {ROUNDS=32, LOG_ROUNDS=5};

protected:
	static const word32 DELTA;
	SecBlock<word32> k;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#TEA">TEA</a>
class TEAEncryption : public TEA
{
public:
	TEAEncryption(const byte *userKey, unsigned int = 0)
		: TEA(userKey) {}

	void ProcessBlock(byte * inoutBlock) const
		{TEAEncryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#TEA">TEA</a>
class TEADecryption : public TEA
{
public:
	TEADecryption(const byte *userKey, unsigned int = 0)
		: TEA(userKey) {}

	void ProcessBlock(byte * inoutBlock) const
		{TEADecryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const;
};

NAMESPACE_END

#endif
