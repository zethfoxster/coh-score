#ifndef CRYPTOPP_SERPENT_H
#define CRYPTOPP_SERPENT_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class Serpent : public FixedBlockSize<16>, public VariableKeyLength<16, 1, 32>
{
protected:
	Serpent(const byte *userKey, unsigned int keylength);

	SecBlock<word32> l_key;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Serpent">Serpent</a>
class SerpentEncryption : public Serpent
{
public:
	SerpentEncryption(const byte *userKey, unsigned int keylength=DEFAULT_KEYLENGTH)
		: Serpent(userKey, keylength) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{SerpentEncryption::ProcessBlock(inoutBlock, inoutBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Serpent">Serpent</a>
class SerpentDecryption : public Serpent
{
public:
	SerpentDecryption(const byte *userKey, unsigned int keylength=DEFAULT_KEYLENGTH)
		: Serpent(userKey, keylength) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{SerpentDecryption::ProcessBlock(inoutBlock, inoutBlock);}
};

NAMESPACE_END

#endif
