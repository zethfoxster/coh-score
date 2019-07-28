#ifndef CRYPTOPP_XTRCRYPT_H
#define CRYPTOPP_XTRCRYPT_H

/** \file
	"The XTR public key system" by Arjen K. Lenstra and Eric R. Verheul
*/

#include "xtr.h"

NAMESPACE_BEGIN(CryptoPP)

//! XTR-DH with key validation

class XTR_DH : public PK_SimpleKeyAgreementDomain
{
public:
	XTR_DH(const Integer &p, const Integer &q, const GFP2Element &g);
	XTR_DH(RandomNumberGenerator &rng, unsigned int pbits, unsigned int qbits);
	XTR_DH(BufferedTransformation &domainParams);

	void DEREncode(BufferedTransformation &domainParams) const;

	bool ValidateDomainParameters(RandomNumberGenerator &rng) const;
	unsigned int AgreedValueLength() const {return 2*p.ByteCount();}
	unsigned int PrivateKeyLength() const {return q.ByteCount();}
	unsigned int PublicKeyLength() const {return 2*p.ByteCount();}

	void GenerateKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const;
	bool Agree(byte *agreedValue, const byte *privateKey, const byte *otherPublicKey, bool validateOtherPublicKey=true) const;

	const Integer &GetPrime() const {return p;}
	const Integer &GetSubprime() const {return q;}
	const GFP2Element &GetGenerator() const {return g;}

private:
	unsigned int ExponentBitLength() const;

	Integer p, q;
	GFP2Element g;
};

NAMESPACE_END

#endif
