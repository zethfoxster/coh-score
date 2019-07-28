/*****************************************************************************
	created:	2001/08/14
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	Integer Rect class which wraps the windows RECT
				structure. This is based loosely on the code in
				afxwin.h and afxwin1.inline; see the MFC docs for
				instructions on using these.
*****************************************************************************/

#ifndef   INCLUDED_utlRect
#define   INCLUDED_utlRect
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/util/utlPoint.h"
#include "arda2/util/utlSize.h"

/////////////////////////////////////////////////////////////////////////////
// class utlRect declaration

class utlRect : public tagRECT
{
public:

	// Constructors
	utlRect();
	utlRect(int l, int t, int r, int b);
	utlRect(const RECT &srcRect);
	utlRect(const POINT &point, const SIZE &size);
	utlRect(const POINT &topLeft, const POINT &bottomRight);

	static const utlRect Zero;	// zero object

	// Attributes (in addition to RECT members)
	int Width() const;
	int Height() const;
	utlSize Size() const;
	utlRect SizeRect() const;
	utlPoint& TopLeft();
	utlPoint& BottomRight();
	const utlPoint& TopLeft() const;
	const utlPoint& BottomRight() const;
	utlPoint TopRight() const;
	utlPoint BottomLeft() const;
	utlPoint Center() const;

	bool IsEmpty() const;		// rect has no area (may be backwards)
	bool IsValid() const;		// rect has area (not backwards)
	bool IsNormal() const;		// rect is organized normally (not backwards, empty is allowed)
	bool IsNull() const;		// all rect params are 0
	operator BOOL() const;

	// Operations
	void Set(int x1, int y1, int x2, int y2);
	void Set(const POINT &topLeft, const POINT &bottomRight);
	void Set(const POINT &point, const SIZE &size);
	void SetEmpty();

	void Inflate(int val);
	void Inflate(int x, int y);
	void Inflate(const SIZE &size);
	void Inflate(const RECT &rect);
	void Inflate(int l, int t, int r, int b);

	void Deflate(int val);
	void Deflate(int x, int y);
	void Deflate(const SIZE &size);
	void Deflate(const RECT &rect);
	void Deflate(int l, int t, int r, int b);

	void Offset(int x, int y);
	void Offset(const SIZE &size);
	void Offset(const POINT &point);
	void OffsetX(int x);
	void OffsetY(int y);
	void NormalizeRect();

	void MoveTo(const POINT &origin);
	void ResizeTo(const SIZE &size);

	bool Intersects(const RECT &r) const;
	const utlRect& Intersect(const RECT &r );
	bool IntersectValid(const RECT &r );
	const utlRect& Union(const RECT &r );

	// boolean operations with separate operands
	const utlRect& Intersect(const RECT &rect1, const RECT &rect2);
	bool IntersectValid(const RECT &rect1, const RECT &rect2);
	const utlRect& Union(const RECT &rect1, const RECT &rect2);

	bool Contains(const POINT &p) const;
	bool ContainsX(const POINT &p) const;	// allows p == BottomRight
	bool Contains(const RECT &other) const;

	// special stuff
	bool ConstrainTo(const utlRect &bounds);
	void CenterIn(const utlRect &container);

	// operator overloads
	const utlRect& operator=(const RECT& srcRect);
	bool operator==(const RECT& rect) const;
	bool operator!=(const RECT& rect) const;
	const utlRect& operator+=(const POINT &point);
	const utlRect& operator+=(const SIZE &size);
	const utlRect& operator-=(const POINT &point);
	const utlRect& operator-=(const SIZE &size);
	const utlRect& operator&=(const RECT& rect);
	const utlRect& operator|=(const RECT& rect);

	// Operators returning utlRect values
	utlRect operator+(const POINT &point) const;
	utlRect operator+(const RECT &rect) const;
	utlRect operator+(const SIZE &size) const;
	utlRect operator-(const POINT &point) const;
	utlRect operator-(const SIZE &size) const;
	utlRect operator-(const RECT &rect) const;
};

utlRect Union( const RECT &r0, const RECT &r1 );
utlRect Intersect( const RECT &r0, const RECT &r1 );

utlRect operator&(const RECT &r1, const RECT &r2);
utlRect operator|(const RECT &r1, const RECT& r2);


//////////////////////////////////////////////////////////////////////////////
// class utlRect inline implementation

