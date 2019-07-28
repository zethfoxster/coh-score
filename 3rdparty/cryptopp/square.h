#ifndef CRYPTOPP_SQUARE_H
#define CRYPTOPP_SQUARE_H

/** \file
*/

#include "config.h"

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class SquareBase : public FixedBlockSize<16>, public FixedKeyLength<16>
{
public:
	enum {ROUNDS=8};

protected:
	SquareBase(const byte *userKey, CipherDir dir);

	SecBlock<word32[4]> roundkeys;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Square">Square</a>
class SquareEncryption : public SquareBase
{
public:
	SquareEncryption(const byte *key, unsigned int = 0) : SquareBase(key, ENCRYPTION) {}

	void ProcessBlock(byte * inoutBlock) const
		{SquareEncryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;

private:
	static const byte Se[256];
	static const word32 Te[4][256];
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Square">Square</a>
class SquareDecryption : public SquareBase
{
public:
	SquareDecryption(const byte *key, unsigned int = 0) : SquareBase(key, DECRYPTION) {}

	void ProcessBlock(byte * inoutBlock) const
		{SquareDecryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;

private:
	static const byte Sd[256];
	static const word32 Td[4][256];
};

NAMESPACE_END

#endif
