#ifndef CRYPTOPP_RC2_H
#define CRYPTOPP_RC2_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class RC2Base : public FixedBlockSize<8>, public VariableKeyLength<16, 1, 128>
{
public:
	enum {BLOCKSIZE=8};
	unsigned int BlockSize() const {return BLOCKSIZE;}

protected:
	// max keyLen is 128, max effectiveLen is 1024
	RC2Base(const byte *key, unsigned int keyLen, unsigned int effectiveLen);

	SecBlock<word16> K;  // expanded key table
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#RC2">RC2</a>
class RC2Encryption : public RC2Base
{
public:
	RC2Encryption(const byte *key, unsigned int keyLen=DEFAULT_KEYLENGTH, unsigned int effectiveLen=1024)
		: RC2Base(key, keyLen, effectiveLen) {}

	void ProcessBlock(byte * inoutBlock) const
		{RC2Encryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#RC2">RC2</a>
class RC2Decryption : public RC2Base
{
public:
	RC2Decryption(const byte *key, unsigned int keyLen=DEFAULT_KEYLENGTH, unsigned int effectiveLen=1024)
		: RC2Base(key, keyLen, effectiveLen) {}

	void ProcessBlock(byte * inoutBlock) const
		{RC2Decryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
};

NAMESPACE_END

#endif
