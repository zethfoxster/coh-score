/*****************************************************************************
	created:	2001/09/25
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_matVector4
#define   INCLUDED_matVector4
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matFirst.h"
#include "arda2/math/matTypes.h"
#include "arda2/math/matVector3.h"

class matVector4 : public matBaseVECTOR4
{
public:
	matVector4() : matBaseVECTOR4() {};
	matVector4(float x, float y, float z, float w) : matBaseVECTOR4(x, y, z, w) {};
	explicit matVector4(const matVector3 &v3) : matBaseVECTOR4(v3.x, v3.y, v3.z, 1.0f) {};

	static matVector4 Zero;

	void Set(float fX, float fY, float fZ, float fW){ x=fX; y=fY; z=fZ; w=fW;}
	void  Normalize();
	void  Transform(const matMatrix4x4& mat);
	float Magnitude() const;
	float MagnitudeSquared() const;
	float DotProduct(const matVector4& v1) const;
	void  StoreCrossProduct(const matVector4& v1, 
							const matVector4& v2,
							const matVector4& v3);
	void StoreScale( const matVector4& vec, float s);
	void StoreTransform( const matVector4& vec, const matMatrix4x4& mat);

	void	StoreMaximize(	const matVector4& v1, const matVector4& v2)	
	{	matBaseVec4Maximize(this, &v1, &v2); }
	
	void	StoreMinimize(	const matVector4& v1, const matVector4& v2) 
	{	matBaseVec4Minimize(this, &v1, &v2); }

	// interpolation methods ///////////////////////////////////////////////////
	void StoreLerp( const matVector4& vStart, const matVector4& vEnd, float s) 
	{	matBaseVec4Lerp(this, &vStart, &vEnd, s); }

	void StoreBaryCentric(	const matVector4& v1, const matVector4& v2, 
							const matVector4& v3, float f, float g) 
	{	matBaseVec4BaryCentric(this, &v1, &v2, &v3, f, g); }

	void StoreCatmullRom(	const matVector4& v1, const matVector4& v2, 
							const matVector4& v3, const matVector4& v4, float s) 
	{	matBaseVec4CatmullRom(this, &v1, &v2, &v3, &v4, s); }

	void StoreHermite(		const matVector4& v1, const matVector4& t1, 
							const matVector4& v2, const matVector4& t2, 
							float s) 
	{	matBaseVec4Hermite(this, &v1, &t1, &v2, &t2, s); }
	////////////////////////////////////////////////////////////////////////////

	void StoreComponentClamp(	const matVector4& min,
								const matVector4& max);

    bool operator ==( const matVector4 &v ) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator !=( const matVector4 &v ) const { return x != v.x || y != v.y || z != v.z || w != v.w; }

	matVector4& operator *= ( const float f )		{ x *= f; y *= f; z *= f; w *= f; return *this; }
};

// global operators
inline matVector4 operator*( float f, const matVector4& rhs ) 
{
	return matVector4(rhs.x * f, rhs.y * f, rhs.z * f, rhs.w * f);
}

inline matVector4 operator*( const matVector4& lhs, float f ) 
{
	return matVector4(lhs.x * f, lhs.y * f, lhs.z * f, lhs.w * f);
}

inline matVector4 operator+( const matVector4& lhs, const matVector4& rhs ) 
{
	return matVector4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

inline matVector4 operator-( const matVector4& lhs, const matVector4& rhs ) 
{
	return matVector4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}


inline matVector4 Maximize(	const matVector4& v1, const matVector4& v2)	
{	
	matVector4 temp;
	temp.StoreMaximize(v1, v2);
	return temp;
}

inline matVector4 Minimize(	const matVector4& v1, const matVector4& v2) 
{
 	matVector4 temp;
	temp.StoreMinimize(v1, v2);
	return temp;
}

inline matVector4  Normalize(const matVector4& vec)
{
	matVector4 temp(vec);
	temp.Normalize();
	return temp;
}

inline float DotProduct(const matVector4& v1, 
						const matVector4& v2)
{
	return v1.DotProduct(v2);
}

inline matVector4 Scale(const matVector4& vec, 
						float s)
{
	matVector4 temp;
	temp.StoreScale(vec, s);
	return temp;
}

inline matVector4 Transform(const matVector4& vec, 
							const matMatrix4x4& mat)
{
	matVector4 temp;
	temp.StoreTransform(vec, mat);
	return temp;
}

inline matVector4 BaryCentric(	const matVector4& v1, 
								const matVector4& v2, 
								const matVector4& v3, 
								float f, 
								float g) 
{
	matVector4 temp;
	temp.StoreBaryCentric(v1, v2, v3, f, g);
	return temp;
}

inline matVector4 CatmullRom(	const matVector4& v1, 
								const matVector4& v2, 
								const matVector4& v3, 
								const matVector4& v4, 
								float s) 
{
	matVector4 temp;
	temp.StoreCatmullRom(v1, v2, v3, v4, s);
	return temp;
}

inline matVector4 Hermite(	const matVector4& v1, 
							const matVector4& t1, 
							const matVector4& v2, 
							const matVector4& t2, 
							float s) 
{
	matVector4 temp;
	temp.StoreHermite(v1, t1, v2, t2, s);
	return temp;
}

inline matVector4 Lerp(const matVector4& vStart, 
					   const matVector4& vEnd, 
					   float s)
{
	matVector4 temp;
	temp.StoreLerp(vStart, vEnd, s);
	return temp;
}

inline matVector4 CrossProduct(	const matVector4& v1, 
								const matVector4& v2,
								const matVector4& v3)
{
	matVector4 temp;
	temp.StoreCrossProduct(v1, v2, v3);
	return temp;
}

matVector4 ComponentClamp(	const matVector4& val, 
							const matVector4& min,
							const matVector4& max);

#endif // INCLUDED_matVector4
