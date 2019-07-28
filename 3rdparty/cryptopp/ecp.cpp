// ecp.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "ecp.h"
#include "asn.h"
#include "nbtheory.h"

#include "algebra.cpp"
#include "eprecomp.cpp"

NAMESPACE_BEGIN(CryptoPP)

ANONYMOUS_NAMESPACE_BEGIN
static inline ECP::Point ToMontgomery(const MontgomeryRepresentation &mr, const ECP::Point &P)
{
	return P.identity ? P : ECP::Point(mr.ConvertIn(P.x), mr.ConvertIn(P.y));
}

static inline ECP::Point FromMontgomery(const MontgomeryRepresentation &mr, const ECP::Point &P)
{
	return P.identity ? P : ECP::Point(mr.ConvertOut(P.x), mr.ConvertOut(P.y));
}
NAMESPACE_END

ECP::ECP(BufferedTransformation &bt)
	: m_fieldPtr(new Field(bt)), m_field(*m_fieldPtr)
{
	BERSequenceDecoder seq(bt);
	m_field.BERDecodeElement(seq, m_a);
	m_field.BERDecodeElement(seq, m_b);
	// skip optional seed
	if (!seq.EndReached())
		BERDecodeOctetString(seq, g_bitBucket);
	seq.MessageEnd();
}

void ECP::DEREncode(BufferedTransformation &bt) const
{
	m_field.DEREncode(bt);
	DERSequenceEncoder seq(bt);
	m_field.DEREncodeElement(seq, m_a);
	m_field.DEREncodeElement(seq, m_b);
	seq.MessageEnd();
}

bool ECP::DecodePoint(ECP::Point &P, const byte *encodedPoint, unsigned int encodedPointLen) const
{
	StringStore store(encodedPoint, encodedPointLen);
	return DecodePoint(P, store, encodedPointLen);
}

bool ECP::DecodePoint(ECP::Point &P, BufferedTransformation &bt, unsigned int encodedPointLen) const
{
	byte type;
	if (encodedPointLen < 1 || !bt.Get(type))
		return false;

	switch (type)
	{
	case 0:
		P.identity = true;
		return true;
	case 2:
	case 3:
	{
		if (encodedPointLen != EncodedPointSize(true))
			return false;

		Integer p = FieldSize();

		P.identity = false;
		P.x.Decode(bt, m_field.MaxElementByteLength()); 
		P.y = ((P.x*P.x+m_a)*P.x+m_b) % p;

		if (Jacobi(P.y, p) !=1)
			return false;

		P.y = ModularSquareRoot(P.y, p);

		if ((type & 1) != P.y.GetBit(0))
			P.y = p-P.y;

		return true;
	}
	case 4:
	{
		if (encodedPointLen != EncodedPointSize(false))
			return false;

		unsigned int len = m_field.MaxElementByteLength();
		P.identity = false;
		P.x.Decode(bt, len);
		P.y.Decode(bt, len);
		return true;
	}
	default:
		return false;
	}
}

void ECP::EncodePoint(byte *encodedPoint, const Point &P, bool compressed) const
{
	if (P.identity)
		memset(encodedPoint, 0, EncodedPointSize(compressed));
	else if (compressed)
	{
		encodedPoint[0] = 2 + P.y.GetBit(0);
		P.x.Encode(encodedPoint+1, m_field.MaxElementByteLength());
	}
	else
	{
		unsigned int len = m_field.MaxElementByteLength();
		encodedPoint[0] = 4;	// uncompressed
		P.x.Encode(encodedPoint+1, len);
		P.y.Encode(encodedPoint+1+len, len);
	}
}

ECP::Point ECP::BERDecodePoint(BufferedTransformation &bt) const
{
	SecByteBlock str;
	BERDecodeOctetString(bt, str);
	Point P;
	if (!DecodePoint(P, str, str.size))
		BERDecodeError();
	return P;
}

void ECP::DEREncodePoint(BufferedTransformation &bt, const Point &P, bool compressed) const
{
	SecByteBlock str(EncodedPointSize(compressed));
	EncodePoint(str, P, compressed);
	DEREncodeOctetString(bt, str);
}

