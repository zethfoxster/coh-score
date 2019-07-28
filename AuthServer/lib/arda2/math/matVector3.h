/*****************************************************************************
	created:	2001/09/25
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_matVector3
#define   INCLUDED_matVector3
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matFirst.h"
#include "arda2/math/matTypes.h"
#include "arda2/math/matRandom.h"

class matMatrix4x4;

const float VEC3_COMPARE_EPSILON = 0.00001f;

class matVector3 : public matBaseVECTOR3
{
public:
	matVector3() : matBaseVECTOR3() {};
	//matVector3( const float *f ) : matBaseVECTOR3(f) {};
	matVector3( const matBaseVECTOR3&v ) : matBaseVECTOR3(v) {};
	matVector3( float x, float y, float z ) : matBaseVECTOR3(x, y, z) {};
	matVector3( matZeroType ) : matBaseVECTOR3(0, 0, 0) {};
	matVector3( matMinimumType ) : matBaseVECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX) {};
	matVector3( matMaximumType ) : matBaseVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX) {};

	static const matVector3 Zero;
	static const matVector3 Min;
	static const matVector3 Max;
    
        // store cardinal axes
        static const matVector3 Up;
        static const matVector3 Right;
	static const matVector3 Forward;

	void Set(float fX, float fY, float fZ) { x=fX; y=fY; z=fZ; }
		
	void	Normalize();

	void	TransformPoint( const matMatrix4x4& mat )	
	{	matBaseVec3TransformCoord(this, this, (const matBaseMATRIX*)&mat); }

	void	TransformVector( const matMatrix4x4& mat )	
	{	matBaseVec3TransformNormal(this, this, (const matBaseMATRIX*)&mat); }

	float	Magnitude() const;
	float	MagnitudeSquared() const;

	float	DistanceBetween(const matVector3 &rv3Point) const;
	float	DistanceBetweenSquared(const matVector3 &rv3Point) const;

	float	DotProduct(const matVector3 &rv3Point) const;
	void	StoreCrossProduct( const matVector3& v0, const matVector3& v1);

	// interpolation methods ///////////////////////////////////////////////////
	void	StoreLerp( const matVector3& vStart, const matVector3& vEnd, float s) 
	{ matBaseVec3Lerp(this, &vStart, &vEnd, s); }

	void	StoreBaryCentric(	const matVector3& v1, const matVector3& v2, const matVector3& v3, float f, float g) 
	{	matBaseVec3BaryCentric(this, &v1, &v2, &v3, f, g); }

		// use this for curvy interpolations when you have the vec3's 
		// "surrounding" the region of interest
	void	StoreCatmullRom(	const matVector3& v1, const matVector3& v2, 
								const matVector3& v3, const matVector3& v4, float s) 
	{	matBaseVec3CatmullRom(this, &v1, &v2, &v3, &v4, s); }

	void	StoreHermite(		const matVector3& v1, const matVector3& t1, 
								const matVector3& v2, const matVector3& t2, float s) 
	{	matBaseVec3Hermite(this, &v1, &t1, &v2, &t2, s); }
	////////////////////////////////////////////////////////////////////////////

	void	StoreMaximize(	const matVector3& v1, const matVector3& v2)	
	{	matBaseVec3Maximize(this, &v1, &v2); }
	
	void	StoreMinimize(	const matVector3& v1, const matVector3& v2) 
	{	matBaseVec3Minimize(this, &v1, &v2); }

	// generate a random vector in a spherical distribution 
	// (always on the unit sphere).  This can be restricted to a section of a cone
	// by setting fConeAngle < Pi that has y unit vector as the cone axis
	void	StoreRandomSpherical( float fConeAngle = matPi );

	void	StoreScale(		const matVector3& v1, float s)	{ matBaseVec3Scale(this, &v1, s); }

	// StoreTransform and StoreTransformNormal project to w=1 
	void	StoreTransformPoint( const matMatrix4x4& mat, const matVector3& vec )
	{	matBaseVec3TransformCoord(this, &vec, (const matBaseMATRIX*)&mat); }

	void	StoreTransformVector( const matMatrix4x4& mat, const matVector3& vec )
	{	matBaseVec3TransformNormal(this, &vec, (const matBaseMATRIX*)&mat); }

	void StoreComponentClamp(	const matVector3& min,
								const matVector3& max);

	bool IsZero( float fEpsilon = kDefaultAbsError ) const 
	{ 
		return MagnitudeSquared() < fEpsilon;
	}

        bool operator ==( const matVector3 &v ) const { return x == v.x && y == v.y && z == v.z; }
        bool operator !=( const matVector3 &v ) const { return x != v.x || y != v.y || z != v.z; }

        /// compare two vectors, somewhat arbitrary, but allows use as a key, etc.
        bool operator  <( const matVector3 &v ) const { return memcmp(this, &v, sizeof(matVector3))<0; }

	bool IsEqual(const matVector3& v, 
        float fRelError = kDefaultRelError, 
        float fAbsError = kDefaultAbsError ) const
	{
        return
            ::IsEqual(x, v.x, fRelError, fAbsError) && 
            ::IsEqual(y, v.y, fRelError, fAbsError) && 
            ::IsEqual(z, v.z, fRelError, fAbsError);
	}

	matVector3& operator *= ( const matVector3 &v )	{ x *= v.x; y *= v.y; z *= v.z; return *this; }
	matVector3& operator *= ( const float f )		{ x *= f; y *= f; z *= f; return *this; }

	matVector3& operator += ( const matVector3 &v )	{ x += v.x; y += v.y; z += v.z; return *this; }
	matVector3& operator += ( const float f )		{ x += f; y += f; z += f; return *this; }

	matVector3& operator -= ( const matVector3 &v )	{ x -= v.x; y -= v.y; z -= v.z; return *this; }
	matVector3& operator -= ( const float f )		{ x -= f; y -= f; z -= f; return *this; }

	matVector3& operator /= ( const matVector3 &v )	{ x /= v.x; y /= v.y; z /= v.z; return *this; }
	matVector3& operator /= ( const float f )		{ x /= f; y /= f; z /= f; return *this; }
};

// global operators
inline matVector3 operator*( float f, const matVector3& rhs ) 
{
	return matVector3(rhs.x * f, rhs.y * f, rhs.z * f);
}

inline matVector3 operator*( const matVector3& lhs, float f ) 
{
	return matVector3(lhs.x * f, lhs.y * f, lhs.z * f);
}

inline matVector3 operator*( const matVector3& lhs, const matVector3& rhs ) 
{
    return matVector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

inline matVector3 operator+( const matVector3& lhs, const matVector3& rhs ) 
{
	return matVector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

inline matVector3 operator-( const matVector3& lhs, const matVector3& rhs ) 
{
	return matVector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

inline bool IsEqual( const matVector3& lhs, const matVector3& rhs,
                    float fRelError = kDefaultRelError, 
                    float fAbsError = kDefaultAbsError )
{
    return lhs.IsEqual(rhs, fRelError, fAbsError);
}


// interpolation methods ///////////////////////////////////////////////////
inline matVector3 Lerp( const matVector3& vStart, const matVector3& vEnd, float s) 
{	
	matVector3 temp;
	temp.StoreLerp(vStart, vEnd, s);
	return temp;
}

inline matVector3  BaryCentric(	const matVector3& v1, const matVector3& v2, 
								const matVector3& v3, float f, float g) 
{	
	matVector3 temp;
	temp.StoreBaryCentric(v1, v2, v3, f, g);
	return temp;
}

inline matVector3 CatmullRom(	const matVector3& v1, const matVector3& v2, 
								const matVector3& v3, const matVector3& v4, float s) 
{	
	matVector3 temp;
	temp.StoreCatmullRom(v1, v2, v3, v4, s);
	return temp;
}

inline matVector3 Hermite(	const matVector3& v1, const matVector3& t1, 
							const matVector3& v2, const matVector3& t2, float s) 
{	
	matVector3 temp;
	temp.StoreHermite(v1, t1, v2, t2, s);
	return temp;
}

////////////////////////////////////////////////////////////////////////////


inline void matVector3::Normalize()
{
	matBaseVec3Normalize(this, this);
}

inline float matVector3::Magnitude() const
{
	return matBaseVec3Length(this);
}

inline float matVector3::MagnitudeSquared() const
{
	return matBaseVec3LengthSq(this);
}

inline float matVector3::DistanceBetween(const matVector3 &rv3Point) const
{
	matVector3 matDistance(*this - rv3Point);
	return matDistance.Magnitude();
}

inline float matVector3::DistanceBetweenSquared(const matVector3 &rv3Point) const
{
	matVector3 matDistance(*this - rv3Point);
	return matDistance.MagnitudeSquared();
}

inline float matVector3::DotProduct( const matVector3 &vOther ) const
{
	return matBaseVec3Dot(this, &vOther);
}

inline void matVector3::StoreCrossProduct( const matVector3 &v0, const matVector3 &v1 )
{
	matBaseVec3Cross(this, &v0, &v1);
}

inline float DotProduct( const matVector3 &v0, const matVector3 &v1 )
{
	return matBaseVec3Dot(&v0, &v1);
}

inline matVector3 CrossProduct( const matVector3 &v0, const matVector3 &v1 )
{
	matVector3 result;
	result.StoreCrossProduct(v0, v1);
	return result;
}

inline matVector3 Maximize(	const matVector3& v1, const matVector3& v2)	
{	
	matVector3 temp;
	temp.StoreMaximize(v1, v2);
	return temp;
}

inline matVector3 Minimize(	const matVector3& v1, const matVector3& v2) 
{
 	matVector3 temp;
	temp.StoreMinimize(v1, v2);
	return temp;
}

inline float Magnitude(const matVector3& v)
{
    return v.Magnitude();
}

//* Can't have math methods that rely on graphics or D3D in here
/*
inline matVector3 Project(	const matVector3& v1, 
							const D3DVIEWPORT8& Viewport, 
							const matMatrix4x4& Proj, 
							const matMatrix4x4& View,
							const matMatrix4x4& World)		
{ 
	matVector3 temp;
	temp.StoreProject(v1, Viewport, Proj, View, World);
	return temp;
}

inline matVector3 Unproject(const matVector3& v1, 
const D3DVIEWPORT8& Viewport, 
const matMatrix4x4& Proj, 
const matMatrix4x4& View,
const matMatrix4x4& World)		
{
matVector3 temp;
temp.StoreUnproject(v1, Viewport, Proj, View, World);
return temp;
}
*/

