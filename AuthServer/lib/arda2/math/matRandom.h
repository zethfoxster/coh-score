/*****************************************************************************
	created:	2002/05/23
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_matRandom_h
#define   INCLUDED_matRandom_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class matRandom
{
public:
	matRandom();
	~matRandom() {};
	void Seed( uint32 nSeed );

	float	Unary();	// random value in range [0..1]
	float	Uniform( float fMax );
	float	Uniform( float fMin, float fMax );
	float	Gaussian( float fMu, float fSigma );

	int		Range( int iMax );				// [0, Max)
	int		Range( int iMin, int iMax );	// [Min, Max)
	uint32 Generate();

	// default singleton
	static matRandom	Random;

protected:

private:
	static const int N = 624;			// length of state	vector
	static const int M = 397;			// a period parameter

	uint32 m_state[N];				// the array for the state vector
	uint32 *m_pNext;
	int32	m_nLeft;
};


#endif // INCLUDED_matRandom_h