bool ECP::ValidateParameters(RandomNumberGenerator &rng) const
{
	Integer p = FieldSize();
	return p.IsOdd() && VerifyPrime(rng, p)
		&& !m_a.IsNegative() && m_a<p && !m_b.IsNegative() && m_b<p
		&& ((4*m_a*m_a*m_a+27*m_b*m_b)%p).IsPositive();
}

bool ECP::VerifyPoint(const Point &P) const
{
	const FieldElement &x = P.x, &y = P.y;
	Integer p = FieldSize();
	return P.identity ||
		(!x.IsNegative() && x<p && !y.IsNegative() && y<p
		&& !(((x*x+m_a)*x+m_b-y*y)%p));
}

bool ECP::Equal(const Point &P, const Point &Q) const
{
	if (P.identity && Q.identity)
		return true;

	if (P.identity && !Q.identity)
		return false;

	if (!P.identity && Q.identity)
		return false;

	return (m_field.Equal(P.x,Q.x) && m_field.Equal(P.y,Q.y));
}

const ECP::Point& ECP::Zero() const
{
	static const Point zero;
	return zero;
}

const ECP::Point& ECP::Inverse(const Point &P) const
{
	if (P.identity)
		return P;
	else
	{
		m_R.identity = false;
		m_R.x = P.x;
		m_R.y = m_field.Inverse(P.y);
		return m_R;
	}
}

const ECP::Point& ECP::Add(const Point &P, const Point &Q) const
{
	if (P.identity) return Q;
	if (Q.identity) return P;
	if (m_field.Equal(P.x, Q.x))
		return m_field.Equal(P.y, Q.y) ? Double(P) : Zero();

	FieldElement t = m_field.Subtract(Q.y, P.y);
	t = m_field.Divide(t, m_field.Subtract(Q.x, P.x));
	FieldElement x = m_field.Subtract(m_field.Subtract(m_field.Square(t), P.x), Q.x);
	m_R.y = m_field.Subtract(m_field.Multiply(t, m_field.Subtract(P.x, x)), P.y);

	m_R.x.swap(x);
	m_R.identity = false;
	return m_R;
}

const ECP::Point& ECP::Double(const Point &P) const
{
	if (P.identity || P.y==m_field.Zero()) return Zero();

	FieldElement t = m_field.Square(P.x);
	t = m_field.Add(m_field.Add(m_field.Double(t), t), m_a);
	t = m_field.Divide(t, m_field.Double(P.y));
	FieldElement x = m_field.Subtract(m_field.Subtract(m_field.Square(t), P.x), P.x);
	m_R.y = m_field.Subtract(m_field.Multiply(t, m_field.Subtract(P.x, x)), P.y);

	m_R.x.swap(x);
	m_R.identity = false;
	return m_R;
}

template <class T, class Iterator> void ParallelInvert(const AbstractRing<T> &ring, Iterator begin, Iterator end)
{
	unsigned int n = end-begin;
	if (n == 1)
		*begin = ring.MultiplicativeInverse(*begin);
	else if (n > 1)
	{
		std::vector<T> vec((n+1)/2);
		unsigned int i;
		Iterator it;

		for (i=0, it=begin; i<n/2; i++, it+=2)
			vec[i] = ring.Multiply(*it, *(it+1));
		if (n%2 == 1)
			vec[n/2] = *it;

		ParallelInvert(ring, vec.begin(), vec.end());

		for (i=0, it=begin; i<n/2; i++, it+=2)
		{
			if (!vec[i])
			{
				*it = ring.MultiplicativeInverse(*it);
				*(it+1) = ring.MultiplicativeInverse(*(it+1));
			}
			else
			{
				std::swap(*it, *(it+1));
				*it = ring.Multiply(*it, vec[i]);
				*(it+1) = ring.Multiply(*(it+1), vec[i]);
			}
		}
		if (n%2 == 1)
			*it = vec[n/2];
	}
}

struct ProjectivePoint
{
	ProjectivePoint() {}
	ProjectivePoint(const Integer &x, const Integer &y, const Integer &z)
		: x(x), y(y), z(z)	{}

	Integer x,y,z;
};

