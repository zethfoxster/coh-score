#ifndef CRYPTOPP_IDEA_H
#define CRYPTOPP_IDEA_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// base class, do not use directly
class IDEA : public FixedBlockSize<8>, public FixedKeyLength<16>
{
public:
	IDEA(const byte *userKey, CipherDir dir);

	void ProcessBlock(byte * inoutBlock) const
		{IDEA::ProcessBlock(inoutBlock, inoutBlock);}
	void ProcessBlock(const byte *inBlock, byte * outBlock) const;

private:
	void EnKey(const byte *);
	void DeKey();
	SecBlock<word> key;

#ifdef IDEA_LARGECACHE
	static inline void LookupMUL(word &a, word b);
	void LookupKeyLogs();
	static void BuildLogTables();
	static bool tablesBuilt;
	static word16 log[0x10000], antilog[0x10000];
#endif
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#IDEA">IDEA</a>
class IDEAEncryption : public IDEA
{
public:
	IDEAEncryption(const byte * userKey, unsigned int = 0)
		: IDEA (userKey, ENCRYPTION) {}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#IDEA">IDEA</a>
class IDEADecryption : public IDEA
{
public:
	IDEADecryption(const byte * userKey, unsigned int = 0)
		: IDEA (userKey, DECRYPTION) {}
};

NAMESPACE_END

#endif
