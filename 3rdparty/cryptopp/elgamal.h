#ifndef CRYPTOPP_ELGAMAL_H
#define CRYPTOPP_ELGAMAL_H

#include "modexppc.h"

NAMESPACE_BEGIN(CryptoPP)

//! ElGamal Encryptor
class ElGamalEncryptor : public PK_WithPrecomputation<PK_FixedLengthEncryptor>
{
public:
	ElGamalEncryptor(const Integer &p, const Integer &g, const Integer &y);
	ElGamalEncryptor(BufferedTransformation &bt);

	void DEREncode(BufferedTransformation &bt) const;

	void Precompute(unsigned int precomputationStorage=16);
	void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

	void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText);

	unsigned int MaxPlainTextLength() const {return STDMIN(255U, m_p.ByteCount()-3);}
	unsigned int CipherTextLength() const {return 2*m_p.ByteCount();}

	void RawEncrypt(const Integer &k, const Integer &m, Integer &a, Integer &b) const;

	const Integer & GetModulus() const {return m_p;}
	const Integer & GetGenerator() const {return m_g;}
	const Integer & GetPublicResidue() const {return m_y;}

	const ModExpPrecomputation & GetGPC() const {return m_gpc;}
	const ModExpPrecomputation & GetYPC() const {return m_ypc;}

protected:
	ElGamalEncryptor() {}
	unsigned int ExponentBitLength() const;

	Integer m_p, m_g, m_y;
	ModExpPrecomputation m_gpc, m_ypc;
};

//! ElGamal Decryptor
class ElGamalDecryptor : public ElGamalEncryptor, public PK_FixedLengthDecryptor
{
public:
	ElGamalDecryptor(const Integer &p, const Integer &g, const Integer &y, const Integer &x);
	ElGamalDecryptor(RandomNumberGenerator &rng, unsigned int pbits);
	// generate a random private key, given p and g
	ElGamalDecryptor(RandomNumberGenerator &rng, const Integer &p, const Integer &g);

	ElGamalDecryptor(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	unsigned int Decrypt(const byte *cipherText, byte *plainText);

	void RawDecrypt(const Integer &a, const Integer &b, Integer &m) const;

	const Integer & GetPrivateExponent() const {return m_x;}

protected:
	Integer m_x;
};

NAMESPACE_END

#endif