class ProjectiveDoubling
{
public:
	ProjectiveDoubling(const ModularArithmetic &mr, const Integer &m_a, const Integer &m_b, const ECPPoint &Q)
		: mr(mr), firstDoubling(true), negated(false)
	{
		if (Q.identity)
		{
			sixteenY4 = P.x = P.y = mr.One();
			aZ4 = P.z = mr.Zero();
		}
		else
		{
			P.x = Q.x;
			P.y = Q.y;
			sixteenY4 = P.z = mr.One();
			aZ4 = m_a;
		}
	}

	void Double()
	{
		twoY = mr.Double(P.y);
		P.z = mr.Multiply(P.z, twoY);
		fourY2 = mr.Square(twoY);
		S = mr.Multiply(fourY2, P.x);
		aZ4 = mr.Multiply(aZ4, sixteenY4);
		M = mr.Square(P.x);
		M = mr.Add(mr.Add(mr.Double(M), M), aZ4);
		P.x = mr.Square(M);
		mr.Reduce(P.x, S);
		mr.Reduce(P.x, S);
		mr.Reduce(S, P.x);
		P.y = mr.Multiply(M, S);
		sixteenY4 = mr.Square(fourY2);
		mr.Reduce(P.y, mr.Half(sixteenY4));
	}

	const ModularArithmetic &mr;
	ProjectivePoint P;
	bool firstDoubling, negated;
	Integer sixteenY4, aZ4, twoY, fourY2, S, M;
};

struct ZIterator
{
	ZIterator() {}
	ZIterator(std::vector<ProjectivePoint>::iterator it) : it(it) {}
	Integer& operator*() {return it->z;}
	int operator-(ZIterator it2) {return it-it2.it;}
	ZIterator operator+(int i) {return ZIterator(it+i);}
	ZIterator& operator+=(int i) {it+=i; return *this;}
	std::vector<ProjectivePoint>::iterator it;
};

ECP::Point ECP::ScalarMultiply(const Point &P, const Integer &k) const
{
	Element result;
	if (k.BitCount() <= 5)
		AbstractGroup<ECPPoint>::SimultaneousMultiply(&result, P, &k, 1);
	else
		ECP::SimultaneousMultiply(&result, P, &k, 1);
	return result;
}

void ECP::SimultaneousMultiply(ECP::Point *results, const ECP::Point &P, const Integer *expBegin, unsigned int expCount) const
{
	if (m_fieldPtr.get())
	{
		MontgomeryRepresentation mr(m_field.GetModulus());
		ECP ecpmr(mr, mr.ConvertIn(m_a), mr.ConvertIn(m_b));
		ecpmr.SimultaneousMultiply(results, ToMontgomery(mr, P), expBegin, expCount);
		for (unsigned int i=0; i<expCount; i++)
			results[i] = FromMontgomery(mr, results[i]);
		return;
	}

	ProjectiveDoubling rd(m_field, m_a, m_b, P);
	std::vector<ProjectivePoint> bases;
	std::vector<WindowSlider> exponents;
	exponents.reserve(expCount);
	std::vector<std::vector<unsigned int> > baseIndices(expCount);
	std::vector<std::vector<bool> > negateBase(expCount);
	std::vector<std::vector<unsigned int> > exponentWindows(expCount);
	unsigned int i;

	for (i=0; i<expCount; i++)
	{
		assert(expBegin->NotNegative());
		exponents.push_back(WindowSlider(*expBegin++, InversionIsFast(), 5));
		exponents[i].FindNextWindow();
	}

	unsigned int expBitPosition = 0;
	bool notDone = true;

	while (notDone)
	{
		notDone = false;
		bool baseAdded = false;
		for (i=0; i<expCount; i++)
		{
			if (!exponents[i].finished && expBitPosition == exponents[i].windowBegin)
			{
				if (!baseAdded)
				{
					bases.push_back(rd.P);
					baseAdded =true;
				}

				exponentWindows[i].push_back(exponents[i].expWindow);
				baseIndices[i].push_back(bases.size()-1);
				negateBase[i].push_back(exponents[i].negateNext);

				exponents[i].FindNextWindow();
			}
			notDone = notDone || !exponents[i].finished;
		}

		if (notDone)
		{
			rd.Double();
			expBitPosition++;
		}
	}

	// convert from projective to affine coordinates
	ParallelInvert(m_field, ZIterator(bases.begin()), ZIterator(bases.end()));
	for (i=0; i<bases.size(); i++)
	{
		if (bases[i].z.NotZero())
		{
			bases[i].y = m_field.Multiply(bases[i].y, bases[i].z);
			bases[i].z = m_field.Square(bases[i].z);
			bases[i].x = m_field.Multiply(bases[i].x, bases[i].z);
			bases[i].y = m_field.Multiply(bases[i].y, bases[i].z);
		}
	}

	std::vector<BaseAndExponent<Point, word> > finalCascade;
	for (i=0; i<expCount; i++)
	{
		finalCascade.resize(baseIndices[i].size());
		for (unsigned int j=0; j<baseIndices[i].size(); j++)
		{
			ProjectivePoint &base = bases[baseIndices[i][j]];
			if (base.z.IsZero())
				finalCascade[j].base.identity = true;
			else
			{
				finalCascade[j].base.identity = false;
				finalCascade[j].base.x = base.x;
				if (negateBase[i][j])
					finalCascade[j].base.y = m_field.Inverse(base.y);
				else
					finalCascade[j].base.y = base.y;
			}
			finalCascade[j].exponent = exponentWindows[i][j];
		}
		results[i] = GeneralCascadeMultiplication(*this, finalCascade.begin(), finalCascade.end());
	}
}

