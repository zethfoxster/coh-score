#ifndef CRYPTOPP_ECCRYPTO_H
#define CRYPTOPP_ECCRTPTO_H

/*! \file
The following classes are explicitly instantiated in eccrypto.cpp

template class ECParameters<EC2N>;
template class ECParameters<ECP>;
template class ECPublicKey<EC2N>;
template class ECPublicKey<ECP>;
template class ECPrivateKey<EC2N>;
template class ECPrivateKey<ECP>;
template class ECDigestVerifier<EC2N, ECDSA>;
template class ECDigestVerifier<ECP, ECDSA>;
template class ECDigestSigner<EC2N, ECDSA>;
template class ECDigestSigner<ECP, ECDSA>;
template class ECDigestVerifier<EC2N, ECNR>;
template class ECDigestVerifier<ECP, ECNR>;
template class ECDigestSigner<EC2N, ECNR>;
template class ECDigestSigner<ECP, ECNR>;
template class ECDHC<EC2N>;
template class ECDHC<ECP>;
template class ECMQVC<EC2N>;
template class ECMQVC<ECP>;
*/

#include "pubkey.h"
#include "integer.h"
#include "asn.h"
#include "hmac.h"
#include "sha.h"

NAMESPACE_BEGIN(CryptoPP)

/*! The ECDSA signature format used by Crypto++ is as defined by IEEE P1363.
	To convert to or from other signature formats, see dsa.h.
*/
enum ECSignatureScheme
{
	ECNR,	///< Elliptic Curve Nyberg-Rueppel
	ECDSA	///< <a href="http://www.weidai.com/scan-mirror/sig.html#ECDSA">Elliptic Curve Digital Signature Algorithm</a>
};

template <class T> class EcPrecomputation;

//! Elliptic Curve Parameters
/*! This class corresponds to the ASN.1 sequence of the same name
    in ANSI X9.62 (also SEC 1).
*/
template <class EC>
class ECParameters : virtual public PK_Precomputation
{
public:
	typedef typename EC::Point Point;

	ECParameters() : m_compress(false), m_encodeAsOID(false) {}
	ECParameters(const OID &oid)
		: m_compress(false), m_encodeAsOID(false) {LoadRecommendedParameters(oid);}
	ECParameters(const EC &ec, const Point &G, const Integer &n)
		: m_ec(ec), m_G(G), m_Gpc(*m_ec, G), m_n(n), m_cofactorPresent(false), m_compress(false), m_encodeAsOID(false) {}
	ECParameters(const EC &ec, const Point &G, const Integer &n, const Integer &k)
		: m_ec(ec), m_G(G), m_Gpc(*m_ec, G), m_n(n), m_cofactorPresent(true), m_compress(false), m_encodeAsOID(false), m_k(k) {}
	ECParameters(BufferedTransformation &bt)
		: m_compress(false), m_encodeAsOID(false) {BERDecode(bt);}

	// enumerate OIDs for recommended parameters, use OID() to get first one
	static OID GetNextRecommendedParametersOID(const OID &oid);
	void LoadRecommendedParameters(const OID &oid);

	void BERDecode(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	bool ValidateParameters(RandomNumberGenerator &rng) const;

	void Precompute(unsigned int precomputationStorage=16);
	void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

	void SetPointCompression(bool compress) {m_compress = compress;}
	bool GetPointCompression() const {return m_compress;}

	void SetEncodeAsOID(bool encodeAsOID) {m_encodeAsOID = encodeAsOID;}
	bool GetEncodeAsOID() const {return m_encodeAsOID;}

	const EC& GetCurve() const {return *m_ec;}
	const Point& GetBasePoint() const {return m_G;}
	const Integer& GetBasePointOrder() const {return m_n;}
	const bool CofactorPresent() const {return m_cofactorPresent;}
	const Integer& GetCofactor() const {return m_k;}

protected:
	unsigned int EncodedPointSize() const {return m_ec->EncodedPointSize(m_compress);}
	void EncodePoint(byte *encodedPoint, const Point &P) const {m_ec->EncodePoint(encodedPoint, P, m_compress);}
	unsigned int FieldElementLength() const {return m_ec->GetField().MaxElementByteLength();}
	unsigned int ExponentLength() const {return m_n.ByteCount();}
	unsigned int ExponentBitLength() const {return m_n.BitCount();}

	OID m_oid;			// set if parameters loaded from a recommended curve
	value_ptr<EC> m_ec;	// field and curve
	Point m_G;			// base
	EcPrecomputation<EC> m_Gpc;	// precomputed table for base
	Integer m_n;		// order of base point
	bool m_cofactorPresent, m_compress, m_encodeAsOID;
	Integer m_k;		// cofactor
};

#define EC_PARAMETERS_CONSTRUCTORS(Self, Base)								\
	Self(const ECParameters<EC> &params)									\
		: Base(params) {}													\
	Self(const OID &oid)													\
		: Base(oid) {}														\
	Self(const EC &ec, const Point &G, const Integer &n, const Integer &k)	\
		: Base(ec, G, n, k) {}												\
	Self(BufferedTransformation &bt)										\
		: Base(bt) {}

/// Elliptic Curve Diffie-Hellman with Cofactor Multiplication, AKA <a href="http://www.weidai.com/scan-mirror/ka.html#ECDHC">ECDHC</a>
template <class EC>
class ECDHC : public ECParameters<EC>, public PK_WithPrecomputation<PK_SimpleKeyAgreementDomain>
{
public:
	typedef typename EC::Point Point;

