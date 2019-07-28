/*****************************************************************************
	created:	2001/07/29
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	purpose:	This template class works similarly to a bitset, but is 
				quite a bit simpler. It allows you to base a bitmapped 
				flag register on an enum with full typesafety. You are 
				prevented from setting/clearing/testing the flags using 
				the wrong enum type.
*********************************************************************/

#ifndef   INCLUDED_utlBitFlag
#define   INCLUDED_utlBitFlag
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template <typename T, int N = 0>
class utlBitFlag
{
	typedef unsigned long _Ty;
	enum
	{
		_Nb = 8 * sizeof (_Ty),
		_Nw = N == 0 ? 0 : (N - 1) / _Nb,
	};
	_Ty A[_Nw + 1];

public:
	utlBitFlag()
	{
		ClearAll();
	}
	void Set(T p)
	{
		A[p / _Nb] |= ((_Ty)1 << p % _Nb);
	}
	void Set(T p, bool set)
	{
		set ? Set(p) : Clear(p);
	}
	void Flip(T p)
	{
		A[p / _Nb] ^= ((_Ty)1 << p % _Nb);
	}
	void Clear(T p)
	{
		A[p / _Nb] &= ~((_Ty)1 << p % _Nb);
	}
	void ClearAll()
	{ 
		for (int i = _Nw; 0 <= i; --i)
			A[i] = 0;
	}
	void SetAll()
	{ 
		for (int i = _Nw; 0 <= i; --i)
			A[i] = (_Ty)~0;
	}
	bool Test(T p) const
	{
		return (A[p / _Nb] & ((_Ty)1 << p % _Nb)) != 0;
	}
};


#endif // INCLUDED_utlBitFlag

