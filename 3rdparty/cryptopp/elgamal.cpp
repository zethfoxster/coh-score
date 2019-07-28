// elgamal.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "elgamal.h"
#include "asn.h"
#include "nbtheory.h"

NAMESPACE_BEGIN(CryptoPP)

ElGamalEncryptor::ElGamalEncryptor(const Integer &p, const Integer &g, const Integer &y)
	: m_p(p), m_g(g), m_y(y), m_gpc(p, g), m_ypc(p, y)
{
}

ElGamalEncryptor::ElGamalEncryptor(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	m_p.BERDecode(seq);
	m_g.BERDecode(seq);
	m_y.BERDecode(seq);
	seq.MessageEnd();

	m_gpc.SetModulusAndBase(m_p, m_g);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

void ElGamalEncryptor::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	m_p.DEREncode(seq);
	m_g.DEREncode(seq);
	m_y.DEREncode(seq);
	seq.MessageEnd();
}

void ElGamalEncryptor::Precompute(unsigned int precomputationStorage)
{
	m_gpc.Precompute(ExponentBitLength(), precomputationStorage);
	m_ypc.Precompute(ExponentBitLength(), precomputationStorage);
}

void ElGamalEncryptor::LoadPrecomputation(BufferedTransformation &bt)
{
	m_gpc.Load(bt);
	m_ypc.Load(bt);
}

void ElGamalEncryptor::SavePrecomputation(BufferedTransformation &bt) const
{
	m_gpc.Save(bt);
	m_ypc.Save(bt);
}

void ElGamalEncryptor::Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText)
{
	assert(plainTextLength <= MaxPlainTextLength());

	unsigned int modulusLen = m_p.ByteCount();
	SecByteBlock block(modulusLen-1);
	rng.GetBlock(block, modulusLen-2-plainTextLength);
	memcpy(block+modulusLen-2-plainTextLength, plainText, plainTextLength);
	block[modulusLen-2] = plainTextLength;

	Integer m(block, modulusLen-1);
	Integer a,b;
	RawEncrypt(Integer(rng, ExponentBitLength()), m, a, b);

	a.Encode(cipherText, modulusLen);
	b.Encode(cipherText+modulusLen, modulusLen);
}

void ElGamalEncryptor::RawEncrypt(const Integer &k, const Integer &m, Integer &a, Integer &b) const
{
	a = m_gpc.Exponentiate(k);
	b = m * m_ypc.Exponentiate(k) % m_p;
}

unsigned int ElGamalEncryptor::ExponentBitLength() const
{
	return 2*DiscreteLogWorkFactor(m_p.BitCount());
}

// *************************************************************

ElGamalDecryptor::ElGamalDecryptor(const Integer &p, const Integer &g, const Integer &y, const Integer &x)
	: ElGamalEncryptor(p, g, y), m_x(x)
{
}

ElGamalDecryptor::ElGamalDecryptor(RandomNumberGenerator &rng, unsigned int pbits)
{
	PrimeAndGenerator pg(1, rng, pbits);
	m_p = pg.Prime();
	m_g = pg.Generator();
	m_x.Randomize(rng, ExponentBitLength());
	m_gpc.SetModulusAndBase(m_p, m_g);
	m_y = m_gpc.Exponentiate(m_x);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

ElGamalDecryptor::ElGamalDecryptor(RandomNumberGenerator &rng, const Integer &pIn, const Integer &gIn)
{
	m_p = pIn;
	m_g = gIn;
	m_x.Randomize(rng, ExponentBitLength());
	m_gpc.SetModulusAndBase(m_p, m_g);
	m_y = m_gpc.Exponentiate(m_x);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

ElGamalDecryptor::ElGamalDecryptor(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	m_p.BERDecode(seq);
	m_g.BERDecode(seq);
	m_y.BERDecode(seq);
	m_x.BERDecode(seq);
	seq.MessageEnd();

	m_gpc.SetModulusAndBase(m_p, m_g);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

void ElGamalDecryptor::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	m_p.DEREncode(seq);
	m_g.DEREncode(seq);
	m_y.DEREncode(seq);
	m_x.DEREncode(seq);
	seq.MessageEnd();
}

unsigned int ElGamalDecryptor::Decrypt(const byte *cipherText, byte *plainText)
{
	unsigned int modulusLen = m_p.ByteCount();
	Integer a(cipherText, modulusLen);
	Integer b(cipherText+modulusLen, modulusLen);
	Integer m;

	RawDecrypt(a, b, m);
	m.Encode(plainText, 1);
	unsigned int plainTextLength = plainText[0];
	if (plainTextLength > MaxPlainTextLength())
		return 0;
	m >>= 8;
	m.Encode(plainText, plainTextLength);
	return plainTextLength;
}

void ElGamalDecryptor::RawDecrypt(const Integer &a, const Integer &b, Integer &m) const
{
	if (m_x.BitCount()+20 < m_p.BitCount()) // if x is short
		m = b * EuclideanMultiplicativeInverse(a_exp_b_mod_c(a, m_x, m_p), m_p) % m_p;
	else	// save a multiplicative inverse calculation
		m = b * a_exp_b_mod_c(a, m_p-1-m_x, m_p) % m_p;
}

NAMESPACE_END
