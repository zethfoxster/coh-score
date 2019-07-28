/*****************************************************************************
	created:	2002/05/23
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matRandom.h"


// default singleton generator
matRandom matRandom::Random;


// This is the "Mersenne Twister" random number generator MT19937, which
// generates pseudorandom integers uniformly distributed in 0..(2^32 - 1)
// starting from any odd seed in 0..(2^32 - 1).  This C++ version is
// a class wrapper based by Peter Freese.
//

#include <stdio.h>
#include <stdlib.h>

/* Period parameters */  
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static uint32 mag01[2] =
{
	0x0, 
	MATRIX_A
};
/* mag01[x] = x * MATRIX_A  for x=0,1 */



matRandom::matRandom() :
	m_pNext(NULL),
	m_nLeft(0)
{
	Seed(4357);

//	for( int j=0; j<2002; j++)
//		printf(" %10u%s", Generate(), (j%7)==6 ? "\n" : "");


}


void matRandom::Seed( uint32 seed )
{
	//
	// We initialize m_state[0..(N-1)] via the generator
	//
	//   x_new = (69069 * x_old) mod 2^32
	//
	// from Line 15 of Table 1, p. 106, Sec. 3.3.4 of Knuth's
	// _The Art of Computer Programming_, Volume 2, 3rd ed.
	//
	// Notes (SJC): I do not know what the initial state requirements
	// of the Mersenne Twister are, but it seems this seeding generator
	// could be better.  It achieves the maximum period for its modulus
	// (2^30) iff x_initial is odd (p. 20-21, Sec. 3.2.1.2, Knuth); if
	// x_initial can be even, you have sequences like 0, 0, 0, ...;
	// 2^31, 2^31, 2^31, ...; 2^30, 2^30, 2^30, ...; 2^29, 2^29 + 2^31,
	// 2^29, 2^29 + 2^31, ..., etc. so I force seed to be odd below.
	//
	// Even if x_initial is odd, if x_initial is 1 mod 4 then
	//
	//   the          lowest bit of x is always 1,
	//   the  next-to-lowest bit of x is always 0,
	//   the 2nd-from-lowest bit of x alternates      ... 0 1 0 1 0 1 0 1 ... ,
	//   the 3rd-from-lowest bit of x 4-cycles        ... 0 1 1 0 0 1 1 0 ... ,
	//   the 4th-from-lowest bit of x has the 8-cycle ... 0 0 0 1 1 1 1 0 ... ,
	//    ...
	//
	// and if x_initial is 3 mod 4 then
	//
	//   the          lowest bit of x is always 1,
	//   the  next-to-lowest bit of x is always 1,
	//   the 2nd-from-lowest bit of x alternates      ... 0 1 0 1 0 1 0 1 ... ,
	//   the 3rd-from-lowest bit of x 4-cycles        ... 0 0 1 1 0 0 1 1 ... ,
	//   the 4th-from-lowest bit of x has the 8-cycle ... 0 0 1 1 1 1 0 0 ... ,
	//    ...
	//
	// The generator's potency (min. s>=0 with (69069-1)^s = 0 mod 2^32) is
	// 16, which seems to be alright by p. 25, Sec. 3.2.1.3 of Knuth.  It
	// also does well in the dimension 2..5 spectral tests, but it could be
	// better in dimension 6 (Line 15, Table 1, p. 106, Sec. 3.3.4, Knuth).
	//
	// Note that the random number user does not see the values generated
	// here directly since reloadMT() will always munge them first, so maybe
	// none of all of this matters.  In fact, the seed values made here could
	// even be extra-special desirable if the Mersenne Twister theory says
	// so-- that's why the only change I made is to restrict to odd seeds.
	//

	uint32 x = seed | 1;

	uint32 *s = m_state;
	for ( int i = N; i != 0; --i )
	{
		*s++ = x;
		x *= 69069;
	}
	m_nLeft = 0;
}


uint32 matRandom::Generate()
{
	unsigned long y;
	if ( m_nLeft == 0 )
	{
		/* generate N words at one time */
		int kk;
		for ( kk = 0; kk < N-M; kk++ )
		{
			y = (m_state[kk]&UPPER_MASK) | (m_state[kk+1]&LOWER_MASK);
			m_state[kk] = m_state[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		for (;kk<N-1;kk++) {
			y = (m_state[kk]&UPPER_MASK)|(m_state[kk+1]&LOWER_MASK);
			m_state[kk] = m_state[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		y = (m_state[N-1]&UPPER_MASK)|(m_state[0]&LOWER_MASK);
		m_state[N-1] = m_state[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

		m_nLeft = N;
		m_pNext = &m_state[0];
	}

	--m_nLeft;
	y = *m_pNext++;
	y ^= TEMPERING_SHIFT_U(y);
	y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
	y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
	y ^= TEMPERING_SHIFT_L(y);

	return y; 
}


float matRandom::Unary()
{
	return Generate() / (float)0xFFFFFFFF;
}


float matRandom::Uniform( float fMax )
{
	return Unary() * fMax;
}


float matRandom::Uniform( float fMin, float fMax )
{
	return fMin + Unary() * (fMax - fMin);
}


float matRandom::Gaussian( float fMu, float fSigma )
{
	// When x and y are two variables from [0, 1), uniformly
	// distributed, then
	//
	//    cos(2*pi*x)*sqrt(-2*log(1-y))
	//    sin(2*pi*x)*sqrt(-2*log(1-y))
	//
	// are two *independent* variables with normal distribution
	// (mu = 0, sigma = 1).

	float fTheta = Unary() * mat2Pi;
	float fRad = sqrtf(-2.0f * logf(1.0f - Unary()));
	float z = cosf(fTheta) * fRad;
	return fMu + z * fSigma;
}


int matRandom::Range( int iMax )
{
	return (int)((int64)Generate() * iMax >> 32);
}


int matRandom::Range( int iMin, int iMax )
{
	return iMin + (int)((int64)Generate() * (iMax - iMin) >> 32);
}