	EC_PARAMETERS_CONSTRUCTORS(ECDHC, ECParameters<EC>)

	bool ValidateDomainParameters(RandomNumberGenerator &rng) const
		{return ValidateParameters(rng);}
	unsigned int AgreedValueLength() const {return FieldElementLength();}
	unsigned int PrivateKeyLength() const {return ExponentLength();}
	unsigned int PublicKeyLength() const {return EncodedPointSize();}

	void GenerateKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const;
	bool Agree(byte *agreedValue, const byte *privateKey, const byte *otherPublicKey, bool validateOtherPublicKey=true) const;
};

/// Elliptic Curve Menezes-Qu-Vanstone with Cofactor Multiplication, AKA <a href="http://www.weidai.com/scan-mirror/ka.html#ECMQVC">ECMQVC</a>
template <class EC>
class ECMQVC : public ECParameters<EC>, public PK_WithPrecomputation<PK_AuthenticatedKeyAgreementDomain>
{
public:
	typedef typename EC::Point Point;
	
	EC_PARAMETERS_CONSTRUCTORS(ECMQVC, ECParameters<EC>)

	bool ValidateDomainParameters(RandomNumberGenerator &rng) const
		{return ValidateParameters(rng);}
	unsigned int AgreedValueLength() const {return FieldElementLength();}

	unsigned int StaticPrivateKeyLength() const {return ExponentLength();}
	unsigned int StaticPublicKeyLength() const {return EncodedPointSize();}
	void GenerateStaticKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const;

	unsigned int EphemeralPrivateKeyLength() const {return ExponentLength()+EncodedPointSize();}
	unsigned int EphemeralPublicKeyLength() const {return EncodedPointSize();}
	void GenerateEphemeralKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const;

	bool Agree(byte *agreedValue,
		const byte *staticPrivateKey, const byte *ephemeralPrivateKey, 
		const byte *staticOtherPublicKey, const byte *ephemeralOtherPublicKey,
		bool validateStaticOtherPublicKey=true) const;
};

/// Elliptic Curve Public Key
template <class EC>
class ECPublicKey : public ECParameters<EC>, virtual public PK_Precomputation
{
public:
	typedef typename EC::Point Point;
	
	ECPublicKey(const ECParameters<EC> &params, const Point &Q)
		: ECParameters<EC>(params), m_Q(Q), m_Qpc(GetCurve(), m_Q) {}
	ECPublicKey(const OID &oidParams, const Point &Q)
		: ECParameters<EC>(oidParams), m_Q(Q), m_Qpc(GetCurve(), m_Q) {}
	ECPublicKey(const EC &ec, const Point &G, const Integer &n, const Point &Q)
		: ECParameters<EC>(ec, G, n), m_Q(Q), m_Qpc(ec, m_Q) {}
	// construct from a SubjectPublicKeyInfo sequence
	ECPublicKey(BufferedTransformation &bt);

	// encode to a SubjectPublicKeyInfo sequence
	void DEREncode(BufferedTransformation &bt) const;

	void Precompute(unsigned int precomputationStorage=16);
	void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

	const Point& GetPublicPoint() const {return m_Q;}

protected:
	ECPublicKey() {}
	Integer EncodeDigest(ECSignatureScheme ss, const byte *digest, unsigned int digestLen) const;

	Point m_Q;
	EcPrecomputation<EC> m_Qpc;
};

#define EC_PUBLIC_KEY_CONSTRUCTORS(Self, Base)								\
	Self(const ECPublicKey<EC> &key)										\
		: Base(key) {}														\
	Self(const ECParameters<EC> &params, const Point &Q)					\
		: Base(params, Q) {}												\
	Self(const OID &oid, const Point &Q)									\
		: Base(oid, Q) {}													\
	Self(const EC &ec, const Point &G, const Integer &n, const Point &Q)	\
		: Base(ec, G, n, Q) {}												\
	Self(BufferedTransformation &bt)										\
		: Base(bt) {}
		
/// Elliptic Curve Private Key
template <class EC>
class ECPrivateKey : public ECPublicKey<EC>
{
public:
	typedef typename EC::Point Point;

