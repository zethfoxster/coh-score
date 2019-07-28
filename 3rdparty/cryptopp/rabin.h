#ifndef CRYPTOPP_RABIN_H
#define CRYPTOPP_RABIN_H

/** \file
*/

#include "oaep.h"
#include "pssr.h"
#include "integer.h"

NAMESPACE_BEGIN(CryptoPP)

//! Rabin
class RabinFunction : virtual public TrapdoorFunction
{
public:
	RabinFunction(const Integer &n, const Integer &r, const Integer &s);
	RabinFunction(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	Integer ApplyFunction(const Integer &x) const;
	Integer PreimageBound() const {return n;}
	Integer ImageBound() const {return n;}

	const Integer& GetModulus() const {return n;}
	const Integer& GetQuadraticResidueModPrime1() const {return r;}
	const Integer& GetQuadraticResidueModPrime2() const {return s;}

protected:
	RabinFunction() {}	// to be used only by InvertibleRabinFunction
	Integer n, r, s;	// these are only modified in constructors
};

//! Invertible Rabin
class InvertibleRabinFunction : public RabinFunction, public InvertibleTrapdoorFunction
{
public:
	InvertibleRabinFunction(const Integer &n, const Integer &r, const Integer &s,
							const Integer &p, const Integer &q, const Integer &u);
	// generate a random private key
	InvertibleRabinFunction(RandomNumberGenerator &rng, unsigned int keybits);
	InvertibleRabinFunction(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	Integer CalculateInverse(const Integer &x) const;

	const Integer& GetPrime1() const {return p;}
	const Integer& GetPrime2() const {return q;}

protected:
	Integer p, q, u;
};

//! Rabin Private Key
template <class B>
class RabinPrivateKeyTemplate : public B
{
public:
	RabinPrivateKeyTemplate(const Integer &n, const Integer &r, const Integer &s,
					const Integer &p, const Integer &q, const Integer &u)
		: PublicKeyBaseTemplate<InvertibleRabinFunction>(
			InvertibleRabinFunction(n, r, s, p, q, u)) {}

	RabinPrivateKeyTemplate(RandomNumberGenerator &rng, unsigned int keybits)
		: PublicKeyBaseTemplate<InvertibleRabinFunction>(
			InvertibleRabinFunction(rng, keybits)) {}

	RabinPrivateKeyTemplate(BufferedTransformation &bt)
		: PublicKeyBaseTemplate<InvertibleRabinFunction>(bt) {}
};

//! Rabin Public Key
template <class B, class V>
class RabinPublicKeyTemplate : public B
{
public:
	RabinPublicKeyTemplate(const Integer &n, const Integer &r, const Integer &s)
		: PublicKeyBaseTemplate<RabinFunction>(RabinFunction(n, r, s)) {}

	RabinPublicKeyTemplate(const V &priv)
		: PublicKeyBaseTemplate<RabinFunction>(priv.GetTrapdoorFunction()) {}

	RabinPublicKeyTemplate(BufferedTransformation &bt)
		: PublicKeyBaseTemplate<RabinFunction>(bt) {}
};

class SHA;

//! Rabin Decryptor
typedef RabinPrivateKeyTemplate<DecryptorTemplate<OAEP<SHA>, InvertibleRabinFunction> >
	RabinDecryptor;
//! Rabin Encryptor
typedef RabinPublicKeyTemplate<EncryptorTemplate<OAEP<SHA>, RabinFunction>, RabinDecryptor>
	RabinEncryptor;

// simulate template typedef
#define RabinSignerWith(H) RabinPrivateKeyTemplate<SignerWithRecoveryTemplate<InvertibleRabinFunction, PSSR<H> > >
#define RabinVerifierWith(H) RabinPublicKeyTemplate<VerifierWithRecoveryTemplate<RabinFunction, PSSR<H> >, RabinSignerWith(H) >

NAMESPACE_END

#endif
