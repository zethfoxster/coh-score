#ifndef CRYPTOPP_EC2N_H
#define CRYPTOPP_EC2N_H

#include "gf2n.h"
#include "eprecomp.h"
#include "smartptr.h"

NAMESPACE_BEGIN(CryptoPP)

//! Elliptic Curve Point
struct EC2NPoint
{
	EC2NPoint() : identity(true) {}
	EC2NPoint(const PolynomialMod2 &x, const PolynomialMod2 &y)
		: identity(false), x(x), y(y) {}

	bool operator==(const EC2NPoint &t) const
		{return (identity && t.identity) || (!identity && !t.identity && x==t.x && y==t.y);}
	bool operator< (const EC2NPoint &t) const
		{return identity ? !t.identity : (!t.identity && (x<t.x || (x==t.x && y<t.y)));}

	bool identity;
	PolynomialMod2 x, y;
};

//! Elliptic Curve over GF(2^n)
class EC2N : public AbstractGroup<EC2NPoint>
{
public:
	typedef GF2NP Field;
	typedef Field::Element FieldElement;
	typedef EC2NPoint Point;

	EC2N(const Field &field, const Field::Element &a, const Field::Element &b)
		: m_field(field), m_a(a), m_b(b) {}
	// construct from BER encoded parameters
	// this constructor will decode and extract the the fields fieldID and curve of the sequence ECParameters
	EC2N(BufferedTransformation &bt);

	// encode the fields fieldID and curve of the sequence ECParameters
	void DEREncode(BufferedTransformation &bt) const;

	bool Equal(const Point &P, const Point &Q) const;
	const Point& Zero() const;
	const Point& Inverse(const Point &P) const;
	bool InversionIsFast() const {return true;}
	const Point& Add(const Point &P, const Point &Q) const;
	const Point& Double(const Point &P) const;

	Point Multiply(const Integer &k, const Point &P) const
		{return ScalarMultiply(P, k);}
	Point CascadeMultiply(const Integer &k1, const Point &P, const Integer &k2, const Point &Q) const
		{return CascadeScalarMultiply(P, k1, Q, k2);}

	bool ValidateParameters(RandomNumberGenerator &rng) const;
	bool VerifyPoint(const Point &P) const;

	unsigned int EncodedPointSize(bool compressed = false) const
		{return 1 + (compressed?1:2)*m_field->MaxElementByteLength();}
	// returns false if point is compressed and not valid (doesn't check if uncompressed)
	bool DecodePoint(Point &P, BufferedTransformation &bt, unsigned int len) const;
	bool DecodePoint(Point &P, const byte *encodedPoint, unsigned int len) const;
	void EncodePoint(byte *encodedPoint, const Point &P, bool compressed = false) const;

	Point BERDecodePoint(BufferedTransformation &bt) const;
	void DEREncodePoint(BufferedTransformation &bt, const Point &P, bool compressed = false) const;

	Integer FieldSize() const {return Integer::Power2(m_field->MaxElementBitLength());}
	const Field & GetField() const {return *m_field;}
	const FieldElement & GetA() const {return m_a;}
	const FieldElement & GetB() const {return m_b;}

private:
	clonable_ptr<Field> m_field;
	FieldElement m_a, m_b;
	mutable Point m_R;
};

template <class T> class EcPrecomputation;

//! .
template<> class EcPrecomputation<EC2N>
{
public:
	EcPrecomputation() : m_ec(NULL) {}
	EcPrecomputation(const EcPrecomputation &a)
		{operator=(a);}
	EcPrecomputation(const EC2N &ec, const EC2N::Point &base)
		{SetCurveAndBase(ec, base);}

	EcPrecomputation& operator=(const EcPrecomputation &rhs);

	void SetCurveAndBase(const EC2N &ec, const EC2N::Point &base);
	void Precompute(unsigned int maxExpBits, unsigned int storage);
	void Load(BufferedTransformation &storedPrecomputation);
	void Save(BufferedTransformation &storedPrecomputation) const;

	EC2N::Point Multiply(const Integer &exponent) const;
	EC2N::Point CascadeMultiply(const Integer &exponent, const EcPrecomputation<EC2N> &pc2, const Integer &exponent2) const;

private:
	value_ptr<EC2N> m_ec;
	ExponentiationPrecomputation<EC2N::Point> m_ep;
};

NAMESPACE_END

#endif
