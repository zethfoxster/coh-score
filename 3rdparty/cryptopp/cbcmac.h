#ifndef CRYPTOPP_CBCMAC_H
#define CRYPTOPP_CBCMAC_H

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

//! <a href="http://www.weidai.com/scan-mirror/mac.html#CBC-MAC">CBC-MAC</a>
/*! Compatible with FIPS 113. T should be an encryption class.
	Secure only for fixed length messages. For variable length
	messages use DMAC.
*/
template <class T> class CBC_MAC : public MessageAuthenticationCode, public SameKeyLengthAs<T>
{
public:
	enum {DIGESTSIZE=T::BLOCKSIZE};

#ifdef __MWERKS__	// CW50 workaround: can't use DEFAULT_KEYLENGTH here
	CBC_MAC(const byte *key, unsigned int keylength = T::DEFAULT_KEYLENGTH);
#else
	CBC_MAC(const byte *key, unsigned int keylength = DEFAULT_KEYLENGTH);
#endif

	void Update(const byte *input, unsigned int length);
	void TruncatedFinal(byte *mac, unsigned int size);
	unsigned int DigestSize() const {return DIGESTSIZE;}

private:
	void ProcessBuf();
	T cipher;
	SecByteBlock reg;
	unsigned int counter;
};

template <class T>
CBC_MAC<T>::CBC_MAC(const byte *key, unsigned int keylength)
	: cipher(key, keylength)
	, reg(T::BLOCKSIZE)
	, counter(0)
{
	memset(reg, 0, T::BLOCKSIZE);
}

template <class T>
void CBC_MAC<T>::Update(const byte *input, unsigned int length)
{
	while (counter && length)
	{
		reg[counter++] ^= *input++;
		if (counter == T::BLOCKSIZE)
			ProcessBuf();
		length--;
	}

	while (length >= T::BLOCKSIZE)
	{
		xorbuf(reg, input, T::BLOCKSIZE);
		ProcessBuf();
		input += T::BLOCKSIZE;
		length -= T::BLOCKSIZE;
	}

	while (length--)
	{
		reg[counter++] ^= *input++;
		if (counter == T::BLOCKSIZE)
			ProcessBuf();
	}
}

template <class T>
void CBC_MAC<T>::TruncatedFinal(byte *mac, unsigned int size)
{
	assert(size <= T::BLOCKSIZE);

	if (counter)
		ProcessBuf();
	memcpy(mac, reg, size);
	memset(reg, 0, T::BLOCKSIZE);
}

template <class T>
void CBC_MAC<T>::ProcessBuf()
{
	cipher.ProcessBlock(reg);
	counter = 0;
}

NAMESPACE_END

#endif
