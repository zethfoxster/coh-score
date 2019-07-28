#ifndef CRYPTOPP_SHARK_H
#define CRYPTOPP_SHARK_H

/** \file
*/

#include "config.h"

#ifdef WORD64_AVAILABLE

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class SHARKBase : public FixedBlockSize<8>, public VariableKeyLength<16, 1, 16>
{
public:
	enum {DEFAULT_ROUNDS=6};

protected:
	static void InitEncryptionRoundKeys(const byte *key, unsigned int keyLen, unsigned int rounds, word64 *roundkeys);
	SHARKBase(unsigned int rounds) : rounds(rounds), roundkeys(rounds+1) {}

	unsigned int rounds;
	SecBlock<word64> roundkeys;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SHARK-E">SHARK-E</a>
class SHARKEncryption : public SHARKBase
{
public:
	SHARKEncryption(const byte *key, unsigned int keyLen=DEFAULT_KEYLENGTH, unsigned int rounds=DEFAULT_ROUNDS);

	void ProcessBlock(byte * inoutBlock) const
		{SHARKEncryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;

private:
	friend class SHARKBase;
	SHARKEncryption();
	static const byte sbox[256];
	static const word64 cbox[8][256];
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SHARK-E">SHARK-E</a>
class SHARKDecryption : public SHARKBase
{
public:
	SHARKDecryption(const byte *key, unsigned int keyLen=DEFAULT_KEYLENGTH, unsigned int rounds=DEFAULT_ROUNDS);

	void ProcessBlock(byte * inoutBlock) const
		{SHARKDecryption::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;

private:
	static const byte sbox[256];
	static const word64 cbox[8][256];
};

NAMESPACE_END

#endif
#endif