ECP::Point ECP::CascadeScalarMultiply(const Point &P, const Integer &k1, const Point &Q, const Integer &k2) const
{
	if (m_fieldPtr.get())
	{
		MontgomeryRepresentation mr(m_field.GetModulus());
		ECP ecpmr(mr, mr.ConvertIn(m_a), mr.ConvertIn(m_b));
		return FromMontgomery(mr, ecpmr.CascadeScalarMultiply(ToMontgomery(mr, P), k1, ToMontgomery(mr, Q), k2));
	}
	else
		return AbstractGroup<Point>::CascadeScalarMultiply(P, k1, Q, k2);
}

// ********************************************************

EcPrecomputation<ECP>& EcPrecomputation<ECP>::operator=(const EcPrecomputation<ECP> &rhs)
{
	m_mr = rhs.m_mr;
	m_ec.reset(new ECP(*m_mr, rhs.m_ec->GetA(), rhs.m_ec->GetB()));
	m_ep = rhs.m_ep;
	m_ep.m_group = m_ec.get();
	return *this;
}

void EcPrecomputation<ECP>::SetCurveAndBase(const ECP &ec, const ECP::Point &base)
{
	m_mr.reset(new MontgomeryRepresentation(ec.GetField().GetModulus()));
	m_ec.reset(new ECP(*m_mr, m_mr->ConvertIn(ec.GetA()), m_mr->ConvertIn(ec.GetB())));
	m_ep.SetGroupAndBase(*m_ec, ToMontgomery(*m_mr, base));
}

void EcPrecomputation<ECP>::Precompute(unsigned int maxExpBits, unsigned int storage)
{
	m_ep.Precompute(maxExpBits, storage);
}

void EcPrecomputation<ECP>::Load(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	word32 version;
	BERDecodeUnsigned<word32>(seq, version, INTEGER, 1, 1);
	m_ep.m_exponentBase.BERDecode(seq);
	m_ep.m_windowSize = m_ep.m_exponentBase.BitCount() - 1;
	m_ep.m_bases.clear();
	while (!seq.EndReached())
		m_ep.m_bases.push_back(m_ec->BERDecodePoint(seq));
	seq.MessageEnd();
}

void EcPrecomputation<ECP>::Save(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	DEREncodeUnsigned<word32>(seq, 1);	// version
	m_ep.m_exponentBase.DEREncode(seq);
	for (unsigned i=0; i<m_ep.m_bases.size(); i++)
		m_ec->DEREncodePoint(seq, m_ep.m_bases[i]);
	seq.MessageEnd();
}

ECP::Point EcPrecomputation<ECP>::Multiply(const Integer &exponent) const
{
	return FromMontgomery(*m_mr, m_ep.Exponentiate(exponent));
}

ECP::Point EcPrecomputation<ECP>::CascadeMultiply(const Integer &exponent, const EcPrecomputation<ECP> &pc2, const Integer &exponent2) const
{
	return FromMontgomery(*m_mr, m_ep.CascadeExponentiate(exponent, pc2.m_ep, exponent2));
}

template class AbstractGroup<ECP::Point>;

NAMESPACE_END