inline matVector3 Scale( const matVector3& v1, float s)	
{ 
	matVector3 temp;
	temp.StoreScale(v1, s);
	return temp;
}

// Transform and TransformNormal project to w=1 
inline matVector3 TransformPoint(const matMatrix4x4& mat, 
							const matVector3& vec)	
{ 
	matVector3 temp;
	temp.StoreTransformPoint(mat, vec);
	return temp;
}

inline matVector3 TransformVector( const matMatrix4x4& mat, 
								   const matVector3& vec)
{ 
	matVector3 temp;
	temp.StoreTransformVector(mat, vec);
	return temp;
}

inline void matVector3::StoreRandomSpherical( float fConeAngle )
{
	matRandom &generator = matRandom::Random;
	float fPhi, fTheta, fSinPhi;

	fPhi	= generator.Uniform(0, fConeAngle);
	fTheta	= generator.Uniform(0, mat2Pi);
	fSinPhi = sinf(fPhi);

	x = cosf(fTheta) * fSinPhi;
	y = cosf(fPhi);
	z = sinf(fTheta) * fSinPhi;
}

matVector3	CrossProduct( const matVector3 &v0, const matVector3 &v1 );
float		DotProduct( const matVector3 &v0, const matVector3 &v1 );

matVector3	ComponentClamp(	const matVector3& val, 
							const matVector3& min,
							const matVector3& max);

#endif // INCLUDED_matVector3
