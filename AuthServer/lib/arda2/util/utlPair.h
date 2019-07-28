/*****************************************************************************
	created:	2002/04/03
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Ryan M. Prescott
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_utlPair_h
#define   INCLUDED_utlPair_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template <typename T> class utlPair
{
public:
	utlPair(){};
	utlPair(const T& first, const T& second){m_First=first; m_Second=second;}
	
	utlPair<T> operator+(const corPair<T>& tup) const;
	utlPair<T> operator-(const corPair<T>& tup) const;
	utlPair<T> operator/(const T& val) const;
	utlPair<T> operator%(const T& val) const;
	bool		operator==(const corPair<T>& val) const;
	T m_First;
	T m_Second;
};

template <typename T> utlPair<T> 
	corPair<T>::operator+(const corPair<T>& tup) const
{
	corPair<T> temp;
	temp.m_First = m_First + tup.m_First;
	temp.m_Second= m_Second+ tup.m_Second;
	return temp;
}

template <typename T> bool
	utlPair<T>::operator==(const utlPair<T>& val) const
{
	return ((m_First == val.m_First) && (m_Second == val.m_Second));
}

template <typename T> utlPair<T> 
	utlPair<T>::operator-(const utlPair<T>& tup) const
{
	utlPair<T> temp;
	temp.m_First = m_First - tup.m_First;
	temp.m_Second= m_Second- tup.m_Second;
	return temp;
}

template <typename T> utlPair<T>
	utlPair<T>::operator/(const T& val) const
{
	utlPair<T> temp;
	temp.m_First = m_First  / val;
	temp.m_Second= m_Second / val;
	return temp;
}

template <typename T> utlPair<T> 
	utlPair<T>::operator%(const T& val) const
{
	utlPair<T> temp;
	temp.m_First = m_First % val;
	temp.m_Second= m_Second% val;
	return temp;
}



#endif // INCLUDED_utlPair_h