#ifndef CRYPTOPP_DIAMOND_H
#define CRYPTOPP_DIAMOND_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"
#include "crc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class Diamond2Base : public FixedBlockSize<16>, public VariableKeyLength<16, 1, 256>
{
public:
	Diamond2Base(const byte *key, unsigned int key_size, unsigned int rounds,
				CipherDir direction);

	enum {DEFAULT_ROUNDS=10};

protected:
	enum {ROUNDSIZE=4096};
	inline void substitute(int round, byte *y) const;

	const int numrounds;
	SecByteBlock s;         // Substitution boxes

	static inline void permute(byte *);
	static inline void ipermute(byte *);
#ifdef DIAMOND_USE_PERMTABLE
	static const word32 permtable[9][256];
	static const word32 ipermtable[9][256];
#endif
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Diamond2">Diamond2</a>
class Diamond2Encryption : public Diamond2Base
{
public:
	Diamond2Encryption(const byte *key, unsigned int key_size=DEFAULT_KEYLENGTH, unsigned int rounds=DEFAULT_ROUNDS)
		: Diamond2Base(key, key_size, rounds, ENCRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Diamond2">Diamond2</a>
class Diamond2Decryption : public Diamond2Base
{
public:
	Diamond2Decryption(const byte *key, unsigned int key_size=DEFAULT_KEYLENGTH, unsigned int rounds=DEFAULT_ROUNDS)
		: Diamond2Base(key, key_size, rounds, DECRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const;
};

/// base class, do not use directly
class Diamond2LiteBase : public FixedBlockSize<8>, public VariableKeyLength<16, 1, 256>
{
public:
	Diamond2LiteBase(const byte *key, unsigned int key_size, unsigned int rounds,
				CipherDir direction);

	enum {DEFAULT_ROUNDS=8};

protected:
	enum {ROUNDSIZE=2048};
	inline void substitute(int round, byte *y) const;
	const int numrounds;
	SecByteBlock s;         // Substitution boxes

	static inline void permute(byte *);
	static inline void ipermute(byte *);
#ifdef DIAMOND_USE_PERMTABLE
	static const word32 permtable[8][256];
	static const word32 ipermtable[8][256];
#endif
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Diamond2">Diamond2 Lite</a>
class Diamond2LiteEncryption : public Diamond2LiteBase
{
public:
	Diamond2LiteEncryption(const byte *key, unsigned int key_size=DEFAULT_KEYLENGTH, unsigned int rounds=DEFAULT_ROUNDS)
		: Diamond2LiteBase(key, key_size, rounds, ENCRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Diamond2">Diamond2 Lite</a>
class Diamond2LiteDecryption : public Diamond2LiteBase
{
public:
	Diamond2LiteDecryption(const byte *key, unsigned int key_size=DEFAULT_KEYLENGTH, unsigned int rounds=DEFAULT_ROUNDS)
		: Diamond2LiteBase(key, key_size, rounds, DECRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const;
};

NAMESPACE_END

#endif
