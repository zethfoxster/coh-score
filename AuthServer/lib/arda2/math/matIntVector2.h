/*****************************************************************************
    created:    2002/06/13
    copyright:  2002, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese
    
    purpose:    
*****************************************************************************/

#ifndef   INCLUDED_matIntVector2
#define   INCLUDED_matIntVector2
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matTypes.h"

class matIntVector2
{
public:
    int x, y;
    matIntVector2(){};
    matIntVector2(const matIntVector2& v) : x(v.x), y(v.y) {}
    matIntVector2(const int *i) : x(i[0]), y(i[1]) {};
    matIntVector2(int iX, int iY) : x(iX), y(iY) {};

    matIntVector2( matZeroType ) : x(0), y(0) {};
    matIntVector2( matMinimumType ) : x(-INT_MAX), y(-INT_MAX) {};
    matIntVector2( matMaximumType ) : x(INT_MAX), y(INT_MAX) {};

    static matIntVector2 Zero;
    static matIntVector2 Min;
    static matIntVector2 Max;
    
    void Set(int iX, int iY) { x=iX; y=iY; }
    int MagnitudeSquared() const { return (x*x + y*y); }

    int   DistanceBetweenSquared(const matIntVector2 &rv2Point) const
    {
        return ( ((x - rv2Point.x) * (x - rv2Point.x)) + ((y - rv2Point.y) * (y - rv2Point.y)) );
    }

    bool IsZero() const 
    { 
        return (x==0 && y==0);
    }

    bool operator ==( const matIntVector2 &v ) const { return x == v.x && y == v.y; }
    bool operator !=( const matIntVector2 &v ) const { return x != v.x || y != v.y; }

    matIntVector2& operator *= ( const matIntVector2 &v ) { x *= v.x; y *= v.y; return *this; }
    matIntVector2& operator *= ( const int i )       { x *= i; y *= i; return *this; }

    matIntVector2& operator += ( const matIntVector2 &v ) { x += v.x; y += v.y; return *this; }
    matIntVector2& operator += ( const int i )       { x += i; y += i; return *this; }

    matIntVector2& operator /= ( const matIntVector2 &v ) { x /= v.x; y /= v.y; return *this; }
    matIntVector2& operator /= ( const int i )       { x /= i; y /= i; return *this; }

};

// global operators
inline matIntVector2 operator*( int i, const matIntVector2& rhs ) 
{
    return matIntVector2(rhs.x * i, rhs.y * i);
}

inline matIntVector2 operator*( const matIntVector2& lhs, int i ) 
{
    return matIntVector2(lhs.x * i, lhs.y * i);
}

inline matIntVector2 operator+( const matIntVector2& lhs, const matIntVector2& rhs ) 
{
    return matIntVector2(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline matIntVector2 operator-( const matIntVector2& lhs, const matIntVector2& rhs ) 
{
    return matIntVector2(lhs.x - rhs.x, lhs.y - rhs.y);
}

#endif // INCLUDED_~vs10


