#ifndef   INCLUDED_matIntVector4
#define   INCLUDED_matIntVector4
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matFirst.h"
#include "arda2/math/matTypes.h"

class matIntVector4
{
public:
    int x, y, z, w;

	matIntVector4() {};
	matIntVector4( int iX, int iY, int iZ, int iW ) : x(iX), y(iY), z(iZ), w(iW) {};
	matIntVector4( matZeroType ) : x(0), y(0), z(0), w(0) {};
    matIntVector4( matMinimumType ) : x(-INT_MAX), y(-INT_MAX), z(-INT_MAX), w(-INT_MAX) {};
    matIntVector4( matMaximumType ) : x(INT_MAX), y(INT_MAX), z(INT_MAX), w(INT_MAX) {};

	static const matIntVector4 Zero;
	static const matIntVector4 Min;
	static const matIntVector4 Max;

	void Set(int iX, int iY, int iZ, int iW) { x=iX; y=iY; z=iZ; w=iW; }
		
	int	MagnitudeSquared() const;

	int	DistanceBetweenSquared(const matIntVector4 &rv3Point) const;

	void	StoreMaximize(	const matIntVector4& v1, const matIntVector4& v2);
	
	void	StoreMinimize(	const matIntVector4& v1, const matIntVector4& v2);

	void StoreComponentClamp(	const matIntVector4& min,
								const matIntVector4& max);

	bool IsZero() const 
	{ 
		return ( x==0 && y==0 && z==0 && w==0 );
	}

    bool operator ==( const matIntVector4 &v ) const { return x == v.x && y == v.y && z == v.z && w==v.w; }
    bool operator !=( const matIntVector4 &v ) const { return x != v.x || y != v.y || z != v.z || w != v.w; }

    /// compare two vectors, somewhat arbitrary, but allows use as a key, etc.
    bool operator  <( const matIntVector4 &v ) const { return memcmp(this, &v, sizeof(matIntVector4))<0; }

	matIntVector4& operator *= ( const matIntVector4 &v )	{ x *= v.x; y *= v.y; z *= v.z; w*= v.w; return *this; }
	matIntVector4& operator *= ( const int i )		{ x *= i; y *= i; z *= i; w *= i; return *this; }

	matIntVector4& operator += ( const matIntVector4 &v )	{ x += v.x; y += v.y; z += v.z; w+= v.w; return *this; }
	matIntVector4& operator += ( const int i )		{ x += i; y += i; z += i; w += i; return *this; }

	matIntVector4& operator -= ( const matIntVector4 &v )	{ x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	matIntVector4& operator -= ( const int i )		{ x -= i; y -= i; z -= i; w -= i; return *this; }

	matIntVector4& operator /= ( const matIntVector4 &v )	{ x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
	matIntVector4& operator /= ( const int i )		{ x /= i; y /= i; z /= i; w /= i; return *this; }
};

// global operators
inline matIntVector4 operator*( int i, const matIntVector4& rhs ) 
{
	return matIntVector4(rhs.x * i, rhs.y * i, rhs.z * i, rhs.w * i);
}

inline matIntVector4 operator*( const matIntVector4& lhs, int i ) 
{
	return matIntVector4(lhs.x * i, lhs.y * i, lhs.z * i, lhs.w * i);
}

inline matIntVector4 operator*( const matIntVector4& lhs, const matIntVector4& rhs ) 
{
    return matIntVector4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
}

inline matIntVector4 operator+( const matIntVector4& lhs, const matIntVector4& rhs ) 
{
	return matIntVector4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

inline matIntVector4 operator-( const matIntVector4& lhs, const matIntVector4& rhs ) 
{
	return matIntVector4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}


inline int matIntVector4::MagnitudeSquared() const
{
	return (x*x + y*y + z*z + w*w);
}

inline int matIntVector4::DistanceBetweenSquared(const matIntVector4 &rv4Point) const
{
	matIntVector4 matDistance(*this - rv4Point);
	return matDistance.MagnitudeSquared();
}

inline void	matIntVector4::StoreMaximize(const matIntVector4& v1, const matIntVector4& v2)	
{
    // note: aliasing is OK
    x = (v1.x < v2.x) ? v2.x : v1.x;
    y = (v1.y < v2.y) ? v2.y : v1.y;
    z = (v1.z < v2.z) ? v2.z : v1.z;
    w = (v1.w < v2.w) ? v2.w : v1.w;
}

inline void	matIntVector4::StoreMinimize(const matIntVector4& v1, const matIntVector4& v2)	
{
    // note: aliasing is OK
    x = (v1.x > v2.x) ? v2.x : v1.x;
    y = (v1.y > v2.y) ? v2.y : v1.y;
    z = (v1.z > v2.z) ? v2.z : v1.z;
    w = (v1.w > v2.w) ? v2.w : v1.w;
}

#endif // INCLUDED_matIntVector4
