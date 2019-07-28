/*****************************************************************************
	created:	2001/08/14
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Integer Point class which wraps the windows POINT
				structure. This is based loosely on the code in
				afxwin.h and afxwin1.inline; see the MFC docs for
				instructions on using these.
*****************************************************************************/

#ifndef   INCLUDED_utlPoint
#define   INCLUDED_utlPoint
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// class CPoint declaration

class utlPoint : public tagPOINT
{
public:

	// Constructors
	utlPoint()
	{ /* random filled */ }

	utlPoint(int initX, int initY)
	{ x = initX; y = initY; }

	utlPoint(POINT initPt)
	{ *(POINT*)this = initPt; }

	utlPoint(SIZE initSize)
	{ *(SIZE*)this = initSize; }

#if CORE_SYSTEM_WINAPI
	explicit utlPoint(DWORD dwPoint)
	{
		x = (short)LOWORD(dwPoint);
		y = (short)HIWORD(dwPoint);
	}
#endif

	static const utlPoint Zero;	// zero object

	// Attributes
	bool IsZero(void) const;

	// Operations
	void Offset(int xOffset, int yOffset);
	void Offset(POINT point);
	void Offset(SIZE size);
	void Set(int xNew, int yNew);
	const utlPoint& operator+=(SIZE size);
	const utlPoint& operator-=(SIZE size);
	const utlPoint& operator+=(POINT point);
	const utlPoint& operator-=(POINT point);
	const utlPoint operator/(int32 divisor) const;

	utlPoint operator-() const;
};

class utlPointCompare
{
public:
	bool operator()(const utlPoint &p1, const utlPoint &p2) const
	{
		if (p1.x < p2.x) return true;
		if (p2.x < p1.x) return false;
		return p1.y < p2.y;
	}
};


// binary operations should not be class members to allow left operand to be base class
utlPoint operator+(const POINT &p1, const POINT &p2);
utlPoint operator+(const SIZE &s1, const POINT &p2);
utlPoint operator+(const POINT &p1, const SIZE &s2);
utlPoint operator-(const POINT &p1, const POINT &p2);
utlPoint operator-(const SIZE &s1, const POINT &p2);
utlPoint operator-(const POINT &p1, const SIZE &s2);
bool operator==(const POINT &p1, const POINT &p2);
bool operator!=(const POINT &p1, const POINT &p2);


inline bool utlPoint::IsZero(void) const
{ return ( x | y ) == 0; }

inline void utlPoint::Offset(int xOffset, int yOffset)
{ x += xOffset; y += yOffset; }

inline void utlPoint::Offset(POINT point)
{ x += point.x; y += point.y; }

inline void utlPoint::Offset(SIZE size)
{ x += size.cx; y += size.cy; }

inline void utlPoint::Set(int xNew, int yNew)
{ x = xNew; y = yNew; }

inline const utlPoint utlPoint::operator/(int32 divisor) const
{ return utlPoint( x/divisor, y/divisor); }

inline const utlPoint& utlPoint::operator+=(SIZE size)
{ x += size.cx; y += size.cy; return *this; }

inline const utlPoint& utlPoint::operator-=(SIZE size)
{ x -= size.cx; y -= size.cy; return *this;  }

inline const utlPoint& utlPoint::operator+=(POINT point)
{ x += point.x; y += point.y; return *this;  }

inline const utlPoint& utlPoint::operator-=(POINT point)
{ x -= point.x; y -= point.y; return *this;  }

inline utlPoint utlPoint::operator-() const
{ return utlPoint(-x, -y); }

#if 0
inline bool operator < (POINT l, POINT r)
{  return ((l.x < r.x) && (l.y < r.y));  }

inline bool operator <= (POINT l, POINT r)
{  return ((l.x <= r.x) && (l.y <= r.y));  }

inline bool operator > (POINT l, POINT r)
{  return ((l.x > r.x) && (l.y > r.y));  }

inline bool operator >= (POINT l, POINT r)
{  return ((l.x >= r.x) && (l.y >= r.y));  }
#endif

inline bool operator==( const POINT &p1, const POINT &p2 )
{ return (p1.x == p2.x && p1.y == p2.y); }

inline bool operator!=( const POINT &p1, const POINT &p2 )
{ return !operator==(p1, p2); }

inline utlPoint operator+(const POINT &p1, const POINT &p2)
{ return utlPoint(p1.x + p2.x, p1.y + p2.y); }

inline utlPoint operator+(const SIZE &s1, const POINT &p2)
{ return utlPoint(s1.cx + p2.x, s1.cy + p2.y); }

inline utlPoint operator+(const POINT &p1, const SIZE &s2)
{ return utlPoint(p1.x + s2.cx, p1.y + s2.cy); }

inline utlPoint operator-(const POINT &p1, const POINT &p2)
{ return utlPoint(p1.x - p2.x, p1.y - p2.y); }

inline utlPoint operator-(const SIZE &s1, const POINT &p2)
{ return utlPoint(s1.cx - p2.x, s1.cy - p2.y); }

inline utlPoint operator-(const POINT &p1, const SIZE &s2)
{ return utlPoint(p1.x - s2.cx, p1.y - s2.cy); }

#endif // INCLUDED_utlPoint

