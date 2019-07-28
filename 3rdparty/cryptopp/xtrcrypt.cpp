// xtrcrypt.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "xtrcrypt.h"
#include "nbtheory.h"
#include "asn.h"

NAMESPACE_BEGIN(CryptoPP)

XTR_DH::XTR_DH(const Integer &p, const Integer &q, const GFP2Element &g)
	: p(p), q(q), g(g)
{
}

XTR_DH::XTR_DH(RandomNumberGenerator &rng, unsigned int pbits, unsigned int qbits)
{
	XTR_FindPrimesAndGenerator(rng, p, q, g, pbits, qbits);
}

XTR_DH::XTR_DH(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	p.BERDecode(seq);
	q.BERDecode(seq);
	g.c1.BERDecode(seq);
	g.c2.BERDecode(seq);
	seq.MessageEnd();
}

void XTR_DH::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	p.DEREncode(seq);
	q.DEREncode(seq);
	g.c1.DEREncode(seq);
	g.c2.DEREncode(seq);
	seq.MessageEnd();
}

bool XTR_DH::ValidateDomainParameters(RandomNumberGenerator &rng) const
{
	GFP2Element three = GFP2_ONB<ModularArithmetic>(p).ConvertIn(3);
	return VerifyPrime(rng, p) && VerifyPrime(rng, q) && (p.Squared()-p+1)%q == Integer::Zero() &&
		XTR_Exponentiate(g, (p.Squared()-p+1)/q, p) != three &&
		XTR_Exponentiate(g, q, p) == three;
}

void XTR_DH::GenerateKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const
{
	Integer x(rng, 1, q-1);
	GFP2Element y = XTR_Exponentiate(g, x, p);
	x.Encode(privateKey, PrivateKeyLength());
	y.Encode(publicKey, PublicKeyLength());
}

bool XTR_DH::Agree(byte *agreedValue, const byte *privateKey, const byte *otherPublicKey, bool validateOtherPublicKey) const
{
	GFP2Element w(otherPublicKey, PublicKeyLength());
	if (validateOtherPublicKey)
	{
		GFP2_ONB<ModularArithmetic> gfp2(p);
		GFP2Element three = gfp2.ConvertIn(3);
		if (w.c1.IsNegative() || w.c2.IsNegative() || w.c1 >= p || w.c2 >= p || w == three)
			return false;
		if (XTR_Exponentiate(w, q, p) != three)
			return false;
	}
	Integer s(privateKey, PrivateKeyLength());
	GFP2Element z = XTR_Exponentiate(w, s, p);
	z.Encode(agreedValue, AgreedValueLength());
	return true;
}

NAMESPACE_END
