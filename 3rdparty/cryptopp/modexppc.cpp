// modexppc.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "modexppc.h"
#include "asn.h"

#include "algebra.cpp"
#include "eprecomp.cpp"

NAMESPACE_BEGIN(CryptoPP)

ModExpPrecomputation& ModExpPrecomputation::operator=(const ModExpPrecomputation &rhs)
{
	m_mr = rhs.m_mr;
	m_ep = rhs.m_ep;
	m_ep.m_group = &m_mr->MultiplicativeGroup();
	return *this;
}

void ModExpPrecomputation::SetModulusAndBase(const Integer &modulus, const Integer &base)
{
	m_mr.reset(new MontgomeryRepresentation(modulus));
	m_ep.SetGroupAndBase(m_mr->MultiplicativeGroup(), m_mr->ConvertIn(base));
}

void ModExpPrecomputation::Precompute(unsigned int maxExpBits, unsigned int storage)
{
	m_ep.Precompute(maxExpBits, storage);
}

void ModExpPrecomputation::Load(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	word32 version;
	BERDecodeUnsigned<word32>(seq, version, INTEGER, 1, 1);
	m_ep.m_exponentBase.BERDecode(seq);
	m_ep.m_windowSize = m_ep.m_exponentBase.BitCount() - 1;
	m_ep.m_bases.clear();
	while (!seq.EndReached())
		m_ep.m_bases.push_back(Integer(seq));
	seq.MessageEnd();
}

void ModExpPrecomputation::Save(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	DEREncodeUnsigned<word32>(seq, 1);	// version
	m_ep.m_exponentBase.DEREncode(seq);
	for (unsigned i=0; i<m_ep.m_bases.size(); i++)
		m_ep.m_bases[i].DEREncode(seq);
	seq.MessageEnd();
}

Integer ModExpPrecomputation::Exponentiate(const Integer &exponent) const
{
	return m_mr->ConvertOut(m_ep.Exponentiate(exponent));
}

Integer ModExpPrecomputation::CascadeExponentiate(const Integer &exponent, const ModExpPrecomputation &pc2, const Integer &exponent2) const
{
	return m_mr->ConvertOut(m_ep.CascadeExponentiate(exponent, pc2.m_ep, exponent2));
}

NAMESPACE_END
