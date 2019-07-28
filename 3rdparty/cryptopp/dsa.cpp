// dsa.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "dsa.h"
#include "asn.h"
#include "oids.h"
#include "nbtheory.h"
#include "sha.h"

NAMESPACE_BEGIN(CryptoPP)

unsigned int DSAConvertSignatureFormat(byte *buffer, unsigned int bufferSize, DSASignatureFormat toFormat, const byte *signature, unsigned int signatureLen, DSASignatureFormat fromFormat)
{
	Integer r, s;
	StringStore store(signature, signatureLen);
	ArraySink sink(buffer, bufferSize);

	switch (fromFormat)
	{
	case DSA_P1363:
		r.Decode(store, signatureLen/2);
		s.Decode(store, signatureLen/2);
		break;
	case DSA_DER:
	{
		BERSequenceDecoder seq(store);
		r.BERDecode(seq);
		s.BERDecode(seq);
		seq.MessageEnd();
		break;
	}
	case DSA_OPENPGP:
		r.OpenPGPDecode(store);
		s.OpenPGPDecode(store);
		break;
	}

	switch (toFormat)
	{
	case DSA_P1363:
		r.Encode(sink, bufferSize/2);
		s.Encode(sink, bufferSize/2);
		break;
	case DSA_DER:
	{
		DERSequenceEncoder seq(sink);
		r.DEREncode(seq);
		s.DEREncode(seq);
		seq.MessageEnd();
		break;
	}
	case DSA_OPENPGP:
		r.OpenPGPEncode(sink);
		s.OpenPGPEncode(sink);
		break;
	}

	return sink.TotalPutLength();
}

Integer DSA_EncodeDigest(unsigned int modulusBits, const byte *digest, unsigned int digestLen)
{
	Integer h;
	if (digestLen*8 <= modulusBits)
		h.Decode(digest, digestLen);
	else
	{
		h.Decode(digest, bitsToBytes(modulusBits));
		h >>= bitsToBytes(modulusBits)*8 - modulusBits;
	}
	return h;
}

GDSADigestVerifier::GDSADigestVerifier(const Integer &p, const Integer &q,
			   const Integer &g, const Integer &y)
	: m_p(p), m_q(q), m_g(g), m_y(y),
	  m_gpc(p, g), m_ypc(p, y)
{
}

void GDSADigestVerifier::Precompute(unsigned int precomputationStorage)
{
	m_gpc.Precompute(ExponentBitLength(), precomputationStorage);
	m_ypc.Precompute(ExponentBitLength(), precomputationStorage);
}

void GDSADigestVerifier::LoadPrecomputation(BufferedTransformation &bt)
{
	m_gpc.Load(bt);
	m_ypc.Load(bt);
}

void GDSADigestVerifier::SavePrecomputation(BufferedTransformation &bt) const
{
	m_gpc.Save(bt);
	m_ypc.Save(bt);
}

Integer GDSADigestVerifier::EncodeDigest(const byte *digest, unsigned int digestLen) const
{
	return DSA_EncodeDigest(m_q.BitCount(), digest, digestLen);
}

unsigned int GDSADigestVerifier::ExponentBitLength() const
{
	return m_q.BitCount();
}

