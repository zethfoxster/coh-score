// mqv.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "mqv.h"
#include "asn.h"
#include "nbtheory.h"
#include "modarith.h"

NAMESPACE_BEGIN(CryptoPP)

MQV::MQV(const Integer &p, const Integer &q, const Integer &g)
	: p(p), q(q), g(g), gpc(p, g)
{
}

MQV::MQV(RandomNumberGenerator &rng, unsigned int pbits)
{
	PrimeAndGenerator pg(1, rng, pbits, 2*DiscreteLogWorkFactor(pbits));
	p = pg.Prime();
	q = pg.SubPrime();
	g = pg.Generator();
	gpc.SetModulusAndBase(p, g);
}

MQV::MQV(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	p.BERDecode(seq);
	q.BERDecode(seq);
	g.BERDecode(seq);
	seq.MessageEnd();

	gpc.SetModulusAndBase(p, g);
}

void MQV::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	p.DEREncode(seq);
	q.DEREncode(seq);
	g.DEREncode(seq);
	seq.MessageEnd();
}

void MQV::Precompute(unsigned int precomputationStorage)
{
	gpc.Precompute(ExponentBitLength(), precomputationStorage);
}

void MQV::LoadPrecomputation(BufferedTransformation &bt)
{
	gpc.Load(bt);
}

void MQV::SavePrecomputation(BufferedTransformation &bt) const
{
	gpc.Save(bt);
}

bool MQV::ValidateDomainParameters(RandomNumberGenerator &rng) const
{
	return VerifyPrime(rng, p) && VerifyPrime(rng, q) && p%q==1 && g > 1 && g < p && a_exp_b_mod_c(g, q, p)==1;
}

void MQV::GenerateStaticKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const
{
	Integer x(rng, ExponentBitLength());
	Integer y = gpc.Exponentiate(x);
	x.Encode(privateKey, StaticPrivateKeyLength());
	y.Encode(publicKey, StaticPublicKeyLength());
}

void MQV::GenerateEphemeralKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const
{
	Integer x(rng, ExponentBitLength());
	Integer y = gpc.Exponentiate(x);
	x.Encode(privateKey, q.ByteCount());
	y.Encode(privateKey+q.ByteCount(), p.ByteCount());
	y.Encode(publicKey, EphemeralPublicKeyLength());
}

bool MQV::Agree(byte *agreedValue, const byte *staticPrivateKey, const byte *ephemeralPrivateKey, const byte *staticOtherPublicKey, const byte *ephemeralOtherPublicKey, bool validateStaticOtherPublicKey) const
{
	Integer ww(staticOtherPublicKey, StaticPublicKeyLength());
	Integer vv(ephemeralOtherPublicKey, EphemeralPublicKeyLength());
	if (!(vv > 1 && vv < p) || (validateStaticOtherPublicKey && !(ww > 1 && ww < p)))
		return false;

	Integer s(staticPrivateKey, StaticPrivateKeyLength());
	Integer u(ephemeralPrivateKey, q.ByteCount());
	Integer v(ephemeralPrivateKey+q.ByteCount(), p.ByteCount());

	Integer h2 = Integer::Power2((q.BitCount()+1)/2);
	ModularArithmetic mr(p);
	Integer b = mr.Exponentiate(ww, h2+vv%h2);
	b = mr.Multiply(b, vv);

	const Integer e[2] = {q, ((h2+v%h2)*s+u) % q};
	Integer r[2];
	mr.SimultaneousExponentiate(r, b, e, 2);

	if (r[0] != mr.One() || r[1] == mr.One())
		return false;

	r[1].Encode(agreedValue, AgreedValueLength());
	return true;
}

unsigned int MQV::ExponentBitLength() const
{
	return q.BitCount();
}

NAMESPACE_END