inline utlRect::utlRect()
{ /* random filled */ }

inline utlRect::utlRect( int l, int t, int r, int b )
{ left = l; top = t; right = r; bottom = b; }

inline utlRect::utlRect( const RECT& rect )
{ *(RECT *)this = rect; }

inline utlRect::utlRect( const POINT &point, const SIZE &size )
{ right = (left = point.x) + size.cx; bottom = (top = point.y) + size.cy; }

inline utlRect::utlRect( const POINT &topLeft, const POINT &bottomRight )
{ TopLeft() = topLeft; BottomRight() = bottomRight; }

inline int utlRect::Width() const
{ return right - left; }

inline int utlRect::Height() const
{ return bottom - top; }

inline utlSize utlRect::Size() const
{ return utlSize(right - left, bottom - top); }

inline utlRect utlRect::SizeRect() const
{ return utlRect(utlPoint::Zero, Size()); }

inline utlPoint& utlRect::TopLeft()
{ return *((utlPoint*)this); }

inline utlPoint& utlRect::BottomRight()
{ return *((utlPoint*)this+1); }

inline const utlPoint& utlRect::TopLeft() const
{ return *((utlPoint*)this); }

inline const utlPoint& utlRect::BottomRight() const
{ return *((utlPoint*)this+1); }

inline utlPoint utlRect::TopRight() const
{  return (utlPoint(right, top));  }

inline utlPoint utlRect::BottomLeft() const
{  return (utlPoint(left, bottom));  }

inline utlPoint utlRect::Center() const
{ return utlPoint((left+right)/2, (top+bottom)/2); }

inline bool utlRect::IsEmpty() const
{ return left >= right || top >= bottom; }

inline bool utlRect::IsNull() const
{ return (left | right | top | bottom) == 0; }

inline bool utlRect::IsValid() const
{ return left < right && top < bottom; }

inline bool utlRect::IsNormal() const
{ return left <= right && top <= bottom; }

inline utlRect::operator BOOL() const
{ return IsValid(); }

inline void utlRect::Set( int x1, int y1, int x2, int y2 )
{ left = x1; top = y1; right = x2; bottom = y2; }

inline void utlRect::Set( const POINT &topLeft, const POINT &bottomRight )
{ TopLeft() = topLeft; BottomRight() = bottomRight; }

inline void utlRect::Set( const POINT &point, const SIZE &size )
{ right = (left = point.x) + size.cx; bottom = (top = point.y) + size.cy; }

inline void utlRect::SetEmpty()
 { left = top = right = bottom = 0; }

inline void utlRect::Inflate( int val )
{ left -= val; top -= val; right += val; bottom += val; }

inline void utlRect::Inflate( int x, int y )
{ left -= x; top -= y; right += x; bottom += y; }

inline void utlRect::Inflate( const SIZE &size )
{ Inflate(size.cx, size.cy); }

inline void utlRect::Inflate(const RECT &rect)
{ left -= rect.left; top -= rect.top; right += rect.right; bottom += rect.bottom; }

inline void utlRect::Inflate(int l, int t, int r, int b)
{ left -= l; top -= t; right += r; bottom += b; }

inline void utlRect::Deflate( int val )
{ Inflate(-val); }

inline void utlRect::Deflate( int x, int y )
{ Inflate(-x, -y); }

inline void utlRect::Deflate( const SIZE &size )
{ Deflate(size.cx, size.cy); }

inline void utlRect::Deflate(const RECT &rect)
{ left += rect.left; top += rect.top; right -= rect.right; bottom -= rect.bottom; }

inline void utlRect::Deflate(int l, int t, int r, int b)
{ left += l; top += t; right -= r; bottom -= b; }

inline void utlRect::Offset( int x, int y )
{ left += x; top += y; right += x; bottom += y; }

inline void utlRect::Offset( const SIZE &size )
{ Offset(size.cx, size.cy); }

inline void utlRect::Offset( const POINT &point )
{ Offset(point.x, point.y); }

inline void utlRect::OffsetX( int x )
{ left += x; right += x; }

inline void utlRect::OffsetY( int y )
{ top += y; bottom += y; }

inline void utlRect::MoveTo( const POINT &origin)
{
	right += origin.x - left; left = origin.x;
	bottom += origin.y - top; top = origin.y;
}

inline void utlRect::ResizeTo( const SIZE &size )
{ right = left + size.cx; bottom = top + size.cy; }

