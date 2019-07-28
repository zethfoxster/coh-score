#ifndef CRYPTOPP_DMAC_H
#define CRYPTOPP_DMAC_H

#include "cbcmac.h"

NAMESPACE_BEGIN(CryptoPP)

//! DMAC
/*! Based on "CBC MAC for Real-Time Data Sources" by Erez Petrank
	and Charles Rackoff. T should be an encryption class.
*/
template <class T> class DMAC : public MessageAuthenticationCode, SameKeyLengthAs<T>
{
public:
	enum {DIGESTSIZE=T::BLOCKSIZE};

#ifdef __MWERKS__	// CW50 workaround: can't use DEFAULT_KEYLENGTH here
	DMAC(const byte *key, unsigned int keylength = T::DEFAULT_KEYLENGTH);
#else
	DMAC(const byte *key, unsigned int keylength = DEFAULT_KEYLENGTH);
#endif

	void Update(const byte *input, unsigned int length);
	void TruncatedFinal(byte *mac, unsigned int size);
	unsigned int DigestSize() const {return DIGESTSIZE;}

private:
	byte *GenerateSubKeys(const byte *key, unsigned int keylength);

	unsigned int subkeylength;
	SecByteBlock subkeys;
	CBC_MAC<T> mac1;
	T f2;
	unsigned int counter;
};

template <class T>
DMAC<T>::DMAC(const byte *key, unsigned int keylength)
	: subkeylength(T::KeyLength(T::BLOCKSIZE))
	, subkeys(2*STDMAX((unsigned int)T::BLOCKSIZE, subkeylength))
	, mac1(GenerateSubKeys(key, keylength), subkeylength)
	, f2(subkeys+subkeys.size/2, subkeylength)
	, counter(0)
{
	subkeys.Resize(0);
}

template <class T>
void DMAC<T>::Update(const byte *input, unsigned int length)
{
	mac1.Update(input, length);
	counter = (counter + length) % T::BLOCKSIZE;
}

template <class T>
void DMAC<T>::TruncatedFinal(byte *mac, unsigned int size)
{
	byte pad[T::BLOCKSIZE];
	byte padByte = byte(T::BLOCKSIZE-counter);
	memset(pad, padByte, padByte);
	mac1.Update(pad, padByte);
	mac1.TruncatedFinal(mac, size);
	f2.ProcessBlock(mac);
}

template <class T>
byte *DMAC<T>::GenerateSubKeys(const byte *key, unsigned int keylength)
{
	T cipher(key, keylength);
	memset(subkeys, 0, subkeys.size);
	cipher.ProcessBlock(subkeys);
	subkeys[subkeys.size/2 + T::BLOCKSIZE - 1] = 1;
	cipher.ProcessBlock(subkeys+subkeys.size/2);
	return subkeys;
}

NAMESPACE_END

#endif
