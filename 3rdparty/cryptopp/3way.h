#ifndef CRYPTOPP_THREEWAY_H
#define CRYPTOPP_THREEWAY_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// <a href="http://www.weidai.com/scan-mirror/cs.html#3-Way">3-Way</a>
class ThreeWayEncryption : public FixedBlockSize<12>, public FixedKeyLength<12>
{
public:
	enum {DEFAULT_ROUNDS=11};

	ThreeWayEncryption(const byte *userKey, unsigned int = 0, unsigned int rounds=DEFAULT_ROUNDS);
	~ThreeWayEncryption();

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{ThreeWayEncryption::ProcessBlock(inoutBlock, inoutBlock);}

private:
	word32 k[3];
	unsigned int rounds;
	SecBlock<word32> rc;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#3-Way">3-Way</a>
class ThreeWayDecryption : public FixedBlockSize<12>, public FixedKeyLength<12>
{
public:
	enum {DEFAULT_ROUNDS=11};

	ThreeWayDecryption(const byte *userKey, unsigned int = 0, unsigned int rounds=DEFAULT_ROUNDS);
	~ThreeWayDecryption();

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{ThreeWayDecryption::ProcessBlock(inoutBlock, inoutBlock);}

private:
	word32 k[3];
	unsigned int rounds;
	SecBlock<word32> rc;
};

NAMESPACE_END

#endif
