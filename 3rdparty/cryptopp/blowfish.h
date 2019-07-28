#ifndef CRYPTOPP_BLOWFISH_H
#define CRYPTOPP_BLOWFISH_H

/** \file */

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class Blowfish : public FixedBlockSize<8>, public VariableKeyLength<16, 1, 72>
{
public:
	Blowfish(const byte *key_string, unsigned int keylength, CipherDir direction);

	void ProcessBlock(byte * inoutBlock) const
		{Blowfish::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const;

	enum {ROUNDS=16};

private:
	void crypt_block(const word32 in[2], word32 out[2]) const;

	static const word32 p_init[ROUNDS+2];
	static const word32 s_init[4*256];
	SecBlock<word32> pbox, sbox;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Blowfish">Blowfish</a>
class BlowfishEncryption : public Blowfish
{
public:
	BlowfishEncryption(const byte *key_string, unsigned int keylength=DEFAULT_KEYLENGTH)
		: Blowfish(key_string, keylength, ENCRYPTION) {}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Blowfish">Blowfish</a>
class BlowfishDecryption : public Blowfish
{
public:
	BlowfishDecryption(const byte *key_string, unsigned int keylength=DEFAULT_KEYLENGTH)
		: Blowfish(key_string, keylength, DECRYPTION) {}
};

NAMESPACE_END

#endif