GDSADigestVerifier::GDSADigestVerifier(BufferedTransformation &bt)
{
	BERSequenceDecoder subjectPublicKeyInfo(bt);
	if (subjectPublicKeyInfo.PeekByte() == INTEGER)
	{
		// for backwards compatibility
		m_p.BERDecode(subjectPublicKeyInfo);
		m_q.BERDecode(subjectPublicKeyInfo);
		m_g.BERDecode(subjectPublicKeyInfo);
		m_y.BERDecode(subjectPublicKeyInfo);
	}
	else
	{
		BERSequenceDecoder algorithm(subjectPublicKeyInfo);
			ASN1::id_dsa().BERDecodeAndCheck(algorithm);
			BERSequenceDecoder parameters(algorithm);
				m_p.BERDecode(parameters);
				m_q.BERDecode(parameters);
				m_g.BERDecode(parameters);
			parameters.MessageEnd();
		algorithm.MessageEnd();

		BERSequenceDecoder subjectPublicKey(subjectPublicKeyInfo, BIT_STRING);
			subjectPublicKey.CheckByte(0);	// unused bits
			m_y.BERDecode(subjectPublicKey);
		subjectPublicKey.MessageEnd();
	}
	subjectPublicKeyInfo.MessageEnd();

	m_gpc.SetModulusAndBase(m_p, m_g);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

void GDSADigestVerifier::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder subjectPublicKeyInfo(bt);

		DERSequenceEncoder algorithm(subjectPublicKeyInfo);
			ASN1::id_dsa().DEREncode(algorithm);
			DERSequenceEncoder parameters(algorithm);
				m_p.DEREncode(parameters);
				m_q.DEREncode(parameters);
				m_g.DEREncode(parameters);
			parameters.MessageEnd();
		algorithm.MessageEnd();

		DERGeneralEncoder subjectPublicKey(subjectPublicKeyInfo, BIT_STRING);
			subjectPublicKey.Put(0);	// unused bits
			m_y.DEREncode(subjectPublicKey);
		subjectPublicKey.MessageEnd();

	subjectPublicKeyInfo.MessageEnd();
}

bool GDSADigestVerifier::VerifyDigest(const byte *digest, unsigned int digestLen, const byte *signature) const
{
	assert(digestLen <= MaxDigestLength());

	Integer h = EncodeDigest(digest, digestLen);
	unsigned int qLen = m_q.ByteCount();
	Integer r(signature, qLen);
	Integer s(signature+qLen, qLen);
	return RawVerify(h, r, s);
}

bool GDSADigestVerifier::RawVerify(const Integer &h, const Integer &r, const Integer &s) const
{
	if (r>=m_q || r<1 || s>=m_q || s<1)
		return false;

	Integer w = EuclideanMultiplicativeInverse(s, m_q);
	Integer u1 = (h * w) % m_q;
	Integer u2 = (r * w) % m_q;
	// verify r == (g^u1 * y^u2 mod p) mod q
	return r == m_gpc.CascadeExponentiate(u1, m_ypc, u2) % m_q;
}

// ******************************************************************

GDSADigestSigner::GDSADigestSigner(const Integer &p, const Integer &q, const Integer &g, const Integer &y, const Integer &x)
	: GDSADigestVerifier(p, q, g, y), m_x(x)
{
}

GDSADigestSigner::GDSADigestSigner(RandomNumberGenerator &rng, unsigned int pbits)
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

