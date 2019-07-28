/*****************************************************************************
	created:	2001/08/14
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Integer Size class which wraps the windows SIZE
				structure. This is based loosely on the code in
				afxwin.h and afxwin1.inline; see the MFC docs for
				instructions on using these.
*****************************************************************************/

#ifndef   INCLUDED_utlSize
#define   INCLUDED_utlSize
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// class CSize declaration
//////////////////////////////////////////////////////////////////////////////

class utlSize : public tagSIZE
{
public:

	// Constructors
	utlSize()
	{ /* random filled */ }

	utlSize(int initCX, int initCY)
	{ cx = initCX; cy = initCY; }

	utlSize(const SIZE &initSize)
	{ *(SIZE*)this = initSize; }

	utlSize(const POINT &initPt)
	{ *(POINT*)this = initPt; }

	utlSize(const utlSize& rhs)
	{ cx = rhs.cx; cy = rhs.cy; }

#if CORE_SYSTEM_WINAPI
	explicit utlSize(DWORD dwSize)
	{
		cx = (short)LOWORD(dwSize);
		cy = (short)HIWORD(dwSize);
	}
#endif

	static const utlSize Zero;	// zero object

	// Attributes
	bool IsZero(void) const;

	// Operations
	const utlSize& operator=(const utlSize& rhs);
	void Set(int cxNew, int cyNew);
	bool operator==(const SIZE &size) const;
	bool operator!=(const SIZE &size) const;
	const utlSize& operator+=(const SIZE &size);
	const utlSize& operator-=(const SIZE &size);
	const utlSize& operator*=(int mult);
	const utlSize& operator/=(int div);

	utlSize operator-() const;
	utlSize operator*(int mult) const;
	utlSize operator/(int div) const;
};

utlSize operator+(const SIZE &s1, const SIZE &s2);
utlSize operator-(const SIZE &s1, const SIZE &s2);

// note: these are all "if and only if" conditions (for both members).
//		 so if !(size1 <= size2) that does NOT mean (size1 > size2).
bool operator <  (SIZE l, SIZE r);
bool operator <= (SIZE l, SIZE r);
bool operator >  (SIZE l, SIZE r);
bool operator >= (SIZE l, SIZE r);

inline utlSize operator%(const utlSize& sz, int32 div)
{
	utlSize temp = sz;
	temp.cx = temp.cx % div;
	temp.cy = temp.cy % div;
	return temp;
}

inline bool utlSize::IsZero() const
{ return (cx | cy) == 0; }

inline const utlSize& utlSize::operator=(const utlSize& rhs)
{ cx = rhs.cx; cy = rhs.cy; return *this; }

inline void utlSize::Set(int cxNew, int cyNew)
{ cx = cxNew; cy = cyNew; }

inline bool utlSize::operator==( const SIZE &size ) const
{ return (cx == size.cx && cy == size.cy); }

inline bool utlSize::operator!=( const SIZE &size ) const
{ return (cx != size.cx || cy != size.cy); }

inline const utlSize& utlSize::operator+=( const SIZE &size )
{ cx += size.cx; cy += size.cy; return *this; }

inline const utlSize& utlSize::operator-=( const SIZE &size )
{ cx -= size.cx; cy -= size.cy; return *this; }

inline const utlSize& utlSize::operator*=( int mult )
{ cx *= mult; cy *= mult; return *this; }

inline const utlSize& utlSize::operator/=( int div )
{ cx /= div; cy /= div; return *this; }

inline utlSize utlSize::operator-() const
{ return utlSize(-cx, -cy); }

inline utlSize utlSize::operator*( int mult ) const
{ return utlSize(cx * mult, cy * mult); }

inline utlSize utlSize::operator/( int div ) const
{ return utlSize(cx / div, cy / div); }

inline utlSize operator+(const SIZE &s1, const SIZE &s2)
{ return utlSize(s1.cx + s2.cx, s1.cy + s2.cy); }

inline utlSize operator-(const SIZE &s1, const SIZE &s2)
{ return utlSize(s1.cx - s2.cx, s1.cy - s2.cy); }

inline bool operator < (SIZE l, SIZE r)
{  return ((l.cx < r.cx) && (l.cy < r.cy));  }

inline bool operator <= (SIZE l, SIZE r)
{  return ((l.cx <= r.cx) && (l.cy <= r.cy));  }

inline bool operator > (SIZE l, SIZE r)
{  return ((l.cx > r.cx) && (l.cy > r.cy));  }

inline bool operator >= (SIZE l, SIZE r)
{  return ((l.cx >= r.cx) && (l.cy >= r.cy));  }


#endif // INCLUDED_utlSize

