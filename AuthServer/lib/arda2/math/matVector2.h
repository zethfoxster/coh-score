/*****************************************************************************
    created:    2002/06/13
    copyright:  2002, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese
    
    purpose:    
*****************************************************************************/

#ifndef   INCLUDED_matVector2
#define   INCLUDED_matVector2
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
#else
#include <float.h>
#endif

const float VEC2_COMPARE_EPSILON = 0.00001f;

// if anyone really cares about performance of 2d vectors on windows
// they can bother with the implementation of D3DXVECTOR2 in the
// compatibility libraries and fix up this class.  In the mean time
// this does not derive from D3DXVECTOR2 or use D3D functions
class matVector2 // : public D3DXVECTOR2
{
public:
  float x, y;  // remove to derive from D3DXVECTOR2
    matVector2() /* : D3DXVECTOR2() */ {};
    matVector2(const float *f) : /* D3DXVECTOR2(f) */ x(f[0]), y(f[1]) {};
    matVector2(float fX, float fY) : /* D3DXVECTOR2(x, y) */ x(fX), y(fY) {};
    matVector2(const matVector2& v) : x(v.x), y(v.y) {}
    void Normalize() { /*D3DXVec2Normalize(this, this);*/ float m = (1.0f / Magnitude()); x *= m; y *= m; }

    matVector2( matZeroType ) : /* D3DXVECTOR2(0, 0) */ x(0), y(0) {};
    matVector2( matMinimumType ) : /* D3DXVECTOR2(-FLT_MAX, -FLT_MAX)*/ x(-FLT_MAX), y(-FLT_MAX) {};
    matVector2( matMaximumType ) : /* D3DXVECTOR2(FLT_MAX, FLT_MAX) */ x(FLT_MAX), y(FLT_MAX) {};

    static matVector2 Zero;
    static matVector2 Min;
    static matVector2 Max;
    
    void Set(float fX, float fY) { x=fX; y=fY; }
        
    float Magnitude() const { return static_cast<float>(sqrt(x * x + y * y)); }
    float MagnitudeSquared() const { return (x*x + y*y); }

    float   DistanceBetween(const matVector2 &rv2Point) const
    {
        return static_cast<float>(sqrt( ((x - rv2Point.x) * (x - rv2Point.x)) + ((y - rv2Point.y) * (y - rv2Point.y))) );
    }
    float   DistanceBetweenSquared(const matVector2 &rv2Point) const
    {
        return ( ((x - rv2Point.x) * (x - rv2Point.x)) + ((y - rv2Point.y) * (y - rv2Point.y)) );
    }

    float   DotProduct(const matVector2 &rv2Point) const
    {
        return ( (x * rv2Point.x) + (y * rv2Point.y) );
    }

    // interpolation methods ///////////////////////////////////////////////////
    void    StoreLerp( const matVector2& vStart, const matVector2& vEnd, float s) 
    {
        x = vStart.x + s * (vEnd.x - vStart.x);
        y = vStart.y + s * (vEnd.y - vStart.y);
    }

    bool IsZero( float fEpsilon = VEC2_COMPARE_EPSILON ) const 
    { 
        return MagnitudeSquared() < fEpsilon;
    }

    bool IsEqual(const matVector2& v, float fEpsilon = VEC2_COMPARE_EPSILON ) const 
    { 
        return ( (((x - v.x) * (x - v.x)) + ((y - v.y) * (y - v.y)) ) < fEpsilon) ? true : false;
    }

    bool operator ==( const matVector2 &v ) const { return x == v.x && y == v.y; }
    bool operator !=( const matVector2 &v ) const { return x != v.x || y != v.y; }

    matVector2& operator *= ( const matVector2 &v ) { x *= v.x; y *= v.y; return *this; }
    matVector2& operator *= ( const float f )       { x *= f; y *= f; return *this; }

    matVector2& operator += ( const matVector2 &v ) { x += v.x; y += v.y; return *this; }
    matVector2& operator += ( const float f )       { x += f; y += f; return *this; }

    matVector2& operator -= ( const matVector2 &v ) { x -= v.x; y -= v.y; return *this; }
    matVector2& operator -= ( const float f )       { x -= f; y -= f; return *this; }

    matVector2& operator /= ( const matVector2 &v ) { x /= v.x; y /= v.y; return *this; }
    matVector2& operator /= ( const float f )       { x /= f; y /= f; return *this; }

};

// global operators
inline matVector2 operator*( float f, const matVector2& rhs ) 
{
    return matVector2(rhs.x * f, rhs.y * f);
}

inline matVector2 operator*( const matVector2& lhs, float f ) 
{
    return matVector2(lhs.x * f, lhs.y * f);
}

inline matVector2 operator+( const matVector2& lhs, const matVector2& rhs ) 
{
    return matVector2(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline matVector2 operator-( const matVector2& lhs, const matVector2& rhs ) 
{
    return matVector2(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline float DotProduct( const matVector2 &v0, const matVector2 &v1 )
{
  return ( (v0.x * v1.x) + (v0.y * v1.y) );
}


#endif // INCLUDED_~vs10