GDSADigestSigner::GDSADigestSigner(RandomNumberGenerator &rng, const Integer &pIn, const Integer &qIn, const Integer &gIn)
{
	m_p = pIn;
	m_q = qIn;
	m_g = gIn;
	m_x.Randomize(rng, 1, m_q-1, Integer::ANY);
	m_gpc.SetModulusAndBase(m_p, m_g);
	m_y = m_gpc.Exponentiate(m_x);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

GDSADigestSigner::GDSADigestSigner(BufferedTransformation &bt)
{
	BERSequenceDecoder privateKeyInfo(bt);
		m_p.BERDecode(privateKeyInfo);
		if (m_p != Integer::Zero())
		{
			// for backwards compatibility
			m_q.BERDecode(privateKeyInfo);
			m_g.BERDecode(privateKeyInfo);
			m_y.BERDecode(privateKeyInfo);
			m_x.BERDecode(privateKeyInfo);
		}
		else
		{
			BERSequenceDecoder algorithm(privateKeyInfo);
				ASN1::id_dsa().BERDecodeAndCheck(algorithm);
				BERSequenceDecoder parameters(algorithm);
					m_p.BERDecode(parameters);
					m_q.BERDecode(parameters);
					m_g.BERDecode(parameters);
				parameters.MessageEnd();
			algorithm.MessageEnd();

			BERGeneralDecoder octetString(privateKeyInfo, OCTET_STRING);
				m_x.BERDecode(octetString);
			octetString.MessageEnd();
		}
	privateKeyInfo.MessageEnd();

	m_gpc.SetModulusAndBase(m_p, m_g);
	m_y = m_gpc.Exponentiate(m_x);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

void GDSADigestSigner::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder privateKeyInfo(bt);

		DEREncodeUnsigned<word32>(privateKeyInfo, 0);	// version

		DERSequenceEncoder algorithm(privateKeyInfo);
			ASN1::id_dsa().DEREncode(algorithm);
			DERSequenceEncoder parameters(algorithm);
				m_p.DEREncode(parameters);
				m_q.DEREncode(parameters);
				m_g.DEREncode(parameters);
			parameters.MessageEnd();
		algorithm.MessageEnd();

		DERGeneralEncoder octetString(privateKeyInfo, OCTET_STRING);
			m_x.DEREncode(octetString);
		octetString.MessageEnd();

	privateKeyInfo.MessageEnd();
}

void GDSADigestSigner::SignDigest(RandomNumberGenerator &rng, const byte *digest, unsigned int digestLen, byte *signature) const
{
	assert(digestLen <= MaxDigestLength());

	Integer h = EncodeDigest(digest, digestLen);
	Integer k(rng, 1, m_q-1);
	Integer r, s;

	RawSign(k, h, r, s);
	r.Encode(signature, m_q.ByteCount());
	s.Encode(signature+m_q.ByteCount(), m_q.ByteCount());
}

void GDSADigestSigner::RawSign(const Integer &k, const Integer &h, Integer &r, Integer &s) const
{
	do
	{
		r = m_gpc.Exponentiate(k) % m_q;
		Integer kInv = EuclideanMultiplicativeInverse(k, m_q);
		s = (kInv * (m_x*r + h)) % m_q;
	} while (!r || !s);
}

bool GenerateDSAPrimes(byte *seed, unsigned int g, int &counter,
						  Integer &p, unsigned int L, Integer &q)
{
	assert(L >= MIN_DSA_PRIME_LENGTH && L <= MAX_DSA_PRIME_LENGTH);
	assert(L % 64 == 0);

	SHA sha;
	SecByteBlock U(SHA::DIGESTSIZE);
	SecByteBlock temp(SHA::DIGESTSIZE);
	SecByteBlock W(((L-1)/160+1) * SHA::DIGESTSIZE);
	const int n = (L-1) / 160;
	const int b = (L-1) % 160;
	Integer X;

	sha.CalculateDigest(U, seed, g/8);

	for (int i=g/8-1, carry=true; i>=0 && carry; i--)
		carry=!++seed[i];

	sha.CalculateDigest(temp, seed, g/8);
	xorbuf(U, temp, SHA::DIGESTSIZE);

	U[0] |= 0x80;
	U[SHA::DIGESTSIZE-1] |= 1;
	q.Decode(U, SHA::DIGESTSIZE);

	if (!IsPrime(q))
		return false;

	for (counter = 0; counter < 4096; counter++)
	{
		for (int k=0; k<=n; k++)
		{
			for (int i=g/8-1, carry=true; i>=0 && carry; i--)
				carry=!++seed[i];
			sha.CalculateDigest(W+(n-k)*SHA::DIGESTSIZE, seed, g/8);
		}
		W[SHA::DIGESTSIZE - 1 - b/8] |= 0x80;
		X.Decode(W + SHA::DIGESTSIZE - 1 - b/8, L/8);
		p = X-((X % (2*q))-1);

		if (p.GetBit(L-1) && IsPrime(p))
			return true;
	}
	return false;
}

DSAPrivateKey::DSAPrivateKey(RandomNumberGenerator &rng, unsigned int keybits)
{
	SecByteBlock seed(SHA::DIGESTSIZE);
	Integer h;
	int c;

	do
	{
		rng.GetBlock(seed, SHA::DIGESTSIZE);
	} while (!GenerateDSAPrimes(seed, SHA::DIGESTSIZE*8, c, m_p, keybits, m_q));

	do
	{
		h.Randomize(rng, 2, m_p-2);
		m_g = a_exp_b_mod_c(h, (m_p-1)/m_q, m_p);
	} while (m_g <= 1);

	m_x.Randomize(rng, 1, m_q-1);
	m_gpc.SetModulusAndBase(m_p, m_g);
	m_y = m_gpc.Exponentiate(m_x);
	m_ypc.SetModulusAndBase(m_p, m_y);
}

NAMESPACE_END
