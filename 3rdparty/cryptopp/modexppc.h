#ifndef CRYPTOPP_MODEXPPC_H
#define CRYPTOPP_MODEXPPC_H

#include "modarith.h"
#include "eprecomp.h"
#include "smartptr.h"

NAMESPACE_BEGIN(CryptoPP)

//! .
class ModExpPrecomputation
{
public:
	ModExpPrecomputation() {}
	ModExpPrecomputation(const ModExpPrecomputation &a)
		{operator=(a);}
	ModExpPrecomputation(const Integer &modulus, const Integer &base)
		{SetModulusAndBase(modulus, base);}

	ModExpPrecomputation& operator=(const ModExpPrecomputation &rhs);

	void SetModulusAndBase(const Integer &modulus, const Integer &base);
	void Precompute(unsigned int maxExpBits, unsigned int storage);
	void Load(BufferedTransformation &storedPrecomputation);
	void Save(BufferedTransformation &storedPrecomputation) const;

	Integer Exponentiate(const Integer &exponent) const;
	Integer CascadeExponentiate(const Integer &exponent, const ModExpPrecomputation &pc2, const Integer &exponent2) const;

private:
	value_ptr<MontgomeryRepresentation> m_mr;
	ExponentiationPrecomputation<Integer> m_ep;
};

NAMESPACE_END

#endif
