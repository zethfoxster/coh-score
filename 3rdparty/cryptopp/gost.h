#ifndef CRYPTOPP_GOST_H
#define CRYPTOPP_GOST_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class GOST : public FixedBlockSize<8>, public FixedKeyLength<32>
{
protected:
	GOST(const byte *userKey, CipherDir);
	static void PrecalculateSTable();

	static const byte sBox[8][16];
	static bool sTableCalculated;
	static word32 sTable[4][256];

	SecBlock<word32> key;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#GOST">GOST</a>
class GOSTEncryption : public GOST
{
public:
	GOSTEncryption(const byte * userKey, unsigned int = 0)
		: GOST (userKey, ENCRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{GOSTEncryption::ProcessBlock(inoutBlock, inoutBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#GOST">GOST</a>
class GOSTDecryption : public GOST
{
public:
	GOSTDecryption(const byte * userKey, unsigned int = 0)
		: GOST (userKey, DECRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{GOSTDecryption::ProcessBlock(inoutBlock, inoutBlock);}
};

NAMESPACE_END

#endif
