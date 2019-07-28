#ifndef CRYPTOPP_MARS_H
#define CRYPTOPP_MARS_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class MARS : public FixedBlockSize<16>, public VariableKeyLength<16, 16, 56, 4>
{
protected:
	MARS(const byte *userKey, unsigned int keylength);

	static const word32 Sbox[512];

	SecBlock<word32> EK;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#MARS">MARS</a>
class MARSEncryption : public MARS
{
public:
	MARSEncryption(const byte *userKey, unsigned int keylength=DEFAULT_KEYLENGTH)
		: MARS(userKey, keylength) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{MARSEncryption::ProcessBlock(inoutBlock, inoutBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#MARS">MARS</a>
class MARSDecryption : public MARS
{
public:
	MARSDecryption(const byte *userKey, unsigned int keylength=DEFAULT_KEYLENGTH)
		: MARS(userKey, keylength) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{MARSDecryption::ProcessBlock(inoutBlock, inoutBlock);}
};

NAMESPACE_END

#endif
