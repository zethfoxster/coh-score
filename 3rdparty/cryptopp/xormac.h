// xormac.h - written and placed in the public domain by Wei Dai

#ifndef CRYPTOPP_XORMAC_H
#define CRYPTOPP_XORMAC_H

#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

/// <a href="http://www.weidai.com/scan-mirror/mac.XMAC">XMACC</a>
template <class T> class XMACC : public IteratedHash<typename T::HashWordType, T::HIGHFIRST, T::BLOCKSIZE>
{
public:
	enum {KEYLENGTH=T::DIGESTSIZE-4, DIGESTSIZE = 4+T::DIGESTSIZE};
	typedef typename T::HashWordType HashWordType;

	// If you need to generate MACs with XMACC (instead of just verifying them),
	// you must save the counter before destroying an XMACC object
	// and reinitialize it the next time you create an XMACC with the same key.
	// Start counter at 0 when using a key for the first time.

	XMACC(const byte *userKey, word32 counter=0xffffffff);

	word32 CurrentCounter() const {return counter;}

	void TruncatedFinal(byte *mac, unsigned int size);
	bool TruncatedVerify(const byte *mac, unsigned int length);
	unsigned int DigestSize() const {return DIGESTSIZE;}

private:
	void Init();
	static void WriteWord32(byte *output, word32 value);
	static void XorDigest(HashWordType *digest, const HashWordType *buffer);
	void vTransform(const HashWordType *data);

	SecByteBlock key;
	SecBlock<HashWordType> buffer;
	word32 counter, index;
};

template <class T> XMACC<T>::XMACC(const byte *userKey, word32 counter)
	: IteratedHash<HashWordType, T::HIGHFIRST, T::BLOCKSIZE>(T::DIGESTSIZE)
	, key(KEYLENGTH)
	, buffer(T::DIGESTSIZE/sizeof(HashWordType))
	, counter(counter)
{
	memcpy(key, userKey, KEYLENGTH);
	Init();
}

template <class T> void XMACC<T>::Init()
{
	index = 0x80000000;
	memset(digest, 0, T::DIGESTSIZE);
}

template <class T> inline void XMACC<T>::WriteWord32(byte *output, word32 value)
{
	output[0] = byte(value >> 24);
	output[1] = byte(value >> 16);
	output[2] = byte(value >> 8);
	output[3] = byte(value);
}

template <class T> inline void XMACC<T>::XorDigest(HashWordType *digest, const HashWordType *buffer)
{
	for (unsigned i=0; i<(T::DIGESTSIZE/sizeof(HashWordType)); i++)
		digest[i] ^= buffer[i];
}

template <class T> void XMACC<T>::vTransform(const HashWordType *input)
{
	memcpy(buffer, key, KEYLENGTH);
	WriteWord32((byte *)buffer.ptr+KEYLENGTH, ++index);
	T::CorrectEndianess(buffer, buffer, T::DIGESTSIZE);
	T::Transform(buffer, input);
	XorDigest(digest, buffer);
}

template <class T> void XMACC<T>::TruncatedFinal(byte *mac, unsigned int size)
{
	assert(4 <= size && size <= DIGESTSIZE);
	assert(counter != 0xffffffff);

	PadLastBlock(BLOCKSIZE - 2*sizeof(HashWordType));
	CorrectEndianess(data, data, BLOCKSIZE - 2*sizeof(HashWordType));
	data[data.size-2] = byteReverse(countHi);	// byteReverse for backwards compatibility
	data[data.size-1] = byteReverse(countLo);
	vTransform(data);

	memcpy(buffer, key, KEYLENGTH);
	WriteWord32((byte *)buffer.ptr+KEYLENGTH, 0);
	memset(data, 0, BLOCKSIZE-4);
	WriteWord32((byte *)data.ptr+BLOCKSIZE-4, ++counter);
	T::CorrectEndianess(buffer, buffer, T::DIGESTSIZE);
	T::CorrectEndianess(data, data, BLOCKSIZE);
	T::Transform(buffer, data);
	XorDigest(digest, buffer);

	WriteWord32(mac, counter);
	T::CorrectEndianess(digest, digest, T::DIGESTSIZE);
	memcpy(mac+4, digest, size-4);

	Reinit();		// reinit for next use
}

template <class T> bool XMACC<T>::TruncatedVerify(const byte *mac, unsigned int size)
{
	assert(4 <= size && size <= DIGESTSIZE);

	PadLastBlock(BLOCKSIZE - 2*sizeof(HashWordType));
	CorrectEndianess(data, data, BLOCKSIZE - 2*sizeof(HashWordType));
	data[data.size-2] = byteReverse(countHi);	// byteReverse for backwards compatibility
	data[data.size-1] = byteReverse(countLo);
	vTransform(data);

	memcpy(buffer, key, KEYLENGTH);
	WriteWord32((byte *)buffer.ptr+KEYLENGTH, 0);
	memset(data, 0, BLOCKSIZE-4);
	memcpy((byte *)data.ptr+BLOCKSIZE-4, mac, 4);
	T::CorrectEndianess(buffer, buffer, T::DIGESTSIZE);
	T::CorrectEndianess(data, data, BLOCKSIZE);
	T::Transform(buffer, data);
	XorDigest(digest, buffer);

	T::CorrectEndianess(digest, digest, T::DIGESTSIZE);
	bool macValid = (memcmp(mac+4, digest, size-4) == 0);
	Reinit();		// reinit for next use
	return macValid;
}

NAMESPACE_END

#endif
