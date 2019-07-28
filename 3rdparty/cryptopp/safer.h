#ifndef CRYPTOPP_SAFER_H
#define CRYPTOPP_SAFER_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class SAFER : public FixedBlockSize<8>
{
public:
	enum {MAX_ROUNDS=13};

protected:
	SAFER(const byte *userkey_1, const byte *userkey_2, unsigned nof_rounds, bool strengthened);

	void Encrypt(const byte *inBlock, byte *outBlock) const;
	void Decrypt(const byte *inBlock, byte *outBlock) const;

	SecByteBlock keySchedule;
	static const byte exp_tab[256];
	static const byte log_tab[256];
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SAFER-K">SAFER-K64</a>
class SAFER_K64_Encryption : public SAFER, public FixedKeyLength<8>
{
public:
	enum {DEFAULT_ROUNDS=6};
	SAFER_K64_Encryption(const byte *userKey, unsigned int = 0, unsigned int rounds=DEFAULT_ROUNDS)
		: SAFER(userKey, userKey, rounds, false) {}

	void ProcessBlock(byte * inoutBlock) const
		{SAFER::Encrypt(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
		{SAFER::Encrypt(inBlock, outBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SAFER-K">SAFER-K64</a>
class SAFER_K64_Decryption : public SAFER, public FixedKeyLength<8>
{
public:
	enum {DEFAULT_ROUNDS=6};
	SAFER_K64_Decryption(const byte *userKey, unsigned int = 0, unsigned int rounds=DEFAULT_ROUNDS)
		: SAFER(userKey, userKey, rounds, false) {}

	void ProcessBlock(byte * inoutBlock) const
		{SAFER::Decrypt(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
		{SAFER::Decrypt(inBlock, outBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SAFER-K">SAFER-K128</a>
class SAFER_K128_Encryption : public SAFER, public FixedKeyLength<16>
{
public:
	enum {DEFAULT_ROUNDS=10};
	SAFER_K128_Encryption(const byte *userKey, unsigned int = 0, unsigned int rounds=DEFAULT_ROUNDS)
		: SAFER(userKey, userKey+8, rounds, false) {}

	void ProcessBlock(byte * inoutBlock) const
		{SAFER::Encrypt(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
		{SAFER::Encrypt(inBlock, outBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SAFER-K">SAFER-K128</a>
class SAFER_K128_Decryption : public SAFER, public FixedKeyLength<16>
{
public:
	enum {DEFAULT_ROUNDS=10};
	SAFER_K128_Decryption(const byte *userKey, unsigned int = 0, unsigned int rounds=DEFAULT_ROUNDS)
		: SAFER(userKey, userKey+8, rounds, false) {}

	void ProcessBlock(byte * inoutBlock) const
		{SAFER::Decrypt(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
		{SAFER::Decrypt(inBlock, outBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SAFER-SK">SAFER-SK64</a>
class SAFER_SK64_Encryption : public SAFER, public FixedKeyLength<8>
{
public:
	enum {DEFAULT_ROUNDS=8};
	SAFER_SK64_Encryption(const byte *userKey, unsigned int = 0, unsigned int rounds=DEFAULT_ROUNDS)
		: SAFER(userKey, userKey, rounds, true) {}

	void ProcessBlock(byte * inoutBlock) const
		{SAFER::Encrypt(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
		{SAFER::Encrypt(inBlock, outBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SAFER-SK">SAFER-SK64</a>
class SAFER_SK64_Decryption : public SAFER, public FixedKeyLength<8>
{
public:
	enum {DEFAULT_ROUNDS=8};
	SAFER_SK64_Decryption(const byte *userKey, unsigned int = 0, unsigned int rounds=DEFAULT_ROUNDS)
		: SAFER(userKey, userKey, rounds, true) {}

	void ProcessBlock(byte * inoutBlock) const
		{SAFER::Decrypt(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
		{SAFER::Decrypt(inBlock, outBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SAFER-SK">SAFER-SK128</a>
class SAFER_SK128_Encryption : public SAFER, public FixedKeyLength<16>
{
public:
	enum {DEFAULT_ROUNDS=10};
	SAFER_SK128_Encryption(const byte *userKey, unsigned int = 0, unsigned int rounds=DEFAULT_ROUNDS)
		: SAFER(userKey, userKey+8, rounds, true) {}

	void ProcessBlock(byte * inoutBlock) const
		{SAFER::Encrypt(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
		{SAFER::Encrypt(inBlock, outBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SAFER-SK">SAFER-SK128</a>
class SAFER_SK128_Decryption : public SAFER, public FixedKeyLength<16>
{
public:
	enum {DEFAULT_ROUNDS=10};
	SAFER_SK128_Decryption(const byte *userKey, unsigned int = 0, unsigned int rounds=DEFAULT_ROUNDS)
		: SAFER(userKey, userKey+8, rounds, true) {}

	void ProcessBlock(byte * inoutBlock) const
		{SAFER::Decrypt(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
		{SAFER::Decrypt(inBlock, outBlock);}
};

NAMESPACE_END

#endif
