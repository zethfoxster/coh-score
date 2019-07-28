// nr.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "nr.h"
#include "asn.h"
#include "nbtheory.h"

NAMESPACE_BEGIN(CryptoPP)

Integer NR_EncodeDigest(unsigned int modulusBits, const byte *digest, unsigned int digestLen)
{
	Integer h;
	if (digestLen*8 < modulusBits)
		h.Decode(digest, digestLen);
	else
	{
		h.Decode(digest, bitsToBytes(modulusBits));
		h >>= bitsToBytes(modulusBits)*8 - modulusBits + 1;
	}
	return h;
}

NRDigestVerifier::NRDigestVerifier(const Integer &p, const Integer &q,
			   const Integer &g, const Integer &y)
	: m_p(p), m_q(q), m_g(g), m_y(y),
	  m_gpc(p, g), m_ypc(p, y)
{
}

void NRDigestVerifier::Precompute(unsigned int precomputationStorage)
{
	m_gpc.Precompute(ExponentBitLength(), precomputationStorage);
	m_ypc.Precompute(ExponentBitLength(), precomputationStorage);
}

void NRDigestVerifier::LoadPrecomputation(BufferedTransformation &bt)
{
	m_gpc.Load(bt);
	m_ypc.Load(bt);
}

void NRDigestVerifier::SavePrecomputation(BufferedTransformation &bt) const
{
	m_gpc.Save(bt);
	m_ypc.Save(bt);
}

Integer NRDigestVerifier::EncodeDigest(const byte *digest, unsigned int digestLen) const
{
	return NR_EncodeDigest(m_q.BitCount(), digest, digestLen);
}

unsigned int NRDigestVerifier::ExponentBitLength() const
{
	return m_q.BitCount();
}

NRDigestVerifier::NRDigestVerifier(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	m_p.BERDecode(seq);
	m_q.BERDecode(seq);
	m_g.BERDecode(seq);
	m_y.BERDecode(seq);
	seq.MessageEnd();

	m_gpc.SetModulusAndBase(m_p, m_g);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

void NRDigestVerifier::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	m_p.DEREncode(seq);
	m_q.DEREncode(seq);
	m_g.DEREncode(seq);
	m_y.DEREncode(seq);
	seq.MessageEnd();
}

bool NRDigestVerifier::VerifyDigest(const byte *digest, unsigned int digestLen, const byte *signature) const
{
	assert(digestLen <= MaxDigestLength());

	Integer h = EncodeDigest(digest, digestLen);
	unsigned int qLen = m_q.ByteCount();
	Integer r(signature, qLen);
	Integer s(signature+qLen, qLen);
	return RawVerify(h, r, s);
}

bool NRDigestVerifier::RawVerify(const Integer &m, const Integer &r, const Integer &s) const
{
	if (r>=m_q || r<1 || s>=m_q)
		return false;

	// check r == (m_g^s * m_y^r + m) mod m_q
	return r == (m_gpc.CascadeExponentiate(s, m_ypc, r) + m) % m_q;
}

// ******************************************************************

NRDigestSigner::NRDigestSigner(const Integer &p, const Integer &q, const Integer &g, const Integer &y, const Integer &x)
	: NRDigestVerifier(p, q, g, y), m_x(x)
{
}

NRDigestSigner::NRDigestSigner(RandomNumberGenerator &rng, unsigned int pbits)
{
	PrimeAndGenerator pg(1, rng, pbits, 2*DiscreteLogWorkFactor(pbits));
	m_p = pg.Prime();
	m_q = pg.SubPrime();
	m_g = pg.Generator();
	m_x.Randomize(rng, 1, m_q-1, Integer::ANY);
	m_gpc.SetModulusAndBase(m_p, m_g);
	m_y = m_gpc.Exponentiate(m_x);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

NRDigestSigner::NRDigestSigner(RandomNumberGenerator &rng, const Integer &pIn, const Integer &qIn, const Integer &gIn)
{
	m_p = pIn;
	m_q = qIn;
	m_g = gIn;
	m_x.Randomize(rng, 1, m_q-1, Integer::ANY);
	m_gpc.SetModulusAndBase(m_p, m_g);
	m_y = m_gpc.Exponentiate(m_x);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

NRDigestSigner::NRDigestSigner(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	m_p.BERDecode(seq);
	m_q.BERDecode(seq);
	m_g.BERDecode(seq);
	m_y.BERDecode(seq);
	m_x.BERDecode(seq);
	seq.MessageEnd();

	m_gpc.SetModulusAndBase(m_p, m_g);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

void NRDigestSigner::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	m_p.DEREncode(seq);
	m_q.DEREncode(seq);
	m_g.DEREncode(seq);
	m_y.DEREncode(seq);
	m_x.DEREncode(seq);
	seq.MessageEnd();
}

void NRDigestSigner::SignDigest(RandomNumberGenerator &rng, const byte *digest, unsigned int digestLen, byte *signature) const
{
	assert(digestLen <= MaxDigestLength());

	Integer h = EncodeDigest(digest, digestLen);
	Integer r;
	Integer s;

	RawSign(rng, h, r, s);
	unsigned int qLen = m_q.ByteCount();
	r.Encode(signature, qLen);
	s.Encode(signature+qLen, qLen);
}

void NRDigestSigner::RawSign(RandomNumberGenerator &rng, const Integer &m, Integer &r, Integer &s) const
{
	do
	{
		Integer k(rng, 1, m_q-1, Integer::ANY);
		r = (m_gpc.Exponentiate(k) + m) % m_q;
		s = (k - m_x*r) % m_q;
	} while (!r);			// make sure r != 0
}

NAMESPACE_END