inline bool utlRect::Intersects( const RECT &r ) const
{ return left < r.right && right > r.left && top < r.bottom && bottom > r.top; }

inline const utlRect& utlRect::Intersect( const RECT &r )
{ if (!IntersectValid(r)) { left = top = right = bottom = 0; } return *this; }	// $ inefficient! fix assumption in future - allow invalid rects!

inline bool utlRect::IntersectValid( const RECT &r )
{ left = Max(left, r.left); top = Max(top, r.top); right = Min(right, r.right); bottom = Min(bottom, r.bottom); return IsValid(); }

inline const utlRect&  utlRect::Intersect( const RECT &r1, const RECT &r2 )
{ if (!IntersectValid(r1, r2)) { left = top = right = bottom = 0; } return *this; }

inline bool utlRect::IntersectValid( const RECT &r1, const RECT &r2 )
{ left = Max(r1.left, r2.left); top = Max(r1.top, r2.top); right = Min(r1.right, r2.right); bottom = Min(r1.bottom, r2.bottom); return IsValid(); }

inline const utlRect&  utlRect::Union( const RECT &r1, const RECT &r2 )
{ left = Min(r1.left, r2.left); top = Min(r1.top, r2.top); right = Max(r1.right, r2.right); bottom = Max(r1.bottom, r2.bottom); return *this; }

inline const utlRect&  utlRect::Union( const RECT &r )
{ left = Min(left, r.left); top = Min(top, r.top); right = Max(right, r.right); bottom = Max(bottom, r.bottom); return *this; }

inline bool utlRect::Contains( const POINT &p ) const
{ return left <= p.x && top <= p.y && p.x < right && p.y < bottom; }

inline bool utlRect::ContainsX( const POINT &p ) const
{ return left <= p.x && top <= p.y && p.x <= right && p.y <= bottom; }

inline bool utlRect::Contains( const RECT &r ) const
{ return left <= r.left && top <= r.top && r.right <= right && r.bottom <= bottom; }

inline void utlRect::CenterIn(const utlRect &container)
{ Offset(container.Center() - Center()); }

inline const utlRect& utlRect::operator=( const RECT& rect )
{ *(RECT *)this = rect; return *this; }

inline bool utlRect::operator==( const RECT& rect ) const
{ return left == rect.left && top == rect.top && right == rect.right && bottom == rect.bottom; }

inline bool utlRect::operator!=( const RECT& rect ) const
{ return !operator==(rect); }

inline const utlRect& utlRect::operator+=( const POINT &point )
{ Offset(point.x, point.y); return *this; }

inline const utlRect& utlRect::operator+=( const SIZE &size )
{ Offset(size.cx, size.cy); return *this; }

inline const utlRect& utlRect::operator-=( const POINT &point )
{ Offset(-point.x, -point.y); return *this; }

inline const utlRect& utlRect::operator-=( const SIZE &size )
{ Offset(-size.cx, -size.cy); return *this; }

inline const utlRect& utlRect::operator&=( const RECT& rect )
{ return Intersect(rect); }

inline const utlRect& utlRect::operator|=( const RECT& rect )
{ return Union(rect); }

inline utlRect utlRect::operator+( const POINT &p ) const
{ utlRect rect(*this); rect.Offset(p.x, p.y); return rect; }

inline utlRect utlRect::operator-( const POINT &p ) const
{ utlRect rect(*this); rect.Offset(-p.x, -p.y); return rect; }

inline utlRect utlRect::operator+( const SIZE &size ) const
{ utlRect rect(*this); rect.Offset(size.cx, size.cy); return rect; }

inline utlRect utlRect::operator-( const SIZE &size ) const
{ utlRect rect(*this); rect.Offset(-size.cx, -size.cy); return rect; }

inline utlRect operator&(const RECT &r1, const RECT &r2)
{ utlRect rect; return rect.Intersect(r1, r2); }

inline utlRect operator|(const RECT &r1, const RECT& r2)
{ utlRect rect; return rect.Union(r1, r2); }

inline void utlRect::NormalizeRect()
{
	int nTemp;
	if (left > right)
	{
		nTemp = left;
		left  = right;
		right = nTemp;
	}
	if (top > bottom)
	{
		nTemp  = top;
		top    = bottom;
		bottom = nTemp;
	}
}


#endif // INCLUDED_utlRect