	ECPrivateKey(const ECParameters<EC> &params, const Point &Q, const Integer &d)
		: ECPublicKey<EC>(params, Q), m_d(d) {}
	ECPrivateKey(const OID &oid, const Point &Q, const Integer &d)
		: ECPublicKey<EC>(oid, Q), m_d(d) {}
	ECPrivateKey(const EC &ec, const Point &G, const Integer &n, const Point &Q, const Integer &d)
		: ECPublicKey<EC>(ec, G, n, Q), m_d(d) {}
	// generate a random private key
	ECPrivateKey(RandomNumberGenerator &rng, const ECParameters<EC> &params)
		: ECPublicKey<EC>(params, Point()) {Randomize(rng);}
	ECPrivateKey(RandomNumberGenerator &rng, const OID &oid)
		: ECPublicKey<EC>(oid, Point()) {Randomize(rng);}
	ECPrivateKey(RandomNumberGenerator &rng, const EC &ec, const Point &G, const Integer &n)
		: ECPublicKey<EC>(ec, G, n, Point()) {Randomize(rng);}
	// decode private key
	ECPrivateKey(BufferedTransformation &bt);

	void DEREncode(BufferedTransformation &bt) const;

	const Integer& GetPrivateExponent() const {return m_d;}

protected:
	typedef typename EC::FieldElement FieldElement;
	void Randomize(RandomNumberGenerator &rng);
	void RawDecode(BERSequenceDecoder &bt, bool needParameters);

	Integer m_d;
};

#define EC_PRIVATE_KEY_CONSTRUCTORS(Self, Base)								\
	Self(const ECPrivateKey<EC> &key)										\
		: Base(key) {}														\
	Self(const ECParameters<EC> &params, const Point &Q, const Integer &d)	\
		: Base(params, Q, d) {}												\
	Self(const OID& oid, const Point &Q, const Integer &d)					\
		: Base(oid, Q, d) {}												\
	Self(const EC &ec, const Point &G, const Integer &n, const Point &Q, const Integer &d)	\
		: Base(ec, G, n, Q, d) {}											\
	Self(RandomNumberGenerator &rng, const ECParameters<EC> &params)		\
		: Base(rng, params) {}												\
	Self(RandomNumberGenerator &rng, const OID& oid)						\
		: Base(rng, oid) {}													\
	Self(RandomNumberGenerator &rng, const EC &ec, const Point &G, const Integer &n)	\
		: Base(rng, ec, G, n) {}											\
	Self(BufferedTransformation &bt)										\
		: Base(bt) {}

/// Elliptic Curve Digest Signature Verifier
template <class EC, ECSignatureScheme SS = ECNR>
class ECDigestVerifier : public ECPublicKey<EC>, public PK_WithPrecomputation<DigestVerifier>
{
public:
	typedef typename EC::Point Point;

	EC_PUBLIC_KEY_CONSTRUCTORS(ECDigestVerifier, ECPublicKey<EC>)
	
	bool VerifyDigest(const byte *digest, unsigned int digestLen, const byte *signature) const;

	unsigned int MaxDigestLength() const {return 0xffff;}
	unsigned int DigestSignatureLength() const {return 2*ExponentLength();}

	// exposed for validation testing
	bool RawVerify(const Integer &e, const Integer &n, const Integer &s) const;
};

/// Elliptic Curve Digest Signer
template <class EC, ECSignatureScheme SS = ECNR>
class ECDigestSigner : public ECPrivateKey<EC>, public PK_WithPrecomputation<DigestSigner>
{
public:
	typedef typename EC::Point Point;

	EC_PRIVATE_KEY_CONSTRUCTORS(ECDigestSigner, ECPrivateKey<EC>)

	void SignDigest(RandomNumberGenerator &, const byte *digest, unsigned int digestLen, byte *signature) const;

	unsigned int MaxDigestLength() const {return 0xffff;}
	unsigned int DigestSignatureLength() const {return 2*ExponentLength();}

	/// exposed for validation testing
	void RawSign(const Integer &k, const Integer &e, Integer &n, Integer &s) const;
};

/// Elliptic Curve Message Signer
template <class EC, class H, ECSignatureScheme SS = ECNR>
class ECSigner : public SignerTemplate<ECDigestSigner<EC, SS>, H>, public PK_WithPrecomputation<PK_Signer>
{
	typedef ECDigestSigner<EC, SS> Base;
public:
	typedef typename EC::Point Point;

