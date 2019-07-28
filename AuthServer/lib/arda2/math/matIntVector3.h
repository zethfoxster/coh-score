#ifndef   INCLUDED_matIntVector3
#define   INCLUDED_matIntVector3
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matFirst.h"
#include "arda2/math/matTypes.h"

class matIntVector3
{
public:
    int x, y, z;

	matIntVector3() {};
	matIntVector3( int iX, int iY, int iZ ) : x(iX), y(iY), z(iZ) {};
	matIntVector3( matZeroType ) : x(0), y(0), z(0) {};
    matIntVector3( matMinimumType ) : x(-INT_MAX), y(-INT_MAX), z(-INT_MAX) {};
    matIntVector3( matMaximumType ) : x(INT_MAX), y(INT_MAX), z(INT_MAX) {};

	static const matIntVector3 Zero;
	static const matIntVector3 Min;
	static const matIntVector3 Max;

	void Set(int iX, int iY, int iZ) { x=iX; y=iY; z=iZ; }
		
	int	MagnitudeSquared() const;

	int	DistanceBetweenSquared(const matIntVector3 &rv3Point) const;

	void	StoreMaximize(	const matIntVector3& v1, const matIntVector3& v2);
	
	void	StoreMinimize(	const matIntVector3& v1, const matIntVector3& v2);

	void StoreComponentClamp(	const matIntVector3& min,
								const matIntVector3& max);

	bool IsZero() const 
	{ 
		return ( x==0 && y==0 && z==0 );
	}

    bool operator ==( const matIntVector3 &v ) const { return x == v.x && y == v.y && z == v.z; }
    bool operator !=( const matIntVector3 &v ) const { return x != v.x || y != v.y || z != v.z; }

    /// compare two vectors, somewhat arbitrary, but allows use as a key, etc.
    bool operator  <( const matIntVector3 &v ) const { return memcmp(this, &v, sizeof(matIntVector3))<0; }

	matIntVector3& operator *= ( const matIntVector3 &v )	{ x *= v.x; y *= v.y; z *= v.z; return *this; }
	matIntVector3& operator *= ( const int i )		{ x *= i; y *= i; z *= i; return *this; }

	matIntVector3& operator += ( const matIntVector3 &v )	{ x += v.x; y += v.y; z += v.z; return *this; }
	matIntVector3& operator += ( const int i )		{ x += i; y += i; z += i; return *this; }

	matIntVector3& operator -= ( const matIntVector3 &v )	{ x -= v.x; y -= v.y; z -= v.z; return *this; }
	matIntVector3& operator -= ( const int i )		{ x -= i; y -= i; z -= i; return *this; }

	matIntVector3& operator /= ( const matIntVector3 &v )	{ x /= v.x; y /= v.y; z /= v.z; return *this; }
	matIntVector3& operator /= ( const int i )		{ x /= i; y /= i; z /= i; return *this; }
};

// global operators
inline matIntVector3 operator*( int i, const matIntVector3& rhs ) 
{
	return matIntVector3(rhs.x * i, rhs.y * i, rhs.z * i);
}

inline matIntVector3 operator*( const matIntVector3& lhs, int i ) 
{
	return matIntVector3(lhs.x * i, lhs.y * i, lhs.z * i);
}

inline matIntVector3 operator*( const matIntVector3& lhs, const matIntVector3& rhs ) 
{
    return matIntVector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

inline matIntVector3 operator+( const matIntVector3& lhs, const matIntVector3& rhs ) 
{
	return matIntVector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

inline matIntVector3 operator-( const matIntVector3& lhs, const matIntVector3& rhs ) 
{
	return matIntVector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}


inline int matIntVector3::MagnitudeSquared() const
{
	return (x*x + y*y + z*z);
}

inline int matIntVector3::DistanceBetweenSquared(const matIntVector3 &rv3Point) const
{
	matIntVector3 matDistance(*this - rv3Point);
	return matDistance.MagnitudeSquared();
}

inline void	matIntVector3::StoreMaximize(const matIntVector3& v1, const matIntVector3& v2)	
{
    // note: aliasing is OK
    x = (v1.x < v2.x) ? v2.x : v1.x;
    y = (v1.y < v2.y) ? v2.y : v1.y;
    z = (v1.z < v2.z) ? v2.z : v1.z;
}

inline void	matIntVector3::StoreMinimize(const matIntVector3& v1, const matIntVector3& v2)	
{
    // note: aliasing is OK
    x = (v1.x > v2.x) ? v2.x : v1.x;
    y = (v1.y > v2.y) ? v2.y : v1.y;
    z = (v1.z > v2.z) ? v2.z : v1.z;
}

#endif // INCLUDED_matIntVector3
