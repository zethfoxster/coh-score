#ifndef CRYPTOPP_MD5MAC_H
#define CRYPTOPP_MD5MAC_H

#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

/// <a href="http://www.weidai.com/scan-mirror/mac.html#MD5-MAC">MD5-MAC</a>
class MD5MAC : public IteratedHash<word32, false, 64>, public FixedKeyLength<16>
{
public:
	enum {DIGESTSIZE = 16};
	MD5MAC(const byte *userKey);
	void TruncatedFinal(byte *mac, unsigned int size);

protected:
	static void Transform (word32 *buf, const word32 *in, const word32 *key);
	void vTransform(const word32 *data) {Transform(digest, data, key+4);}
	void Init();

	static const word32 T[12];
	SecBlock<word32> key;
};

NAMESPACE_END

#endif