	EC_PRIVATE_KEY_CONSTRUCTORS(ECSigner, Base)
};

/// Elliptic Curve Message Signature Verifier
template <class EC, class H, ECSignatureScheme SS = ECNR>
class ECVerifier : public VerifierTemplate<ECDigestVerifier<EC, SS>, H>, public PK_WithPrecomputation<PK_Verifier>
{
	typedef ECDigestVerifier<EC, SS> Base;
public:
	typedef typename EC::Point Point;
	
	EC_PUBLIC_KEY_CONSTRUCTORS(ECVerifier, Base)
};

/// Elliptic Curve ECIES, AKA <a href="http://www.weidai.com/scan-mirror/ca.html#EC-DHAES">EC-DHAES</a>
template <class EC, class MAC = HMAC<SHA>, class KDF = P1363_KDF2<SHA> >
class ECEncryptor : public ECPublicKey<EC>, public PK_WithPrecomputation<PK_Encryptor>
{
public:
	typedef typename EC::Point Point;
	
	EC_PUBLIC_KEY_CONSTRUCTORS(ECEncryptor, ECPublicKey<EC>)

	unsigned int MaxPlainTextLength(unsigned int cipherTextLength) const
		{return cipherTextLength < CipherTextLength(0) ? 0 : cipherTextLength - CipherTextLength(0);}
	unsigned int CipherTextLength(unsigned int plainTextLength) const
		{return plainTextLength + MAC::DIGESTSIZE + EncodedPointSize();}

	void Encrypt(RandomNumberGenerator &rng, const byte *plainText, unsigned int plainTextLength, byte *cipherText)
	{
		Integer x(rng, 1, m_n-1);
		Point Q = m_Gpc.Multiply(x);

		EncodePoint(cipherText, Q);
		cipherText += EncodedPointSize();

		SecByteBlock agreedSecret(FieldElementLength());
		Point Q1 = m_Qpc.Multiply(x);
		Q1.x.Encode(agreedSecret, agreedSecret.size);

		SecByteBlock derivedKey(plainTextLength + MAC::DEFAULT_KEYLENGTH);
		KDF::DeriveKey(derivedKey, derivedKey.size, agreedSecret, agreedSecret.size);
		xorbuf(cipherText, plainText, derivedKey, plainTextLength);

		MAC mac(derivedKey + plainTextLength);
		mac.CalculateDigest(cipherText + plainTextLength, cipherText, plainTextLength);
	}
};

/// Elliptic Curve ECIES, AKA <a href="http://www.weidai.com/scan-mirror/ca.html#/// Elliptic Curve ECIES, AKA <a href="http://www.weidai.com/scan-mirror/ca.html#EC-DHAES">EC-DHAES</a>
template <class EC, class MAC = HMAC<SHA>, class KDF = P1363_KDF2<SHA> >
class ECDecryptor : public ECPrivateKey<EC>, public PK_Decryptor
{
public:
	typedef typename EC::Point Point;
	
	EC_PRIVATE_KEY_CONSTRUCTORS(ECDecryptor, ECPrivateKey<EC>)

	unsigned int MaxPlainTextLength(unsigned int cipherTextLength) const
		{return cipherTextLength < CipherTextLength(0) ? 0 : cipherTextLength - CipherTextLength(0);}
	unsigned int CipherTextLength(unsigned int plainTextLength) const
		{return plainTextLength + MAC::DIGESTSIZE + EncodedPointSize();}

	unsigned int Decrypt(const byte *cipherText, unsigned int cipherTextLength, byte *plainText)
	{
		Point Q;
		if (!GetCurve().DecodePoint(Q, cipherText, EncodedPointSize()) || !GetCurve().VerifyPoint(Q) || Q.identity)
			return 0;
		cipherText += EncodedPointSize();

		const Integer e[2] = {m_n, m_d};
		Point R[2];
		GetCurve().SimultaneousMultiply(R, Q, e, 2);

		if (!R[0].identity || R[1].identity)
			return 0;

		SecByteBlock agreedSecret(FieldElementLength());
		R[1].x.Encode(agreedSecret, agreedSecret.size);

		unsigned int plainTextLength = MaxPlainTextLength(cipherTextLength);
		SecByteBlock derivedKey(plainTextLength + MAC::DEFAULT_KEYLENGTH);
		KDF::DeriveKey(derivedKey, derivedKey.size, agreedSecret, agreedSecret.size);
		
		MAC mac(derivedKey + plainTextLength);
		if (!mac.VerifyDigest(cipherText + plainTextLength, cipherText, plainTextLength))
			return 0;

		xorbuf(plainText, cipherText, derivedKey, plainTextLength);
		return plainTextLength;
	}
};

NAMESPACE_END

#endif
